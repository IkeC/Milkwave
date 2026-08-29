// plugin_renderui.cpp - UI overlay rendering (MyRenderUI)
// Extracted from plugin.cpp

#include "plugin.h"
#include "plugin_internals.h"
#include "utility.h"
#include "support.h"
#include "resource.h"
#include "defines.h"
#include "shell_defines.h"
#include "wasabi.h"
#include "milkwave.h"

#define MTO_UPPER_RIGHT 0
#define MTO_UPPER_LEFT 1
#define MTO_LOWER_RIGHT 2
#define MTO_LOWER_LEFT 3

#define SelectFont(n)     \
  {                       \
    pFont = GetFont(n);   \
    h = GetFontHeight(n); \
  }

#define MyTextOut_BGCOLOR(str, corner, bDarkBox, boxColor)                                                                                                                                                              \
  {                                                                                                                                                                                                                     \
    SetRect(&r, 0, 0, xR - xL, 2048);                                                                                                                                                                                   \
    m_text.DrawTextW(pFont, str, -1, &r, DT_NOPREFIX | ((corner == MTO_UPPER_RIGHT) ? 0 : DT_SINGLELINE) | DT_WORD_ELLIPSIS | DT_CALCRECT | ((corner == MTO_UPPER_RIGHT) ? DT_RIGHT : 0), 0xFFFFFFFF, false, boxColor); \
    int w = r.right - r.left;                                                                                                                                                                                           \
    if (corner == MTO_UPPER_LEFT)                                                                                                                                                                                       \
      SetRect(&r, xL, *upper_left_corner_y, xL + w, *upper_left_corner_y + h);                                                                                                                                          \
    else if (corner == MTO_UPPER_RIGHT)                                                                                                                                                                                 \
      SetRect(&r, xR - w, *upper_right_corner_y, xR, *upper_right_corner_y + h);                                                                                                                                        \
    else if (corner == MTO_LOWER_LEFT)                                                                                                                                                                                  \
      SetRect(&r, xL, *lower_left_corner_y - h, xL + w, *lower_left_corner_y);                                                                                                                                          \
    else if (corner == MTO_LOWER_RIGHT)                                                                                                                                                                                 \
      SetRect(&r, xR - w, *lower_right_corner_y - h, xR, *lower_right_corner_y);                                                                                                                                        \
    m_text.DrawTextW(pFont, str, -1, &r, DT_NOPREFIX | ((corner == MTO_UPPER_RIGHT) ? 0 : DT_SINGLELINE) | DT_WORD_ELLIPSIS | ((corner == MTO_UPPER_RIGHT) ? DT_RIGHT : 0), 0xFFFFFFFF, bDarkBox, boxColor);            \
    if (corner == MTO_UPPER_LEFT)                                                                                                                                                                                       \
      *upper_left_corner_y += h;                                                                                                                                                                                        \
    else if (corner == MTO_UPPER_RIGHT)                                                                                                                                                                                 \
      *upper_right_corner_y += h;                                                                                                                                                                                       \
    else if (corner == MTO_LOWER_LEFT)                                                                                                                                                                                  \
      *lower_left_corner_y -= h;                                                                                                                                                                                        \
    else if (corner == MTO_LOWER_RIGHT)                                                                                                                                                                                 \
      *lower_right_corner_y -= h;                                                                                                                                                                                       \
  }

#define MyTextOut_Color(str, corner, color)                                                                                                                                                                          \
  {                                                                                                                                                                                                                  \
    SetRect(&r, 0, 0, xR - xL, 2048);                                                                                                                                                                                \
    m_text.DrawTextW(pFont, str, -1, &r, DT_NOPREFIX | ((corner == MTO_UPPER_RIGHT) ? 0 : DT_SINGLELINE) | DT_WORD_ELLIPSIS | DT_CALCRECT | ((corner == MTO_UPPER_RIGHT) ? DT_RIGHT : 0), color, false, 0xFF000000); \
    int w = r.right - r.left;                                                                                                                                                                                        \
    if (corner == MTO_UPPER_LEFT)                                                                                                                                                                                    \
      SetRect(&r, xL, *upper_left_corner_y, xL + w, *upper_left_corner_y + h);                                                                                                                                       \
    else if (corner == MTO_UPPER_RIGHT)                                                                                                                                                                              \
      SetRect(&r, xR - w, *upper_right_corner_y, xR, *upper_right_corner_y + h);                                                                                                                                     \
    else if (corner == MTO_LOWER_LEFT)                                                                                                                                                                               \
      SetRect(&r, xL, *lower_left_corner_y - h, xL + w, *lower_left_corner_y);                                                                                                                                       \
    else if (corner == MTO_LOWER_RIGHT)                                                                                                                                                                              \
      SetRect(&r, xR - w, *lower_right_corner_y - h, xR, *lower_right_corner_y);                                                                                                                                     \
    m_text.DrawTextW(pFont, str, -1, &r, DT_NOPREFIX | ((corner == MTO_UPPER_RIGHT) ? 0 : DT_SINGLELINE) | DT_WORD_ELLIPSIS | ((corner == MTO_UPPER_RIGHT) ? DT_RIGHT : 0), color, false, 0xFF000000);               \
    if (corner == MTO_UPPER_LEFT)                                                                                                                                                                                    \
      *upper_left_corner_y += h;                                                                                                                                                                                     \
    else if (corner == MTO_UPPER_RIGHT)                                                                                                                                                                              \
      *upper_right_corner_y += h;                                                                                                                                                                                    \
    else if (corner == MTO_LOWER_LEFT)                                                                                                                                                                               \
      *lower_left_corner_y -= h;                                                                                                                                                                                     \
    else if (corner == MTO_LOWER_RIGHT)                                                                                                                                                                              \
      *lower_right_corner_y -= h;                                                                                                                                                                                    \
  }

#define MyTextOut(str, corner, bDarkBox) MyTextOut_BGCOLOR(str, corner, bDarkBox, 0xFF000000)

#define MyTextOut_Color_Box(str, corner, color)                                                                                                                                                                      \
  {                                                                                                                                                                                                                  \
    SetRect(&r, 0, 0, xR - xL, 2048);                                                                                                                                                                                \
    m_text.DrawTextW(pFont, str, -1, &r, DT_NOPREFIX | ((corner == MTO_UPPER_RIGHT) ? 0 : DT_SINGLELINE) | DT_WORD_ELLIPSIS | DT_CALCRECT | ((corner == MTO_UPPER_RIGHT) ? DT_RIGHT : 0), color, false, 0xFF000000); \
    int w = r.right - r.left;                                                                                                                                                                                        \
    if (corner == MTO_UPPER_LEFT)                                                                                                                                                                                    \
      SetRect(&r, xL, *upper_left_corner_y, xL + w, *upper_left_corner_y + h);                                                                                                                                       \
    else if (corner == MTO_UPPER_RIGHT)                                                                                                                                                                              \
      SetRect(&r, xR - w, *upper_right_corner_y, xR, *upper_right_corner_y + h);                                                                                                                                     \
    else if (corner == MTO_LOWER_LEFT)                                                                                                                                                                               \
      SetRect(&r, xL, *lower_left_corner_y - h, xL + w, *lower_left_corner_y);                                                                                                                                       \
    else if (corner == MTO_LOWER_RIGHT)                                                                                                                                                                              \
      SetRect(&r, xR - w, *lower_right_corner_y - h, xR, *lower_right_corner_y);                                                                                                                                     \
    m_text.DrawTextW(pFont, str, -1, &r, DT_NOPREFIX | ((corner == MTO_UPPER_RIGHT) ? 0 : DT_SINGLELINE) | DT_WORD_ELLIPSIS | ((corner == MTO_UPPER_RIGHT) ? DT_RIGHT : 0), color, true, 0xFF000000);                \
    if (corner == MTO_UPPER_LEFT)                                                                                                                                                                                    \
      *upper_left_corner_y += h;                                                                                                                                                                                     \
    else if (corner == MTO_UPPER_RIGHT)                                                                                                                                                                              \
      *upper_right_corner_y += h;                                                                                                                                                                                    \
    else if (corner == MTO_LOWER_LEFT)                                                                                                                                                                               \
      *lower_left_corner_y -= h;                                                                                                                                                                                     \
    else if (corner == MTO_LOWER_RIGHT)                                                                                                                                                                              \
      *lower_right_corner_y -= h;                                                                                                                                                                                    \
  }

#define MyTextOut_Shadow(str, corner)                                                                                                            \
  {                                                                                                                                              \
    /* calc rect size */                                                                                                                         \
    SetRect(&r, 0, 0, xR - xL, 2048);                                                                                                            \
    m_text.DrawTextW(pFont, (wchar_t*)str, -1, &r, DT_NOPREFIX | DT_SINGLELINE | DT_WORD_ELLIPSIS | DT_CALCRECT, 0xFFFFFFFF, false, 0xFF000000); \
    int w = r.right - r.left;                                                                                                                    \
    /* first the shadow */                                                                                                                       \
    if (corner == MTO_UPPER_LEFT)                                                                                                                \
      SetRect(&r, xL, *upper_left_corner_y, xL + w, *upper_left_corner_y + h);                                                                   \
    else if (corner == MTO_UPPER_RIGHT)                                                                                                          \
      SetRect(&r, xR - w, *upper_right_corner_y, xR, *upper_right_corner_y + h);                                                                 \
    else if (corner == MTO_LOWER_LEFT)                                                                                                           \
      SetRect(&r, xL, *lower_left_corner_y - h, xL + w, *lower_left_corner_y);                                                                   \
    else if (corner == MTO_LOWER_RIGHT)                                                                                                          \
      SetRect(&r, xR - w, *lower_right_corner_y - h, xR, *lower_right_corner_y);                                                                 \
    r.top += 1;                                                                                                                                  \
    r.left += 1;                                                                                                                                 \
    m_text.DrawTextW(pFont, (wchar_t*)str, -1, &r, DT_NOPREFIX | DT_SINGLELINE | DT_WORD_ELLIPSIS, 0xFF000000, false, 0xFF000000);               \
    /* now draw real text */                                                                                                                     \
    r.top -= 1;                                                                                                                                  \
    r.left -= 1;                                                                                                                                 \
    m_text.DrawTextW(pFont, (wchar_t*)str, -1, &r, DT_NOPREFIX | DT_SINGLELINE | DT_WORD_ELLIPSIS, 0xFFFFFFFF, false, 0xFF000000);               \
    if (corner == MTO_UPPER_LEFT)                                                                                                                \
      *upper_left_corner_y += h;                                                                                                                 \
    else if (corner == MTO_UPPER_RIGHT)                                                                                                          \
      *upper_right_corner_y += h;                                                                                                                \
    else if (corner == MTO_LOWER_LEFT)                                                                                                           \
      *lower_left_corner_y -= h;                                                                                                                 \
    else if (corner == MTO_LOWER_RIGHT)                                                                                                          \
      *lower_right_corner_y -= h;                                                                                                                \
  }

#define MyTextOut_Shadow_Color(str, corner, color)                                                                                          \
  {                                                                                                                                         \
    /* calc rect size */                                                                                                                    \
    SetRect(&r, 0, 0, xR - xL, 2048);                                                                                                       \
    m_text.DrawTextW(pFont, (wchar_t*)str, -1, &r, DT_NOPREFIX | DT_SINGLELINE | DT_WORD_ELLIPSIS | DT_CALCRECT, color, false, 0xFF000000); \
    int w = r.right - r.left;                                                                                                               \
    /* first the shadow */                                                                                                                  \
    if (corner == MTO_UPPER_LEFT)                                                                                                           \
      SetRect(&r, xL, *upper_left_corner_y, xL + w, *upper_left_corner_y + h);                                                              \
    else if (corner == MTO_UPPER_RIGHT)                                                                                                     \
      SetRect(&r, xR - w, *upper_right_corner_y, xR, *upper_right_corner_y + h);                                                            \
    else if (corner == MTO_LOWER_LEFT)                                                                                                      \
      SetRect(&r, xL, *lower_left_corner_y - h, xL + w, *lower_left_corner_y);                                                              \
    else if (corner == MTO_LOWER_RIGHT)                                                                                                     \
      SetRect(&r, xR - w, *lower_right_corner_y - h, xR, *lower_right_corner_y);                                                            \
    r.top += 1;                                                                                                                             \
    r.left += 1;                                                                                                                            \
    m_text.DrawTextW(pFont, (wchar_t*)str, -1, &r, DT_NOPREFIX | DT_SINGLELINE | DT_WORD_ELLIPSIS, 0xFF000000, false, 0xFF000000);          \
    /* now draw real text */                                                                                                                \
    r.top -= 1;                                                                                                                             \
    r.left -= 1;                                                                                                                            \
    m_text.DrawTextW(pFont, (wchar_t*)str, -1, &r, DT_NOPREFIX | DT_SINGLELINE | DT_WORD_ELLIPSIS, color, false, 0xFF000000);               \
    if (corner == MTO_UPPER_LEFT)                                                                                                           \
      *upper_left_corner_y += h;                                                                                                            \
    else if (corner == MTO_UPPER_RIGHT)                                                                                                     \
      *upper_right_corner_y += h;                                                                                                           \
    else if (corner == MTO_LOWER_LEFT)                                                                                                      \
      *lower_left_corner_y -= h;                                                                                                            \
    else if (corner == MTO_LOWER_RIGHT)                                                                                                     \
      *lower_right_corner_y -= h;                                                                                                           \
  }

void CPlugin::RenderLyricsOverlay(bool burnIn) {
  if (((m_lyricsBurn > 0.0f) != burnIn) || !m_lyricsDisplayEnabled || !m_lyricsFontObject)
    return;

  const auto lyricState = ::milkwave.CurrentLyricsVisualState(m_lyricsOffsetMs, m_lyricsFade);
  std::wstring lyricText = lyricState.text;
  if (lyricText.empty())
    return;

  int canvasWidth = burnIn ? m_nTexSizeX : GetWidth();
  int canvasHeight = burnIn ? m_nTexSizeY : GetHeight();
  if (canvasWidth <= 0 || canvasHeight <= 0)
    return;

  float opacity = lyricState.opacity;
  if (opacity < 0.0f) opacity = 0.0f;
  if (opacity > 1.0f) opacity = 1.0f;

  // Lyrics move from their start position to the final position while fading.
  float positionX = m_lyricsPositionX * opacity + m_lyricsStartX * (1.0f - opacity);
  float positionY = m_lyricsPositionY * opacity + m_lyricsStartY * (1.0f - opacity);
  if (positionX < 0.0f) positionX = 0.0f;
  if (positionX > 1.0f) positionX = 1.0f;
  if (positionY < 0.0f) positionY = 0.0f;
  if (positionY > 1.0f) positionY = 1.0f;
  float maxWidthRatio = m_lyricsMaxWidth;
  if (maxWidthRatio < 0.05f) maxWidthRatio = 0.05f;
  if (maxWidthRatio > 1.0f) maxWidthRatio = 1.0f;

  // Zoom scales the font at fade extremes (zoom at opacity 0, full size at 1).
  float fontScale = m_lyricsZoom + (1.0f - m_lyricsZoom) * opacity;
  // Auto Scale: size the font so ~AutoScaleLineMaxChars characters fit within
  // the LyricsMaxWidth area, ignoring LyricsFontSize. The measurement is
  // normalized by the current font scale so the result is stable frame to frame.
  if (m_lyricsAutoScale) {
    const int maxChars = m_lyricsAutoScaleLineMaxChars > 0 ? m_lyricsAutoScaleLineMaxChars : 60;
    const int maxWidthPixels = (int)(canvasWidth * maxWidthRatio);
    if (maxWidthPixels > 0) {
      const std::wstring sample(static_cast<std::size_t>(maxChars), L'W');
      RECT sampleRect = {0, 0, maxWidthPixels, 1000};
      m_lyricsFontObject->DrawTextW(NULL, sample.c_str(), -1, &sampleRect,
                                    DT_CALCRECT | DT_SINGLELINE | DT_NOCLIP, 0xFFFFFFFF);
      const int sampleWidth = sampleRect.right - sampleRect.left;
      const float baseScale = m_lyricsCurrentFontScale >= 0.0f ? m_lyricsCurrentFontScale : 1.0f;
      if (sampleWidth > 0) {
        const float autoScaleBase = ((float)maxWidthPixels * baseScale) / (float)sampleWidth;
        if (autoScaleBase > 0.0f) fontScale *= autoScaleBase;
      }
    }
  }
  if (fabsf(fontScale - m_lyricsCurrentFontScale) > 0.01f)
    RecreateLyricsFont(fontScale);

  const int maxWidth = (int)(canvasWidth * maxWidthRatio);
  const int centerX = (int)(canvasWidth * positionX);
  const int centerY = (int)(canvasHeight * positionY);

  // Remember the previous line so it can be "burned" into the texture on fadeout.
  if (burnIn && m_lyricsBurn > 0.0f) {
    const double now = GetTime();
    if (lyricText != m_burnLyricsActiveText) {
      if (!m_burnLyricsActiveText.empty()) {
        m_burnLyricsPrevText = m_burnLyricsActiveText;
        m_burnLyricsPrevChangeTime = now;
      }
      m_burnLyricsActiveText = lyricText;
    }
  }

  // Wrap the lyric text manually, breaking only on blanks (spaces). GDI's
  // DT_WORDBREAK also breaks on punctuation (e.g. between "schedule" and ","),
  // which looks wrong for lyrics, so each line is measured here and drawn
  // separately with DT_SINGLELINE.
  auto drawWrapped = [&](const std::wstring& text, float drawOpacity, int cx, int cy) {
    RECT wrapRect = {cx - maxWidth / 2, 0, cx + maxWidth / 2, canvasHeight};
    if (wrapRect.left < 0) wrapRect.left = 0;
    if (wrapRect.right > canvasWidth) wrapRect.right = canvasWidth;
    const int wrapWidth = wrapRect.right - wrapRect.left;
    if (wrapWidth <= 0) return;

    std::vector<std::wstring> lines;
    {
      std::wstring current;
      std::size_t start = 0;
      while (start < text.size()) {
        const std::size_t wordStart = text.find_first_not_of(L' ', start);
        if (wordStart == std::wstring::npos) break;
        const std::size_t wordEnd = text.find(L' ', wordStart);
        const std::wstring word =
            text.substr(wordStart, wordEnd == std::wstring::npos ? std::wstring::npos : wordEnd - wordStart);
        const bool firstWord = current.empty();
        const std::wstring candidate = firstWord ? word : current + L' ' + word;
        RECT measure = {0, 0, wrapWidth, 1000};
        m_lyricsFontObject->DrawTextW(NULL, candidate.data(), -1, &measure,
                                      DT_CALCRECT | DT_SINGLELINE | DT_NOCLIP, 0xFFFFFFFF);
        if (!firstWord && measure.right - measure.left > wrapWidth) {
          lines.push_back(current);
          current = word;
        } else {
          current = candidate;
        }
        if (wordEnd == std::wstring::npos) break;
        start = wordEnd + 1;
      }
      if (!current.empty()) lines.push_back(current);
    }
    if (lines.empty()) return;

    int colorR = m_lyricsColorR < 0 ? 0 : m_lyricsColorR > 255 ? 255 : m_lyricsColorR;
    int colorG = m_lyricsColorG < 0 ? 0 : m_lyricsColorG > 255 ? 255 : m_lyricsColorG;
    int colorB = m_lyricsColorB < 0 ? 0 : m_lyricsColorB > 255 ? 255 : m_lyricsColorB;
    const DWORD alpha = static_cast<DWORD>(drawOpacity * 255.0f + 0.5f);
    DWORD textColor = (alpha << 24) | ((DWORD)colorR << 16) | ((DWORD)colorG << 8) | (DWORD)colorB;

    // Line height from a reference string; used to stack the wrapped lines.
    RECT lineRect = {0, 0, 1024, 1024};
    m_lyricsFontObject->DrawTextW(NULL, L"Ag", -1, &lineRect, DT_CALCRECT | DT_SINGLELINE, 0xFFFFFFFF);
    const int lineHeight = lineRect.bottom - lineRect.top;
    if (lineHeight <= 0) return;

    const int totalHeight = static_cast<int>(lines.size()) * lineHeight;
    int topY = cy - totalHeight / 2;
    if (topY < 0) topY = 0;
    if (topY + totalHeight > canvasHeight) topY = canvasHeight - totalHeight;
    if (topY < 0) topY = 0;

    int shadow = m_lyricsShadow;
    if (shadow < 0) shadow = 0;
    if (shadow > 16) shadow = 16;

    // DT_NOCLIP: D3DX9's GDI-based DrawTextW can clip a line when the rect is
    // exactly the measured height; without clipping each line always renders.
    const DWORD drawFlags = DT_CENTER | DT_SINGLELINE | DT_NOCLIP;
    for (std::size_t index = 0; index < lines.size(); ++index) {
      RECT lineTextRect = {wrapRect.left, topY + static_cast<int>(index) * lineHeight, wrapRect.right,
                           topY + static_cast<int>(index + 1) * lineHeight};
      if (shadow > 0) {
        RECT shadowRect = lineTextRect;
        OffsetRect(&shadowRect, shadow, shadow);
        if (burnIn) {
          m_lyricsFontObject->DrawTextW(NULL, lines[index].data(), -1, &shadowRect, drawFlags, alpha << 24);
        } else {
          m_text.DrawTextW(m_lyricsFontObject, lines[index].data(), -1, &shadowRect, drawFlags, alpha << 24, false);
        }
      }
      if (burnIn) {
        m_lyricsFontObject->DrawTextW(NULL, lines[index].data(), -1, &lineTextRect, drawFlags, textColor);
      } else {
        m_text.DrawTextW(m_lyricsFontObject, lines[index].data(), -1, &lineTextRect, drawFlags, textColor, false);
      }
    }
  };

  drawWrapped(lyricText, opacity, centerX, centerY);

  // Burn the previous line into the texture: it fades out over m_lyricsBurn
  // seconds after the current line takes over.
  if (burnIn && m_lyricsBurn > 0.0f && m_burnLyricsPrevChangeTime >= 0.0 && !m_burnLyricsPrevText.empty()) {
    const double elapsed = GetTime() - m_burnLyricsPrevChangeTime;
    if (elapsed < m_lyricsBurn) {
      const float burnOpacity = 1.0f - (float)(elapsed / m_lyricsBurn);
      drawWrapped(m_burnLyricsPrevText, burnOpacity, centerX, centerY);
    } else {
      m_burnLyricsPrevText.clear();
      m_burnLyricsPrevChangeTime = -1.0;
    }
  }
}

void CPlugin::MyRenderUI(
    int* upper_left_corner_y,   // increment me!
    int* upper_right_corner_y,  // increment me!
    int* lower_left_corner_y,   // decrement me!
    int* lower_right_corner_y,  // decrement me!
    int xL,
    int xR) {
  // draw text messages directly to the back buffer.
  // when you draw text into one of the four corners,
  //   draw the text at the current 'y' value for that corner
  //   (one of the first 4 params to this function),
  //   and then adjust that y value so that the next time
  //   text is drawn in that corner, it gets drawn above/below
  //   the previous text (instead of overtop of it).
  // when drawing into the upper or lower LEFT corners,
  //   left-align your text to 'xL'.
  // when drawing into the upper or lower RIGHT corners,
  //   right-align your text to 'xR'.

  // note: try to keep the bounding rectangles on the text small;
  //   the smaller the area that has to be locked (to draw the text),
  //   the faster it will be.  (on some cards, drawing text is
  //   ferociously slow, so even if it works okay on yours, it might
  //   not work on another video card.)
  // note: if you want some text to be on the screen often, and the text
  //   won't be changing every frame, please consider the poor folks
  //   whose video cards hate that; in that case you should probably
  //   draw the text just once, to a texture, and then display the
  //   texture each frame.  This is how the help screen is done; see
  //   pluginshell.cpp for example code.

  RECT r = {0};
  wchar_t buf[512] = {0};
  LPD3DXFONT pFont = GetFont(DECORATIVE_FONT);
  int h = GetFontHeight(DECORATIVE_FONT);

  if (!pFont)
    return;

  if (!GetFont(DECORATIVE_FONT))
    return;

  RenderLyricsOverlay(false);

  // 1. render text in upper-right corner - EXCEPT USER MESSAGE - it goes last b/c it draws a box under itself
  //                                        and it should be visible over everything else (usually an error msg)
  {
    // a) preset name
    if (m_bShowPresetInfo && !m_blackmode) {
      SelectFont(DECORATIVE_FONT);
      swprintf(
          buf,
          L"%s %s ",
          (m_bPresetLockedByUser || m_bPresetLockedByCode) && m_ShowLockSymbol ? L"\xD83D\xDD12" : L"",
          (m_nLoadingPreset != 0) ? m_pNewState->m_szDesc : m_pState->m_szDesc);

      DWORD alpha = 255;
      DWORD cr = m_fontinfo[DECORATIVE_FONT].R;
      DWORD cg = m_fontinfo[DECORATIVE_FONT].G;
      DWORD cb = m_fontinfo[DECORATIVE_FONT].B;
      DWORD color = (alpha << 24) | (cr << 16) | (cg << 8) | cb;
      MyTextOut_Color(buf, MTO_UPPER_RIGHT, color);
      // MyTextOut_Shadow(buf, MTO_UPPER_RIGHT, color);
    }

    // b) preset rating
    if (m_bShowRating || GetTime() < m_fShowRatingUntilThisTime) {
      // see also: SetCurrentPresetRating() in milkdrop.cpp
      SelectFont(SIMPLE_FONT);
      swprintf(buf, L" %s: %d ", wasabiApiLangString(IDS_RATING), (int)m_pState->m_fRating);
      if (!m_bEnableRating) lstrcatW(buf, wasabiApiLangString(IDS_DISABLED));
      MyTextOut_Shadow(buf, MTO_UPPER_RIGHT);
    }

    // c) fps display
    if (m_bShowFPS) {
      SelectFont(SIMPLE_FONT);
      swprintf(buf, L"%s: %4.2f ", wasabiApiLangString(IDS_FPS), GetFps());  // leave extra space @ end, so italicized fonts don't get clipped
      MyTextOut_Shadow(buf, MTO_UPPER_RIGHT);
    }

    // d) debug information
    if (m_bShowDebugInfo) {
      SelectFont(SIMPLE_FONT);
      DWORD color = GetFontColor(SIMPLE_FONT);

      if (::milkwave.currentArtist.length() || ::milkwave.currentTitle.length()) {
        std::wstring trackText = ::milkwave.currentArtist;
        if (!trackText.empty() && !::milkwave.currentTitle.empty()) trackText += L" - ";
        trackText += ::milkwave.currentTitle;
        MyTextOut_Shadow(trackText.c_str(), MTO_UPPER_RIGHT);

        if (::milkwave.hasTimeline) {
          auto totalSeconds = ::milkwave.currentPositionMs / 1000;
          auto minutes = totalSeconds / 60;
          auto seconds = totalSeconds % 60;
          swprintf_s(buf, L"~ %02lld:%02lld", minutes, seconds);
          MyTextOut_Shadow(buf, MTO_UPPER_RIGHT);
        }

        std::wstring lyricsText = ::milkwave.LyricsMonitorText(m_lyricsDisplayEnabled, m_lyricsOffsetMs);
        MyTextOut_Shadow(lyricsText.c_str(), MTO_UPPER_RIGHT);
      }

      swprintf(buf, L"  %6.2f %s", (float)(*m_pState->var_pf_monitor), wasabiApiLangString(IDS_PF_MONITOR));
      MyTextOut_Color(buf, MTO_UPPER_LEFT, color);

      if (!m_bPresetLockedByUser && !m_bPresetLockedByCode) {
        swprintf(buf, L"  %6.2f %s", (float)(GetTime() - m_fPresetStartTime), L"time");
        MyTextOut_Color(buf, MTO_UPPER_LEFT, color);
      }

      swprintf(buf, L"%s %6.2f %s", ((double)mysound.imm_rel[0] >= 1.3) ? L"+" : L" ", (float)(*m_pState->var_pf_bass), L"bass");
      MyTextOut_Color(buf, MTO_UPPER_LEFT, color);
      swprintf(buf, L"%s %6.2f %s", ((double)mysound.avg_rel[0] >= 1.3) ? L"+" : L" ", (float)(*m_pState->var_pf_bass_att), L"bass_att");
      MyTextOut_Color(buf, MTO_UPPER_LEFT, color);
      swprintf(buf, L"%s %6.2f %s", ((double)mysound.smooth_rel[0] >= 1.3) ? L"+" : L" ", (float)(*m_pState->var_pf_bass_smooth), L"bass_smooth");
      MyTextOut_Color(buf, MTO_UPPER_LEFT, color);

      swprintf(buf, L"%s %6.2f %s", ((double)mysound.imm_rel[1] >= 1.3) ? L"+" : L" ", (float)(*m_pState->var_pf_mid), L"mid");
      MyTextOut_Color(buf, MTO_UPPER_LEFT, color);
      swprintf(buf, L"%s %6.2f %s", ((double)mysound.avg_rel[1] >= 1.3) ? L"+" : L" ", (float)(*m_pState->var_pf_mid_att), L"mid_att");
      MyTextOut_Color(buf, MTO_UPPER_LEFT, color);
      swprintf(buf, L"%s %6.2f %s", ((double)mysound.smooth_rel[1] >= 1.3) ? L"+" : L" ", (float)(*m_pState->var_pf_mid_smooth), L"mid_smooth");
      MyTextOut_Color(buf, MTO_UPPER_LEFT, color);

      swprintf(buf, L"%s %6.2f %s", ((double)mysound.imm_rel[2] >= 1.3) ? L"+" : L" ", (float)(*m_pState->var_pf_treb), L"treb");
      MyTextOut_Color(buf, MTO_UPPER_LEFT, color);
      swprintf(buf, L"%s %6.2f %s", ((double)mysound.avg_rel[2] >= 1.3) ? L"+" : L" ", (float)(*m_pState->var_pf_treb_att), L"treb_att");
      MyTextOut_Color(buf, MTO_UPPER_LEFT, color);
      swprintf(buf, L"%s %6.2f %s", ((double)mysound.smooth_rel[2] >= 1.3) ? L"+" : L" ", (float)(*m_pState->var_pf_treb_smooth), L"treb_smooth");
      MyTextOut_Color(buf, MTO_UPPER_LEFT, color);

      swprintf(buf, L"q=%.2f hue=%.2f sat=%.2f bri=%.2f", m_fRenderQuality, m_ColShiftHue, m_ColShiftSaturation, m_ColShiftBrightness);
      MyTextOut_Color(buf, MTO_LOWER_RIGHT, color);

      if (m_bEnableMouseInteraction) {
        swprintf(buf, L"%s x=%0.2f y=%0.2f z=%s ", L"mouse", m_mouseX, m_mouseY, m_mouseDown ? L"1" : L"0");
        MyTextOut_Color(buf, MTO_LOWER_RIGHT, color);
      }
    }
    // NOTE: custom timed msg comes at the end!!
  }

  // 2. render text in lower-right corner
  {
    // waitstring tooltip:
    if (m_waitstring.bActive && m_bShowMenuToolTips && m_waitstring.szToolTip[0]) {
      DrawTooltip(m_waitstring.szToolTip, xR, *lower_right_corner_y);
    }
  }

  // 3. render text in lower-left corner
  {
    wchar_t buf2[512] = {0};
    wchar_t buf3[512 + 1] = {0};  // add two extra spaces to end, so italicized fonts don't get clipped

    // render song title in lower-left corner:
    if (m_bShowSongTitle) {
      wchar_t buf4[512] = {0};
      SelectFont(DECORATIVE_FONT);
      GetSongTitle(buf4, sizeof(buf4));  // defined in utility.h/cpp

      MyTextOut_Shadow(buf4, MTO_LOWER_LEFT);
    }

    // render song time & len above that:
    if (m_bShowSongTime || m_bShowSongLen) {
      /*if (playbackService) {
          FormatSongTime(playbackService->GetPosition(), buf); // defined in utility.h/cpp
          FormatSongTime(playbackService->GetDuration(), buf2); // defined in utility.h/cpp
          if (m_bShowSongTime && m_bShowSongLen)
          {
              // only show playing position and track length if it is playing (buffer is valid)
              if (buf[0])
                  swprintf(buf3, L"%s / %s ", buf, buf2);
              else
                  lstrcpynW(buf3, buf2, 512);
          }
          else if (m_bShowSongTime)
              lstrcpynW(buf3, buf, 512);
          else
              lstrcpynW(buf3, buf2, 512);

          SelectFont(DECORATIVE_FONT);
          MyTextOut_Shadow(buf3, MTO_LOWER_LEFT);
      }*/
    }
  }

  // 4. render text in upper-left corner
  {
    wchar_t buf[64000] = {0};  // must fit the longest strings (code strings are 32768 chars)
    // AND leave extra space for &->&&, and [,[,& insertion
    char bufA[64000] = {0};

    SelectFont(SIMPLE_FONT);

    // stuff for loading presets, menus, etc:

    if (m_waitstring.bActive) {
      // 1. draw the prompt string
      MyTextOut(m_waitstring.szPrompt, MTO_UPPER_LEFT, true);

      // extra instructions:
      bool bIsWarp = m_waitstring.bDisplayAsCode && (m_pCurMenu == &m_menuPreset) && !wcscmp(m_menuPreset.GetCurItem()->m_szName, L"[ edit warp shader ]");
      bool bIsComp = m_waitstring.bDisplayAsCode && (m_pCurMenu == &m_menuPreset) && !wcscmp(m_menuPreset.GetCurItem()->m_szName, L"[ edit composite shader ]");
      if (bIsWarp || bIsComp) {
        if (m_bShowShaderHelp) {
          MyTextOut(wasabiApiLangString(IDS_PRESS_F9_TO_HIDE_SHADER_QREF), MTO_UPPER_LEFT, true);
        } else {
          MyTextOut(wasabiApiLangString(IDS_PRESS_F9_TO_SHOW_SHADER_QREF), MTO_UPPER_LEFT, true);
        }
        *upper_left_corner_y += h * 2 / 3;

        if (m_bShowShaderHelp) {
          // draw dark box - based on longest line & # lines...
          SetRect(&r, 0, 0, 2048, 2048);
          m_text.DrawTextW(pFont, wasabiApiLangString(IDS_STRING615), -1, &r, DT_NOPREFIX | DT_SINGLELINE | DT_WORD_ELLIPSIS | DT_CALCRECT, 0xFFFFFFFF, false, 0xFF000000);
          RECT darkbox;
          SetRect(&darkbox, xL, *upper_left_corner_y - 2, xL + r.right - r.left, *upper_left_corner_y + (r.bottom - r.top) * 13 + 2);
          DrawDarkTranslucentBox(&darkbox);

          MyTextOut(wasabiApiLangString(IDS_STRING616), MTO_UPPER_LEFT, false);
          MyTextOut(wasabiApiLangString(IDS_STRING617), MTO_UPPER_LEFT, false);
          MyTextOut(wasabiApiLangString(IDS_STRING618), MTO_UPPER_LEFT, false);
          MyTextOut(wasabiApiLangString(IDS_STRING619), MTO_UPPER_LEFT, false);
          MyTextOut(wasabiApiLangString(IDS_STRING620), MTO_UPPER_LEFT, false);
          MyTextOut(wasabiApiLangString(IDS_STRING621), MTO_UPPER_LEFT, false);
          if (bIsWarp) {
            MyTextOut(wasabiApiLangString(IDS_STRING622), MTO_UPPER_LEFT, false);
            MyTextOut(wasabiApiLangString(IDS_STRING623), MTO_UPPER_LEFT, false);
            MyTextOut(wasabiApiLangString(IDS_STRING624), MTO_UPPER_LEFT, false);
            MyTextOut(wasabiApiLangString(IDS_STRING625), MTO_UPPER_LEFT, false);
            MyTextOut(wasabiApiLangString(IDS_STRING626), MTO_UPPER_LEFT, false);
            MyTextOut(wasabiApiLangString(IDS_STRING627), MTO_UPPER_LEFT, false);
            MyTextOut(wasabiApiLangString(IDS_STRING628), MTO_UPPER_LEFT, false);
          } else if (bIsComp) {
            MyTextOut(wasabiApiLangString(IDS_STRING629), MTO_UPPER_LEFT, false);
            MyTextOut(wasabiApiLangString(IDS_STRING630), MTO_UPPER_LEFT, false);
            MyTextOut(wasabiApiLangString(IDS_STRING631), MTO_UPPER_LEFT, false);
            MyTextOut(wasabiApiLangString(IDS_STRING632), MTO_UPPER_LEFT, false);
            MyTextOut(wasabiApiLangString(IDS_STRING633), MTO_UPPER_LEFT, false);
            MyTextOut(wasabiApiLangString(IDS_STRING634), MTO_UPPER_LEFT, false);
            MyTextOut(wasabiApiLangString(IDS_STRING635), MTO_UPPER_LEFT, false);
          }
          *upper_left_corner_y += h * 2 / 3;
        }
      } else if (m_UI_mode == UI_SAVEAS && (m_bWarpShaderLock || m_bCompShaderLock)) {
        wchar_t buf[256] = {0};
        int shader_msg_id = IDS_COMPOSITE_SHADER_LOCKED;
        if (m_bWarpShaderLock && m_bCompShaderLock)
          shader_msg_id = IDS_WARP_AND_COMPOSITE_SHADERS_LOCKED;
        else if (m_bWarpShaderLock && !m_bCompShaderLock)
          shader_msg_id = IDS_WARP_SHADER_LOCKED;
        else
          shader_msg_id = IDS_COMPOSITE_SHADER_LOCKED;

        wasabiApiLangString(shader_msg_id, buf, 256);
        MyTextOut_BGCOLOR(buf, MTO_UPPER_LEFT, true, 0xFF000000);
        *upper_left_corner_y += h * 2 / 3;
      } else
        *upper_left_corner_y += h * 2 / 3;

      // 2. reformat the waitstring text for display
      int bBrackets = m_waitstring.nSelAnchorPos != -1 && m_waitstring.nSelAnchorPos != m_waitstring.nCursorPos;
      int bCursorBlink = (!bBrackets &&
                          ((int)(GetTime() * 270.0f) % 100 > 50)
                          //((GetFrame() % 3) >= 2)
      );

      lstrcpyW(buf, m_waitstring.szText);
      lstrcpyA(bufA, (char*)m_waitstring.szText);

      int temp_cursor_pos = m_waitstring.nCursorPos;
      int temp_anchor_pos = m_waitstring.nSelAnchorPos;

      if (bBrackets) {
        if (m_waitstring.bDisplayAsCode) {
          // insert [] around the selection
          int start = (temp_cursor_pos < temp_anchor_pos) ? temp_cursor_pos : temp_anchor_pos;
          int end = (temp_cursor_pos > temp_anchor_pos) ? temp_cursor_pos - 1 : temp_anchor_pos - 1;
          int len = lstrlenA(bufA);
          int i;

          for (i = len; i > end; i--)
            bufA[i + 1] = bufA[i];
          bufA[end + 1] = ']';
          len++;

          for (i = len; i >= start; i--)
            bufA[i + 1] = bufA[i];
          bufA[start] = '[';
          len++;
        } else {
          // insert [] around the selection
          int start = (temp_cursor_pos < temp_anchor_pos) ? temp_cursor_pos : temp_anchor_pos;
          int end = (temp_cursor_pos > temp_anchor_pos) ? temp_cursor_pos - 1 : temp_anchor_pos - 1;
          int len = lstrlenW(buf);
          int i;

          for (i = len; i > end; i--)
            buf[i + 1] = buf[i];
          buf[end + 1] = L']';
          len++;

          for (i = len; i >= start; i--)
            buf[i + 1] = buf[i];
          buf[start] = L'[';
          len++;
        }
      } else {
        // underline the current cursor position by rapidly toggling the character with an underscore
        if (m_waitstring.bDisplayAsCode) {
          if (bCursorBlink) {
            if (bufA[temp_cursor_pos] == 0) {
              bufA[temp_cursor_pos] = '_';
              bufA[temp_cursor_pos + 1] = 0;
            } else if (bufA[temp_cursor_pos] == LINEFEED_CONTROL_CHAR) {
              for (int i = strlen(bufA); i >= temp_cursor_pos; i--)
                bufA[i + 1] = bufA[i];
              bufA[temp_cursor_pos] = '_';
            } else if (bufA[temp_cursor_pos] == '_')
              bufA[temp_cursor_pos] = ' ';
            else  // it's a space or symbol or alphanumeric.
              bufA[temp_cursor_pos] = '_';
          } else {
            if (bufA[temp_cursor_pos] == 0) {
              bufA[temp_cursor_pos] = ' ';
              bufA[temp_cursor_pos + 1] = 0;
            } else if (bufA[temp_cursor_pos] == LINEFEED_CONTROL_CHAR) {
              for (int i = strlen(bufA); i >= temp_cursor_pos; i--)
                bufA[i + 1] = bufA[i];
              bufA[temp_cursor_pos] = ' ';
            }
            // else if (buf[temp_cursor_pos] == '_')
            //  do nothing
            // else // it's a space or symbol or alphanumeric.
            //  do nothing
          }
        } else {
          if (bCursorBlink) {
            if (buf[temp_cursor_pos] == 0) {
              buf[temp_cursor_pos] = L'_';
              buf[temp_cursor_pos + 1] = 0;
            } else if (buf[temp_cursor_pos] == LINEFEED_CONTROL_CHAR) {
              for (int i = wcslen(buf); i >= temp_cursor_pos; i--)
                buf[i + 1] = buf[i];
              buf[temp_cursor_pos] = L'_';
            } else if (buf[temp_cursor_pos] == L'_')
              buf[temp_cursor_pos] = L' ';
            else  // it's a space or symbol or alphanumeric.
              buf[temp_cursor_pos] = L'_';
          } else {
            if (buf[temp_cursor_pos] == 0) {
              buf[temp_cursor_pos] = L' ';
              buf[temp_cursor_pos + 1] = 0;
            } else if (buf[temp_cursor_pos] == LINEFEED_CONTROL_CHAR) {
              for (int i = wcslen(buf); i >= temp_cursor_pos; i--)
                buf[i + 1] = buf[i];
              buf[temp_cursor_pos] = L' ';
            }
            // else if (buf[temp_cursor_pos] == '_')
            //  do nothing
            // else // it's a space or symbol or alphanumeric.
            //  do nothing
          }
        }
      }

      RECT rect = {0};
      SetRect(&rect, xL, *upper_left_corner_y, xR, *lower_left_corner_y);
      rect.top += PLAYLIST_INNER_MARGIN;
      rect.left += PLAYLIST_INNER_MARGIN;
      rect.right -= PLAYLIST_INNER_MARGIN;
      rect.bottom -= PLAYLIST_INNER_MARGIN;

      // then draw the edit string
      if (m_waitstring.bDisplayAsCode) {
        char buf2[8192] = {0};
        int top_of_page_pos = 0;

        // compute top_of_page_pos so that the line the cursor is on will show.
        // also compute dims of the black rectangle while we're at it.
        {
          int start = 0;
          int pos = 0;
          int ypixels = 0;
          int page = 1;
          int exit_on_next_page = 0;

          RECT box = rect;
          box.right = box.left;
          box.bottom = box.top;

          while (bufA[pos] != 0)  // for each line of text... (note that it might wrap)
          {
            start = pos;
            while (bufA[pos] != LINEFEED_CONTROL_CHAR && bufA[pos] != 0)
              pos++;

            char ch = bufA[pos];
            bufA[pos] = 0;
            sprintf(buf2, "   %sX", &bufA[start]);  // put a final 'X' instead of ' ' b/c CALCRECT returns w==0 if string is entirely whitespace!
            RECT r2 = rect;
            r2.bottom = 4096;
            m_text.DrawTextA(GetFont(SIMPLE_FONT), buf2, -1, &r2, DT_CALCRECT /*| DT_WORDBREAK*/, 0xFFFFFFFF, false);
            int h = r2.bottom - r2.top;
            ypixels += h;
            bufA[pos] = ch;

            if (start > m_waitstring.nCursorPos)  // make sure 'box' gets updated for each line on this page
              exit_on_next_page = 1;

            if (ypixels > rect.bottom - rect.top)  // this line belongs on the next page
            {
              if (exit_on_next_page) {
                bufA[start] = 0;  // so text stops where the box stops, when we draw the text
                break;
              }

              ypixels = h;
              top_of_page_pos = start;
              page++;

              box = rect;
              box.right = box.left;
              box.bottom = box.top;
            }
            box.bottom += h;
            box.right = max(box.right, box.left + r2.right - r2.left);

            if (bufA[pos] == 0)
              break;
            pos++;
          }

          // use r2 to draw a dark box:
          box.top -= PLAYLIST_INNER_MARGIN;
          box.left -= PLAYLIST_INNER_MARGIN;
          box.right += PLAYLIST_INNER_MARGIN;
          box.bottom += PLAYLIST_INNER_MARGIN;
          DrawDarkTranslucentBox(&box);
          *upper_left_corner_y += box.bottom - box.top + PLAYLIST_INNER_MARGIN * 3;
          swprintf(m_waitstring.szToolTip, wasabiApiLangString(IDS_PAGE_X), page);
        }

        // display multiline (replace all character 13's with a CR)
        {
          int start = top_of_page_pos;
          int pos = top_of_page_pos;

          while (bufA[pos] != 0) {
            while (bufA[pos] != LINEFEED_CONTROL_CHAR && bufA[pos] != 0)
              pos++;

            char ch = bufA[pos];
            bufA[pos] = 0;
            sprintf(buf2, "   %s ", &bufA[start]);
            DWORD color = MENU_COLOR;
            if (m_waitstring.nCursorPos >= start && m_waitstring.nCursorPos <= pos)
              color = MENU_HILITE_COLOR;
            rect.top += m_text.DrawTextA(GetFont(SIMPLE_FONT), buf2, -1, &rect, 0 /*DT_WORDBREAK*/, color, false);
            bufA[pos] = ch;

            if (rect.top > rect.bottom)
              break;

            if (bufA[pos] != 0) pos++;
            start = pos;
          }
        }
        // note: *upper_left_corner_y is updated above, when the dark box is drawn.
      } else {
        wchar_t buf2[8192] = {0};

        // display on one line
        RECT box = rect;
        box.bottom = 4096;
        swprintf(buf2, L"    %sX", buf);  // put a final 'X' instead of ' ' b/c CALCRECT returns w==0 if string is entirely whitespace!
        m_text.DrawTextW(GetFont(SIMPLE_FONT), buf2, -1, &box, DT_CALCRECT, MENU_COLOR, false);

        // use r2 to draw a dark box:
        box.top -= PLAYLIST_INNER_MARGIN;
        box.left -= PLAYLIST_INNER_MARGIN;
        box.right += PLAYLIST_INNER_MARGIN;
        box.bottom += PLAYLIST_INNER_MARGIN;
        DrawDarkTranslucentBox(&box);
        *upper_left_corner_y += box.bottom - box.top + PLAYLIST_INNER_MARGIN * 3;

        swprintf(buf2, L"    %s ", buf);
        m_text.DrawTextW(GetFont(SIMPLE_FONT), buf2, -1, &rect, 0, MENU_COLOR, false);
      }
    } else if (m_UI_mode == UI_MENU) {
      assert(m_pCurMenu);
      SetRect(&r, xL, *upper_left_corner_y, xR, *lower_left_corner_y);

      RECT darkbox = {0};
      m_pCurMenu->DrawMenu(r, xR, *lower_right_corner_y, 1, &darkbox);
      *upper_left_corner_y += darkbox.bottom - darkbox.top + PLAYLIST_INNER_MARGIN * 3;

      darkbox.right += PLAYLIST_INNER_MARGIN * 2;
      darkbox.bottom += PLAYLIST_INNER_MARGIN * 2;
      DrawDarkTranslucentBox(&darkbox);

      r.top += PLAYLIST_INNER_MARGIN;
      r.left += PLAYLIST_INNER_MARGIN;
      r.right += PLAYLIST_INNER_MARGIN;
      r.bottom += PLAYLIST_INNER_MARGIN;
      m_pCurMenu->DrawMenu(r, xR, *lower_right_corner_y);
    } else if (m_UI_mode == UI_UPGRADE_PIXEL_SHADER) {
      RECT rect = {0};
      SetRect(&rect, xL, *upper_left_corner_y, xR, *lower_left_corner_y);

      if (m_pState->m_nWarpPSVersion >= m_nMaxPSVersion &&
          m_pState->m_nCompPSVersion >= m_nMaxPSVersion) {
        assert(m_pState->m_nMaxPSVersion == m_nMaxPSVersion);
        wchar_t buf[1024] = {0};
        swprintf(buf, wasabiApiLangString(IDS_PRESET_USES_HIGHEST_PIXEL_SHADER_VERSION), m_nMaxPSVersion);
        rect.top += m_text.DrawTextW(GetFont(SIMPLE_FONT), buf, -1, &rect, DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX, MENU_COLOR, true);
        rect.top += m_text.DrawTextW(GetFont(SIMPLE_FONT), wasabiApiLangString(IDS_PRESS_ESC_TO_RETURN), -1, &rect, DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX, MENU_COLOR, true);
      } else {
        if (m_pState->m_nMinPSVersion != m_pState->m_nMaxPSVersion) {
          switch (m_pState->m_nMinPSVersion) {
            case MD2_PS_NONE:
              rect.top += m_text.DrawTextW(GetFont(SIMPLE_FONT), wasabiApiLangString(IDS_PRESET_HAS_MIXED_VERSIONS_OF_SHADERS), -1, &rect, DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX, MENU_COLOR, true);
              rect.top += m_text.DrawTextW(GetFont(SIMPLE_FONT), wasabiApiLangString(IDS_UPGRADE_SHADERS_TO_USE_PS2), -1, &rect, DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX, MENU_COLOR, true);
              break;
            case MD2_PS_2_0:
              rect.top += m_text.DrawTextW(GetFont(SIMPLE_FONT), wasabiApiLangString(IDS_PRESET_HAS_MIXED_VERSIONS_OF_SHADERS), -1, &rect, DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX, MENU_COLOR, true);
              rect.top += m_text.DrawTextW(GetFont(SIMPLE_FONT), wasabiApiLangString(IDS_UPGRADE_SHADERS_TO_USE_PS2X), -1, &rect, DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX, MENU_COLOR, true);
              break;
            case MD2_PS_2_X:
              rect.top += m_text.DrawTextW(GetFont(SIMPLE_FONT), wasabiApiLangString(IDS_PRESET_HAS_MIXED_VERSIONS_OF_SHADERS), -1, &rect, DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX, MENU_COLOR, true);
              rect.top += m_text.DrawTextW(GetFont(SIMPLE_FONT), wasabiApiLangString(IDS_UPGRADE_SHADERS_TO_USE_PS3), -1, &rect, DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX, MENU_COLOR, true);
              break;
            case MD2_PS_3_0:
              assert(false);
              break;
            default:
              assert(0);
              break;
          }
        } else {
          switch (m_pState->m_nMinPSVersion) {
            case MD2_PS_NONE:
              rect.top += m_text.DrawTextW(GetFont(SIMPLE_FONT), wasabiApiLangString(IDS_PRESET_DOES_NOT_USE_PIXEL_SHADERS), -1, &rect, DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX, MENU_COLOR, true);
              rect.top += m_text.DrawTextW(GetFont(SIMPLE_FONT), wasabiApiLangString(IDS_UPGRADE_TO_USE_PS2), -1, &rect, DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX, MENU_COLOR, true);
              rect.top += m_text.DrawTextW(GetFont(SIMPLE_FONT), wasabiApiLangString(IDS_WARNING_OLD_GPU_MIGHT_NOT_WORK_WITH_PRESET), -1, &rect, DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX, MENU_COLOR, true);
              break;
            case MD2_PS_2_0:
              rect.top += m_text.DrawTextW(GetFont(SIMPLE_FONT), wasabiApiLangString(IDS_PRESET_CURRENTLY_USES_PS2), -1, &rect, DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX, MENU_COLOR, true);
              rect.top += m_text.DrawTextW(GetFont(SIMPLE_FONT), wasabiApiLangString(IDS_UPGRADE_TO_USE_PS2X), -1, &rect, DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX, MENU_COLOR, true);
              rect.top += m_text.DrawTextW(GetFont(SIMPLE_FONT), wasabiApiLangString(IDS_WARNING_OLD_GPU_MIGHT_NOT_WORK_WITH_PRESET), -1, &rect, DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX, MENU_COLOR, true);
              break;
            case MD2_PS_2_X:
              rect.top += m_text.DrawTextW(GetFont(SIMPLE_FONT), wasabiApiLangString(IDS_PRESET_CURRENTLY_USES_PS2X), -1, &rect, DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX, MENU_COLOR, true);
              rect.top += m_text.DrawTextW(GetFont(SIMPLE_FONT), wasabiApiLangString(IDS_UPGRADE_TO_USE_PS3), -1, &rect, DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX, MENU_COLOR, true);
              rect.top += m_text.DrawTextW(GetFont(SIMPLE_FONT), wasabiApiLangString(IDS_WARNING_OLD_GPU_MIGHT_NOT_WORK_WITH_PRESET), -1, &rect, DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX, MENU_COLOR, true);
              break;
            case MD2_PS_3_0:
              rect.top += m_text.DrawTextW(GetFont(SIMPLE_FONT), wasabiApiLangString(IDS_PRESET_CURRENTLY_USES_PS3), -1, &rect, DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX, MENU_COLOR, true);
              rect.top += m_text.DrawTextW(GetFont(SIMPLE_FONT), wasabiApiLangString(IDS_UPGRADE_TO_USE_PS4), -1, &rect, DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX, MENU_COLOR, true);
              rect.top += m_text.DrawTextW(GetFont(SIMPLE_FONT), wasabiApiLangString(IDS_WARNING_OLD_GPU_MIGHT_NOT_WORK_WITH_PRESET), -1, &rect, DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX, MENU_COLOR, true);
              break;
            default:
              assert(0);
              break;
          }
        }
      }
      *upper_left_corner_y = rect.top;
    } else if (m_UI_mode == UI_LOAD_DEL) {
      RECT rect;
      SetRect(&rect, xL, *upper_left_corner_y, xR, *lower_left_corner_y);
      rect.top += m_text.DrawTextW(GetFont(SIMPLE_FONT), wasabiApiLangString(IDS_ARE_YOU_SURE_YOU_WANT_TO_DELETE_PRESET), -1, &rect, DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX, MENU_COLOR, true);
      swprintf(buf, wasabiApiLangString(IDS_PRESET_TO_DELETE), m_presets[m_nPresetListCurPos].szFilename.c_str());
      rect.top += m_text.DrawTextW(GetFont(SIMPLE_FONT), buf, -1, &rect, DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX, MENU_COLOR, true);
      *upper_left_corner_y = rect.top;
    } else if (m_UI_mode == UI_SAVE_OVERWRITE) {
      RECT rect;
      SetRect(&rect, xL, *upper_left_corner_y, xR, *lower_left_corner_y);
      rect.top += m_text.DrawTextW(GetFont(SIMPLE_FONT), wasabiApiLangString(IDS_FILE_ALREADY_EXISTS_OVERWRITE_IT), -1, &rect, DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX, MENU_COLOR, true);
      swprintf(buf, wasabiApiLangString(IDS_FILE_IN_QUESTION_X_MILK), m_waitstring.szText);
      rect.top += m_text.DrawTextW(GetFont(SIMPLE_FONT), buf, -1, &rect, DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX, MENU_COLOR, true);
      if (m_bWarpShaderLock)
        rect.top += m_text.DrawTextW(GetFont(SIMPLE_FONT), wasabiApiLangString(IDS_WARNING_DO_NOT_FORGET_WARP_SHADER_WAS_LOCKED), -1, &rect, DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX, 0xFFFFFFFF, true, 0xFFCC0000);
      if (m_bCompShaderLock)
        rect.top += m_text.DrawTextW(GetFont(SIMPLE_FONT), wasabiApiLangString(IDS_WARNING_DO_NOT_FORGET_COMPOSITE_SHADER_WAS_LOCKED), -1, &rect, DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX, 0xFFFFFFFF, true, 0xFFCC0000);
      *upper_left_corner_y = rect.top;
    } else if (m_UI_mode == UI_MASHUP) {
      if (m_nPresets - m_nDirs == 0) {
        // note: this error message is repeated in milkdrop.cpp in LoadRandomPreset()
        wchar_t buf[1024];
        swprintf(buf, wasabiApiLangString(IDS_ERROR_NO_PRESET_FILE_FOUND_IN_X_MILK), m_szPresetDir);
        AddError(buf, 6.0f, ERR_MISC, true);
        m_UI_mode = UI_REGULAR;
      } else {
        UpdatePresetList(true);  // make sure list is completely ready

        // quick checks
        int mash;
        for (mash = 0; mash < MASH_SLOTS; mash++) {
          // check validity
          if (m_nMashPreset[mash] < m_nDirs)
            m_nMashPreset[mash] = m_nDirs;
          if (m_nMashPreset[mash] >= m_nPresets)
            m_nMashPreset[mash] = m_nPresets - 1;

          // apply changes, if it's time
          if (m_nLastMashChangeFrame[mash] + MASH_APPLY_DELAY_FRAMES + 1 == GetFrame()) {
            // import just a fragment of a preset!!
            DWORD ApplyFlags = 0;
            switch (mash) {
              case 0:
                ApplyFlags = STATE_GENERAL;
                break;
              case 1:
                ApplyFlags = STATE_MOTION;
                break;
              case 2:
                ApplyFlags = STATE_WAVE;
                break;
              case 3:
                ApplyFlags = STATE_WARP;
                break;
              case 4:
                ApplyFlags = STATE_COMP;
                break;
            }

            wchar_t szFile[MAX_PATH];
            swprintf(szFile, L"%s%s", m_szPresetDir, m_presets[m_nMashPreset[mash]].szFilename.c_str());

            m_pState->Import(szFile, GetTime(), m_pState, ApplyFlags);

            if (ApplyFlags & STATE_WARP)
              SafeRelease(m_shaders.warp.ptr);
            if (ApplyFlags & STATE_COMP)
              SafeRelease(m_shaders.comp.ptr);
            LoadShaders(&m_shaders, m_pState, false, false);

            SetMenusForPresetVersion(m_pState->m_nWarpPSVersion, m_pState->m_nCompPSVersion);
          }
        }

        MyTextOut(wasabiApiLangString(IDS_PRESET_MASH_UP_TEXT1), MTO_UPPER_LEFT, true);
        MyTextOut(wasabiApiLangString(IDS_PRESET_MASH_UP_TEXT2), MTO_UPPER_LEFT, true);
        MyTextOut(wasabiApiLangString(IDS_PRESET_MASH_UP_TEXT3), MTO_UPPER_LEFT, true);
        MyTextOut(wasabiApiLangString(IDS_PRESET_MASH_UP_TEXT4), MTO_UPPER_LEFT, true);
        *upper_left_corner_y += PLAYLIST_INNER_MARGIN;

        RECT rect;
        SetRect(&rect, xL, *upper_left_corner_y, xR, *lower_left_corner_y);
        rect.top += PLAYLIST_INNER_MARGIN;
        rect.left += PLAYLIST_INNER_MARGIN;
        rect.right -= PLAYLIST_INNER_MARGIN;
        rect.bottom -= PLAYLIST_INNER_MARGIN;

        int lines_available = (rect.bottom - rect.top - PLAYLIST_INNER_MARGIN * 2) / GetFontHeight(SIMPLE_FONT);
        lines_available -= MASH_SLOTS;

        if (lines_available < 10) {
          // force it
          rect.bottom = rect.top + GetFontHeight(SIMPLE_FONT) * 10 + 1;
          lines_available = 10;
        }
        if (lines_available > 16)
          lines_available = 16;

        if (m_bUserPagedDown) {
          m_nMashPreset[m_nMashSlot] += lines_available;
          if (m_nMashPreset[m_nMashSlot] >= m_nPresets)
            m_nMashPreset[m_nMashSlot] = m_nPresets - 1;
          m_bUserPagedDown = false;
        }
        if (m_bUserPagedUp) {
          m_nMashPreset[m_nMashSlot] -= lines_available;
          if (m_nMashPreset[m_nMashSlot] < m_nDirs)
            m_nMashPreset[m_nMashSlot] = m_nDirs;
          m_bUserPagedUp = false;
        }

        int i;
        int first_line = m_nMashPreset[m_nMashSlot] - (m_nMashPreset[m_nMashSlot] % lines_available);
        int last_line = first_line + lines_available;
        wchar_t str[512], str2[512];

        if (last_line > m_nPresets)
          last_line = m_nPresets;

        // tooltip:
        if (m_bShowMenuToolTips) {
          wchar_t buf[256];
          swprintf(buf, wasabiApiLangString(IDS_PAGE_X_OF_X), m_nMashPreset[m_nMashSlot] / lines_available + 1, (m_nPresets + lines_available - 1) / lines_available);
          DrawTooltip(buf, xR, *lower_right_corner_y);
        }

        RECT orig_rect = rect;

        RECT box;
        box.top = rect.top;
        box.left = rect.left;
        box.right = rect.left;
        box.bottom = rect.top;

        int mashNames[MASH_SLOTS] = {
            IDS_MASHUP_GENERAL_POSTPROC,
            IDS_MASHUP_MOTION_EQUATIONS,
            IDS_MASHUP_WAVEFORMS_SHAPES,
            IDS_MASHUP_WARP_SHADER,
            IDS_MASHUP_COMP_SHADER,
        };

        int pass;
        for (pass = 0; pass < 2; pass++) {
          box = orig_rect;
          int w = 0;
          int h = 0;

          int start_y = orig_rect.top;
          for (mash = 0; mash < MASH_SLOTS; mash++) {
            int idx = m_nMashPreset[mash];

            wchar_t buf[1024];
            // SPOUT
            // swprintf(buf, L"%s%s", wasabiApiLangString(mashNames[mash]), m_presets[idx].szFilename);
            swprintf(buf, L"%s%s", wasabiApiLangString(mashNames[mash]), m_presets[idx].szFilename.c_str());
            RECT r2 = orig_rect;
            r2.top += h;
            h += m_text.DrawTextW(GetFont(SIMPLE_FONT), buf, -1, &r2, DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX | (pass == 0 ? DT_CALCRECT : 0), (mash == m_nMashSlot) ? PLAYLIST_COLOR_HILITE_TRACK : PLAYLIST_COLOR_NORMAL, false);
            w = max(w, r2.right - r2.left);
          }
          if (pass == 0) {
            box.right = box.left + w;
            box.bottom = box.top + h;
            DrawDarkTranslucentBox(&box);
          } else
            orig_rect.top += h;
        }

        orig_rect.top += GetFontHeight(SIMPLE_FONT) + PLAYLIST_INNER_MARGIN;

        box = orig_rect;
        box.right = box.left;
        box.bottom = box.top;

        // draw a directory listing box right after...
        for (pass = 0; pass < 2; pass++) {
          // if (pass==1)
          //     GetFont(SIMPLE_FONT)->Begin();

          rect = orig_rect;
          for (i = first_line; i < last_line && m_presets[i].szFilename.c_str(); i++) {
            // remove the extension before displaying the filename.  also pad w/spaces.
            // lstrcpy(str, m_pPresetAddr[i]);
            bool bIsDir = (m_presets[i].szFilename.c_str()[0] == '*');
            bool bIsRunning = false;
            bool bIsSelected = (i == m_nMashPreset[m_nMashSlot]);

            if (bIsDir) {
              // directory
              if (wcscmp(m_presets[i].szFilename.c_str() + 1, L"..") == 0)
                swprintf(str2, L" [ %s ] (%s) ", m_presets[i].szFilename.c_str() + 1, wasabiApiLangString(IDS_PARENT_DIRECTORY));
              else
                swprintf(str2, L" [ %s ] ", m_presets[i].szFilename.c_str() + 1);
            } else {
              // preset file
              lstrcpyW(str, m_presets[i].szFilename.c_str());
              RemoveExtension(str);
              swprintf(str2, L" %s ", str);

              if (wcscmp(m_presets[m_nMashPreset[m_nMashSlot]].szFilename.c_str(), str) == 0)
                bIsRunning = true;
            }

            if (bIsRunning && m_bPresetLockedByUser)
              lstrcatW(str2, wasabiApiLangString(IDS_LOCKED));

            DWORD color = bIsDir ? DIR_COLOR : PLAYLIST_COLOR_NORMAL;
            if (bIsRunning)
              color = bIsSelected ? PLAYLIST_COLOR_BOTH : PLAYLIST_COLOR_PLAYING_TRACK;
            else if (bIsSelected)
              color = PLAYLIST_COLOR_HILITE_TRACK;

            RECT r2 = rect;
            rect.top += m_text.DrawTextW(GetFont(SIMPLE_FONT), str2, -1, &r2, DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX | (pass == 0 ? DT_CALCRECT : 0), color, false);

            if (pass == 0)  // calculating dark box
            {
              box.right = max(box.right, box.left + r2.right - r2.left);
              box.bottom += r2.bottom - r2.top;
            }
          }

          // if (pass==1)
          //     GetFont(SIMPLE_FONT)->End();

          if (pass == 0)  // calculating dark box
          {
            box.top -= PLAYLIST_INNER_MARGIN;
            box.left -= PLAYLIST_INNER_MARGIN;
            box.right += PLAYLIST_INNER_MARGIN;
            box.bottom += PLAYLIST_INNER_MARGIN;
            DrawDarkTranslucentBox(&box);
            *upper_left_corner_y = box.bottom + PLAYLIST_INNER_MARGIN;
          } else
            orig_rect.top += box.bottom - box.top;
        }

        orig_rect.top += PLAYLIST_INNER_MARGIN;
      }
    } else if (m_UI_mode == UI_LOAD) {
      if (m_nPresets == 0) {
        // note: this error message is repeated in milkdrop.cpp in LoadRandomPreset()
        wchar_t buf[1024];
        swprintf(buf, wasabiApiLangString(IDS_ERROR_NO_PRESET_FILE_FOUND_IN_X_MILK), m_szPresetDir);
        AddError(buf, 6.0f, ERR_MISC, true);
        m_UI_mode = UI_REGULAR;
      } else {
        SelectFont(PLAYLIST_FONT);
        DWORD menuColor = GetFontColor(PLAYLIST_FONT);
        MyTextOut_Color_Box(wasabiApiLangString(IDS_LOAD_WHICH_PRESET_PLUS_COMMANDS), MTO_UPPER_LEFT, menuColor);

        wchar_t buf[MAX_PATH + 64];
        swprintf(buf, wasabiApiLangString(IDS_DIRECTORY_OF_X), m_szPresetDir);
        MyTextOut_Color_Box(buf, MTO_UPPER_LEFT, menuColor);

        *upper_left_corner_y += h / 2;

        RECT rect;
        SetRect(&rect, xL, *upper_left_corner_y, xR, *lower_left_corner_y);
        rect.top += PLAYLIST_INNER_MARGIN;
        rect.left += PLAYLIST_INNER_MARGIN;
        rect.right -= PLAYLIST_INNER_MARGIN;
        rect.bottom -= PLAYLIST_INNER_MARGIN;

        int lines_available = (rect.bottom - rect.top - PLAYLIST_INNER_MARGIN * 2) / GetFontHeight(PLAYLIST_FONT);

        if (lines_available < 1) {
          // force it
          rect.bottom = rect.top + GetFontHeight(PLAYLIST_FONT) + 1;
          lines_available = 1;
        }
        if (lines_available > MAX_PRESETS_PER_PAGE)
          lines_available = MAX_PRESETS_PER_PAGE;

        if (m_bUserPagedDown) {
          m_nPresetListCurPos += lines_available;
          if (m_nPresetListCurPos >= m_nPresets)
            m_nPresetListCurPos = m_nPresets - 1;

          // remember this preset's name so the next time they hit 'L' it jumps straight to it
          // lstrcpy(m_szLastPresetSelected, m_presets[m_nPresetListCurPos].szFilename.c_str());

          m_bUserPagedDown = false;
        }

        if (m_bUserPagedUp) {
          m_nPresetListCurPos -= lines_available;
          if (m_nPresetListCurPos < 0)
            m_nPresetListCurPos = 0;

          // remember this preset's name so the next time they hit 'L' it jumps straight to it
          // lstrcpy(m_szLastPresetSelected, m_presets[m_nPresetListCurPos].szFilename.c_str());

          m_bUserPagedUp = false;
        }

        int i;
        int first_line = m_nPresetListCurPos - (m_nPresetListCurPos % lines_available);
        int last_line = first_line + lines_available;
        wchar_t str[512], str2[512];

        if (last_line > m_nPresets)
          last_line = m_nPresets;

        // tooltip:
        if (m_bShowMenuToolTips) {
          wchar_t buf[256];
          swprintf(buf, wasabiApiLangString(IDS_PAGE_X_OF_X), m_nPresetListCurPos / lines_available + 1, (m_nPresets + lines_available - 1) / lines_available);
          RECT r, r2;
          SetRect(&r, 0, 0, xR - TEXT_MARGIN * 2, 2048);
          m_text.DrawTextW(GetFont(PLAYLIST_FONT), buf, -1, &r, DT_CALCRECT, 0xFFFFFFFF, false);
          r2.bottom = *lower_right_corner_y - TEXT_MARGIN;
          r2.right = xR - TEXT_MARGIN;
          r2.left = r2.right - (r.right - r.left);
          r2.top = r2.bottom - (r.bottom - r.top);
          RECT r3 = r2;
          r3.left -= 4;
          r3.top -= 2;
          r3.right += 2;
          r3.bottom += 2;
          DrawDarkTranslucentBox(&r3);
          m_text.DrawTextW(GetFont(PLAYLIST_FONT), buf, -1, &r2, 0, GetFontColor(PLAYLIST_FONT), false);
        }

        RECT orig_rect = rect;

        RECT box;
        box.top = rect.top;
        box.left = rect.left;
        box.right = rect.left;
        box.bottom = rect.top;

        for (int pass = 0; pass < 2; pass++) {
          // if (pass==1)
          //     GetFont(SIMPLE_FONT)->Begin();

          rect = orig_rect;
          for (i = first_line; i < last_line && m_presets[i].szFilename.c_str(); i++) {
            // remove the extension before displaying the filename.  also pad w/spaces.
            // lstrcpy(str, m_pPresetAddr[i]);
            bool bIsDir = (m_presets[i].szFilename.c_str()[0] == '*');
            bool bIsRunning = (i == m_nCurrentPreset);  // false;
            bool bIsSelected = (i == m_nPresetListCurPos);

            if (bIsDir) {
              // directory
              if (wcscmp(m_presets[i].szFilename.c_str() + 1, L"..") == 0)
                swprintf(str2, L" [ %s ] (%s) ", m_presets[i].szFilename.c_str() + 1, wasabiApiLangString(IDS_PARENT_DIRECTORY));
              else
                swprintf(str2, L" [ %s ] ", m_presets[i].szFilename.c_str() + 1);
            } else {
              // preset file
              lstrcpyW(str, m_presets[i].szFilename.c_str());
              RemoveExtension(str);
              swprintf(str2, L" %s ", str);

              // if (lstrcmp(m_pState->m_szDesc, str)==0)
              //    bIsRunning = true;
            }

            if (bIsRunning && m_bPresetLockedByUser)
              lstrcatW(str2, wasabiApiLangString(IDS_LOCKED));

            DWORD color = bIsDir ? DIR_COLOR : GetFontColor(PLAYLIST_FONT);
            if (bIsRunning)
              color = bIsSelected ? PLAYLIST_COLOR_BOTH : PLAYLIST_COLOR_PLAYING_TRACK;
            else if (bIsSelected)
              color = PLAYLIST_COLOR_HILITE_TRACK;

            RECT r2 = rect;
            rect.top += m_text.DrawTextW(GetFont(PLAYLIST_FONT), str2, -1, &r2, DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX | (pass == 0 ? DT_CALCRECT : 0), color, false);

            if (pass == 0)  // calculating dark box
            {
              box.right = max(box.right, box.left + r2.right - r2.left);
              box.bottom += r2.bottom - r2.top;
            }
          }

          // if (pass==1)
          //     GetFont(SIMPLE_FONT)->End();

          if (pass == 0)  // calculating dark box
          {
            box.top -= PLAYLIST_INNER_MARGIN;
            box.left -= PLAYLIST_INNER_MARGIN;
            box.right += PLAYLIST_INNER_MARGIN;
            box.bottom += PLAYLIST_INNER_MARGIN;
            DrawDarkTranslucentBox(&box);
            *upper_left_corner_y = box.bottom + PLAYLIST_INNER_MARGIN;
          }
        }
      }
    }
  }

  // 5. render *remaining* text to upper-right corner
  {
    // e) custom timed message:
    if (!m_bWarningsDisabled2) {
      wchar_t buf[512] = {0};
      float t = GetTime();
      int N = m_errors.size();
      for (int i = 0; i < N; i++) {
        if (t >= m_errors[i].birthTime && t < m_errors[i].expireTime) {
          if (m_errors[i].category == ERR_MSG_BOTTOM_EXTRA_1 || m_errors[i].category == ERR_MSG_BOTTOM_EXTRA_2 || m_errors[i].category == ERR_MSG_BOTTOM_EXTRA_3) {
            // ERR_MSG_BOTTOM_EXTRA_1 = 6
            int fontIndex = NUM_BASIC_FONTS + m_errors[i].category - ERR_MSG_BOTTOM_EXTRA_1;
            SelectFont(static_cast<eFontIndex>(fontIndex));

            swprintf(buf, L"%s ", m_errors[i].msg.c_str());

            // 0..1
            float age_rel = (t - m_errors[i].birthTime) / (m_errors[i].expireTime - m_errors[i].birthTime);
            DWORD cr = m_fontinfo[fontIndex].R;
            DWORD cg = m_fontinfo[fontIndex].G;
            DWORD cb = m_fontinfo[fontIndex].B;
            DWORD alpha = 0;
            if (age_rel >= 0.0f && age_rel < 0.05f) {
              alpha = (DWORD)(255 * (age_rel / 0.05f));
            } else if (age_rel > 0.8f && age_rel <= 1.0f) {
              alpha = (DWORD)(255 * ((1.0f - age_rel) / 0.2f));
            } else if (age_rel >= 0.05f && age_rel <= 0.8f) {
              alpha = 255;
            }
            DWORD z = (alpha << 24) | (cr << 16) | (cg << 8) | cb;
            if (m_SongInfoDisplayCorner == 1) {
              MyTextOut_Color(buf, MTO_UPPER_LEFT, z);
            } else if (m_SongInfoDisplayCorner == 2) {
              MyTextOut_Color(buf, MTO_UPPER_RIGHT, z);
            } else if (m_SongInfoDisplayCorner == 4) {
              MyTextOut_Color(buf, MTO_LOWER_RIGHT, z);
            } else {
              MyTextOut_Color(buf, MTO_LOWER_LEFT, z);
            }
          } else {
            float age = t - m_errors[i].birthTime;
            if (!m_errors[i].bSentToRemote) {
              // send once
              int res = SendMessageToMilkwaveRemote((L"STATUS=" + m_errors[i].msg).c_str());
              m_errors[i].bSentToRemote = res != 0;
            }
            if (!m_errors[i].bSentToRemote || !m_HideNotificationsWhenRemoteActive) {
              SelectFont(SIMPLE_FONT);
              swprintf(buf, L"%s ", m_errors[i].msg.c_str());
              DWORD col = GetFontColor(SIMPLE_FONT);
              MyTextOut_Color(buf, MTO_UPPER_RIGHT, col);

              /*
              float age_rel = (age) / (m_errors[i].expireTime - m_errors[i].birthTime);
              DWORD cr = (DWORD)(200 - 199 * powf(age_rel, 4));
              DWORD cg = 0;//(DWORD)(136 - 135*powf(age_rel,1));
              DWORD cb = 0;
              DWORD z = 0xFF000000 | (cr << 16) | (cg << 8) | cb;
              MyTextOut_BGCOLOR(buf, MTO_UPPER_RIGHT, false, m_errors[i].bBold ? z : 0xFF000000);
              */
            }
          }
        } else {
          m_errors.erase(m_errors.begin() + i);
          i--;
          N--;
        }
      }
    }
  }
}
