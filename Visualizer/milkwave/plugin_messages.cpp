// plugin_messages.cpp - Custom messages and song title animations
// Extracted from plugin.cpp

#include "plugin.h"
#include "plugin_internals.h"
#include "utility.h"
#include "support.h"
#include "resource.h"
#include "defines.h"
#include "shell_defines.h"
#include "wasabi.h"
#include <strsafe.h>

void CPlugin::ReadCustomMessages() {
  int n;

  // First, clear all old data
  for (n = 0; n < MAX_CUSTOM_MESSAGE_FONTS; n++) {
    wcscpy(m_CustomMessageFont[n].szFace, L"arial");
    m_CustomMessageFont[n].bBold = false;
    m_CustomMessageFont[n].bItal = false;
    m_CustomMessageFont[n].nColorR = 255;
    m_CustomMessageFont[n].nColorG = 255;
    m_CustomMessageFont[n].nColorB = 255;
  }

  for (n = 0; n < MAX_CUSTOM_MESSAGES; n++) {
    m_CustomMessage[n].szText[0] = 0;
    m_CustomMessage[n].nFont = 0;
    m_CustomMessage[n].fSize = 50.0f;  // [0..100]  note that size is not absolute, but relative to the size of the window
    m_CustomMessage[n].x = 0.5f;
    m_CustomMessage[n].y = 0.5f;
    m_CustomMessage[n].randx = 0;
    m_CustomMessage[n].randy = 0;
    m_CustomMessage[n].growth = 1.0f;
    m_CustomMessage[n].fTime = 1.5f;
    m_CustomMessage[n].fFade = 0.2f;
    m_CustomMessage[n].fFadeOut = 0.0f;

    m_CustomMessage[n].bOverrideBold = false;
    m_CustomMessage[n].bOverrideItal = false;
    m_CustomMessage[n].bOverrideFace = false;
    m_CustomMessage[n].bOverrideColorR = false;
    m_CustomMessage[n].bOverrideColorG = false;
    m_CustomMessage[n].bOverrideColorB = false;
    m_CustomMessage[n].bBold = false;
    m_CustomMessage[n].bItal = false;
    wcscpy(m_CustomMessage[n].szFace, L"arial");
    m_CustomMessage[n].nColorR = 255;
    m_CustomMessage[n].nColorG = 255;
    m_CustomMessage[n].nColorB = 255;
    m_CustomMessage[n].nRandR = 0;
    m_CustomMessage[n].nRandG = 0;
    m_CustomMessage[n].nRandB = 0;
  }

  // Then read in the new file
  for (n = 0; n < MAX_CUSTOM_MESSAGE_FONTS; n++) {
    wchar_t szSectionName[32];
    swprintf(szSectionName, L"font%02d", n);

    // get face, bold, italic, x, y for this custom message FONT
    GetPrivateProfileStringW(szSectionName, L"face", L"arial", m_CustomMessageFont[n].szFace, sizeof(m_CustomMessageFont[n].szFace), m_szMsgIniFile);
    m_CustomMessageFont[n].bBold = GetPrivateProfileBoolW(szSectionName, L"bold", m_CustomMessageFont[n].bBold, m_szMsgIniFile);
    m_CustomMessageFont[n].bItal = GetPrivateProfileBoolW(szSectionName, L"ital", m_CustomMessageFont[n].bItal, m_szMsgIniFile);
    m_CustomMessageFont[n].nColorR = GetPrivateProfileIntW(szSectionName, L"r", m_CustomMessageFont[n].nColorR, m_szMsgIniFile);
    m_CustomMessageFont[n].nColorG = GetPrivateProfileIntW(szSectionName, L"g", m_CustomMessageFont[n].nColorG, m_szMsgIniFile);
    m_CustomMessageFont[n].nColorB = GetPrivateProfileIntW(szSectionName, L"b", m_CustomMessageFont[n].nColorB, m_szMsgIniFile);
  }

  for (n = 0; n < MAX_CUSTOM_MESSAGES; n++) {
    wchar_t szSectionName[64];
    swprintf(szSectionName, L"message%02d", n);

    // get fontID, size, text, etc. for this custom message:
    GetPrivateProfileStringW(szSectionName, L"text", L"", m_CustomMessage[n].szText, sizeof(m_CustomMessage[n].szText), m_szMsgIniFile);
    if (m_CustomMessage[n].szText[0]) {
      m_CustomMessage[n].nFont = GetPrivateProfileIntW(szSectionName, L"font", m_CustomMessage[n].nFont, m_szMsgIniFile);
      m_CustomMessage[n].fSize = GetPrivateProfileFloatW(szSectionName, L"size", m_CustomMessage[n].fSize, m_szMsgIniFile);
      m_CustomMessage[n].x = GetPrivateProfileFloatW(szSectionName, L"x", m_CustomMessage[n].x, m_szMsgIniFile);
      m_CustomMessage[n].y = GetPrivateProfileFloatW(szSectionName, L"y", m_CustomMessage[n].y, m_szMsgIniFile);
      m_CustomMessage[n].randx = GetPrivateProfileFloatW(szSectionName, L"randx", m_CustomMessage[n].randx, m_szMsgIniFile);
      m_CustomMessage[n].randy = GetPrivateProfileFloatW(szSectionName, L"randy", m_CustomMessage[n].randy, m_szMsgIniFile);

      m_CustomMessage[n].growth = GetPrivateProfileFloatW(szSectionName, L"growth", m_CustomMessage[n].growth, m_szMsgIniFile);
      m_CustomMessage[n].fTime = GetPrivateProfileFloatW(szSectionName, L"time", m_CustomMessage[n].fTime, m_szMsgIniFile);

      m_CustomMessage[n].fFade = GetPrivateProfileFloatW(szSectionName, L"fade", m_MessageDefaultFadeinTime, m_szMsgIniFile);
      m_CustomMessage[n].fFadeOut = GetPrivateProfileFloatW(szSectionName, L"fadeout", m_MessageDefaultFadeoutTime, m_szMsgIniFile);
      m_CustomMessage[n].fBurnTime = GetPrivateProfileFloatW(szSectionName, L"burntime", m_MessageDefaultBurnTime, m_szMsgIniFile);

      m_CustomMessage[n].nColorR = GetPrivateProfileIntW(szSectionName, L"r", m_CustomMessage[n].nColorR, m_szMsgIniFile);
      m_CustomMessage[n].nColorG = GetPrivateProfileIntW(szSectionName, L"g", m_CustomMessage[n].nColorG, m_szMsgIniFile);
      m_CustomMessage[n].nColorB = GetPrivateProfileIntW(szSectionName, L"b", m_CustomMessage[n].nColorB, m_szMsgIniFile);
      m_CustomMessage[n].nRandR = GetPrivateProfileIntW(szSectionName, L"randr", m_CustomMessage[n].nRandR, m_szMsgIniFile);
      m_CustomMessage[n].nRandG = GetPrivateProfileIntW(szSectionName, L"randg", m_CustomMessage[n].nRandG, m_szMsgIniFile);
      m_CustomMessage[n].nRandB = GetPrivateProfileIntW(szSectionName, L"randb", m_CustomMessage[n].nRandB, m_szMsgIniFile);

      // overrides: r,g,b,face,bold,ital
      GetPrivateProfileStringW(szSectionName, L"face", L"", m_CustomMessage[n].szFace, sizeof(m_CustomMessage[n].szFace), m_szMsgIniFile);
      m_CustomMessage[n].bBold = GetPrivateProfileIntW(szSectionName, L"bold", -1, m_szMsgIniFile);
      m_CustomMessage[n].bItal = GetPrivateProfileIntW(szSectionName, L"ital", -1, m_szMsgIniFile);
      m_CustomMessage[n].nColorR = GetPrivateProfileIntW(szSectionName, L"r", -1, m_szMsgIniFile);
      m_CustomMessage[n].nColorG = GetPrivateProfileIntW(szSectionName, L"g", -1, m_szMsgIniFile);
      m_CustomMessage[n].nColorB = GetPrivateProfileIntW(szSectionName, L"b", -1, m_szMsgIniFile);

      m_CustomMessage[n].bOverrideFace = (m_CustomMessage[n].szFace[0] != 0);
      m_CustomMessage[n].bOverrideBold = (m_CustomMessage[n].bBold != -1);
      m_CustomMessage[n].bOverrideItal = (m_CustomMessage[n].bItal != -1);
      m_CustomMessage[n].bOverrideColorR = (m_CustomMessage[n].nColorR != -1);
      m_CustomMessage[n].bOverrideColorG = (m_CustomMessage[n].nColorG != -1);
      m_CustomMessage[n].bOverrideColorB = (m_CustomMessage[n].nColorB != -1);
    }
  }
}

void CPlugin::LaunchCustomMessage(int nMsgNum) {
  if (nMsgNum > 99)
    nMsgNum = 99;

  if (nMsgNum < 0) {
    int count = 0;
    // choose randomly
    for (nMsgNum = 0; nMsgNum < 100; nMsgNum++)
      if (m_CustomMessage[nMsgNum].szText[0])
        count++;

    int sel = (rand() % count) + 1;
    count = 0;
    for (nMsgNum = 0; nMsgNum < 100; nMsgNum++) {
      if (m_CustomMessage[nMsgNum].szText[0])
        count++;
      if (count == sel)
        break;
    }
  }

  if (nMsgNum < 0 ||
      nMsgNum >= MAX_CUSTOM_MESSAGES ||
      m_CustomMessage[nMsgNum].szText[0] == 0) {
    return;
  }

  int fontID = m_CustomMessage[nMsgNum].nFont;

  int nextFreeSupertextIndex = GetNextFreeSupertextIndex();
  if (nextFreeSupertextIndex > -1) {
    m_supertexts[nextFreeSupertextIndex].bRedrawSuperText = true;
    m_supertexts[nextFreeSupertextIndex].bIsSongTitle = false;
    lstrcpyW(m_supertexts[nextFreeSupertextIndex].szTextW, m_CustomMessage[nMsgNum].szText);

    // regular properties:
    m_supertexts[nextFreeSupertextIndex].fFontSize = m_CustomMessage[nMsgNum].fSize;
    m_supertexts[nextFreeSupertextIndex].fX = m_CustomMessage[nMsgNum].x + m_CustomMessage[nMsgNum].randx * ((rand() % 1037) / 1037.0f * 2.0f - 1.0f);
    m_supertexts[nextFreeSupertextIndex].fY = m_CustomMessage[nMsgNum].y + m_CustomMessage[nMsgNum].randy * ((rand() % 1037) / 1037.0f * 2.0f - 1.0f);
    m_supertexts[nextFreeSupertextIndex].fGrowth = m_CustomMessage[nMsgNum].growth;
    m_supertexts[nextFreeSupertextIndex].fDuration = m_CustomMessage[nMsgNum].fTime;
    m_supertexts[nextFreeSupertextIndex].fFadeInTime = m_CustomMessage[nMsgNum].fFade;
    m_supertexts[nextFreeSupertextIndex].fFadeOutTime = m_CustomMessage[nMsgNum].fFadeOut;
    m_supertexts[nextFreeSupertextIndex].fBurnTime = m_CustomMessage[nMsgNum].fBurnTime;

    // overrideables:
    if (m_CustomMessage[nMsgNum].bOverrideFace)
      lstrcpyW(m_supertexts[nextFreeSupertextIndex].nFontFace, m_CustomMessage[nMsgNum].szFace);
    else
      lstrcpyW(m_supertexts[nextFreeSupertextIndex].nFontFace, m_CustomMessageFont[fontID].szFace);
    m_supertexts[nextFreeSupertextIndex].bItal = (m_CustomMessage[nMsgNum].bOverrideItal) ? (m_CustomMessage[nMsgNum].bItal != 0) : (m_CustomMessageFont[fontID].bItal != 0);
    m_supertexts[nextFreeSupertextIndex].bBold = (m_CustomMessage[nMsgNum].bOverrideBold) ? (m_CustomMessage[nMsgNum].bBold != 0) : (m_CustomMessageFont[fontID].bBold != 0);
    m_supertexts[nextFreeSupertextIndex].nColorR = (m_CustomMessage[nMsgNum].bOverrideColorR) ? m_CustomMessage[nMsgNum].nColorR : m_CustomMessageFont[fontID].nColorR;
    m_supertexts[nextFreeSupertextIndex].nColorG = (m_CustomMessage[nMsgNum].bOverrideColorG) ? m_CustomMessage[nMsgNum].nColorG : m_CustomMessageFont[fontID].nColorG;
    m_supertexts[nextFreeSupertextIndex].nColorB = (m_CustomMessage[nMsgNum].bOverrideColorB) ? m_CustomMessage[nMsgNum].nColorB : m_CustomMessageFont[fontID].nColorB;

    // randomize color
    m_supertexts[nextFreeSupertextIndex].nColorR += (int)(m_CustomMessage[nMsgNum].nRandR * ((rand() % 1037) / 1037.0f * 2.0f - 1.0f));
    m_supertexts[nextFreeSupertextIndex].nColorG += (int)(m_CustomMessage[nMsgNum].nRandG * ((rand() % 1037) / 1037.0f * 2.0f - 1.0f));
    m_supertexts[nextFreeSupertextIndex].nColorB += (int)(m_CustomMessage[nMsgNum].nRandB * ((rand() % 1037) / 1037.0f * 2.0f - 1.0f));
    if (m_supertexts[nextFreeSupertextIndex].nColorR < 0) m_supertexts[nextFreeSupertextIndex].nColorR = 0;
    if (m_supertexts[nextFreeSupertextIndex].nColorG < 0) m_supertexts[nextFreeSupertextIndex].nColorG = 0;
    if (m_supertexts[nextFreeSupertextIndex].nColorB < 0) m_supertexts[nextFreeSupertextIndex].nColorB = 0;
    if (m_supertexts[nextFreeSupertextIndex].nColorR > 255) m_supertexts[nextFreeSupertextIndex].nColorR = 255;
    if (m_supertexts[nextFreeSupertextIndex].nColorG > 255) m_supertexts[nextFreeSupertextIndex].nColorG = 255;
    if (m_supertexts[nextFreeSupertextIndex].nColorB > 255) m_supertexts[nextFreeSupertextIndex].nColorB = 255;

    // fix &'s for display:
    /*
    {
      int pos = 0;
      int len = lstrlen(m_supertext[nextFreeSupertextIndex].szText);
      while (m_supertext[nextFreeSupertextIndex].szText[pos] && pos<255)
      {
        if (m_supertext[nextFreeSupertextIndex].szText[pos] == '&')
        {
          for (int x=len; x>=pos; x--)
            m_supertext[nextFreeSupertextIndex].szText[x+1] = m_supertext[nextFreeSupertextIndex].szText[x];
          len++;
          pos++;
        }
        pos++;
      }
    }*/

    m_supertexts[nextFreeSupertextIndex].fStartTime = GetTime();
  }
  // no free supertext slots available
  return;
}

void CPlugin::LaunchSongTitleAnim(int supertextIndex) {
  wchar_t debugMsg[128];
  swprintf(debugMsg, sizeof(debugMsg) / sizeof(debugMsg[0]), L"LaunchSongTitleAnim: supertextIndex=%d\n", supertextIndex);
  OutputDebugStringW(debugMsg);

  if (supertextIndex == -1) {
    supertextIndex = GetNextFreeSupertextIndex();
  }
  m_supertexts[supertextIndex].bRedrawSuperText = true;
  m_supertexts[supertextIndex].bIsSongTitle = true;
  lstrcpyW(m_supertexts[supertextIndex].szTextW, m_szSongTitle);
  // lstrcpy(m_supertext[supertextIndex].szText, " ");
  lstrcpyW(m_supertexts[supertextIndex].nFontFace, m_fontinfo[SONGTITLE_FONT].szFace);
  m_supertexts[supertextIndex].fFontSize = (float)m_fontinfo[SONGTITLE_FONT].nSize;
  m_supertexts[supertextIndex].bBold = m_fontinfo[SONGTITLE_FONT].bBold;
  m_supertexts[supertextIndex].bItal = m_fontinfo[SONGTITLE_FONT].bItalic;
  m_supertexts[supertextIndex].fX = 0.5f;
  m_supertexts[supertextIndex].fY = 0.5f;
  m_supertexts[supertextIndex].fGrowth = 1.0f;
  m_supertexts[supertextIndex].fDuration = m_fSongTitleAnimDuration;
  m_supertexts[supertextIndex].nColorR = 255;
  m_supertexts[supertextIndex].nColorG = 255;
  m_supertexts[supertextIndex].nColorB = 255;

  m_supertexts[supertextIndex].fStartTime = GetTime();
}

// Convert std::wstring to LPCWSTR
LPCWSTR ConvertToLPCWSTR(const std::wstring& wstr) {
  return wstr.c_str();
}

void CPlugin::LaunchMessage(wchar_t* sMessage) {
  if (wcsncmp(sMessage, L"MSG|", 4) == 0) {
    std::wstring message(sMessage + 4);  // Remove "MSG|"
    std::wstringstream ss(message);
    std::wstring token;
    std::map<std::wstring, std::wstring> params;

    // Parse key-value pairs
    while (std::getline(ss, token, L'|')) {
      size_t pos = token.find(L'=');
      if (pos != std::wstring::npos) {
        std::wstring key = token.substr(0, pos);
        std::wstring value = token.substr(pos + 1);
        params[key] = value;
      }
    }

    int nextFreeSupertextIndex = GetNextFreeSupertextIndex();
    // Set m_supertext properties
    if (params.find(L"text") != params.end()) {
      lstrcpyW(m_supertexts[nextFreeSupertextIndex].szTextW, ConvertToLPCWSTR(params[L"text"]));
    } else {
      return;  // 'text' parameter is required
    }

    m_supertexts[nextFreeSupertextIndex].bRedrawSuperText = true;
    m_supertexts[nextFreeSupertextIndex].bIsSongTitle = false;

    if (params.find(L"font") != params.end()) {
      lstrcpyW(m_supertexts[nextFreeSupertextIndex].nFontFace, ConvertToLPCWSTR(params[L"font"]));
    } else {
      // Default font
      lstrcpyW(m_supertexts[nextFreeSupertextIndex].nFontFace, L"Segoe UI");
    }

    if (params.find(L"size") != params.end()) {
      m_supertexts[nextFreeSupertextIndex].fFontSize = std::stof(params[L"size"]);
    } else {
      m_supertexts[nextFreeSupertextIndex].fFontSize = 30.0f;  // Default size
    }

    if (params.find(L"x") != params.end()) {
      m_supertexts[nextFreeSupertextIndex].fX = std::stof(params[L"x"]);
    } else {
      m_supertexts[nextFreeSupertextIndex].fX = 0.49f;  // Default x position
    }

    if (params.find(L"y") != params.end()) {
      m_supertexts[nextFreeSupertextIndex].fY = std::stof(params[L"y"]);
    } else {
      m_supertexts[nextFreeSupertextIndex].fY = 0.5f;  // Default y position
    }

    if (params.find(L"randx") != params.end()) {
      m_supertexts[nextFreeSupertextIndex].fX += std::stof(params[L"randx"]) * ((rand() % 1037) / 1037.0f * 2.0f - 1.0f);
    }

    if (params.find(L"randy") != params.end()) {
      m_supertexts[nextFreeSupertextIndex].fY += std::stof(params[L"randy"]) * ((rand() % 1037) / 1037.0f * 2.0f - 1.0f);
    }

    if (params.find(L"growth") != params.end()) {
      m_supertexts[nextFreeSupertextIndex].fGrowth = std::stof(params[L"growth"]);
    } else {
      m_supertexts[nextFreeSupertextIndex].fGrowth = 1.0f;  // Default growth
    }

    if (params.find(L"time") != params.end()) {
      m_supertexts[nextFreeSupertextIndex].fDuration = std::stof(params[L"time"]);
    } else {
      m_supertexts[nextFreeSupertextIndex].fDuration = 5.0f;  // Default duration
    }

    if (params.find(L"fade") != params.end()) {
      m_supertexts[nextFreeSupertextIndex].fFadeInTime = std::stof(params[L"fade"]);
    } else {
      m_supertexts[nextFreeSupertextIndex].fFadeInTime = m_MessageDefaultFadeinTime;
    }

    if (params.find(L"fadeout") != params.end()) {
      m_supertexts[nextFreeSupertextIndex].fFadeOutTime = std::stof(params[L"fadeout"]);
    } else {
      m_supertexts[nextFreeSupertextIndex].fFadeOutTime = m_MessageDefaultFadeoutTime;
    }

    if (params.find(L"bold") != params.end()) {
      m_supertexts[nextFreeSupertextIndex].bBold = std::stoi(params[L"bold"]);
    } else {
      m_supertexts[nextFreeSupertextIndex].bBold = 0;  // Default bold
    }

    if (params.find(L"ital") != params.end()) {
      m_supertexts[nextFreeSupertextIndex].bItal = std::stoi(params[L"ital"]);
    } else {
      m_supertexts[nextFreeSupertextIndex].bItal = 0;  // Default italic
    }

    if (params.find(L"r") != params.end()) {
      m_supertexts[nextFreeSupertextIndex].nColorR = std::stoi(params[L"r"]);
    } else {
      m_supertexts[nextFreeSupertextIndex].nColorR = 255;  // Default red color
    }

    if (params.find(L"g") != params.end()) {
      m_supertexts[nextFreeSupertextIndex].nColorG = std::stoi(params[L"g"]);
    } else {
      m_supertexts[nextFreeSupertextIndex].nColorG = 255;  // Default green color
    }

    if (params.find(L"b") != params.end()) {
      m_supertexts[nextFreeSupertextIndex].nColorB = std::stoi(params[L"b"]);
    } else {
      m_supertexts[nextFreeSupertextIndex].nColorB = 255;  // Default blue color
    }

    if (params.find(L"randr") != params.end()) {
      m_supertexts[nextFreeSupertextIndex].nColorR += (int)(std::stof(params[L"randr"]) * ((rand() % 1037) / 1037.0f * 2.0f - 1.0f));
    }

    if (params.find(L"randg") != params.end()) {
      m_supertexts[nextFreeSupertextIndex].nColorG += (int)(std::stof(params[L"randg"]) * ((rand() % 1037) / 1037.0f * 2.0f - 1.0f));
    }

    if (params.find(L"randb") != params.end()) {
      m_supertexts[nextFreeSupertextIndex].nColorB += (int)(std::stof(params[L"randb"]) * ((rand() % 1037) / 1037.0f * 2.0f - 1.0f));
    }

    if (m_supertexts[nextFreeSupertextIndex].nColorR < 0) m_supertexts[nextFreeSupertextIndex].nColorR = 0;
    if (m_supertexts[nextFreeSupertextIndex].nColorG < 0) m_supertexts[nextFreeSupertextIndex].nColorG = 0;
    if (m_supertexts[nextFreeSupertextIndex].nColorB < 0) m_supertexts[nextFreeSupertextIndex].nColorB = 0;
    if (m_supertexts[nextFreeSupertextIndex].nColorR > 255) m_supertexts[nextFreeSupertextIndex].nColorR = 255;
    if (m_supertexts[nextFreeSupertextIndex].nColorG > 255) m_supertexts[nextFreeSupertextIndex].nColorG = 255;
    if (m_supertexts[nextFreeSupertextIndex].nColorB > 255) m_supertexts[nextFreeSupertextIndex].nColorB = 255;

    if (params.find(L"startx") != params.end()) {
      m_supertexts[nextFreeSupertextIndex].fStartX = std::stof(params[L"startx"]);
    }

    if (params.find(L"starty") != params.end()) {
      m_supertexts[nextFreeSupertextIndex].fStartY = std::stof(params[L"starty"]);
    }

    if (params.find(L"movetime") != params.end()) {
      m_supertexts[nextFreeSupertextIndex].fMoveTime = std::stof(params[L"movetime"]);
    }

    if (params.find(L"easemode") != params.end()) {
      m_supertexts[nextFreeSupertextIndex].nEaseMode = std::stoi(params[L"easemode"]);
    }

    if (params.find(L"easefactor") != params.end()) {
      m_supertexts[nextFreeSupertextIndex].fEaseFactor = std::stof(params[L"easefactor"]);
    }

    if (params.find(L"shadowoffset") != params.end()) {
      m_supertexts[nextFreeSupertextIndex].fShadowOffset = std::stof(params[L"shadowoffset"]);
    }

    if (params.find(L"burntime") != params.end()) {
      m_supertexts[nextFreeSupertextIndex].fBurnTime = std::stof(params[L"burntime"]);
    } else {
      m_supertexts[nextFreeSupertextIndex].fBurnTime = m_MessageDefaultBurnTime;
    }

    if (params.find(L"box_alpha") != params.end()) {
      m_supertexts[nextFreeSupertextIndex].fBoxAlpha = std::stof(params[L"box_alpha"]);
    }
    if (params.find(L"box_col") != params.end()) {
      std::wstring colorStr = params[L"box_col"];
      std::wistringstream ss(colorStr);
      std::wstring token;
      std::vector<float> rgb;

      while (std::getline(ss, token, L',')) {
        try {
          rgb.push_back(std::stof(token));
        } catch (...) {
          rgb.push_back(0.0f);  // fallback if parsing fails
        }
      }

      if (rgb.size() == 3) {
        m_supertexts[nextFreeSupertextIndex].fBoxColR = rgb[0];
        m_supertexts[nextFreeSupertextIndex].fBoxColG = rgb[1];
        m_supertexts[nextFreeSupertextIndex].fBoxColB = rgb[2];
      }
    }

    if (params.find(L"box_left") != params.end()) {
      m_supertexts[nextFreeSupertextIndex].fBoxLeft = std::stof(params[L"box_left"]);
    }
    if (params.find(L"box_right") != params.end()) {
      m_supertexts[nextFreeSupertextIndex].fBoxRight = std::stof(params[L"box_right"]);
    }
    if (params.find(L"box_top") != params.end()) {
      m_supertexts[nextFreeSupertextIndex].fBoxTop = std::stof(params[L"box_top"]);
    }
    if (params.find(L"box_bottom") != params.end()) {
      m_supertexts[nextFreeSupertextIndex].fBoxBottom = std::stof(params[L"box_bottom"]);
    }

    m_supertexts[nextFreeSupertextIndex].fStartTime = GetTime();

    for (int i = 0; i < NUM_SUPERTEXTS; i++) {
      if (i != nextFreeSupertextIndex && m_supertexts[i].fStartTime != -1.0f && m_supertexts[i].fStartX == -100 && m_supertexts[i].fStartY == -100 && m_supertexts[i].fX == m_supertexts[nextFreeSupertextIndex].fX && m_supertexts[i].fY == m_supertexts[nextFreeSupertextIndex].fY) {
        // If the new supertext overlaps with an existing non-animated one, end it
        float fProgress = (GetTime() - m_supertexts[i].fStartTime) / m_supertexts[i].fDuration;
        // If text was growing, try keeping the current size
        if (m_supertexts[i].fGrowth != 1) {
          m_supertexts[i].fGrowth *= fProgress;
        }
        // set duration to the elapsed time, so burn-in is still applied
        m_supertexts[i].fDuration = GetTime() - m_supertexts[i].fStartTime;
      }
    }
  } else if (wcsncmp(sMessage, L"AMP|", 4) == 0) {
    // EQ message
    std::wstring message(sMessage + 4);  // Remove "AMP|"
    std::wstringstream ss(message);
    std::wstring token;
    std::map<std::wstring, std::wstring> params;
    // Parse key-value pairs
    while (std::getline(ss, token, L'|')) {
      size_t pos = token.find(L'=');
      if (pos != std::wstring::npos) {
        std::wstring key = token.substr(0, pos);
        std::wstring value = token.substr(pos + 1);
        params[key] = value;
      }
    }
    if (params.find(L"l") != params.end() && params.find(L"r") != params.end()) {
      // Convert the std::wstring to a float using std::stof
      try {
        milkwave_amp_left = std::stof(params[L"l"]);
        milkwave_amp_right = std::stof(params[L"r"]);
      } catch (const std::exception&) {
        // Handle the error if the conversion fails
        milkwave_amp_left = 1.0f;   // Default value
        milkwave_amp_right = 1.0f;  // Default value
      }
    }
  } else if (wcsncmp(sMessage, L"PRESET=", 7) == 0) {
    // Find the position of ".milk" in the string
    // wchar_t* pos = wcsstr(sMessage, L".milk");
    // if (pos) {
    //   // Keep everything up to and including ".milk"
    //   pos[5] = L'\0'; // Truncate the string after ".milk"
    // }
    std::wstring message(sMessage + 7);  // Remove "PRESET="

    size_t pos = message.find_last_of(L"\\/");
    std::wstring sPath;
    std::wstring sFilename;
    if (pos != std::wstring::npos) {
      // Extract the path up to and including the last separator
      sPath = message.substr(0, pos + 1);
      // Extract the filename after the last separator
      sFilename = message.substr(pos + 1);
    } else {
      // If no separator is found, assume the fullPath is just a filename
      sFilename = message;
    }

    if (sPath.length() > 0) {
      // Ensure 'sNewPath' is zero-terminated before using it in wcscmp
      wchar_t sNewPath[MAX_PATH];
      wcscpy_s(sNewPath, sPath.c_str());
      // ensure it is zero-terminated
      sNewPath[MAX_PATH - 1] = L'\0';
      if (wcscmp(sNewPath, g_plugin.m_szPresetDir) != 0) {
        g_plugin.ChangePresetDir(sNewPath, g_plugin.m_szPresetDir);
      }
    }

    // try to set the current preset index
    for (size_t i = 0; i < m_presets.size(); i++) {
      if (wcscmp(m_presets[i].szFilename.c_str(), sFilename.c_str()) == 0) {
        m_nCurrentPreset = (int)i;
        break;
      }
    }

    LoadPreset(message.c_str(), 1);
    // Handle other message types here if needed
  } else if (wcsncmp(sMessage, L"WAVE|", 5) == 0) {
    std::wstring message(sMessage + 5);
    SetWaveParamsFromMessage(message);
  } else if (wcsncmp(sMessage, L"DEVICE=", 7) == 0) {
    std::wstring message(sMessage + 7);
    int newRequestType = 0;
    if (wcsncmp(message.c_str(), L"IN|", 3) == 0) {
      message = message.substr(3);
      newRequestType = 1;
    } else if (wcsncmp(message.c_str(), L"OUT|", 4) == 0) {
      message = message.substr(4);
      newRequestType = 2;
    } else {
      newRequestType = 0;
    }
    m_nAudioDeviceRequestType = newRequestType;
    wcscpy_s(g_plugin.m_szAudioDevicePrevious, g_plugin.m_szAudioDevice);
    g_plugin.m_nAudioDevicePreviousType = g_plugin.m_nAudioDeviceActiveType;
    wcscpy(g_plugin.m_szAudioDevice, message.c_str());
    bool isRenderDevice = true;
    if (newRequestType == 1) {
      isRenderDevice = false;
    } else if (newRequestType == 2) {
      isRenderDevice = true;
    }
    g_plugin.SetAudioDeviceDisplayName(message.c_str(), isRenderDevice);
    // Restart audio
    m_nAudioLoopState = 1;
  } else if (wcsncmp(sMessage, L"VIDEOINPUT=", 11) == 0) {
    std::wstring message(sMessage + 11);
    size_t pos = message.find(L'|');
    if (pos != std::wstring::npos) {
      std::wstring enabledStr = message.substr(0, pos);
      std::wstring deviceName = message.substr(pos + 1);
      bool enabled = (enabledStr == L"1");

      // For now, just enable/disable - device selection could be added later
      EnableVideoMixing(enabled);
    }
  } else if (wcsncmp(sMessage, L"SPOUTINPUT=", 11) == 0) {
    std::wstring message(sMessage + 11);
    size_t pos = message.find(L'|');
    if (pos != std::wstring::npos) {
      std::wstring enabledStr = message.substr(0, pos);
      std::wstring senderName = message.substr(pos + 1);
      bool enabled = (enabledStr == L"1");

      // Set the sender name first
      if (!senderName.empty()) {
        SetSpoutSender(senderName.c_str());
      }

      // Then enable/disable mixing
      EnableSpoutMixing(enabled);
    }
  } else if (wcsncmp(sMessage, L"OPACITY=", 8) == 0) {
    std::wstring message(sMessage + 8);
    fOpacity = std::stof(message);
    SetOpacity(GetPluginWindow());
  } else if (wcsncmp(sMessage, L"STATE", 5) == 0) {
    int display = static_cast<int>(std::ceil(100 * fOpacity));
    wchar_t buf[1024];
    swprintf(buf, 64, L"Opacity: %d%%", display);  // Use %d for integers
    SendMessageToMilkwaveRemote((L"OPACITY=" + std::to_wstring(display)).c_str());
    SendPresetChangedInfoToMilkwaveRemote();
    SendSettingsInfoToMilkwaveRemote();
    if (m_nNumericInputMode == NUMERIC_INPUT_MODE_CUST_MSG) {
      PostMessageToMilkwaveRemote(WM_USER_MESSAGE_MODE);
    } else {
      PostMessageToMilkwaveRemote(WM_USER_SPRITE_MODE);
    }
  } else if (wcsncmp(sMessage, L"LINK=", 5) == 0) {
    std::wstring message(sMessage + 5);
    m_RemotePresetLink = std::stoi(message);
  } else if (wcsncmp(sMessage, L"QUICKSAVE", 9) == 0) {
    g_plugin.SaveCurrentPresetToQuicksave(false);
  } else if (wcsncmp(sMessage, L"CONFIG", 6) == 0) {
    ReadConfig();
    // to update fonts
    AllocateDX9Stuff();
    milkwave->logLevel = m_LogLevel;
    milkwave->LogEvent(L"Configuration reloaded: " + std::wstring(GetConfigIniFile()));
  } else if (wcsncmp(sMessage, L"SETTINGS", 8) == 0) {
    m_fTimeBetweenPresets = GetPrivateProfileFloatW(L"Settings", L"fTimeBetweenPresets", m_fTimeBetweenPresets, GetConfigIniFile());
    m_fPresetStartTime = GetTime();
    m_fNextPresetTime = -1.0f;  // force recalculation
  } else if (wcsncmp(sMessage, L"TESTFONTS", 9) == 0) {
    ClearErrors(ERR_MSG_BOTTOM_EXTRA_1);
    ClearErrors(ERR_MSG_BOTTOM_EXTRA_2);
    ClearErrors(ERR_MSG_BOTTOM_EXTRA_3);
    // Send text to appear at the bottom first, assuming a bottom corner is used
    g_plugin.AddError(L"Finally the Album", g_plugin.m_SongInfoDisplaySeconds, ERR_MSG_BOTTOM_EXTRA_3, false);
    g_plugin.AddError(L"Here goes the Title", g_plugin.m_SongInfoDisplaySeconds, ERR_MSG_BOTTOM_EXTRA_2, false);
    g_plugin.AddError(L"This is the Artist", g_plugin.m_SongInfoDisplaySeconds, ERR_MSG_BOTTOM_EXTRA_1, false);
    if (!g_plugin.m_bShowPresetInfo) g_plugin.m_bShowPresetInfo = true;
    g_plugin.AddNotification(L"This is a notification");
  } else if (wcsncmp(sMessage, L"CLEARPRESET", 11) == 0) {
    ClearPreset();
  } else if (wcsncmp(sMessage, L"CLEARSPRITES", 12) == 0) {
    g_plugin.KillAllSprites();
  } else if (wcsncmp(sMessage, L"CLEARTEXTS", 10) == 0) {
    g_plugin.KillAllSupertexts();
  } else if (wcsncmp(sMessage, L"VAR_TIME=", 9) == 0) {
    std::wstring message(sMessage + 9);
    g_plugin.m_timeFactor = std::stof(message);
  } else if (wcsncmp(sMessage, L"VAR_FRAME=", 10) == 0) {
    std::wstring message(sMessage + 10);
    g_plugin.m_frameFactor = std::stof(message);
  } else if (wcsncmp(sMessage, L"VAR_FPS=", 8) == 0) {
    std::wstring message(sMessage + 8);
    g_plugin.m_fpsFactor = std::stof(message);
  } else if (wcsncmp(sMessage, L"VAR_INTENSITY=", 14) == 0) {
    std::wstring message(sMessage + 14);
    g_plugin.m_VisIntensity = std::stof(message);
  } else if (wcsncmp(sMessage, L"VAR_SHIFT=", 10) == 0) {
    std::wstring message(sMessage + 10);
    g_plugin.m_VisShift = std::stof(message);
  } else if (wcsncmp(sMessage, L"VAR_VERSION=", 12) == 0) {
    std::wstring message(sMessage + 12);
    g_plugin.m_VisVersion = std::stof(message);
  } else if (wcsncmp(sMessage, L"COL_HUE=", 8) == 0) {
    std::wstring message(sMessage + 8);
    g_plugin.m_ColShiftHue = std::stof(message);
  } else if (wcsncmp(sMessage, L"HUE_AUTO=", 9) == 0) {
    g_plugin.m_AutoHue = (sMessage[9] == L'1');
  } else if (wcsncmp(sMessage, L"HUE_AUTO_SECONDS=", 17) == 0) {
    std::wstring message(sMessage + 17);
    g_plugin.m_AutoHueSeconds = std::stof(message);
  } else if (wcsncmp(sMessage, L"COL_SATURATION=", 15) == 0) {
    std::wstring message(sMessage + 15);
    g_plugin.m_ColShiftSaturation = std::stof(message);
  } else if (wcsncmp(sMessage, L"COL_BRIGHTNESS=", 15) == 0) {
    std::wstring message(sMessage + 15);
    g_plugin.m_ColShiftBrightness = std::stof(message);
  } else if (wcsncmp(sMessage, L"EQ_ATTACK=", 10) == 0) {
    std::wstring message(sMessage + 10);
    g_plugin.m_fEQAttackGlobal = max(0.0f, min(1.0f, std::stof(message)));
    if (g_plugin.m_pState)
      g_plugin.m_pState->m_fFFTAttack = g_plugin.m_fEQAttackGlobal;
    wchar_t buf[64];
    swprintf(buf, 64, L"EQ Attack: %.2f", g_plugin.m_fEQAttackGlobal);
    g_plugin.AddError(buf, 2.0f, ERR_NOTIFY, false);
  } else if (wcsncmp(sMessage, L"FFT_ATTACK=", 11) == 0) {
    std::wstring message(sMessage + 11);
    g_plugin.m_fEQAttackGlobal = max(0.0f, min(1.0f, std::stof(message)));
    if (g_plugin.m_pState)
      g_plugin.m_pState->m_fFFTAttack = g_plugin.m_fEQAttackGlobal;
    wchar_t buf[64];
    swprintf(buf, 64, L"EQ Attack: %.2f", g_plugin.m_fEQAttackGlobal);
    g_plugin.AddError(buf, 2.0f, ERR_NOTIFY, false);
  } else if (wcsncmp(sMessage, L"EQ_DECAY=", 9) == 0) {
    std::wstring message(sMessage + 9);
    g_plugin.m_fEQDecayGlobal = max(0.0f, min(1.0f, std::stof(message)));
    if (g_plugin.m_pState)
      g_plugin.m_pState->m_fFFTDecay = g_plugin.m_fEQDecayGlobal;
    wchar_t buf[64];
    swprintf(buf, 64, L"EQ Decay: %.2f", g_plugin.m_fEQDecayGlobal);
    g_plugin.AddError(buf, 2.0f, ERR_NOTIFY, false);
  } else if (wcsncmp(sMessage, L"FFT_DECAY=", 10) == 0) {
    std::wstring message(sMessage + 10);
    g_plugin.m_fEQDecayGlobal = max(0.0f, min(1.0f, std::stof(message)));
    if (g_plugin.m_pState)
      g_plugin.m_pState->m_fFFTDecay = g_plugin.m_fEQDecayGlobal;
    wchar_t buf[64];
    swprintf(buf, 64, L"EQ Decay: %.2f", g_plugin.m_fEQDecayGlobal);
    g_plugin.AddError(buf, 2.0f, ERR_NOTIFY, false);
  } else if (wcsncmp(sMessage, L"EQ_BOOST=", 9) == 0) {
    std::wstring message(sMessage + 9);
    g_plugin.m_fEQBoostGlobal = max(0.0f, min(9.99f, std::stof(message)));
    wchar_t buf[64];
    swprintf(buf, 64, L"EQ Boost: %.2f", g_plugin.m_fEQBoostGlobal);
    g_plugin.AddError(buf, 2.0f, ERR_NOTIFY, false);
  } else if (wcsncmp(sMessage, L"FFT_BOOST=", 10) == 0) {
    std::wstring message(sMessage + 10);
    g_plugin.m_fEQBoostGlobal = max(0.0f, min(9.99f, std::stof(message)));
    wchar_t buf[64];
    swprintf(buf, 64, L"EQ Boost: %.2f", g_plugin.m_fEQBoostGlobal);
    g_plugin.AddError(buf, 2.0f, ERR_NOTIFY, false);
  } else if (wcsncmp(sMessage, L"VAR_QUALITY=", 12) == 0) {
    std::wstring message(sMessage + 12);
    g_plugin.m_fRenderQuality = std::stof(message);
    ResetBufferAndFonts();
  } else if (wcsncmp(sMessage, L"VAR_AUTO=", 9) == 0) {
    g_plugin.bQualityAuto = (sMessage[9] == L'1');
    ResetBufferAndFonts();
  } else if (wcsncmp(sMessage, L"SPOUT_ACTIVE=", 13) == 0) {
    wchar_t status = sMessage[13];
    if ((status == L'0' && bSpoutOut) || (status == L'1' && !bSpoutOut)) {
      ToggleSpout();
    }
  } else if (wcsncmp(sMessage, L"SPOUT_FIXEDSIZE=", 16) == 0) {
    wchar_t status = sMessage[16];
    if ((status == L'0' && bSpoutFixedSize) || (status == L'1' && !bSpoutFixedSize)) {
      SetSpoutFixedSize(true, true);
    }
  } else if (wcsncmp(sMessage, L"SPOUT_RESOLUTION=", 17) == 0) {
    std::wstring message(sMessage + 17);
    size_t pos = message.find(L'x');
    if (pos != std::wstring::npos) {
      std::wstring width = message.substr(0, pos);
      std::wstring height = message.substr(pos + 1);
      nSpoutFixedWidth = std::stoi(width);
      nSpoutFixedHeight = std::stoi(height);
      SetSpoutFixedSize(false, true);
    }
  } else if (wcsncmp(sMessage, L"CAPTURE", 7) == 0) {
    OutputDebugStringW(L"[CAPTURE] Message received\n");
    milkwave->LogInfo(L"CAPTURE message received, calling CaptureScreenshot()");
    CaptureScreenshot();
    OutputDebugStringW(L"[CAPTURE] CaptureScreenshot() returned\n");
  } else if (wcsncmp(sMessage, L"PRECOMPILE_CACHE", 16) == 0) {
    milkwave->LogInfo(L"PRECOMPILE_CACHE message received");
    extern void StartSetupThread(bool manualTrigger);
    StartSetupThread(true);
  } else if (wcsncmp(sMessage, L"SEND=", 5) == 0) {
    // Keystroke sent via pipe
    std::wstring val(sMessage + 5);
    HWND hWnd = GetPluginWindow();
    if (hWnd && !val.empty()) {
      if (val.length() >= 2 && val[0] == L'0' && (val[1] == L'x' || val[1] == L'X')) {
        // Hex keycode (e.g. "0x73" for F4)
        int vkCode = (int)wcstol(val.c_str(), nullptr, 16);
        if (vkCode > 0) {
          PostMessageW(hWnd, WM_KEYDOWN, (WPARAM)vkCode, 0);
          // For character keys, also post WM_CHAR since HandleRegularKey
          // processes WM_CHAR, not WM_KEYDOWN. PostMessage bypasses
          // TranslateMessage which normally generates WM_CHAR.
          if ((vkCode >= 0x30 && vkCode <= 0x39) ||  // 0-9
              (vkCode >= 0x41 && vkCode <= 0x5A) ||  // A-Z
              vkCode == 0x20)                        // Space
            PostMessageW(hWnd, WM_CHAR, (WPARAM)vkCode, 0);
          PostMessageW(hWnd, WM_KEYUP, (WPARAM)vkCode, 0);
        }
      } else {
        // Send as WM_CHAR sequence (e.g. "00" for sprite trigger, "~" for tilde)
        for (wchar_t ch : val)
          PostMessageW(hWnd, WM_CHAR, (WPARAM)ch, 0);
      }
    }
  }
}
