// plugin_blendpatterns.cpp - Preset blend transition pattern generation
// Extracted from plugin.cpp

#include "plugin.h"
#include "plugin_internals.h"
#include "utility.h"
#include <cmath>

namespace {
// Keep in sync with MilkwaveRemoteForm.TransitionModeNames in the Remote.
const wchar_t* const kTransitionTypeNames[] = {
    L"Uniform Fade",
    L"Directional Wipe",
    L"Plasma",
    L"Radial",
    L"Clock",
    L"Spiral",
    L"Rhombus",
    L"Nuclear Clock",
    L"Square/Diamond",
    L"Checkerboard",
    L"Curtain",
    L"Bubbles",
    L"Kaleidoscope",
    L"Moebius Strip",
    L"Stars",
    L"Disco Floor",
    L"Fire",
    L"Drain Swirl",
    L"Julia Set",
    L"Vertical Wipe",
    L"Horizontal Wipe",
};
}

int GetTransitionTypeCount() {
  return static_cast<int>(sizeof(kTransitionTypeNames) / sizeof(kTransitionTypeNames[0]));
}

const wchar_t* GetTransitionTypeName(int mixType) {
  if (mixType < 0 || mixType >= GetTransitionTypeCount())
    return L"Random";
  return kTransitionTypeNames[mixType];
}

void CPlugin::SetTransitionType(int mixType, bool notifyRemote) {
  if (mixType < -1)
    mixType = -1;
  if (mixType >= GetTransitionTypeCount())
    mixType = GetTransitionTypeCount() - 1;
  if (mixType == m_nMixType)
    return;

  m_nMixType = mixType;
  WritePrivateProfileIntW(mixType, L"Mixtype", GetConfigIniFile(), L"Settings");

  wchar_t message[128];
  swprintf_s(message, L"Transition: %s", GetTransitionTypeName(mixType));
  AddNotification(message);

  if (notifyRemote)
    SendSettingsInfoToMilkwaveRemote();
}

void CPlugin::CycleTransitionType() {
  int next = m_nMixType + 1;
  if (next >= GetTransitionTypeCount())
    next = -1;
  SetTransitionType(next, true);
}

void CPlugin::RandomizeBlendPattern() {
  if (!m_vertinfo)
    return;

  // note: we now avoid constant uniform blend b/c it's half-speed for shader blending.
  //       (both old & new shaders would have to run on every pixel...)           reenabled due to further notice
  int mixtype = 0 + (rand() % 19);
  if (m_nMixType > -1) mixtype = m_nMixType;

  if (mixtype == 0) {
    // constant, uniform blend
    int nVert = 0;
    for (int y = 0; y <= m_nGridY; y++) {
      for (int x = 0; x <= m_nGridX; x++) {
        m_vertinfo[nVert].a = 1;
        m_vertinfo[nVert].c = 0;
        nVert++;
      }
    }
  } else if (mixtype == 1) {
    // directional wipe
    float ang = FRAND * 6.28f;
    float band = 0.1f + 0.2f * FRAND;  // 0.2 is good
    bool use_arrow_curve = false;
    if (m_bLoadingMilk2 && m_nMilk2MixType == 1 && !m_bMilk2ArrowWipe) {
      ang = m_fMilk2Random1 * 6.2831853f;
      band = 0.1f + 0.2f * m_fMilk2Random2;
    }
    if (m_bLoadingMilk2 && m_nMilk2MixType == 1 && m_bMilk2ArrowWipe) {
      // Arrow wipe: horizontal wipe (left-to-right) with arrowhead shape.
      // Direction is encoded in fBlend (flipped for direction=-1), not in the angle.
      // random_3 controls the arrowhead curvature (0=straight, 1=sharp >).
      // random_4 controls curvature direction (< 0.5 = standard >, >= 0.5 = reverse <).
      ang = 0.0f;  // always horizontal left-to-right
      band = 0.1f + 0.2f * m_fMilk2Random2;
      use_arrow_curve = true;
    }
    float vx = cosf(ang);
    float vy = sinf(ang);
    float inv_band = 1.0f / band;

    int nVert = 0;
    for (int y = 0; y <= m_nGridY; y++) {
      float fy;
      if (m_bScreenDependentRenderMode)
        fy = (y / (float)m_nGridY);
      else
        fy = (y / (float)m_nGridY) * m_fAspectY;

      for (int x = 0; x <= m_nGridX; x++) {
        float fx;
        if (m_bScreenDependentRenderMode)
          fx = (x / (float)m_nGridX);
        else
          fx = (x / (float)m_nGridX) * m_fAspectX;

        // at t==0, mix rangse from -10..0
        // at t==1, mix ranges from   1..11

        float t;
        if (use_arrow_curve) {
          // Arrow / C-shape wipe:
          // The wipe sweeps across gracefully with a curved front.
          float ux = x / (float)m_nGridX;
          float dy = (y / (float)m_nGridY) - 0.50f;
          float B = 2.0f;  // Increased from 1.6f for a steeper curve / sharper arrow

          float base_t;
          if (m_fMilk2BlendDirection == -1.0f) {
            // Sweep Right-To-Left (right side transitions to new preset first)
            // Creates a '<' shaped boundary
            base_t = ux;
          } else {
            // Sweep Left-To-Right (left side transitions to new preset first)
            // Creates a '>' shaped boundary
            base_t = 1.0f - ux;
          }

          // Middle transitions earlier than edges
          t = base_t - B * dy * dy;

          // Final locked offset between original 53 (0.25) and 54 (0.35)
          float offset = 0.30f;

          // Reposition back nicely into the expected [0, 1] evaluation range
          t = t / (1.0f + B * 0.2f) + offset;
        } else {
          t = (fx - 0.5f) * vx + (fy - 0.5f) * vy + 0.5f;
        }
        if (!use_arrow_curve)
          t = (t - 0.5f) / sqrtf(2.0f) + 0.5f;

        m_vertinfo[nVert].a = inv_band * (1 + band);
        m_vertinfo[nVert].c = -inv_band + inv_band * t;  //(x/(float)m_nGridX - 0.5f)/band;
        nVert++;
      }
    }
  } else if (mixtype == 2) {
    // plasma transition
    float band = 0.12f + 0.13f * FRAND;  // 0.02f + 0.18f*FRAND;
    float inv_band = 1.0f / band;

    // first generate plasma array of height values
    float rotation = 0.0f;  // keep the star upright for milk2 parity
    if (m_bLoadingMilk2 && m_nMilk2MixType == 2) {
      // Bias the fractal so the bright plasma mass starts in the top-left corner.
      m_vertinfo[0].c = 0.80f + 0.20f * m_fMilk2Random1;
      m_vertinfo[m_nGridX].c = 0.45f + 0.20f * m_fMilk2Random2;
      m_vertinfo[m_nGridY * (m_nGridX + 1)].c = 0.40f + 0.20f * m_fMilk2Random3;
      m_vertinfo[m_nGridY * (m_nGridX + 1) + m_nGridX].c = 0.10f + 0.15f * m_fMilk2Random4;
    } else {
      m_vertinfo[0].c = FRAND;
      m_vertinfo[m_nGridX].c = FRAND;
      m_vertinfo[m_nGridY * (m_nGridX + 1)].c = FRAND;
      m_vertinfo[m_nGridY * (m_nGridX + 1) + m_nGridX].c = FRAND;
    }
    GenPlasma(0, m_nGridX, 0, m_nGridY, 0.25f);

    // then find min,max so we can normalize to [0..1] range and then to the proper 'constant offset' range.
    float minc = m_vertinfo[0].c;
    float maxc = m_vertinfo[0].c;
    int x, y, nVert;

    nVert = 0;
    for (y = 0; y <= m_nGridY; y++) {
      for (x = 0; x <= m_nGridX; x++) {
        if (minc > m_vertinfo[nVert].c)
          minc = m_vertinfo[nVert].c;
        if (maxc < m_vertinfo[nVert].c)
          maxc = m_vertinfo[nVert].c;
        nVert++;
      }
    }

    float mult = 1.0f / (maxc - minc);
    nVert = 0;
    for (y = 0; y <= m_nGridY; y++) {
      for (x = 0; x <= m_nGridX; x++) {
        float t = (m_vertinfo[nVert].c - minc) * mult;
        m_vertinfo[nVert].a = inv_band * (1 + band);
        m_vertinfo[nVert].c = -inv_band + inv_band * t;
        nVert++;
      }
    }
  } else if (mixtype == 3) {
    // radial blend
    float band = 0.025f + 0.14f * FRAND + 0.34f * FRAND;
    float inv_band = 1.0f / band;
    float dir = (float)((rand() % 2) * 2 - 1);  // 1=outside-in, -1=inside-out
    const bool bMilk2Corner = m_bLoadingMilk2 && m_nMilk2MixType == 3 && m_bMilk2CornerWipe;
    if (m_fMilk2BlendDirection != 0.0f) {
      dir = m_fMilk2BlendDirection;
      band = 0.7f;  // broader feather for .milk2 deterministic circle size
      inv_band = 1.0f / band;
    }

    int nVert = 0;
    for (int y = 0; y <= m_nGridY; y++) {
      float dy;
      if (m_bScreenDependentRenderMode)
        dy = (y / (float)m_nGridY - 0.5f);
      else
        dy = (y / (float)m_nGridY - 0.5f) * m_fAspectY;

      for (int x = 0; x <= m_nGridX; x++) {
        float dx;
        if (m_bScreenDependentRenderMode)
          dx = (x / (float)m_nGridX - 0.5f);
        else
          dx = (x / (float)m_nGridX - 0.5f) * m_fAspectX;

        float t;
        if (bMilk2Corner) {
          float cornerCenterX = 0.25f;
          float cornerCenterY = 0.25f;
          dx = (x / (float)m_nGridX - cornerCenterX);
          if (m_bScreenDependentRenderMode)
            dy = (y / (float)m_nGridY - cornerCenterY);
          else
            dy = (y / (float)m_nGridY - cornerCenterY) * m_fAspectY;
          t = sqrtf(dx * dx + dy * dy) * 1.41421f;
        } else {
          t = sqrtf(dx * dx + dy * dy) * 1.41421f;
        }
        if (dir == -1)
          t = 1 - t;

        m_vertinfo[nVert].a = inv_band * (1 + band);
        m_vertinfo[nVert].c = -inv_band + inv_band * t;
        nVert++;
      }
    }
  } else if (mixtype == 4) {
    // DeepSeek - seamless clock transition
    float band = 0.08f + 0.14f * FRAND;  // optimal band width for clock transition
    float inv_band = 1.0f / band;
    float dir = (rand() % 2) ? 1.0f : -1.0f;  // random direction
    float start_angle = FRAND * 6.2831853f;   // random starting angle

    int nVert = 0;
    for (int y = 0; y <= m_nGridY; y++) {
      float fy;
      if (m_bScreenDependentRenderMode)
        fy = (y / (float)m_nGridY - 0.5f);
      else
        fy = (y / (float)m_nGridY - 0.5f) * m_fAspectY;
      for (int x = 0; x <= m_nGridX; x++) {
        float fx;
        if (m_bScreenDependentRenderMode)
          fx = (x / (float)m_nGridX - 0.5f);
        else
          fx = (x / (float)m_nGridX - 0.5f) * m_fAspectX;

        // Calculate angle and distance from center
        float angle = atan2f(fy, fx);                         // range: -PI to PI
        float dist = sqrtf(fx * fx + fy * fy) * 1.41421356f;  // normalized 0-1

        // Convert angle to 0-2PI range and apply direction/start
        if (angle < 0) angle += 6.2831853f;
        angle = fmodf(angle * dir + start_angle + 10.0f * 6.2831853f, 6.2831853f);

        // Calculate blend factor with seamless wrap-around
        float t = angle / 6.2831853f;
        float t_adjusted = t;

        // Handle wrap-around for smooth transition
        if (t < band) {
          t_adjusted = t + 1.0f;  // treat as next cycle
        }

        // Combine with distance for better visual (optional)
        float blend = (t_adjusted - dist * 0.1f);  // slight radial component

        m_vertinfo[nVert].a = inv_band * (1.0f + band);
        m_vertinfo[nVert].c = -inv_band + inv_band * blend;
        nVert++;
      }
    }
  } else if (mixtype == 5) {
    // Spiral/Snail transition - symmetric spiral band with even feathering
    const bool bMilk2Snail = m_bLoadingMilk2 && m_nMilk2MixType == 5;
    const bool bMilk2Snail2 = bMilk2Snail && m_bMilk2Snail2;
    const bool bMilk2Snail3 = bMilk2Snail && m_bMilk2Snail3;

    // Keep the band very wide so the blend region reads as a broad feather.
    float band;
    if (bMilk2Snail3)
      band = 0.16f + 0.06f * m_fMilk2Random2;
    else if (bMilk2Snail2)
      band = 0.36f + 0.16f * m_fMilk2Random2;
    else if (bMilk2Snail)
      band = 0.42f + 0.18f * m_fMilk2Random2;
    else
      band = 0.42f + 0.18f * FRAND;
    float inv_band = 1.0f / band;

    // snail3 gets the strongest spiral so the turn reads more clearly.
    float loops = bMilk2Snail3 ? (2.80f + 1.40f * m_fMilk2Random3) : (bMilk2Snail2 ? (0.95f + 0.75f * m_fMilk2Random3) : (bMilk2Snail ? (0.65f + 0.55f * m_fMilk2Random3) : (0.65f + 0.95f * FRAND)));
    float phase = bMilk2Snail ? (m_fMilk2Random1 * 6.2831853f) : (FRAND * 6.2831853f);
    bool inward = bMilk2Snail ? (m_fMilk2BlendDirection < 0.0f) : ((rand() % 2) == 0);

    int nVert = 0;
    for (int y = 0; y <= m_nGridY; y++) {
      float fy;
      if (m_bScreenDependentRenderMode)
        fy = (y / (float)m_nGridY - 0.5f);
      else
        fy = (y / (float)m_nGridY - 0.5f) * m_fAspectY;

      for (int x = 0; x <= m_nGridX; x++) {
        float fx;
        if (m_bScreenDependentRenderMode)
          fx = (x / (float)m_nGridX - 0.5f);
        else
          fx = (x / (float)m_nGridX - 0.5f) * m_fAspectX;

        float angle = atan2f(fy, fx);                           // -PI..PI
        float radius = sqrtf(fx * fx + fy * fy) * 1.41421356f;  // 0..~1

        // Signed spiral distance: the arm is centered on a smooth 0.5 contour.
        float spiral = (angle + 3.14159265f) / 6.2831853f;
        spiral += loops * radius;
        spiral += phase / 6.2831853f;
        spiral -= floorf(spiral);

        // Turn the wrap-around into a symmetric distance from the spiral center.
        float dist = fabsf(spiral - 0.5f) * 2.0f;
        if (dist > 1.0f)
          dist = 1.0f;

        // Soften the ramp, then keep the center neutral longer before ramping outward.
        float t = dist * dist * (3.0f - 2.0f * dist);
        float center_soften = radius * radius * (3.0f - 2.0f * radius);
        if (bMilk2Snail3)
          center_soften = 0.80f + 0.20f * center_soften;
        else if (bMilk2Snail2)
          center_soften = 0.20f + 0.80f * center_soften;
        else
          center_soften = 0.05f + 0.95f * center_soften;
        t = 0.5f + (t - 0.5f) * center_soften;

        if (inward)
          t = 1.0f - t;

        m_vertinfo[nVert].a = inv_band * (1.0f + band);
        m_vertinfo[nVert].c = -inv_band + inv_band * t;
        nVert++;
      }
    }
  } else if (mixtype == 6) {
    // DeepSeek - Rhombus/Diamond transition
    float band = 0.07f + 0.12f * FRAND;  // slightly narrower band for sharper edges
    float inv_band = 1.0f / band;
    float angle = 0.0f;   // keep the diamond upright for milk2 parity
    float aspect = 1.0f;  // keep the diamond symmetric
    bool reverse = (m_bLoadingMilk2 && m_nMilk2MixType == 6)
                       ? (m_fMilk2BlendDirection < 0.0f)
                       : ((rand() % 2) == 0);  // random direction

    // Precompute rotation matrix and normalization factor
    float cos_a = cosf(angle);
    float sin_a = sinf(angle);
    float norm_factor = 1.0f / (1.0f + aspect);

    int nVert = 0;
    for (int y = 0; y <= m_nGridY; y++) {
      float fy;
      if (m_bScreenDependentRenderMode)
        fy = (y / (float)m_nGridY - 0.5f);
      else
        fy = (y / (float)m_nGridY - 0.5f) * m_fAspectY;
      for (int x = 0; x <= m_nGridX; x++) {
        float fx;
        if (m_bScreenDependentRenderMode)
          fx = (x / (float)m_nGridX - 0.5f);
        else
          fx = (x / (float)m_nGridX - 0.5f) * m_fAspectX;

        // Rotate coordinates
        float rx = fx * cos_a - fy * sin_a;
        float ry = fx * sin_a + fy * cos_a;

        // Rhombus distance function (manhattan distance)
        float diamond = (fabsf(rx) * aspect + fabsf(ry)) * norm_factor;

        // Apply direction
        float t = reverse ? (1.0f - diamond) : diamond;

        // Apply band blending with edge clamping
        m_vertinfo[nVert].a = inv_band * (1.0f + band);
        m_vertinfo[nVert].c = -inv_band + inv_band * t;
        nVert++;
      }
    }
  } else if (mixtype == 7) {
    // DeepSeek - Nuclear Clock Wipe Transition
    float band = 0.05f + 0.15f * FRAND;  // band width for the transition edge
    float inv_band = 1.0f / band;
    const int exact_repeats = 3;  // exactly 3 full rotations
    bool reverse_direction = (rand() % 2) == 0;
    float glow_intensity = 0.5f + FRAND * 1.5f;  // nuclear glow effect

    // Calculate center point with slight random offset
    float center_x = 0.5f + (FRAND - 0.5f) * 0.1f;
    float center_y = 0.5f + (FRAND - 0.5f) * 0.1f;

    int nVert = 0;
    for (int y = 0; y <= m_nGridY; y++) {
      float fy;
      if (m_bScreenDependentRenderMode)
        fy = (y / (float)m_nGridY - center_y);
      else
        fy = (y / (float)m_nGridY - center_y) * m_fAspectY;
      for (int x = 0; x <= m_nGridX; x++) {
        float fx;
        if (m_bScreenDependentRenderMode)
          fx = (x / (float)m_nGridX - center_x);
        else
          fx = (x / (float)m_nGridX - center_x) * m_fAspectX;

        // Calculate angle and distance from center
        float angle = atan2f(fy, fx);                         // range: -PI to PI
        float dist = sqrtf(fx * fx + fy * fy) * 1.41421356f;  // normalized distance

        // Convert angle to 0-2PI range
        if (angle < 0) angle += 6.2831853f;

        // Calculate exact 3-repeat position (0-3 range)
        float clock_pos = angle / 6.2831853f * exact_repeats;

        if (reverse_direction)
          clock_pos = exact_repeats - clock_pos;

        // Keep only fractional part for seamless looping
        clock_pos = clock_pos - floorf(clock_pos);

        // Create nuclear effect by combining distance and angle
        float t = clock_pos;

        // Add distance-based falloff for glow effect
        float glow = (1.0f - dist) * glow_intensity;
        t += glow * 0.3f;  // blend in some glow

        // Apply band blending
        m_vertinfo[nVert].a = inv_band * (1.0f + band);
        m_vertinfo[nVert].c = -inv_band + inv_band * t;
        nVert++;
      }
    }
  } else if (mixtype == 8) {
    if (m_bLoadingMilk2 && wcsstr(m_szLoadingPreset, L"corner") != NULL) {
      // Corner transition: quarter-circle anchored in the bottom-left corner.
      float band = 0.38f;
      float inv_band = 1.0f / band;
      float dir = (m_fMilk2BlendDirection == 0.0f) ? 1.0f : m_fMilk2BlendDirection;

      int nVert = 0;
      for (int y = 0; y <= m_nGridY; y++) {
        float fy;
        if (m_bScreenDependentRenderMode)
          fy = (y / (float)m_nGridY);
        else
          fy = (y / (float)m_nGridY) * m_fAspectY;

        for (int x = 0; x <= m_nGridX; x++) {
          float fx;
          if (m_bScreenDependentRenderMode)
            fx = (x / (float)m_nGridX);
          else
            fx = (x / (float)m_nGridX) * m_fAspectX;

          float dx = fx;
          float dy = 1.0f - fy;
          float t = sqrtf(dx * dx + dy * dy) * 0.70710678f;
          if (dir == -1.0f)
            t = 1.0f - t;

          m_vertinfo[nVert].a = inv_band * (1.0f + band);
          m_vertinfo[nVert].c = -inv_band + inv_band * t;
          nVert++;
        }
      }
    } else {
      // DeepSeek - Square/Diamond Transition
      float band = 0.08f + 0.12f * FRAND;  // transition edge width
      float inv_band = 1.0f / band;
      bool diagonal = (rand() % 2) == 0;        // true = X-shape, false = +-shape
      float center_bias = 0.3f + FRAND * 0.4f;  // 0.3-0.7, controls center emphasis
      float softness = 0.1f + FRAND * 0.2f;     // edge softness

      // Define our own clamp function
      auto clamp = [](float value, float min, float max) {
        return (value < min) ? min : ((value > max) ? max : value);
      };

      int nVert = 0;
      for (int y = 0; y <= m_nGridY; y++) {
        float fy;
        if (m_bScreenDependentRenderMode)
          fy = (y / (float)m_nGridY - 0.5f);
        else
          fy = (y / (float)m_nGridY - 0.5f) * m_fAspectY;
        for (int x = 0; x <= m_nGridX; x++) {
          float fx;
          if (m_bScreenDependentRenderMode)
            fx = (x / (float)m_nGridX - 0.5f);
          else
            fx = (x / (float)m_nGridX - 0.5f) * m_fAspectX;

          float t;
          if (diagonal) {
            // X-shaped wipe (diagonal)
            float d1 = (fx + fy) * 0.7071f;  // 1/sqrt(2)
            float d2 = (fx - fy) * 0.7071f;
            t = (fabsf(d1) > fabsf(d2)) ? fabsf(d1) : fabsf(d2);
          } else {
            // +-shaped wipe (cardinal directions)
            t = (fabsf(fx) > fabsf(fy)) ? fabsf(fx) : fabsf(fy);
          }

          // Apply center bias for more interesting pattern
          t = powf(t, center_bias);

          // Add optional softness to edges
          t = t * (1.0f + softness) - softness * 0.5f;
          t = clamp(t, 0.0f, 1.0f);

          // Apply band blending
          m_vertinfo[nVert].a = inv_band * (1.0f + band);
          m_vertinfo[nVert].c = -inv_band + inv_band * t;
          nVert++;
        }
      }
    }
  } else if (mixtype == 9) {
    // MilkDrop 3 patches: flat diamond checkerboard wipe.
    float progress = m_fMilk2BlendProgress;
    if (progress < 0.0f) progress = 0.0f;
    if (progress > 1.0f) progress = 1.0f;

    const int cellsX = 3;
    const int cellsY = 3;
    float band = 0.26f + 0.04f * m_fMilk2Random2;
    float inv_band = 1.0f / band;
    float aspectX = m_bScreenDependentRenderMode ? 1.0f : m_fAspectX;
    float aspectY = m_bScreenDependentRenderMode ? 1.0f : m_fAspectY;

    float phase = (m_fMilk2Random1 - 0.5f) * 0.08f;
    bool flipParity = (m_fMilk2Random5 >= 0.5f);
    float rotation = 0.78539816f;
    float cos_r = cosf(rotation);
    float sin_r = sinf(rotation);

    auto smoothstep01 = [](float x) {
      x = max(0.0f, min(1.0f, x));
      return x * x * (3.0f - 2.0f * x);
    };

    int nVert = 0;
    for (int y = 0; y <= m_nGridY; y++) {
      float fy = (y / (float)m_nGridY - 0.5f) * aspectY;

      for (int x = 0; x <= m_nGridX; x++) {
        float fx = (x / (float)m_nGridX - 0.5f) * aspectX;

        float rx = fx * cos_r - fy * sin_r;
        float ry = fx * sin_r + fy * cos_r;

        float u = rx * cellsX + 0.5f + phase;
        float v = ry * cellsY + 0.5f - phase;

        float cellXF = u;
        float cellYF = v;
        int cellX = (int)floorf(cellXF);
        int cellY = (int)floorf(cellYF);

        float localX = cellXF - floorf(cellXF);
        float localY = cellYF - floorf(cellYF);
        float edge = min(min(localX, 1.0f - localX), min(localY, 1.0f - localY));

        float t = ((((cellX + 1024) + (cellY + 1024)) & 1) != 0) ? 1.0f : 0.0f;

        if (flipParity)
          t = 1.0f - t;

        float edgeSoft = smoothstep01(edge / (band * 1.05f));
        t = 0.5f + (t - 0.5f) * edgeSoft;

        m_vertinfo[nVert].a = inv_band * (1.0f + band);
        m_vertinfo[nVert].c = -inv_band + inv_band * t;
        nVert++;
      }
    }
  } else if (mixtype == 10) {
    if (m_bLoadingMilk2 && m_nMilk2MixType == 10 && m_bMilk2LinesVertical) {
      // MilkDrop 3 linesvertical:
      // Use a repeating vertical field, but keep it broad enough that the
      // reference reads as a couple of major regions rather than many narrow lines.
      const float repeats = 2.0f;
      const float phase = 0.18f;
      float band = 0.10f + 0.03f * FRAND;
      float inv_band = 1.0f / band;

      int nVert = 0;
      for (int y = 0; y <= m_nGridY; y++) {
        for (int x = 0; x <= m_nGridX; x++) {
          float raw_fx = x / (float)m_nGridX;

          // Use a repeating sawtooth field; progress should move the blend from
          // green (old) toward red (new).
          float t = fmodf(raw_fx * repeats + phase, 1.0f);

          m_vertinfo[nVert].a = inv_band * (1.0f + band);
          m_vertinfo[nVert].c = -inv_band + inv_band * t;
          nVert++;
        }
      }
    } else if (m_bLoadingMilk2 && m_nMilk2MixType == 10) {
      // MilkDrop 3 curtain: a single centered vertical curtain with a broad feather.
      float band = 0.45f + 0.05f * FRAND;
      float inv_band = 1.0f / band;
      float progress = m_fMilk2BlendProgress * 0.75f + 0.02f;
      if (progress < 0.0f) progress = 0.0f;
      if (progress > 1.0f) progress = 1.0f;
      float halfWidth = 0.08f + 0.55f * progress;

      int nVert = 0;
      for (int y = 0; y <= m_nGridY; y++) {
        float fy;
        if (m_bScreenDependentRenderMode)
          fy = (y / (float)m_nGridY);
        else
          fy = (y / (float)m_nGridY) * m_fAspectY;

        for (int x = 0; x <= m_nGridX; x++) {
          float fx;
          if (m_bScreenDependentRenderMode)
            fx = (x / (float)m_nGridX);
          else
            fx = (x / (float)m_nGridX) * m_fAspectX;

          float dist = fabsf(fx - 0.5f);
          float t;
          if (dist <= halfWidth) {
            t = 1.0f;
          } else if (dist >= halfWidth + band) {
            t = 0.0f;
          } else {
            float u = (dist - halfWidth) / band;
            t = 1.0f - (u * u * (3.0f - 2.0f * u));
          }

          m_vertinfo[nVert].a = inv_band * (1.0f + band);
          m_vertinfo[nVert].c = -inv_band + inv_band * t;
          nVert++;
        }
      }
    } else {
      // DeepSeek - Curtain Transition
      float band = 0.05f + 0.15f * FRAND;  // transition edge width
      float inv_band = 1.0f / band;
      bool opening = (rand() % 2) == 0;              // true = opening, false = closing
      bool vertical = (rand() % 2) == 0;             // true = vertical curtains, false = horizontal
      float curtain_wrinkles = 0.5f + FRAND * 2.0f;  // amount of wrinkles/folds (0.5-2.5)
      float center_gap = 0.05f + FRAND * 0.15f;      // gap between curtains (0.05-0.2)
      bool reverse_motion = (rand() % 2) == 0;       // reverse motion direction

      // NEW: Configure repeats/wipe patterns
      int repeats = 1 + (rand() % 4);                // 1-4 repeats (1=normal curtain, 2-4=striped patterns)
      float repeat_width = 1.0f / repeats;           // width of each repeat segment
      float repeat_variation = 0.3f * FRAND;         // 0-0.3 variation in repeat timing
      bool alternate_direction = (rand() % 2) == 0;  // alternate stripe directions

      int nVert = 0;
      for (int y = 0; y <= m_nGridY; y++) {
        float fy = (y / (float)m_nGridY);
        for (int x = 0; x <= m_nGridX; x++) {
          float fx = (x / (float)m_nGridX);

          float t;
          if (vertical) {
            // Vertical curtains (left and right)
            float pos = fx;
            float segment_pos = pos * repeats;                // position within repeat segments
            int segment_idx = (int)floorf(segment_pos);       // which segment we're in
            float segment_local = segment_pos - segment_idx;  // 0-1 within segment

            float center_dist = fabsf(segment_local - 0.5f) - center_gap / 2;
            if (center_dist < 0) center_dist = 0;

            // Determine which curtain this pixel belongs to
            float curtain_side = (segment_local < 0.5f) ? -1.0f : 1.0f;

            // Calculate base transition value
            t = center_dist * 2.0f;  // ranges 0-1 for each curtain segment

            // Add per-segment variation
            float segment_variation = sinf(segment_idx * 1.618f) * repeat_variation;
            t += segment_variation;

            // Add wrinkles/folds effect using sine wave
            float wrinkles = sinf(fy * 3.14159f * curtain_wrinkles) * 0.1f;
            t += wrinkles * (1.0f - t);

            // Adjust for opening/closing
            if (opening)
              t = 1.0f - t;

            // Adjust for curtain side and alternate directions
            if (alternate_direction && (segment_idx % 2 == 1))
              curtain_side *= -1.0f;

            if (reverse_motion)
              t = curtain_side > 0 ? t : 1.0f - t;
            else
              t = curtain_side > 0 ? 1.0f - t : t;
          } else {
            // Horizontal curtains (top and bottom)
            float pos = fy;
            float segment_pos = pos * repeats;                // position within repeat segments
            int segment_idx = (int)floorf(segment_pos);       // which segment we're in
            float segment_local = segment_pos - segment_idx;  // 0-1 within segment

            float center_dist = fabsf(segment_local - 0.5f) - center_gap / 2;
            if (center_dist < 0) center_dist = 0;

            // Determine which curtain this pixel belongs to
            float curtain_side = (segment_local < 0.5f) ? -1.0f : 1.0f;

            // Calculate base transition value
            t = center_dist * 2.0f;  // ranges 0-1 for each curtain segment

            // Add per-segment variation
            float segment_variation = sinf(segment_idx * 1.618f) * repeat_variation;
            t += segment_variation;

            // Add wrinkles/folds effect using sine wave
            float wrinkles = sinf(fx * 3.14159f * curtain_wrinkles) * 0.1f;
            t += wrinkles * (1.0f - t);

            // Adjust for opening/closing
            if (opening)
              t = 1.0f - t;

            // Adjust for curtain side and alternate directions
            if (alternate_direction && (segment_idx % 2 == 1))
              curtain_side *= -1.0f;

            if (reverse_motion)
              t = curtain_side > 0 ? t : 1.0f - t;
            else
              t = curtain_side > 0 ? 1.0f - t : t;
          }

          // Apply band blending
          m_vertinfo[nVert].a = inv_band * (1.0f + band);
          m_vertinfo[nVert].c = -inv_band + inv_band * t;
          nVert++;
        }
      }
    }
  } else if (mixtype == 11) {
    if (m_bLoadingMilk2 && m_nMilk2MixType == 11) {
      // MilkDrop 3 donuts: a centered bullseye with a broad smooth middle ring.
      float band = 0.22f + 0.04f * FRAND;
      float inv_band = 1.0f / band;
      float progress = m_fMilk2BlendProgress;
      progress = progress * progress;
      if (progress < 0.0f) progress = 0.0f;
      if (progress > 1.0f) progress = 1.0f;

      float innerRadius = 0.05f + 0.26f * progress;
      float outerRadius = 0.68f + 0.12f * progress;
      if (outerRadius < innerRadius + 0.20f)
        outerRadius = innerRadius + 0.20f;

      int nVert = 0;
      for (int y = 0; y <= m_nGridY; y++) {
        float fy;
        if (m_bScreenDependentRenderMode)
          fy = (y / (float)m_nGridY - 0.5f);
        else
          fy = (y / (float)m_nGridY - 0.5f) * m_fAspectY;

        for (int x = 0; x <= m_nGridX; x++) {
          float fx;
          if (m_bScreenDependentRenderMode)
            fx = (x / (float)m_nGridX - 0.5f);
          else
            fx = (x / (float)m_nGridX - 0.5f) * m_fAspectX;

          float dist = sqrtf(fx * fx + fy * fy) * 1.41421356f;
          float t;
          if (dist <= innerRadius) {
            t = 1.0f;
          } else if (dist < innerRadius + band) {
            float u = (dist - innerRadius) / band;
            t = 1.0f - (u * u * (3.0f - 2.0f * u));
          } else if (dist < outerRadius - band) {
            t = 0.0f;
          } else if (dist < outerRadius) {
            float u = (dist - (outerRadius - band)) / band;
            t = u * u * (3.0f - 2.0f * u);
          } else {
            t = 1.0f;
          }

          m_vertinfo[nVert].a = inv_band * (1.0f + band);
          m_vertinfo[nVert].c = -inv_band + inv_band * t;
          nVert++;
        }
      }
    } else {
      // DeepSeek - Bubble Transition
      float band = 0.05f + 0.15f * FRAND;  // transition edge width
      float inv_band = 1.0f / band;
      int bubble_count = 10 + (rand() % 30);         // number of bubbles (10-40)
      float bubble_size_min = 0.05f + FRAND * 0.1f;  // min bubble size (0.05-0.15)
      float bubble_size_max = 0.15f + FRAND * 0.2f;  // max bubble size (0.15-0.35)
      bool growing_bubbles = (rand() % 2) == 0;      // true = bubbles grow, false = shrink

      // Generate random bubble positions and sizes
      struct Bubble {
        float x, y;   // position (0-1 range)
        float size;   // radius (0-1 range)
        float speed;  // growth/shrink speed
      };

      Bubble* bubbles = new Bubble[bubble_count];
      for (int i = 0; i < bubble_count; i++) {
        bubbles[i].x = FRAND;
        bubbles[i].y = FRAND;
        bubbles[i].size = bubble_size_min + FRAND * (bubble_size_max - bubble_size_min);
        bubbles[i].speed = 0.5f + FRAND * 1.5f;  // speed multiplier (0.5-2.0)
      }

      int nVert = 0;
      for (int y = 0; y <= m_nGridY; y++) {
        float fy = (y / (float)m_nGridY);
        for (int x = 0; x <= m_nGridX; x++) {
          float fx = (x / (float)m_nGridX);

          // Find the maximum bubble influence at this pixel
          float max_influence = 0.0f;

          for (int i = 0; i < bubble_count; i++) {
            // Calculate distance to bubble center
            float dx, dy;
            if (m_bScreenDependentRenderMode) {
              dx = (fx - bubbles[i].x);
              dy = (fy - bubbles[i].y);
            } else {
              dx = (fx - bubbles[i].x) * m_fAspectX;
              dy = (fy - bubbles[i].y) * m_fAspectY;
            }
            float dist = sqrtf(dx * dx + dy * dy);

            // Calculate bubble influence (1 at center, 0 at edge)
            float influence = 1.0f - (dist / bubbles[i].size);
            if (influence < 0) influence = 0;

            // Apply smoothstep for smoother edges
            influence = influence * influence * (3.0f - 2.0f * influence);

            if (influence > max_influence)
              max_influence = influence;
          }

          // If we're shrinking bubbles, invert the influence
          float t = growing_bubbles ? max_influence : (1.0f - max_influence);

          // Apply band blending
          m_vertinfo[nVert].a = inv_band * (1.0f + band);
          m_vertinfo[nVert].c = -inv_band + inv_band * t;
          nVert++;
        }
      }
      delete[] bubbles;
    }
  } else if (mixtype == 12) {
    // DeepSeek - Kaleidoscope Wipe Transition
    float band = 0.06f + 0.14f * FRAND;  // transition edge width
    float inv_band = 1.0f / band;

    // Kaleidoscope parameters
    int segments = 3 + (rand() % 9);              // 3-12 segments (triangular to dodecagonal)
    float segment_angle = 6.2831853f / segments;  // angle per segment in radians
    float rotation = FRAND * 6.2831853f;          // random initial rotation
    bool mirror_effect = (rand() % 2) == 0;       // true = mirrored segments, false = just rotated
    float radial_factor = 0.5f + FRAND;           // 0.5-1.5 - how much radial distance affects the pattern

    int nVert = 0;
    for (int y = 0; y <= m_nGridY; y++) {
      float fy;
      if (m_bScreenDependentRenderMode)
        fy = (y / (float)m_nGridY - 0.5f);
      else
        fy = (y / (float)m_nGridY - 0.5f) * m_fAspectY;
      for (int x = 0; x <= m_nGridX; x++) {
        float fx;
        if (m_bScreenDependentRenderMode)
          fx = (x / (float)m_nGridX - 0.5f);
        else
          fx = (x / (float)m_nGridX - 0.5f) * m_fAspectX;

        // Calculate polar coordinates
        float angle = atan2f(fy, fx) + rotation;                // range: -PI to PI plus rotation
        float radius = sqrtf(fx * fx + fy * fy) * 1.41421356f;  // normalized distance

        // Wrap angle to 0-2PI range
        if (angle < 0) angle += 6.2831853f;
        if (angle >= 6.2831853f) angle -= 6.2831853f;

        // Find which segment we're in and map to first segment
        int segment = (int)(angle / segment_angle);
        float segment_offset = angle - segment * segment_angle;

        // For mirrored segments, reflect angles past the halfway point
        if (mirror_effect && segment_offset > segment_angle * 0.5f) {
          segment_offset = segment_angle - segment_offset;
        }

        // Normalize the segment angle to 0-1 range
        float normalized_angle = segment_offset / segment_angle;

        // Combine angle and radius for the pattern
        float t = (normalized_angle * 0.7f + radius * 0.3f * radial_factor);

        // Apply band blending
        m_vertinfo[nVert].a = inv_band * (1.0f + band);
        m_vertinfo[nVert].c = -inv_band + inv_band * t;
        nVert++;
      }
    }
  } else if (mixtype == 13) {
    // DeepSeek - Moebius Strip Transition
    float band = 0.07f + 0.13f * FRAND;  // transition edge width
    float inv_band = 1.0f / band;

    // Moebius parameters
    float twist_factor = 1.0f + FRAND * 2.0f;  // 1-3 controls twist intensity
    bool reverse_twist = (rand() % 2) == 0;    // random twist direction
    float strip_width = 0.3f + FRAND * 0.4f;   // 0.3-0.7 width of the moebius strip
    float progress_offset = FRAND * 0.5f;      // 0-0.5 random phase offset

    int nVert = 0;
    for (int y = 0; y <= m_nGridY; y++) {
      float fy;
      if (m_bScreenDependentRenderMode)
        fy = (y / (float)m_nGridY - 0.5f);
      else
        fy = (y / (float)m_nGridY - 0.5f) * m_fAspectY;
      for (int x = 0; x <= m_nGridX; x++) {
        float fx;
        if (m_bScreenDependentRenderMode)
          fx = (x / (float)m_nGridX - 0.5f);
        else
          fx = (x / (float)m_nGridX - 0.5f) * m_fAspectX;

        // Convert to polar coordinates
        float angle = atan2f(fy, fx);                           // range: -PI to PI
        float radius = sqrtf(fx * fx + fy * fy) * 1.41421356f;  // normalized 0-1

        // Create moebius strip effect
        float normalized_angle = (angle + 3.14159265f) / 6.2831853f;  // 0-1

        // Calculate the twist - makes a half-twist as we go around the circle
        float twist_progress = (normalized_angle + progress_offset) * twist_factor;
        if (reverse_twist) twist_progress = -twist_progress;

        // Moebius strip effect combines radius with twisted angle
        float moebius_value = radius + 0.3f * sinf(twist_progress * 3.14159265f);

        // Apply strip width to create the banding effect
        float t = fmodf(moebius_value * (1.0f / strip_width), 1.0f);

        // Make the transition flow outward
        t = 1.0f - t;

        // Apply band blending
        m_vertinfo[nVert].a = inv_band * (1.0f + band);
        m_vertinfo[nVert].c = -inv_band + inv_band * t;
        nVert++;
      }
    }
  } else if (mixtype == 14) {
    // MilkDrop 3 stars: 10 broad radial sectors from the center outward.
    const bool bMilk2Stars = m_bLoadingMilk2 && m_nMilk2MixType == 14;
    float progress = bMilk2Stars ? min(1.0f, max(0.0f, m_fMilk2BlendProgress)) : 0.5f;
    float band = bMilk2Stars ? (0.05f + 0.03f * m_fMilk2Random2) : (0.07f + 0.03f * FRAND);
    float inv_band = 1.0f / band;
    float phase = bMilk2Stars ? (m_fMilk2Random1 * 6.2831853f) : (FRAND * 6.2831853f);
    bool reverse = bMilk2Stars ? (m_fMilk2BlendDirection < 0.0f) : ((rand() % 2) == 0);
    const int segmentCount = 10;
    float segment = 6.2831853f / segmentCount;
    float edgeSoft = bMilk2Stars ? (0.14f + 0.06f * m_fMilk2Random3) : (0.16f + 0.08f * FRAND);

    int nVert = 0;
    for (int y = 0; y <= m_nGridY; y++) {
      float fy;
      if (m_bScreenDependentRenderMode)
        fy = (y / (float)m_nGridY - 0.5f);
      else
        fy = (y / (float)m_nGridY - 0.5f) * m_fAspectY;
      for (int x = 0; x <= m_nGridX; x++) {
        float fx;
        if (m_bScreenDependentRenderMode)
          fx = (x / (float)m_nGridX - 0.5f);
        else
          fx = (x / (float)m_nGridX - 0.5f) * m_fAspectX;

        float angle = atan2f(fy, fx) + phase;
        float radius = sqrtf(fx * fx + fy * fy) * 1.41421356f;

        if (angle < 0.0f) angle += 6.2831853f;
        if (angle >= 6.2831853f) angle -= 6.2831853f;

        // Build 10 flat angular sectors with smooth boundaries.
        float sectorPos = angle / segment;
        int sector = (int)floorf(sectorPos);
        float local = sectorPos - sector;
        float boundary = min(local, 1.0f - local);
        float edge = boundary / edgeSoft;
        if (edge > 1.0f)
          edge = 1.0f;
        edge = edge * edge * (3.0f - 2.0f * edge);

        float sectorColor = ((sector + (int)(phase / 6.2831853f * 10.0f)) % 2 == 0) ? 1.0f : 0.0f;
        float fill = progress;
        float t = fill + (sectorColor - 0.5f) * edge * (1.0f - fill) * 2.0f;
        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;
        if (reverse)
          t = 1.0f - t;

        m_vertinfo[nVert].a = inv_band * (1.0f + band);
        m_vertinfo[nVert].c = -inv_band + inv_band * t;
        nVert++;
      }
    }
  } else if (mixtype == 15) {
    // DeepSeek - Disco Floor Transition
    float band = 0.08f + 0.12f * FRAND;  // transition edge width
    float inv_band = 1.0f / band;

    // Disco floor parameters
    int tile_size = 8 + (rand() % 25);          // 8-32 pixel tile size (approximate)
    float beat_sync = 0.5f + FRAND * 1.5f;      // 0.5-2.0 beat sync intensity
    bool diagonal_pattern = (rand() % 2) == 0;  // alternate diagonal pattern
    bool color_cycling = (rand() % 2) == 0;     // enable color cycling effect
    float speed_factor = 0.5f + FRAND * 2.0f;   // animation speed (0.5-2.5)

    // Get current time for animation (using a fake time if not available)
    static float fake_time = 0.0f;
    fake_time += 1 / GetFps();
    float time = fake_time * speed_factor;

    // Simulate beat detection with a sine wave if real beat info isn't available
    float beat = sinf(time * 3.0f) * 0.5f + 0.5f;
    beat = powf(beat, beat_sync);

    int nVert = 0;
    for (int y = 0; y <= m_nGridY; y++) {
      float fy = y / (float)m_nGridY;
      for (int x = 0; x <= m_nGridX; x++) {
        float fx = x / (float)m_nGridX;

        // Calculate tile coordinates
        int tile_x = (int)(fx * m_nGridX / tile_size);
        int tile_y = (int)(fy * m_nGridY / tile_size);

        // Create alternating pattern
        float pattern;
        if (diagonal_pattern) {
          // Diagonal checkerboard pattern
          pattern = ((tile_x + tile_y) % 2) * 0.8f + 0.1f;
        } else {
          // Standard checkerboard pattern
          pattern = ((tile_x % 2) == (tile_y % 2)) * 0.8f + 0.1f;
        }

        // Add animation based on tile position and time
        float anim = sinf(time * 2.0f + tile_x * 0.3f + tile_y * 0.7f) * 0.5f + 0.5f;

        // Combine with beat detection
        float t = (pattern * 0.7f + anim * 0.3f) * beat;

        // Add color cycling effect if enabled
        if (color_cycling) {
          float hue = fmodf(time * 0.2f + tile_x * 0.1f + tile_y * 0.15f, 1.0f);
          t = fmodf(t + hue * 0.3f, 1.0f);
        }

        // Apply band blending
        m_vertinfo[nVert].a = inv_band * (1.0f + band);
        m_vertinfo[nVert].c = -inv_band + inv_band * t;
        nVert++;
      }
    }
  } else if (mixtype == 16) {
    // DeepSeek - Fire/Flame Transition - rising upward with random patterns
    float band = 0.08f + 0.04f * FRAND;  // flame edge thickness
    float inv_band = 1.0f / band;

    // Fire parameters
    float flame_speed = 0.7f + FRAND * 0.6f;  // speed (0.7-1.3)
    float base_height = 0.0f;                 // always start at bottom

    // Pre-compute some random flame properties
    float seed1 = FRAND * 10.0f;
    float seed2 = FRAND * 20.0f;
    float seed3 = FRAND * 30.0f;

    // Get current time for animation
    static float fire_time = 0.0f;
    fire_time += 1 / GetFps();
    float time = fire_time;

    int nVert = 0;
    for (int y = 0; y <= m_nGridY; y++) {
      float fy = (y / (float)m_nGridY);  // 0-1 from bottom to top
      for (int x = 0; x <= m_nGridX; x++) {
        float fx = (x / (float)m_nGridX);

        // Generate deterministic random patterns using noise functions
        float random_flame =
            sinf(fx * 15.0f + seed1 + time * 2.0f) * 0.4f +
            sinf(fx * 30.0f + seed2 + time * 3.7f) * 0.2f +
            sinf(fx * 45.0f + seed3 + time * 5.3f) * 0.1f;

        // Shape the flame (wider at bottom, narrower at top)
        float flame_shape = (1.0f - fy) * (0.3f + random_flame * 0.7f);

        // Calculate flame front position (rising from bottom)
        float flame_front = fmodf(time * flame_speed, 1.5f);

        // Flame transition value - positive when below flame front
        float t = 1.0f - (fy - flame_front + flame_shape);

        // Basic 0-1 clamping
        t = (t < 0) ? 0 : ((t > 1) ? 1 : t);

        // Apply band blending
        m_vertinfo[nVert].a = inv_band * (1.0f + band);
        m_vertinfo[nVert].c = -inv_band + inv_band * t;
        nVert++;
      }
    }
  } else if (mixtype == 17) {
    // DeepSeek - Drain Swirl Transition, modified by Incubo_
    float band = 0.05f + 0.15f * FRAND;  // transition edge width
    float inv_band = 1.0f / band;

    // Drain parameters
    float swirl_intensity = 2.0f + FRAND * 3.0f;  // 2-5 - controls how tight the swirl is
    float drain_speed = 0.5f + FRAND * 1.5f;      // 0.5-2.0 - speed of the drain effect
    bool clockwise = (rand() % 2) == 0;           // random swirl direction
    float center_pull = 0.7f + FRAND * 0.6f;      // 0.7-1.3 - how strongly it pulls to center
    bool invert = (rand() % 2) == 0;              // random inversion

    // Get current time for animation
    static float drain_time = 0.0f;
    drain_time += 1 / GetFps();
    float time = drain_time * drain_speed;

    int nVert = 0;
    for (int y = 0; y <= m_nGridY; y++) {
      float fy;
      if (m_bScreenDependentRenderMode)
        fy = (y / (float)m_nGridY - 0.5f);
      else
        fy = (y / (float)m_nGridY - 0.5f) * m_fAspectY;
      for (int x = 0; x <= m_nGridX; x++) {
        float fx;
        if (m_bScreenDependentRenderMode)
          fx = (x / (float)m_nGridX - 0.5f);
        else
          fx = (x / (float)m_nGridX - 0.5f) * m_fAspectX;

        // Calculate polar coordinates
        float radius = sqrtf(fx * fx + fy * fy) * 1.41421356f;  // normalized distance
        float angle = atan2f(fy, fx);                           // range: -PI to PI

        // Apply swirl effect - angle changes more as you get closer to center
        float swirl_factor = (1.0f - radius) * swirl_intensity;
        if (clockwise) swirl_factor = -swirl_factor;

        // Combine with time-based animation
        float swirled_angle = angle + swirl_factor + time * 2.0f;

        // Create the drain effect - combines radial and angular motion
        float t = radius * center_pull + (1.0f - center_pull) *
                                             (0.5f + 0.5f * sinf(swirled_angle * 2.0f + radius * 5.0f));

        // Invert the drain if needed.
        if (invert)
          t = 1.0f - t;

        // Apply band blending
        m_vertinfo[nVert].a = inv_band * (1.0f + band);
        m_vertinfo[nVert].c = -inv_band + inv_band * t;
        nVert++;
      }
    }
  } else if (mixtype == 18) {
    // DeepSeek - Smooth Julia Set Fractal Transition
    float band = 0.08f + 0.12f * FRAND;  // Wider band for smoother transitions
    float inv_band = 1.0f / band;

    // Julia set parameters with constrained ranges for better blending
    float julia_real = -0.8f + FRAND * 1.6f;  // (-0.8 to 0.8)
    float julia_imag = -0.8f + FRAND * 1.6f;  // (-0.8 to 0.8)
    int max_iterations = 20 + (rand() % 20);  // 20-40 iterations (good balance)
    float zoom = 0.7f + FRAND * 1.6f;         // 0.7-2.3 zoom level
    float rotation = FRAND * 6.2831853f;      // random rotation

    // Always use smooth coloring for this version
    const bool smooth_coloring = true;

    // Additional smoothing parameters
    float edge_softness = 0.3f + FRAND * 0.5f;  // 0.3-0.8 edge softness
    float contrast = 0.7f + FRAND * 0.6f;       // 0.7-1.3 contrast adjustment

    // Precompute rotation values
    float cos_rot = cosf(rotation);
    float sin_rot = sinf(rotation);

    // Find min/max for normalization
    float min_val = FLT_MAX;
    float max_val = -FLT_MAX;
    std::vector<float> values((m_nGridY + 1) * (m_nGridX + 1));

    // First pass: compute all values and find range
    int nVert = 0;
    for (int y = 0; y <= m_nGridY; y++) {
      float fy;
      if (m_bScreenDependentRenderMode)
        fy = (y / (float)m_nGridY - 0.5f);
      else
        fy = (y / (float)m_nGridY - 0.5f) * m_fAspectY;

      for (int x = 0; x <= m_nGridX; x++) {
        float fx;
        if (m_bScreenDependentRenderMode)
          fx = (x / (float)m_nGridX - 0.5f);
        else
          fx = (x / (float)m_nGridX - 0.5f) * m_fAspectX;

        // Apply rotation and zoom
        float zx = (fx * cos_rot - fy * sin_rot) * zoom;
        float zy = (fx * sin_rot + fy * cos_rot) * zoom;

        // Julia set iteration
        float cx = julia_real;
        float cy = julia_imag;
        int i;
        for (i = 0; i < max_iterations; i++) {
          float tmp = zx * zx - zy * zy + cx;
          zy = 2 * zx * zy + cy;
          zx = tmp;

          if (zx * zx + zy * zy > 4.0f)
            break;
        }

        // Calculate smooth value
        float t;
        if (i < max_iterations) {
          float log_zn = logf(zx * zx + zy * zy) / 2.0f;
          float nu = logf(log_zn / logf(2.0f)) / logf(2.0f);
          t = (i + 1 - nu) / max_iterations;
        } else {
          t = 1.0f;  // Interior points
        }

        // Apply contrast adjustment
        t = powf(t, contrast);

        values[nVert] = t;
        if (t < min_val) min_val = t;
        if (t > max_val) max_val = t;
        nVert++;
      }
    }

    // Normalize and apply blending
    float range = max_val - min_val;
    if (range < 0.0001f) range = 1.0f;  // Prevent division by zero

    nVert = 0;
    for (int y = 0; y <= m_nGridY; y++) {
      for (int x = 0; x <= m_nGridX; x++) {
        // Normalize value to 0-1 range
        float t = (values[nVert] - min_val) / range;

        // Apply edge softness using smoothstep function
        t = t * t * (3.0f - 2.0f * t) * (1.0f - edge_softness) + t * edge_softness;

        // Final blending calculation with smoother transition
        m_vertinfo[nVert].a = inv_band * (1.0f + band * 1.5f);  // Increased blend area
        m_vertinfo[nVert].c = -inv_band + inv_band * t * 1.1f;  // Slightly extended range

        // Ensure values stay within reasonable bounds
        m_vertinfo[nVert].c = max(-10.0f, min(10.0f, m_vertinfo[nVert].c));
        nVert++;
      }
    }
  } else if (mixtype == 19) {
    // Fixed vertical wipe (left-to-right) for .milk2 "vertical"
    float band = 0.34f;
    float inv_band = 1.0f / band;
    int nVert = 0;
    for (int y = 0; y <= m_nGridY; y++) {
      for (int x = 0; x <= m_nGridX; x++) {
        float fx = (x / (float)m_nGridX);
        float t = fx;
        m_vertinfo[nVert].a = inv_band * (1 + band);
        m_vertinfo[nVert].c = -inv_band + inv_band * t;
        nVert++;
      }
    }
  } else if (mixtype == 20) {
    // Fixed horizontal wipe (top-to-bottom) for .milk2 "horizontal"
    float band = 0.34f;
    float inv_band = 1.0f / band;
    int nVert = 0;
    for (int y = 0; y <= m_nGridY; y++) {
      float fy = (y / (float)m_nGridY);
      for (int x = 0; x <= m_nGridX; x++) {
        float t = fy;
        m_vertinfo[nVert].a = inv_band * (1 + band);
        m_vertinfo[nVert].c = -inv_band + inv_band * t;
        nVert++;
      }
    }
  }
}

void CPlugin::GenPlasma(int x0, int x1, int y0, int y1, float dt) {
  int midx = (x0 + x1) / 2;
  int midy = (y0 + y1) / 2;
  float t00 = m_vertinfo[y0 * (m_nGridX + 1) + x0].c;
  float t01 = m_vertinfo[y0 * (m_nGridX + 1) + x1].c;
  float t10 = m_vertinfo[y1 * (m_nGridX + 1) + x0].c;
  float t11 = m_vertinfo[y1 * (m_nGridX + 1) + x1].c;

  if (y1 - y0 >= 2) {
    if (x0 == 0)
      if (m_bScreenDependentRenderMode)
        m_vertinfo[midy * (m_nGridX + 1) + x0].c = 0.5f * (t00 + t10) + (FRAND * 2 - 1) * dt;
      else
        m_vertinfo[midy * (m_nGridX + 1) + x0].c = 0.5f * (t00 + t10) + (FRAND * 2 - 1) * dt * m_fAspectY;
    if (m_bScreenDependentRenderMode)
      m_vertinfo[midy * (m_nGridX + 1) + x1].c = 0.5f * (t01 + t11) + (FRAND * 2 - 1) * dt;
    else
      m_vertinfo[midy * (m_nGridX + 1) + x1].c = 0.5f * (t01 + t11) + (FRAND * 2 - 1) * dt * m_fAspectY;
  }
  if (x1 - x0 >= 2) {
    if (y0 == 0)
      if (m_bScreenDependentRenderMode)
        m_vertinfo[y0 * (m_nGridX + 1) + midx].c = 0.5f * (t00 + t01) + (FRAND * 2 - 1) * dt;
      else
        m_vertinfo[y0 * (m_nGridX + 1) + midx].c = 0.5f * (t00 + t01) + (FRAND * 2 - 1) * dt * m_fAspectX;
    if (m_bScreenDependentRenderMode)
      m_vertinfo[y1 * (m_nGridX + 1) + midx].c = 0.5f * (t10 + t11) + (FRAND * 2 - 1) * dt;
    else
      m_vertinfo[y1 * (m_nGridX + 1) + midx].c = 0.5f * (t10 + t11) + (FRAND * 2 - 1) * dt * m_fAspectX;
  }

  if (y1 - y0 >= 2 && x1 - x0 >= 2) {
    // do midpoint & recurse:
    t00 = m_vertinfo[midy * (m_nGridX + 1) + x0].c;
    t01 = m_vertinfo[midy * (m_nGridX + 1) + x1].c;
    t10 = m_vertinfo[y0 * (m_nGridX + 1) + midx].c;
    t11 = m_vertinfo[y1 * (m_nGridX + 1) + midx].c;
    m_vertinfo[midy * (m_nGridX + 1) + midx].c = 0.25f * (t10 + t11 + t00 + t01) + (FRAND * 2 - 1) * dt;

    GenPlasma(x0, midx, y0, midy, dt * 0.5f);
    GenPlasma(midx, x1, y0, midy, dt * 0.5f);
    GenPlasma(x0, midx, midy, y1, dt * 0.5f);
    GenPlasma(midx, x1, midy, y1, dt * 0.5f);
  }
}
