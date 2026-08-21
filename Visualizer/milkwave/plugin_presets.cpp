// plugin_presets.cpp - Preset loading, list management, and milk2 parsing
// Extracted from plugin.cpp

#include "plugin.h"
#include "plugin_internals.h"
#include "pipe_server.h"
#include "utility.h"
#include "support.h"
#include "resource.h"
#include "defines.h"
#include "shell_defines.h"
#include "wasabi.h"
#include <process.h>

void CPlugin::PrevPreset(float fBlendTime) {
  if (m_RemotePresetLink) {
    PostMessageToMilkwaveRemote(WM_USER_PREV_PRESET);
    return;
  }

  if (m_bSequentialPresetOrder) {
    m_nCurrentPreset--;
    if (m_nCurrentPreset < m_nDirs)
      m_nCurrentPreset = m_nPresets - 1;
    if (m_nCurrentPreset >= m_nPresets)  // just in case
      m_nCurrentPreset = m_nDirs;

    wchar_t szFile[MAX_PATH];
    lstrcpyW(szFile, m_szPresetDir);  // note: m_szPresetDir always ends with '\'
    lstrcatW(szFile, m_presets[m_nCurrentPreset].szFilename.c_str());

    LoadPreset(szFile, fBlendTime);
  } else {
    int prev = (m_presetHistoryPos - 1 + PRESET_HIST_LEN) % PRESET_HIST_LEN;
    if (m_presetHistoryPos != m_presetHistoryBackFence) {
      m_presetHistoryPos = prev;
      LoadPreset(m_presetHistory[m_presetHistoryPos].c_str(), fBlendTime);
    }
  }
}

void CPlugin::NextPreset(float fBlendTime)  // if not retracing our former steps, it will choose a random one.
{
  LoadRandomPreset(fBlendTime);
}

void CPlugin::LoadRandomPreset(float fBlendTime) {
  if (m_RemotePresetLink) {
    PostMessageToMilkwaveRemote(WM_USER_NEXT_PRESET);
    return;
  }

  // make sure file list is ok
  if (m_nPresets - m_nDirs == 0) {
    if (m_nPresets - m_nDirs == 0) {
      // note: this error message is repeated in milkdropfs.cpp in DrawText()
      wchar_t buf[1024];
      swprintf(buf, wasabiApiLangString(IDS_ERROR_NO_PRESET_FILE_FOUND_IN_X_MILK), m_szPresetDir);
      AddError(buf, 6.0f, ERR_MISC, true);

      // also bring up the dir. navigation menu...
      if (m_UI_mode == UI_REGULAR || m_UI_mode == UI_MENU) {
        m_UI_mode = UI_LOAD;
        m_bUserPagedUp = false;
        m_bUserPagedDown = false;
      }
      return;
    }
  }

  bool bHistoryEmpty = (m_presetHistoryFwdFence == m_presetHistoryBackFence);

  // if we have history to march back forward through, do that first
  if (!m_bSequentialPresetOrder) {
    int next = (m_presetHistoryPos + 1) % PRESET_HIST_LEN;
    if (next != m_presetHistoryFwdFence && !bHistoryEmpty) {
      m_presetHistoryPos = next;
      LoadPreset(m_presetHistory[m_presetHistoryPos].c_str(), fBlendTime);
      return;
    }
  }

  // --TEMPORARY--
  // this comes in handy if you want to mass-modify a batch of presets;
  // just automatically tweak values in Import, then they immediately get exported to a .MILK in a new dir.
  /*
  for (int i=0; i<m_nPresets; i++)
  {
    char szPresetFile[512];
    lstrcpy(szPresetFile, m_szPresetDir);	// note: m_szPresetDir always ends with '\'
    lstrcat(szPresetFile, m_pPresetAddr[i]);
    //CState newstate;
    m_state2.Import(szPresetFile, GetTime());

    lstrcpy(szPresetFile, "c:\\t7\\");
    lstrcat(szPresetFile, m_pPresetAddr[i]);
    m_state2.Export(szPresetFile);
  }
  */
  // --[END]TEMPORARY--

  if (m_bSequentialPresetOrder) {
    m_nCurrentPreset++;
    if (m_nCurrentPreset < m_nDirs || m_nCurrentPreset >= m_nPresets)
      m_nCurrentPreset = m_nDirs;
  } else {
    // pick a random file
    if (!m_bEnableRating || (m_presets[m_nPresets - 1].fRatingCum < 0.1f))  // || (m_nRatingReadProgress < m_nPresets))
    {
      m_nCurrentPreset = m_nDirs + (rand() % (m_nPresets - m_nDirs));
    } else {
      float cdf_pos = (rand() % 14345) / 14345.0f * m_presets[m_nPresets - 1].fRatingCum;

      /*
      char buf[512];
      sprintf(buf, "max = %f, rand = %f, \tvalues: ", m_presets[m_nPresets - 1].fRatingCum, cdf_pos);
      for (int i=m_nDirs; i<m_nPresets; i++)
      {
        char buf2[32];
        sprintf(buf2, "%3.1f ", m_presets[i].fRatingCum);
        lstrcat(buf, buf2);
      }
      dumpmsg(buf);
      */

      if (cdf_pos < m_presets[m_nDirs].fRatingCum) {
        m_nCurrentPreset = m_nDirs;
      } else {
        int lo = m_nDirs;
        int hi = m_nPresets;
        while (lo + 1 < hi) {
          int mid = (lo + hi) / 2;
          if (m_presets[mid].fRatingCum > cdf_pos)
            hi = mid;
          else
            lo = mid;
        }
        m_nCurrentPreset = hi;
      }
    }
  }

  // m_pPresetAddr[m_nCurrentPreset] points to the preset file to load (w/o the path);
  // first prepend the path, then load section [preset00] within that file
  wchar_t szFile[MAX_PATH] = {0};
  lstrcpyW(szFile, m_szPresetDir);  // note: m_szPresetDir always ends with '\'
  lstrcatW(szFile, m_presets[m_nCurrentPreset].szFilename.c_str());

  if (!bHistoryEmpty)
    m_presetHistoryPos = (m_presetHistoryPos + 1) % PRESET_HIST_LEN;

  LoadPreset(szFile, fBlendTime);
}

void CPlugin::ClearPreset() {
  m_pState->Default(STATE_ALL);
  wcscpy(m_szCurrentPresetFile, m_pState->m_szDesc);
  RemoveAngleBrackets(m_szCurrentPresetFile);

  // Append ".milk" to m_szCurrentPresetFile
  if (wcslen(m_szCurrentPresetFile) + wcslen(L".milk") < MAX_PATH) {
    wcscat_s(m_szCurrentPresetFile, MAX_PATH, L".milk");
  }

  // release stuff from m_OldShaders, then move m_shaders to m_OldShaders, then load the new shaders.
  SafeRelease(m_OldShaders.comp.ptr);
  SafeRelease(m_OldShaders.warp.ptr);
  SafeRelease(m_OldShaders.comp.CT);
  SafeRelease(m_OldShaders.warp.CT);
  m_OldShaders = m_shaders;
  ZeroMemory(&m_shaders, sizeof(PShaderSet));

  LoadShaders(&m_shaders, m_pState, false, false);
  NumTotalPresetsLoaded++;
  OnFinishedLoadingPreset();
}

void CPlugin::RemoveAngleBrackets(wchar_t* str) {
  wchar_t cleaned[MAX_PATH] = {0};  // Temporary buffer for the cleaned string
  int j = 0;

  for (int i = 0; str[i] != L'\0'; i++) {
    if (str[i] != L'<' && str[i] != L'>') {
      cleaned[j++] = str[i];
    }
  }

  cleaned[j] = L'\0';                // Null-terminate the cleaned string
  wcscpy_s(str, MAX_PATH, cleaned);  // Copy the cleaned string back to the original
}

// .milk2 double-preset support
// ---------------------------------------------------------------------------

// Maps MilkDrop3 blend-pattern names to RandomizeBlendPattern() mixtype indices.
// Returns -1 (random) for any name that is not explicitly mapped.

static int Milk2PatternNameToMixtype(const char* name) {
  struct {
    const char* name;
    int type;
  } kMap[] = {
      {"zoom", 0},    // uniform fade
      {"side", 1},    // directional wipe
      {"plasma", 2},  // fractal plasma
      {"plasma2", 2},
      {"plasma3", 2},
      {"circle", 3},  // radial / circle
      {"cercle", 3},  // radial / circle (MilkDrop3 spelling)
      {"clock", 4},   // angular clock sweep
      {"snail", 5},   // spiral
      {"snail2", 5},
      {"snail3", 5},
      {"triangle", 6},
      {"square", 8},          // square/diamond
      {"curtain", 10},        // curtain
      {"linesvertical", 10},  // vertical lines / curtain variant
      {"donuts", 11},         // bubble / donuts
      {"stars", 14},          // star wipe
      {"patches", 9},         // checkerboard / patches
      {"arrow", 1},           // directional arrow wipe
      {"corner", 3},          // corner / bottom-left quarter-circle
      {"vertical", 19},       // fixed left-to-right wipe
      {"horizontal", 20},     // fixed top-to-bottom wipe
  };
  for (auto& e : kMap)
    if (_stricmp(name, e.name) == 0) return e.type;
  return -1;
}

// Forward declaration: resets _GetLineByName's static FILE* cache in state.cpp.
// Required to prevent stale data when two Import() calls use consecutively-allocated FILE*s.
extern void GetFast_CLEAR();

// Parses a .milk2 file and writes its two preset blocks to temporary .milk files.
// On success, outTemp1/outTemp2 hold MAX_PATH paths to temp files that the caller must delete.
// Returns false on parse failure (malformed .milk2); temp files are not written.

bool CPlugin::ParseMilk2File(const wchar_t* szPath,
                             wchar_t* outTemp1, wchar_t* outTemp2,
                             int& outMixType, float& outProgress, float& outDirection,
                             unsigned int& outSeed) {
  outMixType = -1;
  outProgress = 0.5f;
  outDirection = 0.0f;
  outSeed = 0;

  // Read entire file into a string buffer.
  FILE* f = _wfopen(szPath, L"rb");
  if (!f) return false;
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET);
  std::string buf(fsize, '\0');
  fread(&buf[0], 1, fsize, f);
  fclose(f);

  // Parse header key=value lines before [PRESET1_BEGIN].
  {
    size_t hdrEnd = buf.find("[PRESET1_BEGIN]");
    if (hdrEnd == std::string::npos) return false;
    std::string hdr = buf.substr(0, hdrEnd);
    auto getVal = [&](const char* key) -> std::string {
      std::string k = std::string(key) + "=";
      size_t pos = hdr.find(k);
      if (pos == std::string::npos) return "";
      size_t start = pos + k.size();
      size_t end = hdr.find_first_of("\r\n ", start);
      if (end == std::string::npos) end = hdr.length();
      return hdr.substr(start, end - start);
    };
    std::string pat = getVal("blending_pattern");
    if (!pat.empty()) outMixType = Milk2PatternNameToMixtype(pat.c_str());
    // "horizontal" pattern = horizontal split line => vertical wipe axis
    m_bMilk2VerticalWipe = (!pat.empty() && _stricmp(pat.c_str(), "horizontal") == 0);
    m_bMilk2LinesVertical = (!pat.empty() && _stricmp(pat.c_str(), "linesvertical") == 0);
    m_bMilk2CornerWipe = (!pat.empty() && _stricmp(pat.c_str(), "corner") == 0);
    m_bMilk2ArrowWipe = (!pat.empty() && _stricmp(pat.c_str(), "arrow") == 0);
    m_bMilk2Snail2 = (!pat.empty() && _stricmp(pat.c_str(), "snail2") == 0);
    m_bMilk2Snail3 = (!pat.empty() && _stricmp(pat.c_str(), "snail3") == 0);
    m_bMilk2Plasma3 = (!pat.empty() && _stricmp(pat.c_str(), "plasma3") == 0);
    std::string prog = getVal("blending_progress");
    if (!prog.empty()) outProgress = (float)atof(prog.c_str());
    std::string dir = getVal("blending_direction");
    if (!dir.empty()) outDirection = (float)atof(dir.c_str());

    // Parse random_1..5 and compute a deterministic seed for blend pattern generation
    float randoms[5] = {0.0f};
    for (int i = 0; i < 5; i++) {
      char key[16];
      snprintf(key, sizeof(key), "random_%d", i + 1);
      std::string val = getVal(key);
      if (!val.empty()) randoms[i] = (float)atof(val.c_str());
    }
    m_fMilk2Random1 = randoms[0];
    m_fMilk2Random2 = randoms[1];
    m_fMilk2Random3 = randoms[2];
    m_fMilk2Random4 = randoms[3];
    m_fMilk2Random5 = randoms[4];

    unsigned int seed = 0;
    // Hash-combine the 5 float values into a single seed.
    for (int i = 0; i < 5; i++) {
      unsigned int bits;
      memcpy(&bits, &randoms[i], sizeof(bits));
      seed ^= bits + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    outSeed = seed;
  }

  // Helper: extract text between two markers.
  auto extractPreset = [&](const char* beginMarker, const char* endMarker) -> std::string {
    size_t bPos = buf.find(beginMarker);
    if (bPos == std::string::npos) return "";
    size_t ePos = buf.find(endMarker, bPos);
    if (ePos == std::string::npos) return "";
    size_t contentStart = bPos + strlen(beginMarker);
    // Skip past the marker line ending
    contentStart = buf.find_first_of("\r\n", contentStart);
    if (contentStart == std::string::npos) return "";
    contentStart = buf.find_first_not_of("\r\n", contentStart);
    if (contentStart == std::string::npos) return "";
    return buf.substr(contentStart, ePos - contentStart);
  };

  std::string p1 = extractPreset("[PRESET1_BEGIN]", "[PRESET1_END]");
  std::string p2 = extractPreset("[PRESET2_BEGIN]", "[PRESET2_END]");
  if (p1.empty() || p2.empty()) return false;

  // â”€â”€ Parse embedded sprites ([SPRITE1_BEGIN]..[SPRITE1_END], etc.) â”€â”€
  m_nMilk2SpriteCount = 0;
  {
    // Read the 'sprite' count from the header portion of the buffer.
    int sprCount = 0;
    {
      size_t hdrEnd = buf.find("[PRESET1_BEGIN]");
      if (hdrEnd != std::string::npos) {
        std::string hdr = buf.substr(0, hdrEnd);
        std::string key = "sprite=";
        size_t pos = hdr.find(key);
        if (pos != std::string::npos) {
          size_t start = pos + key.size();
          size_t end = hdr.find_first_of("\r\n ", start);
          if (end == std::string::npos) end = hdr.length();
          sprCount = atoi(hdr.substr(start, end - start).c_str());
        }
      }
    }
    if (sprCount > MAX_MILK2_SPRITES) sprCount = MAX_MILK2_SPRITES;

    for (int si = 0; si < sprCount; si++) {
      char beginTag[32], endTag[32];
      snprintf(beginTag, sizeof(beginTag), "[SPRITE%d_BEGIN]", si + 1);
      snprintf(endTag, sizeof(endTag), "[SPRITE%d_END]", si + 1);

      size_t bPos = buf.find(beginTag);
      size_t ePos = (bPos != std::string::npos) ? buf.find(endTag, bPos) : std::string::npos;
      if (bPos == std::string::npos || ePos == std::string::npos)
        continue;

      std::string sprBlock = buf.substr(bPos, ePos - bPos);

      // Helper to get a value from the sprite block.
      auto sprGetVal = [&](const char* key) -> std::string {
        std::string k = std::string(key) + "=";
        size_t pos = sprBlock.find(k);
        if (pos == std::string::npos) return "";
        size_t start = pos + k.size();
        size_t end = sprBlock.find_first_of("\r\n", start);
        if (end == std::string::npos) end = sprBlock.length();
        return sprBlock.substr(start, end - start);
      };

      Milk2Sprite& spr = m_milk2Sprites[m_nMilk2SpriteCount];
      spr = Milk2Sprite{};  // reset

      std::string imgName = sprGetVal("SpriteName");
      if (imgName.empty()) continue;

      // Resolve image path: if not absolute, prepend m_szMilkdrop2Path (resources dir).
      if (imgName.size() >= 2 && imgName[1] == ':') {
        // Absolute path
        std::wstring wImg(imgName.begin(), imgName.end());
        wcsncpy_s(spr.szImgPath, wImg.c_str(), _TRUNCATE);
      } else {
        // Relative: replace forward slashes with backslashes
        for (auto& ch : imgName)
          if (ch == '/') ch = '\\';
        std::wstring wImg(imgName.begin(), imgName.end());
        swprintf(spr.szImgPath, L"%s%s", m_szMilkdrop2Path, wImg.c_str());
      }

      std::string ck = sprGetVal("SpriteColorKey");
      spr.nColorKey = ck.empty() ? 0x000000 : (unsigned int)strtoul(ck.c_str(), nullptr, 16);
      std::string layer = sprGetVal("SpriteLayer");
      spr.nLayer = layer.empty() ? 0 : atoi(layer.c_str());
      std::string blend = sprGetVal("SpriteBlend");
      spr.nBlend = blend.empty() ? 0 : atoi(blend.c_str());
      std::string alpha = sprGetVal("SpriteAlpha");
      spr.fAlpha = alpha.empty() ? 1.0f : (float)atof(alpha.c_str());
      std::string burn = sprGetVal("SpriteBurn");
      spr.bBurn = burn.empty() ? true : (atoi(burn.c_str()) != 0);
      std::string sx = sprGetVal("SpriteSX");
      spr.fSX = sx.empty() ? 1.0f : (float)atof(sx.c_str());
      std::string sy = sprGetVal("SpriteSY");
      spr.fSY = sy.empty() ? 1.0f : (float)atof(sy.c_str());
      std::string px = sprGetVal("SpriteX");
      spr.fX = px.empty() ? 0.5f : (float)atof(px.c_str());
      std::string py = sprGetVal("SpriteY");
      spr.fY = py.empty() ? 0.5f : (float)atof(py.c_str());
      std::string rot = sprGetVal("SpriteRot");
      spr.fRot = rot.empty() ? 0.0f : (float)atof(rot.c_str());
      std::string speed = sprGetVal("SpriteSpeed");
      spr.fSpeed = speed.empty() ? 0.0f : (float)atof(speed.c_str());
      std::string repX = sprGetVal("SpriteRepeatX");
      spr.fRepeatX = repX.empty() ? 1.0f : (float)atof(repX.c_str());
      std::string repY = sprGetVal("SpriteRepeatY");
      spr.fRepeatY = repY.empty() ? 1.0f : (float)atof(repY.c_str());

      // Extract init_N= and code_N= lines from the sprite block.
      spr.szInitCode[0] = 0;
      spr.szCode[0] = 0;
      for (int pass = 0; pass < 2; pass++) {
        char* pDest = (pass == 0) ? spr.szInitCode : spr.szCode;
        const char* prefix = (pass == 0) ? "init_" : "code_";
        int charPos = 0;
        for (int lineNum = 1; lineNum < 256; lineNum++) {
          char lineKey[32];
          snprintf(lineKey, sizeof(lineKey), "%s%d=", prefix, lineNum);
          size_t lpos = sprBlock.find(lineKey);
          if (lpos == std::string::npos) break;
          size_t lstart = lpos + strlen(lineKey);
          size_t lend = sprBlock.find_first_of("\r\n", lstart);
          if (lend == std::string::npos) lend = sprBlock.length();
          std::string lineVal = sprBlock.substr(lstart, lend - lstart);
          if (charPos + (int)lineVal.size() + 2 < 8192) {
            memcpy(&pDest[charPos], lineVal.c_str(), lineVal.size());
            charPos += (int)lineVal.size();
            pDest[charPos++] = LINEFEED_CONTROL_CHAR;
          }
        }
        pDest[charPos] = 0;
      }

      spr.valid = true;
      m_nMilk2SpriteCount++;
    }
  }

  // Write to Windows temp files.
  wchar_t tempDir[MAX_PATH];
  GetTempPathW(MAX_PATH, tempDir);

  if (GetTempFileNameW(tempDir, L"mk2", 0, outTemp1) == 0) return false;
  if (GetTempFileNameW(tempDir, L"mk2", 0, outTemp2) == 0) {
    DeleteFileW(outTemp1);
    return false;
  }

  auto writeTmp = [](const wchar_t* path, const std::string& text) -> bool {
    FILE* out = _wfopen(path, L"wb");
    if (!out) return false;
    fwrite(text.data(), 1, text.size(), out);
    fclose(out);
    return true;
  };

  if (!writeTmp(outTemp1, p1) || !writeTmp(outTemp2, p2)) {
    DeleteFileW(outTemp1);
    DeleteFileW(outTemp2);
    return false;
  }
  return true;
}

void CPlugin::LoadPreset(const wchar_t* szPresetFilename, float fBlendTime) {
  // clear old error msg...
  if (m_nFramesSinceResize > 4)
    ClearErrors(ERR_PRESET);

  // Reset permanent .milk2 blend when loading any new preset
  m_bMilk2PermanentBlend = false;
  m_fMilk2BlendDirection = 0.0f;
  m_bMilk2VerticalWipe = false;
  m_bMilk2LinesVertical = false;
  m_bMilk2CornerWipe = false;
  m_bMilk2Snail2 = false;
  m_bMilk2Snail3 = false;
  m_bMilk2Plasma3 = false;

  // Kill any active milk2 sprites from the previous preset
  KillMilk2Sprites();

  // make sure preset still exists.  (might not if they are using the "back"/fwd buttons
  //  in RANDOM preset order and a file was renamed or deleted!)
  if (GetFileAttributesW(szPresetFilename) == 0xFFFFFFFF) {
    wchar_t fullPath[MAX_PATH];
    GetFullPathNameW(szPresetFilename, MAX_PATH, fullPath, NULL);
    // Log the full path (to debugger or console)
    OutputDebugStringW(fullPath);
    OutputDebugStringW(L"\n");

    wchar_t buf[1024];
    swprintf(buf, wasabiApiLangString(IDS_ERROR_PRESET_NOT_FOUND_X), fullPath);
    AddError(buf, 6.0f, ERR_PRESET, true);
    m_fPresetStartTime = GetTime();
    m_fNextPresetTime = -1.0f;  // flags UpdateTime() to recompute this
    return;
  }

  if (!m_bSequentialPresetOrder) {
    // save preset in the history.  keep in mind - maybe we are searching back through it already!
    if (m_presetHistoryFwdFence == m_presetHistoryPos) {
      // we're at the forward frontier; add to history
      m_presetHistory[m_presetHistoryPos] = szPresetFilename;
      m_presetHistoryFwdFence = (m_presetHistoryFwdFence + 1) % PRESET_HIST_LEN;

      // don't let the two fences touch
      if (m_presetHistoryBackFence == m_presetHistoryFwdFence)
        m_presetHistoryBackFence = (m_presetHistoryBackFence + 1) % PRESET_HIST_LEN;
    } else {
      // we're retracing our steps, either forward or backward...
    }
  }

  // if no preset was valid before, make sure there is no blend, because there is nothing valid to blend from.
  if (!wcscmp(m_pState->m_szDesc, INVALID_PRESET_DESC))
    fBlendTime = 0;

  // Detect .milk2 double-preset files
  const wchar_t* lastDot = wcsrchr(szPresetFilename, L'.');
  bool bIsMilk2 = lastDot && _wcsicmp(lastDot, L".milk2") == 0;

  if (bIsMilk2) {
    // .milk2 double-preset: parse the file, import both presets, compile shaders for both
    m_bLoadingMilk2 = true;

    int mixType = -1;
    float progress = 0.5f;
    float direction = 0.0f;
    unsigned int seed = 0;
    if (!ParseMilk2File(szPresetFilename, m_szMilk2Temp1, m_szMilk2Temp2,
                        mixType, progress, direction, seed)) {
      wchar_t buf[1024];
      swprintf(buf, L"Error: Failed to parse .milk2 file: %s", szPresetFilename);
      AddError(buf, 6.0f, ERR_PRESET, true);
      m_bLoadingMilk2 = false;
      return;
    }
    m_nMilk2MixType = mixType;
    m_fMilk2BlendDirection = direction;
    // Plasma and star transitions need different progress remaps to match MilkDrop 3.33.
    if (mixType == 2) {
      float clampedProgress = min(1.0f, max(0.0f, progress));
      if (m_bMilk2Plasma3) {
        m_fMilk2BlendProgress = 0.10f + 0.90f * clampedProgress;
      } else {
        if (clampedProgress <= 0.5f)
          m_fMilk2BlendProgress = clampedProgress * 0.64f;
        else
          m_fMilk2BlendProgress = 0.32f + (clampedProgress - 0.5f) * 1.36f;
      }
    } else if (mixType == 14) {
      float clampedProgress = min(1.0f, max(0.0f, progress));
      // Stars stay visibly in progress at 0.5, then finish quickly by 0.7.
      if (clampedProgress <= 0.5f)
        m_fMilk2BlendProgress = 0.15f + 0.40f * clampedProgress;
      else
        m_fMilk2BlendProgress = 0.35f + (clampedProgress - 0.5f) * 3.00f;
      if (m_fMilk2BlendProgress > 1.0f)
        m_fMilk2BlendProgress = 1.0f;
    } else if (mixType == 6) {
      float clampedProgress = min(1.0f, max(0.0f, progress));
      m_fMilk2BlendProgress = 0.1f + clampedProgress;
      if (m_fMilk2BlendProgress > 1.0f)
        m_fMilk2BlendProgress = 1.0f;
    } else if (mixType == 5) {
      // Snail works best close to the authored progress from the .milk2 header.
      m_fMilk2BlendProgress = min(1.0f, max(0.0f, progress));
    } else
      m_fMilk2BlendProgress = min(1.0f, max(0.0f, progress));
    m_fMilk2FrozenProgress = m_fMilk2BlendProgress;
    m_nMilk2PatternSeed = seed;

    // Import preset 1 (blend-from) into m_pMilk2OldState
    m_pMilk2OldState->Import(m_szMilk2Temp1, GetTime(), nullptr, STATE_ALL);
    GetFast_CLEAR();
    // Import preset 2 (blend-to) into m_pNewState
    m_pNewState->Import(m_szMilk2Temp2, GetTime(), m_pMilk2OldState, STATE_ALL);

    // Compile shaders for preset 1 (blend-from)
    SafeRelease(m_Milk2OldShaders.comp.ptr);
    SafeRelease(m_Milk2OldShaders.warp.ptr);
    SafeRelease(m_Milk2OldShaders.comp.CT);
    SafeRelease(m_Milk2OldShaders.warp.CT);
    ZeroMemory(&m_Milk2OldShaders, sizeof(PShaderSet));
    LoadShaders(&m_Milk2OldShaders, m_pMilk2OldState, false, false);

    // Set up incremental loading for preset 2 (blend-to) shaders via LoadPresetTick()
    SafeRelease(m_NewShaders.comp.ptr);
    SafeRelease(m_NewShaders.warp.ptr);
    ZeroMemory(&m_NewShaders, sizeof(PShaderSet));

    m_nLoadingPreset = 1;
    m_fLoadingPresetBlendTime = (fBlendTime > 0) ? fBlendTime : 1.0f;
    lstrcpyW(m_szLoadingPreset, szPresetFilename);

    // Clean up temp files
    DeleteFileW(m_szMilk2Temp1);
    DeleteFileW(m_szMilk2Temp2);

    NumTotalPresetsLoaded++;
    return;
  }

  if (fBlendTime == 0) {
    // do it all NOW!
    if (szPresetFilename != m_szCurrentPresetFile)  //[sic]
      lstrcpyW(m_szCurrentPresetFile, szPresetFilename);

    CState* temp = m_pState;
    m_pState = m_pOldState;
    m_pOldState = temp;

    DWORD ApplyFlags = STATE_ALL;
    ApplyFlags ^= (m_bWarpShaderLock ? STATE_WARP : 0);
    ApplyFlags ^= (m_bCompShaderLock ? STATE_COMP : 0);

    m_pState->Import(m_szCurrentPresetFile, GetTime(), m_pOldState, ApplyFlags);

    if (fBlendTime >= 0.001f) {
      RandomizeBlendPattern();
      m_pState->StartBlendFrom(m_pOldState, GetTime(), fBlendTime);
    }

    m_fPresetStartTime = GetTime();
    m_fNextPresetTime = -1.0f;  // flags UpdateTime() to recompute this

    // release stuff from m_OldShaders, then move m_shaders to m_OldShaders, then load the new shaders.
    SafeRelease(m_OldShaders.comp.ptr);
    SafeRelease(m_OldShaders.warp.ptr);
    SafeRelease(m_OldShaders.comp.CT);
    SafeRelease(m_OldShaders.warp.CT);
    m_OldShaders = m_shaders;
    ZeroMemory(&m_shaders, sizeof(PShaderSet));

    LoadShaders(&m_shaders, m_pState, false, false);
    NumTotalPresetsLoaded++;
    OnFinishedLoadingPreset();
  } else {
    // set ourselves up to load the preset (and esp. compile shaders) a little bit at a time
    SafeRelease(m_NewShaders.comp.ptr);
    SafeRelease(m_NewShaders.warp.ptr);
    ZeroMemory(&m_NewShaders, sizeof(PShaderSet));

    DWORD ApplyFlags = STATE_ALL;
    ApplyFlags ^= (m_bWarpShaderLock ? STATE_WARP : 0);
    ApplyFlags ^= (m_bCompShaderLock ? STATE_COMP : 0);

    m_pNewState->Import(szPresetFilename, GetTime(), m_pOldState, ApplyFlags);

    m_nLoadingPreset = 1;  // this will cause LoadPresetTick() to get called over the next few frames...

    m_fLoadingPresetBlendTime = fBlendTime;
    lstrcpyW(m_szLoadingPreset, szPresetFilename);
    NumTotalPresetsLoaded++;
  }
}

void CPlugin::OnFinishedLoadingPreset() {
  // note: only used this if you loaded the preset *intact* (or mostly intact)

  SetMenusForPresetVersion(m_pState->m_nWarpPSVersion, m_pState->m_nCompPSVersion);
  m_nPresetsLoadedTotal++;  // only increment this on COMPLETION of the load.

  for (int mash = 0; mash < MASH_SLOTS; mash++)
    m_nMashPreset[mash] = m_nCurrentPreset;

  milkwave->LogEvent(L"Preset activated: " + std::wstring(m_szCurrentPresetFile));
  SendPresetChangedInfoToMilkwaveRemote();
}
// â”€â”€â”€ IPC via Named Pipe â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
// Outgoing messages are sent through g_pipeServer (pipe_server.h).
// The old WM_COPYDATA / FindWindow code has been removed.

int CPlugin::SendMessageToMilkwaveRemote(const wchar_t* messageToSend) {
  return SendMessageToMilkwaveRemote(messageToSend, false);
}

int CPlugin::SendMessageToMilkwaveRemote(const wchar_t* messageToSend, bool doForce) {
  using namespace std::chrono;
  try {
    if (!messageToSend || !*messageToSend)
      return 0;

    // Thread-safe timing check
    EnterCriticalSection(&g_csRemoteMessage);
    uint64_t Now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    if (!doForce && Now - LastSentMilkwaveMessage < 100) {
      LeaveCriticalSection(&g_csRemoteMessage);
      return 0;
    }
    LastSentMilkwaveMessage = Now;
    LeaveCriticalSection(&g_csRemoteMessage);

    extern PipeServer g_pipeServer;
    g_pipeServer.Send(messageToSend);
  } catch (...) {
    return 0;
  }
  return 1;
}

void CPlugin::PostMessageToMilkwaveRemote(UINT msg) {
  try {
    extern PipeServer g_pipeServer;
    // Map WM_USER+N constants to SIGNAL| pipe messages
    const wchar_t* signal = nullptr;
    if (msg == WM_USER + 100)
      signal = L"SIGNAL|NEXT_PRESET";
    else if (msg == WM_USER + 101)
      signal = L"SIGNAL|PREV_PRESET";
    else if (msg == WM_USER + 102)
      signal = L"SIGNAL|COVER_CHANGED";
    else if (msg == WM_USER + 103)
      signal = L"SIGNAL|SPRITE_MODE";
    else if (msg == WM_USER + 104)
      signal = L"SIGNAL|MESSAGE_MODE";
    if (signal)
      g_pipeServer.Send(signal);
  } catch (...) {
    // ignore
  }
}

void CPlugin::LoadPresetTick() {
  if (m_nLoadingPreset == 2 || m_nLoadingPreset == 5) {
    // just loads one shader (warp or comp) then returns.
    LoadShaders(&m_NewShaders, m_pNewState, true, false);
  } else if (m_nLoadingPreset == 8) {
    // finished loading the shaders - apply the preset!
    lstrcpyW(m_szCurrentPresetFile, m_szLoadingPreset);
    m_szLoadingPreset[0] = 0;

    CState* temp = m_pState;
    m_pState = m_pOldState;
    m_pOldState = temp;

    temp = m_pState;
    m_pState = m_pNewState;
    m_pNewState = temp;

    // .milk2: swap in preset 1 as the blend-from state
    if (m_bLoadingMilk2) {
      temp = m_pOldState;
      m_pOldState = m_pMilk2OldState;
      m_pMilk2OldState = temp;  // recycled â€” will be reused on next milk2 load

      // Fix descriptions: Import() derived m_szDesc from temp file paths.
      // Override with the .milk2 filename (without path or extension).
      const wchar_t* p = wcsrchr(m_szCurrentPresetFile, L'\\');
      if (!p)
        p = m_szCurrentPresetFile;
      else
        p++;
      wcsncpy_s(m_pState->m_szDesc, p, MAX_PATH - 1);
      wchar_t* dot = wcsrchr(m_pState->m_szDesc, L'.');
      if (dot) *dot = L'\0';
      wcscpy_s(m_pOldState->m_szDesc, MAX_PATH, m_pState->m_szDesc);
    }

    if (m_bLoadingMilk2) {
      // .milk2 uses its own blend pattern and direction from metadata
      int savedMixType = m_nMixType;
      m_nMixType = m_nMilk2MixType;
      // Seed RNG for deterministic pattern generation (from .milk2 random_1..5 values)
      srand(m_nMilk2PatternSeed);
      // m_fMilk2BlendDirection is already set; RandomizeBlendPattern reads it for cercle/side
      RandomizeBlendPattern();
      srand((unsigned int)GetTickCount());  // restore randomness for normal operation
      m_nMixType = savedMixType;
      // Note: m_fMilk2BlendDirection stays set so resize can regenerate the same pattern
    } else {
      RandomizeBlendPattern();
    }

    // if (fBlendTime >= 0.001f)
    m_pState->StartBlendFrom(m_pOldState, GetTime(), m_fLoadingPresetBlendTime);

    // .milk2: activate permanent blend â€” pin progress immediately at target value (no animation)
    if (m_bLoadingMilk2) {
      // Clear VS[0] on the next frame so feedback-dependent shaders (e.g. SDF/distance-field
      // presets) start from a known black state rather than from the previous preset's content.
      // This prevents the distance field from getting permanently stuck, e.g. when frosty-caves'
      // high blue channel causes GetDistâ‰ˆ0.2 everywhere â†’ rdistâ‰ˆ0.8 â†’ near-black comp output.
      m_nFramesSinceResize = 0;

      m_bMilk2PermanentBlend = true;
      m_pState->m_bBlending = true;
      m_pState->m_fBlendProgress = m_fMilk2FrozenProgress;  // pin immediately, no transition
    }

    m_fPresetStartTime = GetTime();
    m_fNextPresetTime = -1.0f;  // flags UpdateTime() to recompute this

    // release stuff from m_OldShaders, then move m_shaders to m_OldShaders, then load the new shaders.
    SafeRelease(m_OldShaders.comp.ptr);
    SafeRelease(m_OldShaders.warp.ptr);
    if (m_bLoadingMilk2) {
      // .milk2: use preset 1's shaders as old, preset 2's as new
      m_OldShaders = m_Milk2OldShaders;
      ZeroMemory(&m_Milk2OldShaders, sizeof(PShaderSet));
    } else {
      m_OldShaders = m_shaders;
    }
    m_shaders = m_NewShaders;
    ZeroMemory(&m_NewShaders, sizeof(PShaderSet));

    // end slow-preset-load mode
    m_nLoadingPreset = 0;

    // Launch embedded milk2 sprites before clearing the loading flag
    if (m_bLoadingMilk2 && m_nMilk2SpriteCount > 0)
      LaunchMilk2Sprites();

    m_bLoadingMilk2 = false;

    OnFinishedLoadingPreset();
  }

  if (m_nLoadingPreset > 0)
    m_nLoadingPreset++;
}

void CPlugin::SeekToPreset(wchar_t cStartChar) {
  if (cStartChar >= L'a' && cStartChar <= L'z')
    cStartChar -= L'a' - L'A';

  for (int i = m_nDirs; i < m_nPresets; i++) {
    wchar_t ch = m_presets[i].szFilename.c_str()[0];
    if (ch >= L'a' && ch <= L'z')
      ch -= L'a' - L'A';
    if (ch == cStartChar) {
      m_nPresetListCurPos = i;
      return;
    }
  }
}

void CPlugin::FindValidPresetDir() {
  swprintf(m_szPresetDir, L"%spresets\\", m_szMilkdrop2Path);
  if (GetFileAttributesW(m_szPresetDir) != -1)
    return;
  lstrcpyW(m_szPresetDir, m_szMilkdrop2Path);
  if (GetFileAttributesW(m_szPresetDir) != -1)
    return;
  lstrcpyW(m_szPresetDir, GetPluginsDirPath());
  if (GetFileAttributesW(m_szPresetDir) != -1)
    return;
  lstrcpyW(m_szPresetDir, L"c:\\program files\\winamp\\");  // getting desperate here
  if (GetFileAttributesW(m_szPresetDir) != -1)
    return;
  lstrcpyW(m_szPresetDir, L"c:\\program files\\");  // getting desperate here
  if (GetFileAttributesW(m_szPresetDir) != -1)
    return;
  lstrcpyW(m_szPresetDir, L"c:\\");
}

char* NextLine(char* p) {
  // p points to the beginning of a line
  // we'll return a pointer to the first char of the next line
  // if we hit a NULL char before that, we'll return NULL.
  if (!p)
    return NULL;

  char* s = p;
  while (*s != '\r' && *s != '\n' && *s != 0)
    s++;

  while (*s == '\r' || *s == '\n')
    s++;

  if (*s == 0)
    return NULL;

  return s;
}

static unsigned int WINAPI __UpdatePresetList(void* lpVoid) {
  // NOTE - this is run in a separate thread!!!

  DWORD flags = (DWORD)lpVoid;
  bool bForce = (flags & 1) ? true : false;
  bool bTryReselectCurrentPreset = (flags & 2) ? true : false;

  WIN32_FIND_DATAW fd;
  ZeroMemory(&fd, sizeof(fd));
  HANDLE h = INVALID_HANDLE_VALUE;

  int nTry = 0;
  bool bRetrying = false;

  EnterCriticalSection(&g_cs);
retry:

  // make sure the path exists; if not, go to winamp plugins dir
  if (GetFileAttributesW(g_plugin.m_szPresetDir) == -1) {
    // FIXME...
    g_plugin.FindValidPresetDir();
  }

  // if Mask (dir) changed, do a full re-scan;
  // if not, just finish our old scan.
  wchar_t szMask[MAX_PATH];
  swprintf(szMask, L"%s*.*", g_plugin.m_szPresetDir);  // cuz dirnames could have extensions, etc.
  if (bForce || !g_plugin.m_szUpdatePresetMask[0] || wcscmp(szMask, g_plugin.m_szUpdatePresetMask)) {
    // if old dir was "" or the dir changed, reset our search
    if (h != INVALID_HANDLE_VALUE)
      FindClose(h);
    h = INVALID_HANDLE_VALUE;
    g_plugin.m_bPresetListReady = false;
    lstrcpyW(g_plugin.m_szUpdatePresetMask, szMask);
    ZeroMemory(&fd, sizeof(fd));

    g_plugin.m_nPresets = 0;
    g_plugin.m_nDirs = 0;
    g_plugin.m_presets.clear();

    // find first .MILK file
    // if( (hFile = _findfirst(szMask, &c_file )) != -1L )		// note: returns filename -without- path
    if ((h = FindFirstFileW(g_plugin.m_szUpdatePresetMask, &fd)) == INVALID_HANDLE_VALUE)  // note: returns filename -without- path
    {
      // --> revert back to plugins dir
      wchar_t buf[1024];
      swprintf(buf, wasabiApiLangString(IDS_ERROR_NO_PRESET_FILES_OR_DIRS_FOUND_IN_X), g_plugin.m_szPresetDir);
      g_plugin.AddError(buf, 4.0f, ERR_MISC, true);

      if (bRetrying) {
        LeaveCriticalSection(&g_cs);
        g_bThreadAlive = false;
        _endthreadex(0);
        return 0;
      }

      g_plugin.FindValidPresetDir();

      bRetrying = true;
      goto retry;
    }

    // g_plugin.AddError(wasabiApiLangString(IDS_SCANNING_PRESETS), 8.0f, ERR_SCANNING_PRESETS, false);
  }

  if (g_plugin.m_bPresetListReady) {
    LeaveCriticalSection(&g_cs);
    g_bThreadAlive = false;
    _endthreadex(0);
    return 0;
  }

  int nMaxPSVersion = g_plugin.m_nMaxPSVersion;
  wchar_t szPresetDir[MAX_PATH];
  lstrcpyW(szPresetDir, g_plugin.m_szPresetDir);

  LeaveCriticalSection(&g_cs);

  PresetList temp_presets;
  int temp_nDirs = 0;
  int temp_nPresets = 0;

  // scan for the desired # of presets, this call...
  while (!g_bThreadShouldQuit && h != INVALID_HANDLE_VALUE) {
    bool bSkip = false;
    bool bIsDir = (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
    float fRating = 0;

    wchar_t szFilename[512];
    lstrcpyW(szFilename, fd.cFileName);

    if (bIsDir) {
      // skip "." directory
      if (wcscmp(fd.cFileName, L".") == 0)  // || lstrlen(ffd.cFileName) < 1)
        bSkip = true;
      else
        swprintf(szFilename, L"*%s", fd.cFileName);
    } else {
      // skip normal files not ending in ".milk" or ".milk2"
      int len = lstrlenW(fd.cFileName);
      bool bHasPresetExt = (len >= 6 && _wcsicmp(fd.cFileName + len - 6, L".milk2") == 0) || (len >= 5 && _wcsicmp(fd.cFileName + len - 5, L".milk") == 0);
      if (!bHasPresetExt)
        bSkip = true;

      // if it is .milk, make sure we know how to run its pixel shaders -
      // otherwise we don't want to show it in the preset list!
      if (!bSkip) {
        // If the first line of the file is not "MILKDROP_PRESET_VERSION XXX",
        //   then it's a MilkDrop 1 era preset, so it is definitely runnable. (no shaders)
        // Otherwise, check for the value "PSVERSION".  It will be 0, 2, or 3.
        //   If missing, assume it is 2.
        wchar_t szFullPath[MAX_PATH];
        swprintf(szFullPath, L"%s%s", szPresetDir, fd.cFileName);
        FILE* f = _wfopen(szFullPath, L"r");
        if (!f)
          bSkip = true;
        else {
#define PRESET_HEADER_SCAN_BYTES 160
          char szLine[PRESET_HEADER_SCAN_BYTES];
          char* p = szLine;

          int bytes_to_read = sizeof(szLine) - 1;
          int count = fread(szLine, bytes_to_read, 1, f);
          if (count < 1) {
            fseek(f, SEEK_SET, 0);
            count = fread(szLine, 1, bytes_to_read, f);
            szLine[count] = 0;
          } else
            szLine[bytes_to_read - 1] = 0;

          bool bScanForPreset00AndRating = false;
          bool bRatingKnown = false;

          // try to read the PSVERSION and the fRating= value.
          // most presets (unless hand-edited) will have these right at the top.
          // if not, [at least for fRating] use GetPrivateProfileFloat to search whole file.
          // read line 1
          // p = NextLine(p);//fgets(p, sizeof(p)-1, f);
          if (!strncmp(p, "MILKDROP_PRESET_VERSION", 23)) {
            p = NextLine(p);  // fgets(p, sizeof(p)-1, f);
            int ps_version = 2;
            if (p && !strncmp(p, "PSVERSION", 9)) {
              sscanf(&p[10], "%d", &ps_version);
              if (ps_version > nMaxPSVersion)
                bSkip = true;
              else {
                p = NextLine(p);  // fgets(p, sizeof(p)-1, f);
                bScanForPreset00AndRating = true;
              }
            }
          } else {
            // otherwise it's a MilkDrop 1 preset - we can run it.
            bScanForPreset00AndRating = true;
          }

          // scan up to 10 more lines in the file, looking for [preset00] and fRating=...
          // (this is WAY faster than GetPrivateProfileFloat, when it works!)
          int reps = (bScanForPreset00AndRating) ? 10 : 0;
          for (int z = 0; z < reps; z++) {
            if (p && !strncmp(p, "[preset00]", 10)) {
              p = NextLine(p);
              if (p && !strncmp(p, "fRating=", 8)) {
                _sscanf_l(&p[8], "%f", g_use_C_locale, &fRating);
                bRatingKnown = true;
                break;
              }
            }
            p = NextLine(p);
          }

          fclose(f);

          if (!bRatingKnown)
            fRating = GetPrivateProfileFloatW(L"preset00", L"fRating", 3.0f, szFullPath);
          fRating = max(0.0f, min(5.0f, fRating));
        }
      }
    }

    if (!bSkip) {
      float fPrevPresetRatingCum = 0;
      if (temp_nPresets > 0)
        fPrevPresetRatingCum += temp_presets[temp_nPresets - 1].fRatingCum;

      PresetInfo x;
      x.szFilename = szFilename;
      x.fRatingThis = fRating;
      x.fRatingCum = fPrevPresetRatingCum + fRating;
      temp_presets.push_back(x);

      temp_nPresets++;
      if (bIsDir)
        temp_nDirs++;
    }

    if (!FindNextFileW(h, &fd)) {
      FindClose(h);
      h = INVALID_HANDLE_VALUE;

      break;
    }

    // every so often, add some presets...
#define PRESET_UPDATE_INTERVAL 64
    if (temp_nPresets == 30 || ((temp_nPresets % PRESET_UPDATE_INTERVAL) == 0)) {
      EnterCriticalSection(&g_cs);

      // g_plugin.m_presets  = temp_presets;
      int curPreset = g_plugin.m_nPresets;
      while (!g_bThreadShouldQuit && curPreset < temp_nPresets) {
        g_plugin.m_presets.push_back(temp_presets[curPreset]);
        curPreset++;
      }
      g_plugin.m_nPresets = curPreset;
      g_plugin.m_nDirs = temp_nDirs;

      LeaveCriticalSection(&g_cs);
    }
  }

  if (g_bThreadShouldQuit) {
    // just abort... we are exiting the program or restarting the scan.
    g_bThreadAlive = false;
    _endthreadex(0);
    return 0;
  }

  EnterCriticalSection(&g_cs);

  // g_plugin.m_presets  = temp_presets;
  for (int i = g_plugin.m_nPresets; i < temp_nPresets; i++)
    g_plugin.m_presets.push_back(temp_presets[i]);
  g_plugin.m_nPresets = temp_nPresets;
  g_plugin.m_nDirs = temp_nDirs;
  g_plugin.m_bPresetListReady = true;

  if (g_plugin.m_bPresetListReady && g_plugin.m_nPresets == 0) {
    // no presets OR directories found - weird - but it happens.
    // --> revert back to plugins dir
    wchar_t buf[1024];
    swprintf(buf, wasabiApiLangString(IDS_ERROR_NO_PRESET_FILES_OR_DIRS_FOUND_IN_X), g_plugin.m_szPresetDir);
    g_plugin.AddError(buf, 4.0f, ERR_MISC, true);

    if (bRetrying) {
      LeaveCriticalSection(&g_cs);
      g_bThreadAlive = false;
      _endthreadex(0);
      return 0;
    }

    g_plugin.FindValidPresetDir();

    bRetrying = true;
    goto retry;
  }

  if (g_plugin.m_bPresetListReady) {
    g_plugin.MergeSortPresets(0, g_plugin.m_nPresets - 1);

    // update cumulative ratings, since order changed...
    g_plugin.m_presets[0].fRatingCum = g_plugin.m_presets[0].fRatingThis;
    for (int i = 0; i < g_plugin.m_nPresets; i++)
      g_plugin.m_presets[i].fRatingCum = i == 0 ? 0 : g_plugin.m_presets[i - 1].fRatingCum + g_plugin.m_presets[i].fRatingThis;

    // clear the "scanning presets" msg
    g_plugin.ClearErrors(ERR_SCANNING_PRESETS);

    // finally, try to re-select the most recently-used preset in the list
    g_plugin.m_nPresetListCurPos = 0;
    if (bTryReselectCurrentPreset) {
      if (g_plugin.m_szCurrentPresetFile[0]) {
        // try to automatically seek to the last preset loaded
        wchar_t* p = wcsrchr(g_plugin.m_szCurrentPresetFile, L'\\');
        p = (p) ? (p + 1) : g_plugin.m_szCurrentPresetFile;
        for (int i = g_plugin.m_nDirs; i < g_plugin.m_nPresets; i++) {
          if (wcscmp(p, g_plugin.m_presets[i].szFilename.c_str()) == 0) {
            g_plugin.m_nPresetListCurPos = i;
            break;
          }
        }
      }
    }
  }

  LeaveCriticalSection(&g_cs);

  g_bThreadAlive = false;
  _endthreadex(0);
  return 0;
}

void CPlugin::UpdatePresetList(bool bBackground, bool bForce, bool bTryReselectCurrentPreset) {
  // note: if dir changed, make sure bForce is true!

  if (bForce) {
    if (g_bThreadAlive)
      CancelThread(3000);  // flags it to exit; the param is the # of ms to wait before forcefully killing it
  } else {
    if (bBackground && (g_bThreadAlive || m_bPresetListReady))
      return;
    if (!bBackground && m_bPresetListReady)
      return;
  }

  assert(!g_bThreadAlive);

  // spawn new thread:
  DWORD flags = (bForce ? 1 : 0) | (bTryReselectCurrentPreset ? 2 : 0);
  g_bThreadShouldQuit = false;
  g_bThreadAlive = true;
  g_hThread = (HANDLE)_beginthreadex(NULL, 0, __UpdatePresetList, (void*)flags, 0, 0);

  if (!bBackground) {
    // crank up priority, wait for it to finish, and then return
    SetThreadPriority(g_hThread, THREAD_PRIORITY_HIGHEST);  // THREAD_PRIORITY_IDLE,    THREAD_PRIORITY_LOWEST,    THREAD_PRIORITY_NORMAL,    THREAD_PRIORITY_HIGHEST,

    // wait for it to finish
    while (g_bThreadAlive)
      Sleep(30);

    assert(g_hThread != INVALID_HANDLE_VALUE);
    CloseHandle(g_hThread);
    g_hThread = INVALID_HANDLE_VALUE;
  } else {
    // it will just run in the background til it finishes.
    // however, we want to wait until at least ~32 presets are found (or failure) before returning,
    // so we know we have *something* in the preset list to start with.

    SetThreadPriority(g_hThread, THREAD_PRIORITY_HIGHEST);  // THREAD_PRIORITY_IDLE,    THREAD_PRIORITY_LOWEST,    THREAD_PRIORITY_NORMAL,    THREAD_PRIORITY_HIGHEST,

    // wait until either the thread exits, or # of presets is >32, before returning.
    // also make sure you enter the CS whenever you check on it!
    // (thread will update preset list every so often, with the newest presets scanned in...)
    while (g_bThreadAlive) {
      Sleep(30);

      EnterCriticalSection(&g_cs);
      int nPresets = g_plugin.m_nPresets;
      LeaveCriticalSection(&g_cs);

      if (nPresets >= 30)
        break;
    }

    if (g_bThreadAlive) {
      // the load still takes a while even at THREAD_PRIORITY_ABOVE_NORMAL,
      // because it is waiting on the HDD so much...
      // but the OS is smart, and the CPU stays nice and zippy in other threads =)
      SetThreadPriority(g_hThread, THREAD_PRIORITY_HIGHEST);  // THREAD_PRIORITY_IDLE,    THREAD_PRIORITY_LOWEST,    THREAD_PRIORITY_NORMAL,    THREAD_PRIORITY_HIGHEST,
    }
  }

  return;
}

void CPlugin::MergeSortPresets(int left, int right) {
  // note: left..right range is inclusive
  int nItems = right - left + 1;

  if (nItems > 2) {
    // recurse to sort 2 halves (but don't actually recurse on a half if it only has 1 element)
    int mid = (left + right) / 2;
    /*if (mid   != left) */ MergeSortPresets(left, mid);
    /*if (mid+1 != right)*/ MergeSortPresets(mid + 1, right);

    // then merge results
    int a = left;
    int b = mid + 1;
    while (a <= mid && b <= right) {
      bool bSwap;

      // merge the sorted arrays; give preference to strings that start with a '*' character
      int nSpecial = 0;
      if (m_presets[a].szFilename.c_str()[0] == '*') nSpecial++;
      if (m_presets[b].szFilename.c_str()[0] == '*') nSpecial++;

      if (nSpecial == 1) {
        bSwap = (m_presets[b].szFilename.c_str()[0] == '*');
      } else {
        bSwap = (mystrcmpiW(m_presets[a].szFilename.c_str(), m_presets[b].szFilename.c_str()) > 0);
      }

      if (bSwap) {
        PresetInfo temp = m_presets[b];
        for (int k = b; k > a; k--)
          m_presets[k] = m_presets[k - 1];
        m_presets[a] = temp;
        mid++;
        b++;
      }
      a++;
    }
  } else if (nItems == 2) {
    // sort 2 items; give preference to 'special' strings that start with a '*' character
    int nSpecial = 0;
    if (m_presets[left].szFilename.c_str()[0] == '*') nSpecial++;
    if (m_presets[right].szFilename.c_str()[0] == '*') nSpecial++;

    if (nSpecial == 1) {
      if (m_presets[right].szFilename.c_str()[0] == '*') {
        PresetInfo temp = m_presets[left];
        m_presets[left] = m_presets[right];
        m_presets[right] = temp;
      }
    } else if (mystrcmpiW(m_presets[left].szFilename.c_str(), m_presets[right].szFilename.c_str()) > 0) {
      PresetInfo temp = m_presets[left];
      m_presets[left] = m_presets[right];
      m_presets[right] = temp;
    }
  }
}

void CPlugin::SavePresetAs(wchar_t* szNewFile) {
  // overwrites the file if it was already there,
  // so you should check if the file exists first & prompt user to overwrite,
  //   before calling this function

  if (!m_pState->Export(szNewFile)) {
    // error
    AddError(wasabiApiLangString(IDS_ERROR_UNABLE_TO_SAVE_THE_FILE), 6.0f, ERR_PRESET, true);
  } else {
    // pop up confirmation
    AddNotification(wasabiApiLangString(IDS_SAVE_SUCCESSFUL));

    // update m_pState->m_szDesc with the new name
    lstrcpyW(m_pState->m_szDesc, m_waitstring.szText);

    // refresh file listing
    UpdatePresetList(true, true);
  }
}

void CPlugin::DeletePresetFile(wchar_t* szDelFile) {
  // NOTE: this function additionally assumes that m_nPresetListCurPos indicates
  //		 the slot that the to-be-deleted preset occupies!

  // delete file
  if (!DeleteFileW(szDelFile)) {
    // error
    AddError(wasabiApiLangString(IDS_ERROR_UNABLE_TO_DELETE_THE_FILE), 6.0f, ERR_MISC, true);
  } else {
    // pop up confirmation
    wchar_t buf[1024];
    swprintf(buf, wasabiApiLangString(IDS_PRESET_X_DELETED), m_presets[m_nPresetListCurPos].szFilename.c_str());
    AddNotification(buf);

    // refresh file listing & re-select the next file after the one deleted
    int newPos = m_nPresetListCurPos;
    UpdatePresetList(true, true);
    m_nPresetListCurPos = max(0, min(m_nPresets - 1, newPos));
  }
}

void CPlugin::RenamePresetFile(wchar_t* szOldFile, wchar_t* szNewFile) {
  // NOTE: this function additionally assumes that m_nPresetListCurPos indicates
  //		 the slot that the to-be-renamed preset occupies!

  if (GetFileAttributesW(szNewFile) != -1)  // check if file already exists
  {
    // error
    AddError(wasabiApiLangString(IDS_ERROR_A_FILE_ALREADY_EXISTS_WITH_THAT_FILENAME), 6.0f, ERR_PRESET, true);

    // (user remains in UI_LOAD_RENAME mode to try another filename)
  } else {
    // rename
    if (!MoveFileW(szOldFile, szNewFile)) {
      // error
      AddError(wasabiApiLangString(IDS_ERROR_UNABLE_TO_RENAME_FILE), 6.0f, ERR_MISC, true);
    } else {
      // pop up confirmation
      AddError(wasabiApiLangString(IDS_RENAME_SUCCESSFUL), 3.0f, ERR_NOTIFY, false);

      // if this preset was the active one, update m_pState->m_szDesc with the new name
      wchar_t buf[512];
      swprintf(buf, L"%s.milk", m_pState->m_szDesc);
      if (wcscmp(m_presets[m_nPresetListCurPos].szFilename.c_str(), buf) == 0) {
        lstrcpyW(m_pState->m_szDesc, m_waitstring.szText);
      }

      // refresh file listing & do a trick to make it re-select the renamed file
      wchar_t buf2[512];
      lstrcpyW(buf2, m_waitstring.szText);
      lstrcatW(buf2, L".milk");
      m_presets[m_nPresetListCurPos].szFilename = buf2;
      UpdatePresetList(true, true, false);

      // jump to (highlight) the new file:
      m_nPresetListCurPos = 0;
      wchar_t* p = wcsrchr(szNewFile, L'\\');
      if (p) {
        p++;
        for (int i = m_nDirs; i < m_nPresets; i++) {
          if (wcscmp(p, m_presets[i].szFilename.c_str()) == 0) {
            m_nPresetListCurPos = i;
            break;
          }
        }
      }
    }

    // exit waitstring mode (return to load menu)
    m_UI_mode = UI_LOAD;
    m_waitstring.bActive = false;
  }
}

/*

void CPlugin::UpdatePresetRatings()
{
  if (!m_bEnableRating)
    return;

    if (m_nRatingReadProgress==-1 || m_nRatingReadProgress==m_nPresets)
        return;

  int k;

    if (m_nRatingReadProgress==0 && m_nDirs>0)
    {
      for (k=0; k<m_nDirs; k++)
      {
        m_presets[m_nRatingReadProgress].fRatingCum = 0.0f;
            m_nRatingReadProgress++;
      }

        if (!m_bInstaScan)
            return;
    }

    int presets_per_frame = m_bInstaScan ? 4096 : 1;
    int k1 = m_nRatingReadProgress;
    int k2 = min(m_nRatingReadProgress + presets_per_frame, m_nPresets);
  for (k=k1; k<k2; k++)
  {
    char szFullPath[512];
    sprintf(szFullPath, "%s%s", m_szPresetDir, m_presets[k].szFilename.c_str());
    float f = GetPrivateProfileFloat("preset00", "fRating", 3.0f, szFullPath);
    if (f < 0) f = 0;
    if (f > 5) f = 5;

    if (k==0)
      m_presets[k].fRatingCum = f;
    else
      m_presets[k].fRatingCum = m_presets[k-1].fRatingCum + f;

        m_nRatingReadProgress++;
  }
}
*/

void CPlugin::SetCurrentPresetRating(float fNewRating) {
  if (!m_bEnableRating)
    return;

  if (fNewRating < 0) fNewRating = 0;
  if (fNewRating > 5) fNewRating = 5;
  float change = (fNewRating - m_pState->m_fRating);

  // update the file on disk:
  // char szPresetFileNoPath[512];
  // char szPresetFileWithPath[512];
  // sprintf(szPresetFileNoPath,   "%s.milk", m_pState->m_szDesc);
  // sprintf(szPresetFileWithPath, "%s%s.milk", GetPresetDir(), m_pState->m_szDesc);
  WritePrivateProfileFloatW(fNewRating, L"fRating", m_szCurrentPresetFile, L"preset00");

  // update the copy of the preset in memory
  m_pState->m_fRating = fNewRating;

  // update the cumulative internal listing:
  m_presets[m_nCurrentPreset].fRatingThis += change;
  if (m_nCurrentPreset != -1)  // && m_nRatingReadProgress >= m_nCurrentPreset)		// (can be -1 if dir. changed but no new preset was loaded yet)
    for (int i = m_nCurrentPreset; i < m_nPresets; i++)
      m_presets[i].fRatingCum += change;

  /* keep in view:
    -test switching dirs w/o loading a preset, and trying to change the rating
      ->m_nCurrentPreset is out of range!
    -soln: when adjusting rating:
      1. file to modify is m_szCurrentPresetFile
      2. only update CDF if m_nCurrentPreset is not -1
    -> set m_nCurrentPreset to -1 whenever dir. changes
    -> set m_szCurrentPresetFile whenever you load a preset
  */

  // show a message
  if (!m_bShowRating) {
    // see also: DrawText() in milkdropfs.cpp
    m_fShowRatingUntilThisTime = GetTime() + 2.0f;
  }
}
