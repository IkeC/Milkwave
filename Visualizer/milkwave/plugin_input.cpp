// plugin_input.cpp - Window procedure and keyboard input handling
// Extracted from plugin.cpp

#include "plugin.h"
#include "plugin_internals.h"
#include "utility.h"
#include "support.h"
#include "resource.h"
#include "defines.h"
#include "shell_defines.h"
#include "wasabi.h"
#include <shellapi.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

void CPlugin::ToggleAlwaysOnTop(HWND hwnd) {
  RECT rect;
  GetWindowRect(hwnd, &rect);
  int x = rect.left;
  int y = rect.top;
  int width = rect.right - rect.left;
  int height = rect.bottom - rect.top;

  if (m_bAlwaysOnTop) {
    SetWindowPos(hwnd, HWND_TOPMOST, x, y, width, height, SWP_DRAWFRAME | SWP_FRAMECHANGED);
  } else {
    SetWindowPos(hwnd, HWND_NOTOPMOST, x, y, width, height, SWP_DRAWFRAME | SWP_FRAMECHANGED);
  }
}

void ToggleTransparency(HWND hwnd) {
  RECT rect;
  GetWindowRect(hwnd, &rect);
  int x = rect.left;
  int y = rect.top;
  int width = rect.right - rect.left;
  int height = rect.bottom - rect.top;

  LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);

  // Enable the layered window attribute without affecting other styles
  exStyle |= WS_EX_LAYERED;
  SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle);

  SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_DRAWFRAME | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);  // Redraws the window to fix the transparency mode issue for Windows 7, 8 and 8.1.
  if (TranspaMode) {
    // Disable window transition animations while in transparent/color-key mode
    BOOL fDisable = TRUE;
    DwmSetWindowAttribute(hwnd, DWMWA_TRANSITIONS_FORCEDISABLED, &fDisable, sizeof(fDisable));
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 255, LWA_COLORKEY);
    g_plugin.fOpacity = 1.0f;
    DragAcceptFiles(hwnd, TRUE);
  } else {
    // Re-enable window transition animations when leaving transparent mode
    BOOL fDisable = FALSE;
    DwmSetWindowAttribute(hwnd, DWMWA_TRANSITIONS_FORCEDISABLED, &fDisable, sizeof(fDisable));
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 255, LWA_ALPHA);
    DragAcceptFiles(hwnd, TRUE);
  }
}

void CPlugin::SetOpacity(HWND hwnd) {
  if (IsBorderlessFullscreen(hwnd)) {
    g_plugin.m_WindowWatermarkModeOpacity = fOpacity;
  }

  // Retrieve the current extended window style
  LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);

  // Check if the window is currently in clickthrough mode
  bool isClickthrough = (exStyle & WS_EX_TRANSPARENT) != 0;

  // Ensure the window is layered (required for transparency)
  if (!(exStyle & WS_EX_LAYERED)) {
    exStyle |= WS_EX_LAYERED;
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle);
  }

  // Set the new opacity
  BYTE alpha = static_cast<BYTE>(fOpacity * 255);  // Convert opacity (0.0 to 1.0) to alpha (0 to 255)
  if (!SetLayeredWindowAttributes(hwnd, 0, alpha, LWA_ALPHA)) {
    DWORD error = GetLastError();
    printf("Failed to set window opacity. Error: %lu\n", error);
  }

  // Modify the clickthrough state
  if (isClickthrough) {
    exStyle |= WS_EX_TRANSPARENT;
  } else {
    exStyle &= ~WS_EX_TRANSPARENT;
  }

  // Reapply the extended window styles
  SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle);

  // Reapply the alpha value after modifying the extended styles
  if (!SetLayeredWindowAttributes(hwnd, 0, alpha, LWA_ALPHA)) {
    DWORD error = GetLastError();
    printf("Failed to reapply window opacity. Error: %lu\n", error);
  }

  int display = static_cast<int>(std::ceil(100 * fOpacity));
  wchar_t buf[1024];
  swprintf(buf, 64, L"Opacity: %d%%", display);  // Use %d for integers
  g_plugin.AddNotification(buf);

  SendMessageToMilkwaveRemote((L"OPACITY=" + std::to_wstring(display)).c_str());
}

void ToggleWindowOpacity(HWND hwnd, bool bDown) {
  RECT rect;
  GetWindowRect(hwnd, &rect);
  int x = rect.left;
  int y = rect.top;
  int width = rect.right - rect.left;
  int height = rect.bottom - rect.top;

  float changeVal = 0.1f;
  if (g_plugin.fOpacity < 0.09 || (g_plugin.fOpacity <= 0.1 && bDown)) {
    changeVal = 0.01f;
  } else {
    changeVal = 0.05f;
  }
  if (bDown) {
    g_plugin.fOpacity -= changeVal;
  } else {
    g_plugin.fOpacity += changeVal;
  }

  if (g_plugin.fOpacity < 0.01f)
    g_plugin.fOpacity = 0.01f;
  else if (g_plugin.fOpacity > 1.0f)
    g_plugin.fOpacity = 1.0f;

  // Set the opacity of the window
  g_plugin.SetOpacity(hwnd);
}

bool CPlugin::IsBorderlessFullscreen(HWND hWnd) {
  // Check if the window is borderless fullscreen
  RECT workArea;
  MONITORINFO monitorInfo = {sizeof(MONITORINFO)};
  HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
  if (GetMonitorInfo(hMonitor, &monitorInfo)) {
    workArea = monitorInfo.rcWork;
  }
  RECT currentRect;
  GetWindowRect(hWnd, &currentRect);
  return (currentRect.left == workArea.left &&
          currentRect.top == workArea.top &&
          currentRect.right == workArea.right &&
          currentRect.bottom == workArea.bottom);
}

void LoadPresetFilesViaDragAndDrop(WPARAM wParam) {
#ifdef UNICODE
  TCHAR szDroppedPresetName[MAX_PATH];  // Unicode string
#else
  TCHAR szDroppedPresetName[MAX_PATH];  // ANSI string
#endif

  // TCHAR szDroppedPresetName[MAX_PATH];
  HDROP hDrop = (HDROP)wParam;

  int count = DragQueryFile(hDrop, 0xFFFFFFFF, szDroppedPresetName, 0);

  // int len = MultiByteToWideChar(MB_PRECOMPOSED, 0, szDroppedPresetName, -1, NULL, 0);
  // wchar_t* wConvertedDroppedPresetName = new wchar_t[len];
  // MultiByteToWideChar(MB_PRECOMPOSED, 0, szDroppedPresetName, -1, wConvertedDroppedPresetName, len);
  // int len2 = lstrlenW(wConvertedDroppedPresetName);

  for (int i = 0; i < count; i++) {
    DragQueryFile(hDrop, i, szDroppedPresetName, MAX_PATH);
  }

  // ChatGPT
#ifdef UNICODE
  // No conversion needed for Unicode build
  const wchar_t* convertedFileName = szDroppedPresetName;
#else
  // Convert ANSI string to Unicode
  wchar_t convertedFileName[MAX_PATH];
  MultiByteToWideChar(CP_ACP, 0, szDroppedPresetName, -1, convertedFileName, MAX_PATH);
#endif

  const wchar_t* extension = wcsrchr(convertedFileName, L'.');
  if (extension && (_wcsicmp(extension, L".milk") == 0 || _wcsicmp(extension, L".milk2") == 0)) {
    wchar_t presetDir[MAX_PATH];
    lstrcpyW(presetDir, convertedFileName);
    wchar_t* lastSlash = wcsrchr(presetDir, L'\\');
    wchar_t* lastForwardSlash = wcsrchr(presetDir, L'/');
    if (!lastSlash || (lastForwardSlash && lastForwardSlash > lastSlash))
      lastSlash = lastForwardSlash;
    if (lastSlash) {
      *lastSlash = L'\0';
      g_plugin.ChangePresetDir(presetDir, g_plugin.m_szPresetDir, false);

      wchar_t* fileName = wcsrchr(convertedFileName, L'\\');
      wchar_t* fileNameForwardSlash = wcsrchr(convertedFileName, L'/');
      if (!fileName || (fileNameForwardSlash && fileNameForwardSlash > fileName))
        fileName = fileNameForwardSlash;
      if (fileName)
        fileName++;
      else
        fileName = const_cast<wchar_t*>(convertedFileName);

      g_plugin.m_nPresetListCurPos = -1;
      g_plugin.m_nCurrentPreset = -1;
      for (int i = g_plugin.m_nDirs; i < g_plugin.m_nPresets; i++) {
        if (g_plugin.m_presets[i].szFilename.c_str()[0] == '*')
          continue;
        if (_wcsicmp(fileName, g_plugin.m_presets[i].szFilename.c_str()) == 0) {
          g_plugin.m_nPresetListCurPos = i;
          g_plugin.m_nCurrentPreset = i;
          break;
        }
      }
    }
    g_plugin.LoadPreset(convertedFileName, 0.0f);
  } else {
    wchar_t buf[1024];
    swprintf(buf, 1024, L"Error: Failed to load dropped preset file: %s", convertedFileName);
    g_plugin.AddError(buf, 5.0f, ERR_NOTIFY, true);
  }
  DragFinish(hDrop);
}
//----------------------------------------------------------------------

LRESULT CPlugin::MyWindowProc(HWND hWnd, unsigned uMsg, WPARAM wParam, LPARAM lParam) {
  // Debug: Log all WM_USER messages for input mixing
  if (uMsg >= WM_USER && uMsg < WM_USER + 200) {
    if (milkwave) {
      wchar_t buf[256];
      swprintf_s(buf, L"[MSG] uMsg=0x%04X, wP=%Id, lP=%Id", uMsg, (size_t)wParam, (size_t)lParam);
      milkwave->LogInfo(buf);
    }
  }

  USHORT mask = 1 << (sizeof(SHORT) * 8 - 1);
  bool bShiftHeldDown = (GetKeyState(VK_SHIFT) & mask) != 0;
  bool bCtrlHeldDown = (GetKeyState(VK_CONTROL) & mask) != 0;

  int nRepeat = 1;  // updated as appropriate
  int rep;

  switch (uMsg) {
    // WM_COPYDATA removed â€” replaced by named pipe IPC (pipe_server.h)

    // Handle WM_ENABLE_SPOUT_MIX via PostMessage (not WM_COPYDATA)
    case WM_USER_ENABLE_SPOUT_MIX: {
      BOOL bEnable = (BOOL)wParam;
      g_plugin.EnableSpoutMixing(bEnable ? true : false);
      return 0;
    }

    // Handle WM_SET_VIDEO_DEVICE via PostMessage
    case WM_USER_SET_VIDEO_DEVICE: {
      int deviceIndex = (int)wParam;
      g_plugin.SetVideoDevice(deviceIndex);
      return 0;
    }

    // Handle WM_ENABLE_VIDEO_MIX via PostMessage
    case WM_USER_ENABLE_VIDEO_MIX: {
      BOOL bEnable = (BOOL)wParam;
      g_plugin.EnableVideoMixing(bEnable ? true : false);
      return 0;
    }

    // Handle layer position (Top/Background) via PostMessage
    case WM_USER_SET_INPUTMIX_ONTOP: {
      this->SetInputMixOnTop(wParam != 0);
      return 0;
    }

    // Handle Input Mix Opacity (0..100 -> 0.0..1.0)
    case WM_USER_SET_INPUTMIX_OPACITY: {
      this->SetInputMixOpacity((float)wParam / 100.0f);
      wchar_t buf[64];
      swprintf_s(buf, L"Opacity: %d%%", (int)wParam);
      AddNotification(buf, 1.0f);
      return 0;
    }

    // Handle Input Mix Tint Color via PostMessage
    case WM_USER_SET_INPUTMIX_TINT: {
      this->m_cInputMixTint = (D3DCOLOR)wParam;
      return 0;
    }

    // Handle Input Mix Luma Key
    case WM_USER_SET_INPUTMIX_LUMAKEY: {
      int thresholdInt = (int)wParam;
      int softnessInt = (int)lParam;

      if (milkwave) {
        wchar_t buf[256];
        swprintf_s(buf, L"LumaMsg REC: thr=%d, soft=%d", thresholdInt, softnessInt);
        milkwave->LogInfo(buf);
      }

      if (thresholdInt >= 0) {
        m_bInputMixLumaActive = true;
        m_fInputMixLumakeyThreshold = (float)thresholdInt / 100.0f;
      } else {
        m_bInputMixLumaActive = false;
      }
      m_fInputMixLumakeySoftness = (float)lParam / 100.0f;

      if (m_bInputMixLumaActive)
        AddNotification(L"Luma Key On", 2.0f);
      else
        AddNotification(L"Luma Key Off", 2.0f);
      return 0;
    }

    case WM_COMMAND:

    case WM_CHAR:  // plain & simple alphanumeric keys
      nRepeat = LOWORD(lParam);
      if (m_waitstring.bActive)  // if user is in the middle of editing a string
      {
        if ((wParam >= ' ' && wParam <= 'z') || wParam == '{' || wParam == '}') {
          int len;
          if (m_waitstring.bDisplayAsCode)
            len = lstrlenA((char*)m_waitstring.szText);
          else
            len = lstrlenW(m_waitstring.szText);

          if (m_waitstring.bFilterBadChars &&
              (wParam == '\"' ||
               wParam == '\\' ||
               wParam == '/' ||
               wParam == ':' ||
               wParam == '*' ||
               wParam == '?' ||
               wParam == '|' ||
               wParam == '<' ||
               wParam == '>' ||
               wParam == '&'))  // NOTE: '&' is legal in filenames, but we try to avoid it since during GDI display it acts as a control code (it will not show up, but instead, underline the character following it).
          {
            // illegal char
            AddError(wasabiApiLangString(IDS_ILLEGAL_CHARACTER), 2.5f, ERR_MISC, true);
          } else if (len + nRepeat >= m_waitstring.nMaxLen) {
            // m_waitstring.szText has reached its limit
            AddError(wasabiApiLangString(IDS_STRING_TOO_LONG), 2.5f, ERR_MISC, true);
          } else {
            // m_fShowUserMessageUntilThisTime = GetTime();	// if there was an error message already, clear it

            if (m_waitstring.bDisplayAsCode) {
              char buf[16];
              sprintf(buf, "%c", wParam);

              if (m_waitstring.nSelAnchorPos != -1)
                WaitString_NukeSelection();

              if (m_waitstring.bOvertypeMode) {
                // overtype mode
                for (rep = 0; rep < nRepeat; rep++) {
                  if (m_waitstring.nCursorPos == len) {
                    lstrcatA((char*)m_waitstring.szText, buf);
                    len++;
                  } else {
                    char* ptr = (char*)m_waitstring.szText;
                    *(ptr + m_waitstring.nCursorPos) = buf[0];
                  }
                  m_waitstring.nCursorPos++;
                }
              } else {
                // insert mode:
                char* ptr = (char*)m_waitstring.szText;
                for (rep = 0; rep < nRepeat; rep++) {
                  for (int i = len; i >= m_waitstring.nCursorPos; i--)
                    *(ptr + i + 1) = *(ptr + i);
                  *(ptr + m_waitstring.nCursorPos) = buf[0];
                  m_waitstring.nCursorPos++;
                  len++;
                }
              }
            } else {
              wchar_t buf[16];
              swprintf(buf, L"%c", wParam);

              if (m_waitstring.nSelAnchorPos != -1)
                WaitString_NukeSelection();

              if (m_waitstring.bOvertypeMode) {
                // overtype mode
                for (rep = 0; rep < nRepeat; rep++) {
                  if (m_waitstring.nCursorPos == len) {
                    lstrcatW(m_waitstring.szText, buf);
                    len++;
                  } else
                    m_waitstring.szText[m_waitstring.nCursorPos] = buf[0];
                  m_waitstring.nCursorPos++;
                }
              } else {
                // insert mode:
                for (rep = 0; rep < nRepeat; rep++) {
                  for (int i = len; i >= m_waitstring.nCursorPos; i--)
                    m_waitstring.szText[i + 1] = m_waitstring.szText[i];
                  m_waitstring.szText[m_waitstring.nCursorPos] = buf[0];
                  m_waitstring.nCursorPos++;
                  len++;
                }
              }
            }
          }
        }
        return 0;                           // we processed (or absorbed) the key
      } else if (m_UI_mode == UI_LOAD_DEL)  // waiting to confirm file delete
      {
        if (wParam == 'y' || wParam == 'Y')  // 'y' or 'Y'
        {
          // first add pathname to filename
          wchar_t szDelFile[512];
          swprintf(szDelFile, L"%s%s", GetPresetDir(), m_presets[m_nPresetListCurPos].szFilename.c_str());

          DeletePresetFile(szDelFile);
          // m_nCurrentPreset = -1;
        }

        m_UI_mode = UI_LOAD;

        return 0;  // we processed (or absorbed) the key
      } else if (m_UI_mode == UI_UPGRADE_PIXEL_SHADER) {
        if (wParam == 'y' || wParam == 'Y')  // 'y' or 'Y'
        {
          if (m_pState->m_nMinPSVersion == m_pState->m_nMaxPSVersion) {
            switch (m_pState->m_nMinPSVersion) {
              case MD2_PS_NONE:
                m_pState->m_nWarpPSVersion = MD2_PS_2_0;
                m_pState->m_nCompPSVersion = MD2_PS_2_0;
                m_pState->GenDefaultWarpShader();
                m_pState->GenDefaultCompShader();
                break;
              case MD2_PS_2_0:
                m_pState->m_nWarpPSVersion = MD2_PS_2_X;
                m_pState->m_nCompPSVersion = MD2_PS_2_X;
                break;
              case MD2_PS_2_X:
                m_pState->m_nWarpPSVersion = MD2_PS_3_0;
                m_pState->m_nCompPSVersion = MD2_PS_3_0;
                break;
              default:
                assert(0);
                break;
            }
          } else {
            switch (m_pState->m_nMinPSVersion) {
              case MD2_PS_NONE:
                if (m_pState->m_nWarpPSVersion < MD2_PS_2_0) {
                  m_pState->m_nWarpPSVersion = MD2_PS_2_0;
                  m_pState->GenDefaultWarpShader();
                }
                if (m_pState->m_nCompPSVersion < MD2_PS_2_0) {
                  m_pState->m_nCompPSVersion = MD2_PS_2_0;
                  m_pState->GenDefaultCompShader();
                }
                break;
              case MD2_PS_2_0:
                m_pState->m_nWarpPSVersion = max(m_pState->m_nWarpPSVersion, MD2_PS_2_X);
                m_pState->m_nCompPSVersion = max(m_pState->m_nCompPSVersion, MD2_PS_2_X);
                break;
              case MD2_PS_2_X:
                m_pState->m_nWarpPSVersion = max(m_pState->m_nWarpPSVersion, MD2_PS_3_0);
                m_pState->m_nCompPSVersion = max(m_pState->m_nCompPSVersion, MD2_PS_3_0);
                break;
              default:
                assert(0);
                break;
            }
          }
          m_pState->m_nMinPSVersion = min(m_pState->m_nWarpPSVersion, m_pState->m_nCompPSVersion);
          m_pState->m_nMaxPSVersion = max(m_pState->m_nWarpPSVersion, m_pState->m_nCompPSVersion);

          LoadShaders(&m_shaders, m_pState, false, false);
          SetMenusForPresetVersion(m_pState->m_nWarpPSVersion, m_pState->m_nCompPSVersion);
        }
        if (wParam != 13)
          m_UI_mode = UI_MENU;
        return 0;                                 // we processed (or absorbed) the key
      } else if (m_UI_mode == UI_SAVE_OVERWRITE)  // waiting to confirm overwrite file on save
      {
        if (wParam == 'y' || wParam == 'Y')  // 'y' or 'Y'
        {
          // first add pathname + extension to filename
          wchar_t szNewFile[512];
          swprintf(szNewFile, L"%s%s.milk", GetPresetDir(), m_waitstring.szText);

          SavePresetAs(szNewFile);

          // exit waitstring mode
          m_UI_mode = UI_REGULAR;
          m_waitstring.bActive = false;
          // m_bPresetLockedByCode = false;
        } else if ((wParam >= ' ' && wParam <= 'z') || wParam == 27)  // 27 is the ESCAPE key
        {
          // go back to SAVE AS mode
          m_UI_mode = UI_SAVEAS;
          m_waitstring.bActive = true;
        }

        return 0;  // we processed (or absorbed) the key
      } else       // normal handling of a simple key (all non-virtual-key hotkeys end up here)
      {
        if (HandleRegularKey(wParam) == 0)
          return 0;
      }
      return 1;  // end case WM_CHAR

      // Handle other messages here...

    case WM_MOUSEWHEEL:

      if (GET_WHEEL_DELTA_WPARAM(wParam) < 0 && !m_bPresetLockedByCode)
        if (bShiftHeldDown)
          ToggleWindowOpacity(hWnd, true);
        else
          NextPreset(0);

      else if (GET_WHEEL_DELTA_WPARAM(wParam) > 0 && !m_bPresetLockedByCode)
        if (bShiftHeldDown)
          ToggleWindowOpacity(hWnd, false);
        else
          PrevPreset(0);

      return 0;

    case WM_CREATE:
      DragAcceptFiles(hWnd, TRUE);
      return 0;

    case WM_DROPFILES:
      LoadPresetFilesViaDragAndDrop(wParam);
      return 0;

      // case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
      m_mouseDown = 1;
      m_mouseClicked = 2;  // no. of frames you set when you click (not to be confused with mouse held down)
      m_lastMouseX = m_mouseX;
      m_lastMouseY = -m_mouseY + 1;
      break;

      // case WM_LBUTTONUP:
    case WM_RBUTTONUP:
      m_mouseDown = 0;
      break;

    case WM_KEYDOWN:  // virtual-key codes

      // Note that some keys will never reach this point, since they are
      //   intercepted by the plugin shell (see PluginShellWindowProc(),
      //   at the end of pluginshell.cpp for which ones).
      // For a complete list of virtual-key codes, look up the keyphrase
      //   "virtual-key codes [win32]" in the msdn help.
      nRepeat = LOWORD(lParam);

      // SPOUT DEBUG
      // Special case for F1 help display in pluginshell
      // to clear the vj screen of any existing text
      if (wParam == VK_F1) {
        // Bring up the VJ console if it has been minimised
        if (GetFocus() == GetPluginWindow()) {
          if (IsIconic(m_hTextWnd))
            ShowWindow(m_hTextWnd, SW_RESTORE);
        }
        // Change to regular
        m_UI_mode = UI_REGULAR;
        m_waitstring.bActive = false;  // For F8
        // Toggle help display
        m_show_press_f1_msg = 0;
        ToggleHelp();
        return 0;
      }

      switch (wParam) {
          // case VK_F9:
          // m_bShowSongTitle = !m_bShowSongTitle; // we processed (or absorbed) the key
          // m_bShowSongTime = !m_bShowSongTime;
          // m_bShowSongLen  = !m_bShowSongLen;
          // m_bShowPresetInfo = !m_bShowPresetInfo; //I didn't need this.
          // return 0; // we processed (or absorbed) the key
        case VK_F3: {
          if ((GetKeyState(VK_CONTROL) & 0x8000) != 0) {
            wchar_t buf[1024];
            if (m_max_fps_fs == 0) {
              swprintf(buf, L"Unlimited fps");
            } else {
              swprintf(buf, 1024, L"%d fps", m_max_fps_fs);
            }
            AddNotification(buf);
          } else {
            ToggleFPSNumPressed++;
            if (ToggleFPSNumPressed == 1) {
              m_max_fps_fs = 60;
              m_max_fps_dm = 60;
              m_max_fps_w = 60;
              AddNotification(L"60 fps");
            } else if (ToggleFPSNumPressed == 2) {
              m_max_fps_fs = 90;
              m_max_fps_dm = 90;
              m_max_fps_w = 90;
              AddNotification(L"90 fps");
            } else if (ToggleFPSNumPressed == 3) {
              m_max_fps_fs = 120;
              m_max_fps_dm = 120;
              m_max_fps_w = 120;
              AddNotification(L"120 fps");
            } else if (ToggleFPSNumPressed == 4) {
              m_max_fps_fs = 144;
              m_max_fps_dm = 144;
              m_max_fps_w = 144;
              AddNotification(L"144 fps");
            } else if (ToggleFPSNumPressed == 5) {
              m_max_fps_fs = 240;
              m_max_fps_dm = 240;
              m_max_fps_w = 240;
              AddNotification(L"240 fps");
            } else if (ToggleFPSNumPressed == 6) {
              m_max_fps_fs = 360;
              m_max_fps_dm = 360;
              m_max_fps_w = 360;
              AddNotification(L"360 fps");
            } else if (ToggleFPSNumPressed == 7) {
              m_max_fps_fs = 0;
              m_max_fps_dm = 0;
              m_max_fps_w = 0;
              AddNotification(L"Unlimited fps");
            } else if (ToggleFPSNumPressed == 8) {
              ToggleFPSNumPressed = 0;
              m_max_fps_fs = 30;
              m_max_fps_dm = 30;
              m_max_fps_w = 30;
              AddNotification(L"30 fps");
            }
          }
        }
          return 0;  // we processed (or absorbed) the key
        case VK_F4:
          m_bShowPresetInfo = !m_bShowPresetInfo;
          return 0;  // we processed (or absorbed) the key
        case VK_F5:
          m_bShowFPS = !m_bShowFPS;
          return 0;  // we processed (or absorbed) the key
        case VK_F6:
          m_bShowRating = !m_bShowRating;
          return 0;  // we processed (or absorbed) the key
        case VK_F7:
          m_bAlwaysOnTop = !m_bAlwaysOnTop;
          if (m_bAlwaysOnTop) {
            ToggleAlwaysOnTop(hWnd);
            AddNotification(L"Always On Top enabled");
          } else {
            ToggleAlwaysOnTop(hWnd);
            AddNotification(L"Always On Top disabled");
          }
          return 0;
        case VK_F12:
          if ((GetKeyState(VK_CONTROL) & 0x8000) != 0) {
            m_blackmode = !m_blackmode;
            if (m_blackmode) {
              AddNotification(L"Black Mode enabled");
            } else {
              AddNotification(L"Black Mode disabled");
            }
          } else {
            TranspaMode = !TranspaMode;
            if (TranspaMode) {
              ToggleTransparency(hWnd);
              AddNotification(L"Transparency Mode enabled");
            } else {
              ToggleTransparency(hWnd);
              AddNotification(L"Transparency Mode disabled");
            }
          }
          return 0;
        case VK_F8:
          OpenMilkwaveRemote();
          return 0;
          // F9 is handled in MilkwaveVisualizer.cpp
        case VK_F10:
          if (bShiftHeldDown) {
            SetSpoutFixedSize(true, true);
          } else {
            ToggleSpout();
          }
          return 0;
        case VK_F11:
          // Only changing the HardcutModes value!
          // Functionalities are moved on void MyRenderFn()
          {
            HardcutMode++;
            if (HardcutMode == 1) {
              m_bHardCutsDisabled = false;
              AddNotification(L"Hard Cut Mode: Normal");
              case 'Q': {
                if (bCtrlHeldDown) {
                  const float multiplier = bShiftHeldDown ? 2.0f : 0.5f;
                  float newQuality = clamp(m_fRenderQuality * multiplier, 0.01f, 1.0f);
                  if (fabsf(newQuality - m_fRenderQuality) > 0.0001f) {
                    m_fRenderQuality = newQuality;
                    ResetBufferAndFonts();
                    SendSettingsInfoToMilkwaveRemote();
                  }
                  return 0;
                }
                break;
              }
              case 'H': {
                if (bCtrlHeldDown) {
                  if (bShiftHeldDown) {
                    m_ColShiftHue -= 0.02f;
                    if (m_ColShiftHue <= -1.0f) {
                      m_ColShiftHue = 1.0f;
                    }
                  } else {
                    m_ColShiftHue += 0.02f;
                    if (m_ColShiftHue >= 1.0f) {
                      m_ColShiftHue = -1.0f;
                    }
                  }
                  SendSettingsInfoToMilkwaveRemote();
                  return 0;
                }
                break;
              }
            }
            if (HardcutMode == 2) {
              m_bHardCutsDisabled = true;
              AddNotification(L"Hard Cut Mode: Bass Blend");
            }
            if (HardcutMode == 3) {
              m_bHardCutsDisabled = true;
              AddNotification(L"Hard Cut Mode: Bass");
            }
            if (HardcutMode == 4) {
              m_bHardCutsDisabled = true;
              AddNotification(L"Hard Cut Mode: Middle");
            }
            if (HardcutMode == 5) {
              m_bHardCutsDisabled = true;
              AddNotification(L"Hard Cut Mode: Treble");
            }
            if (HardcutMode == 6) {
              m_bHardCutsDisabled = true;
              AddNotification(L"Hard Cut Mode: Bass Fast Blend");
            }
            if (HardcutMode == 7) {
              m_bHardCutsDisabled = true;
              AddNotification(L"Hard Cut Mode: Treble Fast Blend");
            }
            if (HardcutMode == 8) {
              m_bHardCutsDisabled = true;
              AddNotification(L"Hard Cut Mode: Bass Blend and Hardcut Treble");
            }
            if (HardcutMode == 9) {
              m_bHardCutsDisabled = true;
              AddNotification(L"Hard Cut Mode: Rhythmic Hardcut");
            }
            if (HardcutMode == 10) {
              m_bHardCutsDisabled = true;
              AddNotification(L"Hard Cut Mode: 2 beats");
              beatcount = -1;
            }
            if (HardcutMode == 11) {
              m_bHardCutsDisabled = true;
              AddNotification(L"Hard Cut Mode: 4 beats");
              beatcount = -1;
            }
            if (HardcutMode == 12) {
              m_bHardCutsDisabled = true;
              AddNotification(L"Hard Cut Mode: Kinetronix (Vizikord)");
              beatcount = -1;
            }
            if (HardcutMode == 13) {
              HardcutMode = 0;
              m_bHardCutsDisabled = true;
              AddNotification(L"Hard Cut Mode: OFF");
            }
          }
          return 0;  // we processed (or absorbed) the key

          // reenabling this feature soon. (This will be Shift+F9)
          //	if (m_nNumericInputMode == NUMERIC_INPUT_MODE_CUST_MSG)
          //		ReadCustomMessages();		// re-read custom messages
          //	return 0; // we processed (or absorbed) the key
          // case VK_F8:

          //	{
          //		m_UI_mode = UI_CHANGEDIR;

          //		// enter WaitString mode
          //		m_waitstring.bActive = true;
          //		m_waitstring.bFilterBadChars = false;
          //		m_waitstring.bDisplayAsCode = false;
          //		m_waitstring.nSelAnchorPos = -1;
          //		m_waitstring.nMaxLen = min(sizeof(m_waitstring.szText)-1, MAX_PATH - 1);
          //		lstrcpyW(m_waitstring.szText, GetPresetDir());
          //		{
          //			// for subtle beauty - remove the trailing '\' from the directory name (if it's not just "x:\")
          //			int len = lstrlenW(m_waitstring.szText);
          //			if (len > 3 && m_waitstring.szText[len-1] == '\\')
          //				m_waitstring.szText[len-1] = 0;
          //		}
          //		wasabiApiLangString(IDS_DIRECTORY_TO_JUMP_TO, m_waitstring.szPrompt, 512);
          //		m_waitstring.szToolTip[0] = 0;
          //		m_waitstring.nCursorPos = lstrlenW(m_waitstring.szText);	// set the starting edit position
          //	}
          //	return 0; // we processed (or absorbed) the key

        case VK_F9:
          m_bShowShaderHelp = !m_bShowShaderHelp;
          return FALSE;  // Alr. Fixed the shader help.

        case VK_SCROLL:
          m_bPresetLockedByUser = GetKeyState(VK_SCROLL) & 1;
          // SetScrollLock(m_bPresetLockedByUser);
          // int set = m_bPresetLockedByUser ?

          // check ???
          // case VK_F6:	break;
          // case VK_F7: conflict
          // case VK_F8:	break;
          // case VK_F9: conflict

          // Default - fall through

      }  // end switch(wParam)
      //------------------------------------------

      // next handle the waitstring case (for string-editing),
      //	then the menu navigation case,
      //  then handle normal case (handle the message normally or pass on to winamp)

      // case 1: waitstring mode
      if (m_waitstring.bActive) {
        // handle arrow keys, home, end, etc.

        USHORT mask = 1 << (sizeof(SHORT) * 8 - 1);  // we want the highest-order bit
        bool bShiftHeldDown = (GetKeyState(VK_SHIFT) & mask) != 0;
        bool bCtrlHeldDown = (GetKeyState(VK_CONTROL) & mask) != 0;

        if (wParam == VK_LEFT || wParam == VK_RIGHT ||
            wParam == VK_HOME || wParam == VK_END ||
            wParam == VK_UP || wParam == VK_DOWN) {
          if (bShiftHeldDown) {
            if (m_waitstring.nSelAnchorPos == -1)
              m_waitstring.nSelAnchorPos = m_waitstring.nCursorPos;
          } else {
            m_waitstring.nSelAnchorPos = -1;
          }
        }

        if (bCtrlHeldDown)  // copy/cut/paste
        {
          switch (wParam) {
            case 'c':
            case 'C':
            case VK_INSERT:
              WaitString_Copy();
              return 0;  // we processed (or absorbed) the key
            case 'x':
            case 'X':
              WaitString_Cut();
              return 0;  // we processed (or absorbed) the key
            case 'v':
            case 'V':
              WaitString_Paste();
              return 0;  // we processed (or absorbed) the key
            case VK_LEFT:
              WaitString_SeekLeftWord();
              return 0;  // we processed (or absorbed) the key
            case VK_RIGHT:
              WaitString_SeekRightWord();
              return 0;  // we processed (or absorbed) the key
            case VK_HOME:
              m_waitstring.nCursorPos = 0;
              return 0;  // we processed (or absorbed) the key
            case VK_END:
              if (m_waitstring.bDisplayAsCode) {
                m_waitstring.nCursorPos = lstrlenA((char*)m_waitstring.szText);
              } else {
                m_waitstring.nCursorPos = lstrlenW(m_waitstring.szText);
              }
              return 0;  // we processed (or absorbed) the key
            case VK_RETURN:
              if (m_waitstring.bDisplayAsCode) {
                // CTRL+ENTER accepts the string -> finished editing
                // assert(m_pCurMenu);
                m_pCurMenu->OnWaitStringAccept(m_waitstring.szText);
                // OnWaitStringAccept calls the callback function.  See the
                // calls to CMenu::AddItem from milkdrop.cpp to find the
                // callback functions for different "waitstrings".
                m_waitstring.bActive = false;
                m_UI_mode = UI_MENU;
              }
              return 0;  // we processed (or absorbed) the key
          }
        } else  // waitstring mode key pressed, and ctrl NOT held down
        {
          switch (wParam) {
            case VK_INSERT:
              m_waitstring.bOvertypeMode = !m_waitstring.bOvertypeMode;
              return 0;  // we processed (or absorbed) the key

            case VK_LEFT:
              for (rep = 0; rep < nRepeat; rep++)
                if (m_waitstring.nCursorPos > 0)
                  m_waitstring.nCursorPos--;
              return 0;  // we processed (or absorbed) the key

            case VK_RIGHT:
              for (rep = 0; rep < nRepeat; rep++) {
                if (m_waitstring.bDisplayAsCode) {
                  if (m_waitstring.nCursorPos < (int)lstrlenA((char*)m_waitstring.szText))
                    m_waitstring.nCursorPos++;
                } else {
                  if (m_waitstring.nCursorPos < (int)lstrlenW(m_waitstring.szText))
                    m_waitstring.nCursorPos++;
                }
              }
              return 0;  // we processed (or absorbed) the key

            case VK_HOME:
              m_waitstring.nCursorPos -= WaitString_GetCursorColumn();
              return 0;  // we processed (or absorbed) the key

            case VK_END:
              m_waitstring.nCursorPos += WaitString_GetLineLength() - WaitString_GetCursorColumn();
              return 0;  // we processed (or absorbed) the key

            case VK_UP:
              for (rep = 0; rep < nRepeat; rep++)
                WaitString_SeekUpOneLine();
              return 0;  // we processed (or absorbed) the key

            case VK_DOWN:
              for (rep = 0; rep < nRepeat; rep++)
                WaitString_SeekDownOneLine();
              return 0;  // we processed (or absorbed) the key

            case VK_BACK:
              if (m_waitstring.nSelAnchorPos != -1) {
                WaitString_NukeSelection();
              } else if (m_waitstring.nCursorPos > 0) {
                int len;
                if (m_waitstring.bDisplayAsCode) {
                  len = lstrlenA((char*)m_waitstring.szText);
                } else {
                  len = lstrlenW(m_waitstring.szText);
                }
                int src_pos = m_waitstring.nCursorPos;
                int dst_pos = m_waitstring.nCursorPos - nRepeat;
                int gap = nRepeat;
                int copy_chars = len - m_waitstring.nCursorPos + 1;  // includes NULL @ end
                if (dst_pos < 0) {
                  gap += dst_pos;
                  // copy_chars += dst_pos;
                  dst_pos = 0;
                }

                if (m_waitstring.bDisplayAsCode) {
                  char* ptr = (char*)m_waitstring.szText;
                  for (int i = 0; i < copy_chars; i++)
                    *(ptr + dst_pos + i) = *(ptr + src_pos + i);
                } else {
                  for (int i = 0; i < copy_chars; i++)
                    m_waitstring.szText[dst_pos + i] = m_waitstring.szText[src_pos + i];
                }
                m_waitstring.nCursorPos -= gap;
              }
              return 0;  // we processed (or absorbed) the key

            case VK_DELETE:
              if (m_waitstring.nSelAnchorPos != -1) {
                WaitString_NukeSelection();
              } else {
                if (m_waitstring.bDisplayAsCode) {
                  int len = lstrlenA((char*)m_waitstring.szText);
                  char* ptr = (char*)m_waitstring.szText;
                  for (int i = m_waitstring.nCursorPos; i <= len - nRepeat; i++)
                    *(ptr + i) = *(ptr + i + nRepeat);
                } else {
                  int len = lstrlenW(m_waitstring.szText);
                  for (int i = m_waitstring.nCursorPos; i <= len - nRepeat; i++)
                    m_waitstring.szText[i] = m_waitstring.szText[i + nRepeat];
                }
              }
              return 0;  // we processed (or absorbed) the key

            case VK_RETURN:
              if (m_UI_mode == UI_LOAD_RENAME)  // rename (move) the file
              {
                // first add pathnames to filenames
                wchar_t szOldFile[512];
                wchar_t szNewFile[512];
                lstrcpyW(szOldFile, GetPresetDir());
                lstrcpyW(szNewFile, GetPresetDir());
                lstrcatW(szOldFile, m_presets[m_nPresetListCurPos].szFilename.c_str());
                lstrcatW(szNewFile, m_waitstring.szText);
                lstrcatW(szNewFile, L".milk");

                RenamePresetFile(szOldFile, szNewFile);
              } else if (m_UI_mode == UI_IMPORT_WAVE ||
                         m_UI_mode == UI_EXPORT_WAVE ||
                         m_UI_mode == UI_IMPORT_SHAPE ||
                         m_UI_mode == UI_EXPORT_SHAPE) {
                int bWave = (m_UI_mode == UI_IMPORT_WAVE || m_UI_mode == UI_EXPORT_WAVE);
                int bImport = (m_UI_mode == UI_IMPORT_WAVE || m_UI_mode == UI_IMPORT_SHAPE);

                int i = m_pCurMenu->GetCurItem()->m_lParam;
                int ret;
                switch (m_UI_mode) {
                  case UI_IMPORT_WAVE:
                    ret = m_pState->m_wave[i].Import(NULL, m_waitstring.szText, 0);
                    break;
                  case UI_EXPORT_WAVE:
                    ret = m_pState->m_wave[i].Export(NULL, m_waitstring.szText, 0);
                    break;
                  case UI_IMPORT_SHAPE:
                    ret = m_pState->m_shape[i].Import(NULL, m_waitstring.szText, 0);
                    break;
                  case UI_EXPORT_SHAPE:
                    ret = m_pState->m_shape[i].Export(NULL, m_waitstring.szText, 0);
                    break;
                }

                if (bImport)
                  m_pState->RecompileExpressions(1);

                // m_fShowUserMessageUntilThisTime = GetTime() - 1.0f;	// if there was an error message already, clear it
                if (!ret) {
                  wchar_t buf[1024];
                  if (m_UI_mode == UI_IMPORT_WAVE || m_UI_mode == UI_IMPORT_SHAPE)
                    wasabiApiLangString(IDS_ERROR_IMPORTING_BAD_FILENAME, buf, 1024);
                  else
                    wasabiApiLangString(IDS_ERROR_IMPORTING_BAD_FILENAME_OR_NOT_OVERWRITEABLE, buf, 1024);
                  AddError(wasabiApiLangString(IDS_STRING_TOO_LONG), 2.5f, ERR_MISC, true);
                }

                m_waitstring.bActive = false;
                m_UI_mode = UI_MENU;
                // m_bPresetLockedByCode = false;
              } else if (m_UI_mode == UI_SAVEAS) {
                // first add pathname + extension to filename
                wchar_t szNewFile[512];
                swprintf(szNewFile, L"%s%s.milk", GetPresetDir(), m_waitstring.szText);

                if (GetFileAttributesW(szNewFile) != -1)  // check if file already exists
                {
                  // file already exists -> overwrite it?
                  m_waitstring.bActive = false;
                  m_UI_mode = UI_SAVE_OVERWRITE;
                } else {
                  SavePresetAs(szNewFile);

                  // exit waitstring mode
                  m_UI_mode = UI_REGULAR;
                  m_waitstring.bActive = false;
                  // m_bPresetLockedByCode = false;
                }
              } else if (m_UI_mode == UI_EDIT_MENU_STRING) {
                if (m_waitstring.bDisplayAsCode) {
                  if (m_waitstring.nSelAnchorPos != -1)
                    WaitString_NukeSelection();

                  int len = lstrlenA((char*)m_waitstring.szText);
                  char* ptr = (char*)m_waitstring.szText;
                  if (len + 1 < m_waitstring.nMaxLen) {
                    // insert a linefeed.  Use CTRL+return to accept changes in this case.
                    for (int pos = len + 1; pos > m_waitstring.nCursorPos; pos--)
                      *(ptr + pos) = *(ptr + pos - 1);
                    *(ptr + m_waitstring.nCursorPos++) = LINEFEED_CONTROL_CHAR;

                    // m_fShowUserMessageUntilThisTime = GetTime() - 1.0f;	// if there was an error message already, clear it
                  } else {
                    // m_waitstring.szText has reached its limit
                    AddError(wasabiApiLangString(IDS_STRING_TOO_LONG), 2.5f, ERR_MISC, true);
                  }
                } else {
                  // finished editing
                  // assert(m_pCurMenu);
                  m_pCurMenu->OnWaitStringAccept(m_waitstring.szText);
                  // OnWaitStringAccept calls the callback function.  See the
                  // calls to CMenu::AddItem from milkdrop.cpp to find the
                  // callback functions for different "waitstrings".
                  m_waitstring.bActive = false;
                  m_UI_mode = UI_MENU;
                }
              } else if (m_UI_mode == UI_CHANGEDIR) {
                // m_fShowUserMessageUntilThisTime = GetTime();	// if there was an error message already, clear it

                bool bSuccess = ChangePresetDir(m_waitstring.szText, g_plugin.m_szPresetDir);
                if (bSuccess) {
                  // set current preset index to -1 because current preset is no longer in the list
                  m_nCurrentPreset = -1;

                  // go to file load menu
                  m_waitstring.bActive = false;
                  m_UI_mode = UI_LOAD;

                  ClearErrors(ERR_MISC);
                }
              }
              return 0;  // we processed (or absorbed) the key

            case VK_ESCAPE:
              if (m_UI_mode == UI_LOAD_RENAME) {
                m_waitstring.bActive = false;
                m_UI_mode = UI_LOAD;
              } else if (
                  m_UI_mode == UI_SAVEAS ||
                  m_UI_mode == UI_SAVE_OVERWRITE ||
                  m_UI_mode == UI_EXPORT_SHAPE ||
                  m_UI_mode == UI_IMPORT_SHAPE ||
                  m_UI_mode == UI_EXPORT_WAVE ||
                  m_UI_mode == UI_IMPORT_WAVE) {
                // m_bPresetLockedByCode = false;
                m_waitstring.bActive = false;
                m_UI_mode = UI_REGULAR;
              } else if (m_UI_mode == UI_EDIT_MENU_STRING) {
                m_waitstring.bActive = false;
                if (m_waitstring.bDisplayAsCode)  // if were editing code...
                  m_UI_mode = UI_MENU;            // return to menu
                else
                  m_UI_mode = UI_REGULAR;  // otherwise don't (we might have been editing a filename, for example)
              } else                       /*if (m_UI_mode == UI_EDIT_MENU_STRING || m_UI_mode == UI_CHANGEDIR || 1)*/
              {
                m_waitstring.bActive = false;
                m_UI_mode = UI_REGULAR;
              }
              return 0;  // we processed (or absorbed) the key
          }
        }

        // don't let keys go anywhere else
        return 0;  // we processed (or absorbed) the key
      }

      // case 2: menu is up & gets the keyboard input
      if (m_UI_mode == UI_MENU) {
        // assert(m_pCurMenu);
        if (m_pCurMenu->HandleKeydown(hWnd, uMsg, wParam, lParam) == 0)
          return 0;  // we processed (or absorbed) the key
      }

      // case 3: handle non-character keys (virtual keys) and return 0.
      //         if we don't handle them, return 1, and the shell will
      //         (passing some to the shell's key bindings, some to Winamp,
      //          and some to DefWindowProc)
      //		note: regular hotkeys should be handled in HandleRegularKey.
      switch (wParam) {
        case VK_LEFT:
          if (m_UI_mode == UI_REGULAR) {
            if (bCtrlHeldDown) {
              AddError(L"Rewind", m_MediaKeyNotifyTime, ERR_NOTIFY, false);
              SendNotifyMessage(HWND_BROADCAST, WM_APPCOMMAND, 0, MAKELPARAM(0, APPCOMMAND_MEDIA_REWIND));
            } else {
              AddError(L"Previous", m_MediaKeyNotifyTime, ERR_NOTIFY, false);
              keybd_event(VK_MEDIA_PREV_TRACK, 0, 0, 0);
              keybd_event(VK_MEDIA_PREV_TRACK, 0, KEYEVENTF_KEYUP, 0);
            }
          }
          break;
        case VK_RIGHT:
          if (m_UI_mode == UI_LOAD) {
            // it's annoying when the music skips if you hit the left arrow from the Load menu, so instead, we exit the menu
            if (wParam == VK_LEFT) m_UI_mode = UI_REGULAR;
            return 0;  // we processed (or absorbed) the key
          } else if (m_UI_mode == UI_UPGRADE_PIXEL_SHADER) {
            m_UI_mode = UI_MENU;
            return 0;  // we processed (or absorbed) the key
          } else if (m_UI_mode == UI_MASHUP) {
            if (wParam == VK_LEFT)
              m_nMashSlot = max(0, m_nMashSlot - 1);
            else
              m_nMashSlot = min(MASH_SLOTS - 1, m_nMashSlot + 1);
            return 0;  // we processed (or absorbed) the key
          } else if (m_UI_mode == UI_REGULAR) {
            if (bCtrlHeldDown) {
              AddError(L"Fast Forward", m_MediaKeyNotifyTime, ERR_NOTIFY, false);
              SendNotifyMessage(HWND_BROADCAST, WM_APPCOMMAND, 0, MAKELPARAM(0, APPCOMMAND_MEDIA_FAST_FORWARD));
            } else {
              AddError(L"Next", m_MediaKeyNotifyTime, ERR_NOTIFY, false);
              keybd_event(VK_MEDIA_NEXT_TRACK, 0, 0, 0);
              keybd_event(VK_MEDIA_NEXT_TRACK, 0, KEYEVENTF_KEYUP, 0);
            }
          }

          break;

        case VK_ESCAPE:
          if (m_UI_mode == UI_LOAD || m_UI_mode == UI_MENU || m_UI_mode == UI_MASHUP) {
            m_UI_mode = UI_REGULAR;
            return 0;  // we processed (or absorbed) the key
          } else if (m_UI_mode == UI_LOAD_DEL) {
            m_UI_mode = UI_LOAD;
            return 0;  // we processed (or absorbed) the key
          } else if (m_UI_mode == UI_UPGRADE_PIXEL_SHADER) {
            m_UI_mode = UI_MENU;
            return 0;  // we processed (or absorbed) the key
          } else if (m_UI_mode == UI_SAVE_OVERWRITE) {
            m_UI_mode = UI_SAVEAS;
            // return to waitstring mode, leaving all the parameters as they were before:
            m_waitstring.bActive = true;
            return 0;  // we processed (or absorbed) the key
          }
          // SPOUT - put back in for vj mode.
          else {
            // Don't close if esc pressed when vj window has focus
            if (GetFocus() == GetPluginWindow()) {
              if (!IsBorderlessFullscreen(GetPluginWindow())) {
                bool isShiftPressed = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
                if (isShiftPressed || MessageBoxA(GetPluginWindow(), "Close Milkwave Visualizer?\n\n(You may also use SHIFT+ESC or RIGHT+LEFT MOUSE BUTTON\nto close without confirmation)", "Milkwave Visualizer", MB_YESNO | MB_TOPMOST) == IDYES) {
                  PostMessage(hWnd, WM_CLOSE, 0, 0);
                }
                return 0;
              }
            }
          }
          /*else if (hwnd == GetPluginWindow())		// (don't close on ESC for text window)
          {
            dumpmsg("User pressed ESCAPE");
            //m_bExiting = true;
            PostMessage(hwnd, WM_CLOSE, 0, 0);
            return 0; // we processed (or absorbed) the key
          }*/
          break;

        case VK_UP:
          if (m_UI_mode == UI_MASHUP) {
            for (rep = 0; rep < nRepeat; rep++)
              m_nMashPreset[m_nMashSlot] = max(m_nMashPreset[m_nMashSlot] - 1, m_nDirs);
            m_nLastMashChangeFrame[m_nMashSlot] = GetFrame();  // causes delayed apply
            return 0;                                          // we processed (or absorbed) the key
          } else if (m_UI_mode == UI_LOAD) {
            for (rep = 0; rep < nRepeat; rep++)
              if (m_nPresetListCurPos > 0)
                m_nPresetListCurPos--;
            return 0;  // we processed (or absorbed) the key

            // remember this preset's name so the next time they hit 'L' it jumps straight to it
            // lstrcpy(m_szLastPresetSelected, m_presets[m_nPresetListCurPos].szFilename.c_str());
          } else if (bShiftHeldDown) {
            ToggleWindowOpacity(hWnd, false);
          } else {
            AddError(L"Stop", m_MediaKeyNotifyTime, ERR_NOTIFY, false);
            keybd_event(VK_MEDIA_STOP, 0, 0, 0);
            keybd_event(VK_MEDIA_STOP, 0, KEYEVENTF_KEYUP, 0);
          }
          break;

        case VK_DOWN:
          if (m_UI_mode == UI_MASHUP) {
            for (rep = 0; rep < nRepeat; rep++)
              m_nMashPreset[m_nMashSlot] = min(m_nMashPreset[m_nMashSlot] + 1, m_nPresets - 1);
            m_nLastMashChangeFrame[m_nMashSlot] = GetFrame();  // causes delayed apply
            return 0;                                          // we processed (or absorbed) the key
          } else if (m_UI_mode == UI_LOAD) {
            for (rep = 0; rep < nRepeat; rep++)
              if (m_nPresetListCurPos < m_nPresets - 1)
                m_nPresetListCurPos++;
            return 0;  // we processed (or absorbed) the key

            // remember this preset's name so the next time they hit 'L' it jumps straight to it
            // lstrcpy(m_szLastPresetSelected, m_presets[m_nPresetListCurPos].szFilename.c_str());
          } else if (bShiftHeldDown) {
            ToggleWindowOpacity(hWnd, true);
          } else {
            AddError(L"Play/Pause", m_MediaKeyNotifyTime, ERR_NOTIFY, false);
            keybd_event(VK_MEDIA_PLAY_PAUSE, 0, 0, 0);
            keybd_event(VK_MEDIA_PLAY_PAUSE, 0, KEYEVENTF_KEYUP, 0);
          }
          break;

        case 'X':
          if (m_UI_mode == UI_REGULAR) {
            if ((GetKeyState(VK_CONTROL) & mask) != 0) {
              wchar_t filename[MAX_PATH];
              if (CaptureScreenshotWithFilename(filename, MAX_PATH)) {
                wchar_t msg[MAX_PATH + 32];
                swprintf_s(msg, MAX_PATH + 32, L"capture/%s saved", filename);
                AddNotification(msg);
              } else {
                AddNotification(L"Failed to save screenshot");
              }
              return 0;
            }
            AddError(L"Play/Pause", m_MediaKeyNotifyTime, ERR_NOTIFY, false);
            keybd_event(VK_MEDIA_PLAY_PAUSE, 0, 0, 0);
            keybd_event(VK_MEDIA_PLAY_PAUSE, 0, KEYEVENTF_KEYUP, 0);
            /*
            SendNotifyMessage(HWND_BROADCAST, WM_APPCOMMAND, 0, MAKELPARAM(0, APPCOMMAND_MEDIA_PLAY_PAUSE));
            Sleep(200);
            */
            // YouTube inaccurately plays/pauses the video when you do this.
          }
          break;
        case 'A':
          if (m_UI_mode == UI_REGULAR) {
            if ((GetKeyState(VK_CONTROL) & mask) != 0) {
              m_ChangePresetWithSong = !m_ChangePresetWithSong;
              WritePrivateProfileIntW(m_ChangePresetWithSong, L"ChangePresetWithSong", GetConfigIniFile(), L"Milkwave");
              SendSettingsInfoToMilkwaveRemote();
              if (m_ChangePresetWithSong) {
                AddError(L"Auto Preset Change enabled", 5.0f, ERR_NOTIFY, false);
              } else {
                AddError(L"Auto Preset Change disabled", 5.0f, ERR_NOTIFY, false);
              }
              return 0;  // we processed (or absorbed) the key
            }
            /*
            SendNotifyMessage(HWND_BROADCAST, WM_APPCOMMAND, 0, MAKELPARAM(0, APPCOMMAND_MEDIA_STOP));
            Sleep(200);
            */
            // Any media players force stops the track without fading (Ex: AIMP) - so I think I don't need this.
          }
          break;
        case 'C':
          if (m_UI_mode == UI_REGULAR) {
            if ((GetKeyState(VK_SHIFT) & mask) == 0 && (GetKeyState(VK_CONTROL) & mask) == 0) {
              AddError(L"Stop", m_MediaKeyNotifyTime, ERR_NOTIFY, false);
              keybd_event(VK_MEDIA_STOP, 0, 0, 0);
              keybd_event(VK_MEDIA_STOP, 0, KEYEVENTF_KEYUP, 0);
            }
            /*
            SendNotifyMessage(HWND_BROADCAST, WM_APPCOMMAND, 0, MAKELPARAM(0, APPCOMMAND_MEDIA_STOP));
            Sleep(200);
            */
            // Any media players force stops the track without fading (Ex: AIMP) - so I think I don't need this.
          }
          break;
        case 'V':
          if (m_UI_mode == UI_REGULAR) {
            AddError(L"Next", m_MediaKeyNotifyTime, ERR_NOTIFY, false);
            keybd_event(VK_MEDIA_NEXT_TRACK, 0, 0, 0);
            keybd_event(VK_MEDIA_NEXT_TRACK, 0, KEYEVENTF_KEYUP, 0);
            /*
            SendNotifyMessage(HWND_BROADCAST, WM_APPCOMMAND, 0, MAKELPARAM(0, APPCOMMAND_MEDIA_NEXTTRACK));
            Sleep(200);
            */
            // AIMP freezes when you do this.
          }
          break;

        case VK_SPACE:
          if (m_UI_mode == UI_LOAD)
            goto HitEnterFromLoadMenu;
          if (!m_bPresetLockedByCode) {
            LoadRandomPreset(m_fBlendTimeUser);
            return 0;  // we processed (or absorbed) the key
          }
          break;

        case VK_PRIOR:
          if (m_UI_mode == UI_LOAD || m_UI_mode == UI_MASHUP) {
            m_bUserPagedUp = true;
            if (m_UI_mode == UI_MASHUP)
              m_nLastMashChangeFrame[m_nMashSlot] = GetFrame();  // causes delayed apply
            return 0;                                            // we processed (or absorbed) the key
          }
          break;
        case VK_NEXT:
          if (m_UI_mode == UI_LOAD || m_UI_mode == UI_MASHUP) {
            m_bUserPagedDown = true;
            if (m_UI_mode == UI_MASHUP)
              m_nLastMashChangeFrame[m_nMashSlot] = GetFrame();  // causes delayed apply
            return 0;                                            // we processed (or absorbed) the key
          }
          break;
        case VK_HOME:
          if (m_UI_mode == UI_LOAD) {
            m_nPresetListCurPos = 0;
            return 0;  // we processed (or absorbed) the key
          } else if (m_UI_mode == UI_MASHUP) {
            m_nMashPreset[m_nMashSlot] = m_nDirs;
            m_nLastMashChangeFrame[m_nMashSlot] = GetFrame();  // causes delayed apply
            return 0;                                          // we processed (or absorbed) the key
          }
          break;
        case VK_END:
          // printf("VK_END (%d)\n", m_UI_mode);
          if (m_UI_mode == UI_LOAD)  // 2
          {
            m_nPresetListCurPos = m_nPresets - 1;
            return 0;                         // we processed (or absorbed) the key
          } else if (m_UI_mode == UI_MASHUP)  // 14
          {
            m_nMashPreset[m_nMashSlot] = m_nPresets - 1;
            m_nLastMashChangeFrame[m_nMashSlot] = GetFrame();  // causes delayed apply
            return 0;                                          // we processed (or absorbed) the key
          }
          break;

        case VK_DELETE:
          if (m_UI_mode == UI_LOAD) {
            if (m_presets[m_nPresetListCurPos].szFilename.c_str()[0] != '*')  // can't delete directories
              m_UI_mode = UI_LOAD_DEL;
            return 0;  // we processed (or absorbed) the key
          } else       // if (m_nNumericInputDigits == 0)
          {
            if (m_nNumericInputMode == NUMERIC_INPUT_MODE_CUST_MSG) {
              m_nNumericInputDigits = 0;
              m_nNumericInputNum = 0;

              // stop display of text messages
              KillAllSupertexts();

              return 0;  // we processed (or absorbed) the key
            } else if (m_nNumericInputMode == NUMERIC_INPUT_MODE_SPRITE) {
              // kill newest sprite (regular DELETE key)
              // oldest sprite (SHIFT + DELETE),
              // or all sprites (CTRL + SHIFT + DELETE).

              m_nNumericInputDigits = 0;
              m_nNumericInputNum = 0;

              USHORT mask = 1 << (sizeof(SHORT) * 8 - 1);  // we want the highest-order bit
              bool bShiftHeldDown = (GetKeyState(VK_SHIFT) & mask) != 0;
              bool bCtrlHeldDown = (GetKeyState(VK_CONTROL) & mask) != 0;

              if (bShiftHeldDown && bCtrlHeldDown) {
                for (int x = 0; x < NUM_TEX; x++)
                  m_texmgr.KillTex(x);
              } else {
                int newest = -1;
                int frame;
                for (int x = 0; x < NUM_TEX; x++) {
                  if (m_texmgr.m_tex[x].pSurface) {
                    if ((newest == -1) ||
                        (!bShiftHeldDown && m_texmgr.m_tex[x].nStartFrame > frame) ||
                        (bShiftHeldDown && m_texmgr.m_tex[x].nStartFrame < frame)) {
                      newest = x;
                      frame = m_texmgr.m_tex[x].nStartFrame;
                    }
                  }
                }

                if (newest != -1)
                  m_texmgr.KillTex(newest);
              }
              return 0;  // we processed (or absorbed) the key
            }
          }
          break;

        case VK_INSERT:  // RENAME
          if (m_UI_mode == UI_LOAD) {
            if (m_presets[m_nPresetListCurPos].szFilename.c_str()[0] != '*')  // can't rename directories
            {
              // go into RENAME mode
              m_UI_mode = UI_LOAD_RENAME;
              m_waitstring.bActive = true;
              m_waitstring.bFilterBadChars = true;
              m_waitstring.bDisplayAsCode = false;
              m_waitstring.nSelAnchorPos = -1;
              m_waitstring.nMaxLen = min(sizeof(m_waitstring.szText) - 1, MAX_PATH - lstrlenW(GetPresetDir()) - 6);  // 6 for the extension + null char.  We set this because win32 LoadFile, MoveFile, etc. barf if the path+filename+ext are > MAX_PATH chars.

              // initial string is the filename, minus the extension
              lstrcpyW(m_waitstring.szText, m_presets[m_nPresetListCurPos].szFilename.c_str());
              RemoveExtension(m_waitstring.szText);

              // set the prompt & 'tooltip'
              swprintf(m_waitstring.szPrompt, wasabiApiLangString(IDS_ENTER_THE_NEW_NAME_FOR_X), m_waitstring.szText);
              m_waitstring.szToolTip[0] = 0;

              // set the starting edit position
              m_waitstring.nCursorPos = lstrlenW(m_waitstring.szText);
            }
            return 0;  // we processed (or absorbed) the key
          }
          break;

        case VK_RETURN:

          if (m_UI_mode == UI_MASHUP) {
            m_nLastMashChangeFrame[m_nMashSlot] = GetFrame() + MASH_APPLY_DELAY_FRAMES;  // causes instant apply
            return 0;                                                                    // we processed (or absorbed) the key
          } else if (m_UI_mode == UI_LOAD) {
          HitEnterFromLoadMenu:

            if (m_presets[m_nPresetListCurPos].szFilename.c_str()[0] == '*') {
              // CHANGE DIRECTORY
              wchar_t* p = GetPresetDir();

              if (wcscmp(m_presets[m_nPresetListCurPos].szFilename.c_str(), L"*..") == 0) {
                // back up one dir
                wchar_t* p2 = wcsrchr(p, L'\\');
                if (p2) {
                  *p2 = 0;
                  p2 = wcsrchr(p, L'\\');
                  if (p2) *(p2 + 1) = 0;
                }
              } else {
                // open subdir
                lstrcatW(p, &m_presets[m_nPresetListCurPos].szFilename.c_str()[1]);
                lstrcatW(p, L"\\");
              }

              WritePrivateProfileStringW(L"Settings", L"szPresetDir", GetPresetDir(), GetConfigIniFile());

              UpdatePresetList(true, true, false);

              // set current preset index to -1 because current preset is no longer in the list
              m_nCurrentPreset = -1;
            } else {
              // LOAD NEW PRESET
              m_nCurrentPreset = m_nPresetListCurPos;

              // first take the filename and prepend the path.  (already has extension)
              wchar_t s[MAX_PATH];
              lstrcpyW(s, GetPresetDir());  // note: m_szPresetDir always ends with '\'
              lstrcatW(s, m_presets[m_nCurrentPreset].szFilename.c_str());

              // now load (and blend to) the new preset
              m_presetHistoryPos = (m_presetHistoryPos + 1) % PRESET_HIST_LEN;
              LoadPreset(s, (wParam == VK_SPACE) ? m_fBlendTimeUser : 0);
            }
            return 0;  // we processed (or absorbed) the key
          }
          break;

        case VK_BACK:
          // pass on to parent
          // PostMessage(m_hWndParent,message,wParam,lParam);
          PrevPreset(0);
          m_fHardCutThresh *= 2.0f;  // make it a little less likely that a random hard cut follows soon.
          // m_nNumericInputDigits = 0;
          // m_nNumericInputNum = 0;
          return 0;

          // ========================================
          // SPOUT
          //
          //		CTRL-Z - start or stop spout output
          //
        case 'Z':
          if (bCtrlHeldDown) {
            if (bShiftHeldDown) {
              SetSpoutFixedSize(true, true);
            } else {
              ToggleSpout();
            }
          } else {
            AddError(L"Previous", m_MediaKeyNotifyTime, ERR_NOTIFY, false);
            keybd_event(VK_MEDIA_PREV_TRACK, 0, 0, 0);
            keybd_event(VK_MEDIA_PREV_TRACK, 0, KEYEVENTF_KEYUP, 0);
          }
          break;

        case 'S':
          if (bCtrlHeldDown) {
            g_plugin.SaveCurrentPresetToQuicksave(bShiftHeldDown);
            return 0;
          }
          break;

        case 'T':
          if (bCtrlHeldDown) {
            // stop display of custom message or song title.
            KillAllSupertexts();
            return 0;
          }
          break;

        case 'K':
          if (bCtrlHeldDown)  // kill all sprites
          {
            KillAllSprites();
            return 0;
          }
          break;
          /*case keyMappings[2]: // 'Y'
              if (bCtrlHeldDown)      // stop display of custom message or song title.
              {
            m_supertext.fStartTime = -1.0f;
                  return 0;
              }
              break;*/
      }
      if (wParam == keyMappings[2])  // 'Y'
      {
        if (bCtrlHeldDown)  // stop display of custom message or song title.
        {
          KillAllSupertexts();
          return 0;
        }
      }
      return 1;  // end case WM_KEYDOWN

    case WM_KEYUP:
      return 1;
      break;

    default:
      return DefWindowProcW(hWnd, uMsg, wParam, lParam);
      break;
  }

  return 0;
}

void CPlugin::KillAllSprites() {
  for (int x = 0; x < NUM_TEX; x++)
    if (m_texmgr.m_tex[x].pSurface)
      m_texmgr.KillTex(x);
}

void CPlugin::KillAllSupertexts() {
  for (int x = 0; x < NUM_SUPERTEXTS; x++) {
    m_supertexts[x].fStartTime = -1.0f;
    m_supertexts[x].bRedrawSuperText = false;
  }
}

bool CPlugin::ChangePresetDir(wchar_t* newDir, wchar_t* oldDir, bool bBackground) {
  // change dir
  wchar_t szOldDir[512];
  wchar_t szNewDir[512];
  lstrcpyW(szOldDir, oldDir);
  lstrcpyW(szNewDir, newDir);

  int len = lstrlenW(szNewDir);
  if (len > 0 && szNewDir[len - 1] != L'\\')
    lstrcatW(szNewDir, L"\\");

  lstrcpyW(g_plugin.m_szPresetDir, szNewDir);

  bool bSuccess = true;
  if (GetFileAttributesW(g_plugin.m_szPresetDir) == -1)
    bSuccess = false;
  if (bSuccess) {
    UpdatePresetList(bBackground, true, false);

    // bSuccess = (m_nPresets > 0);
    // success
    lstrcpyW(g_plugin.m_szPresetDir, szNewDir);

    // save new path to registry
    WritePrivateProfileStringW(L"Settings", L"szPresetDir", g_plugin.m_szPresetDir, GetConfigIniFile());
  } else {
    // new dir. was invalid -> allow them to try again
    lstrcpyW(g_plugin.m_szPresetDir, oldDir);

    // give them a warning
    AddError(wasabiApiLangString(IDS_INVALID_PATH), 3.5f, ERR_MISC, true);
  }

  return bSuccess;
}

int CPlugin::ToggleSpout() {
  bSpoutChanged = true;  // write config on exit
  bSpoutOut = !bSpoutOut;
  if (bSpoutOut) {
    // Start spout
    AddNotification(L"Spout output enabled");
  } else {
    // Stop Spout
    AddNotification(L"Spout output disabled");
  }
  SetSpoutFixedSize(false, false);

  if (bInitialized) {
    spoutsender.ReleaseDX9sender();
    bInitialized = false;
    // Initialized next render frame
    // milkdropfs.cpp - RenderFrame / OpenSender
  }

  ResetBufferAndFonts();
  SendSettingsInfoToMilkwaveRemote();
  return 0;
}

int CPlugin::SetSpoutFixedSize(bool toggleSwitch, bool showNotifications) {
  bSpoutChanged = true;  // write config on exit
  if (toggleSwitch) {
    bSpoutFixedSize = !bSpoutFixedSize;
  }
  if (IsSpoutActiveAndFixed()) {
    if (toggleSwitch && showNotifications) {
      std::wstring msg = L"Fixed Spout output size enabled (" + std::to_wstring(nSpoutFixedWidth) + L"x" + std::to_wstring(nSpoutFixedHeight) + L")";
      AddNotification(msg.data());
    } else if (showNotifications) {
      std::wstring msg = L"Spout output size set to " + std::to_wstring(nSpoutFixedWidth) + L"x" + std::to_wstring(nSpoutFixedHeight);
      AddNotification(msg.data());
    }
    ResetBufferAndFonts();

    d3dPp.BackBufferWidth = nSpoutFixedWidth;
    d3dPp.BackBufferHeight = nSpoutFixedHeight;
    UpdateBackBufferTracking(d3dPp.BackBufferWidth, d3dPp.BackBufferHeight);

    LPDIRECT3DDEVICE9EX pDevice = GetDevice();
    if (pDevice) {
      pDevice->Reset(&d3dPp);
    }
  } else {
    // bSpoutFixedSize OR bSpoutOut is false
    // Update window properties
    SetVariableBackBuffer(m_WindowWidth, m_WindowFixedHeight);
    UpdateBackBufferTracking(d3dPp.BackBufferWidth, d3dPp.BackBufferHeight);

    LPDIRECT3DDEVICE9EX pDevice = GetDevice();
    if (pDevice) {
      pDevice->Reset(&d3dPp);
    }
    if (toggleSwitch && showNotifications && bSpoutOut) {
      AddNotification(L"Fixed Spout output size disabled");
    }
    ResetBufferAndFonts();
  }
  SendSettingsInfoToMilkwaveRemote();
  return 0;
}

//----------------------------------------------------------------------

int CPlugin::HandleRegularKey(WPARAM wParam) {
  // here we handle all the normal keys for milkdrop-
  // these are the hotkeys that are used when you're not
  // in the middle of editing a string, navigating a menu, etc.

  // do not make references to virtual keys here; only
  // straight WM_CHAR messages should be sent in.

  // return 0 if you process/absorb the key; otherwise return 1.

  // SPOUT DEBUG for BeatDrop vj mode
  // For "L, "M', "S" and "VK_F8"
  // if pluginshell VK_F1 help has been pressed
  // reset help and clear the window

  if (m_UI_mode == UI_LOAD && ((wParam >= 'A' && wParam <= 'Z') || (wParam >= 'a' && wParam <= 'z'))) {
    SeekToPreset((char)wParam);
    return 0;  // we processed (or absorbed) the key
  } else if (m_UI_mode == UI_MASHUP && wParam >= '1' && wParam <= ('0' + MASH_SLOTS)) {
    m_nMashSlot = wParam - '1';
  } else
    switch (wParam) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9': {
        int digit = wParam - '0';
        m_nNumericInputNum = (m_nNumericInputNum * 10) + digit;
        m_nNumericInputDigits++;

        if (m_nNumericInputDigits >= 2) {
          if (m_nNumericInputMode == NUMERIC_INPUT_MODE_CUST_MSG)
            LaunchCustomMessage(m_nNumericInputNum);
          else if (m_nNumericInputMode == NUMERIC_INPUT_MODE_SPRITE)
            LaunchSprite(m_nNumericInputNum, -1);
          else if (m_nNumericInputMode == NUMERIC_INPUT_MODE_SPRITE_KILL) {
            for (int x = 0; x < NUM_TEX; x++)
              if (m_texmgr.m_tex[x].nUserData == m_nNumericInputNum)
                m_texmgr.KillTex(x);
          }

          m_nNumericInputDigits = 0;
          m_nNumericInputNum = 0;
        }
      }
        return 0;  // we processed (or absorbed) the key

      // row 1 keys
      case 'q':
      case 'Q': {
        USHORT mask = 1 << (sizeof(SHORT) * 8 - 1);  // we want the highest-order bit
        bool bCtrlHeldDown = (GetKeyState(VK_CONTROL) & mask) != 0;

        if (!bCtrlHeldDown) {
          if (wParam == 'q') {
            m_pState->m_fVideoEchoZoom /= 1.05f;
          } else {
            m_pState->m_fVideoEchoZoom *= 1.05f;
          }
          SendPresetWaveInfoToMilkwaveRemote();
        } else {
          const float multiplier = (wParam == 'q') ? 0.5f : 2.0f;
          float newQuality = clamp(m_fRenderQuality * multiplier, 0.01f, 1.0f);
          if (fabsf(newQuality - m_fRenderQuality) > 0.0001f) {
            m_fRenderQuality = newQuality;
            ResetBufferAndFonts();
            SendSettingsInfoToMilkwaveRemote();
          }
        }
        return 0;  // we processed (or absorbed) the key
      }
      case 'w':
        m_pState->m_nWaveMode++;
        if (m_pState->m_nWaveMode >= NUM_WAVES) m_pState->m_nWaveMode = 0;
        SendPresetWaveInfoToMilkwaveRemote();
        return 0;  // we processed (or absorbed) the key
      case 'W':
        m_pState->m_nWaveMode--;
        if (m_pState->m_nWaveMode < 0) m_pState->m_nWaveMode = NUM_WAVES - 1;
        SendPresetWaveInfoToMilkwaveRemote();
        return 0;  // we processed (or absorbed) the key
      case 'e':
        m_pState->m_fWaveAlpha -= 0.1f;
        if (m_pState->m_fWaveAlpha.eval(-1) < 0.0f) m_pState->m_fWaveAlpha = 0.0f;
        SendPresetWaveInfoToMilkwaveRemote();
        return 0;  // we processed (or absorbed) the key
      case 'E':
        m_pState->m_fWaveAlpha += 0.1f;
        SendPresetWaveInfoToMilkwaveRemote();
        // if (m_pState->m_fWaveAlpha.eval(-1) > 1.0f) m_pState->m_fWaveAlpha = 1.0f;
        return 0;  // we processed (or absorbed) the key

      case 'I':
        m_pState->m_fZoom -= 0.01f;
        SendPresetWaveInfoToMilkwaveRemote();
        return 0;  // we processed (or absorbed) the key
      case 'i':
        m_pState->m_fZoom += 0.01f;
        SendPresetWaveInfoToMilkwaveRemote();
        return 0;  // we processed (or absorbed) the key

      case 'n':
      case 'N':
        m_bShowDebugInfo = !m_bShowDebugInfo;
        return 0;  // we processed (or absorbed) the key

      case 'r':
      case 'R':
        m_bSequentialPresetOrder = !m_bSequentialPresetOrder;
        {
          wchar_t buf[1024], tmp[64];
          swprintf(buf, wasabiApiLangString(IDS_PRESET_ORDER_IS_NOW_X),
                   wasabiApiLangString((m_bSequentialPresetOrder) ? IDS_SEQUENTIAL : IDS_RANDOM, tmp, 64));
          AddNotification(buf);
        }

        // erase all history, too:
        m_presetHistory[0] = m_szCurrentPresetFile;
        m_presetHistoryPos = 0;
        m_presetHistoryFwdFence = 1;
        m_presetHistoryBackFence = 0;

        return 0;  // we processed (or absorbed) the key

      case 'u':
        m_pState->m_fWarpScale /= 1.1f;
        break;
      case 'U':
        m_pState->m_fWarpScale *= 1.1f;
        break;
        // case 'b':	m_pState->m_fWarpAnimSpeed /= 1.1f;		break;
        // case 'B':	m_pState->m_fWarpAnimSpeed *= 1.1f;		break;

      case 't':
      case 'T':
        LaunchSongTitleAnim(-1);
        return 0;  // we processed (or absorbed) the key
      case 'o':
        m_pState->m_fWarpAmount /= 1.1f;
        SendPresetWaveInfoToMilkwaveRemote();
        return 0;  // we processed (or absorbed) the key
      case 'O':
        m_pState->m_fWarpAmount *= 1.1f;
        SendPresetWaveInfoToMilkwaveRemote();
        return 0;  // we processed (or absorbed) the key
      case '!':
        // randomize warp shader
        {
          bool bWarpLock = m_bWarpShaderLock;
          wchar_t szOldPreset[MAX_PATH];
          lstrcpyW(szOldPreset, m_szCurrentPresetFile);
          m_bWarpShaderLock = false;
          LoadRandomPreset(0.0f);
          m_bWarpShaderLock = true;
          LoadPreset(szOldPreset, 0.0f);
          m_bWarpShaderLock = bWarpLock;
        }
        break;
      case '@':
        // randomize comp shader
        {
          bool bCompLock = m_bCompShaderLock;
          wchar_t szOldPreset[MAX_PATH];
          lstrcpyW(szOldPreset, m_szCurrentPresetFile);
          m_bCompShaderLock = false;
          LoadRandomPreset(0.0f);
          m_bCompShaderLock = true;
          LoadPreset(szOldPreset, 0.0f);
          m_bCompShaderLock = bCompLock;
        }
        break;

      case 'a':
      case 'A':
        // load a random preset, a random warp shader, and a random comp shader.
        // not quite as extreme as a mash-up.
        {
          USHORT mask = 1 << (sizeof(SHORT) * 8 - 1);  // we want the highest-order bit
          bool bShiftHeldDown = (GetKeyState(VK_SHIFT) & mask) != 0;
          if (!bShiftHeldDown) {
            bool bCompLock = m_bCompShaderLock;
            bool bWarpLock = m_bWarpShaderLock;
            m_bCompShaderLock = false;
            m_bWarpShaderLock = false;
            LoadRandomPreset(0.0f);
            m_bCompShaderLock = true;
            m_bWarpShaderLock = false;
            LoadRandomPreset(0.0f);
            m_bCompShaderLock = false;
            m_bWarpShaderLock = true;
            LoadRandomPreset(0.0f);
            m_bCompShaderLock = bCompLock;
            m_bWarpShaderLock = bWarpLock;
          }
        }
        break;
      case 'd':
      case 'D':
        if ((GetKeyState(VK_CONTROL) & 0x8000) == 0) {
          // Ctrl+D handled in MilkwaveVisualizer.cpp
          if (!m_bCompShaderLock && !m_bWarpShaderLock) {
            m_bCompShaderLock = true;
            m_bWarpShaderLock = false;
            AddNotification(wasabiApiLangString(IDS_COMPSHADER_LOCKED));
          } else if (m_bCompShaderLock && !m_bWarpShaderLock) {
            m_bCompShaderLock = false;
            m_bWarpShaderLock = true;
            AddNotification(wasabiApiLangString(IDS_WARPSHADER_LOCKED));
          } else if (!m_bCompShaderLock && m_bWarpShaderLock) {
            m_bCompShaderLock = true;
            m_bWarpShaderLock = true;
            AddNotification(wasabiApiLangString(IDS_ALLSHADERS_LOCKED));
          } else {
            m_bCompShaderLock = false;
            m_bWarpShaderLock = false;
            AddNotification(wasabiApiLangString(IDS_ALLSHADERS_UNLOCKED));
          }
          break;
        }
        // row 2 keys
        // 'A' KEY IS FREE!!
        // 'D' KEY IS FREE!!
      case 'p':
        m_pState->m_fVideoEchoAlpha -= 0.1f;
        if (m_pState->m_fVideoEchoAlpha.eval(-1) < 0) m_pState->m_fVideoEchoAlpha = 0;
        return 0;  // we processed (or absorbed) the key
      case 'P':
        m_pState->m_fVideoEchoAlpha += 0.1f;
        if (m_pState->m_fVideoEchoAlpha.eval(-1) > 1.0f) m_pState->m_fVideoEchoAlpha = 1.0f;
        return 0;  // we processed (or absorbed) the key
        /*case 'd':
          m_pState->m_fDecay += 0.01f;
          if (m_pState->m_fDecay.eval(-1) > 1.0f) m_pState->m_fDecay = 1.0f;
          return 0; // we processed (or absorbed) the key
        case 'D':
          m_pState->m_fDecay -= 0.01f;
          if (m_pState->m_fDecay.eval(-1) < 0.9f) m_pState->m_fDecay = 0.9f;
          return 0; // we processed (or absorbed) the key*/
      case 'h':
      case 'H':
        // instant hard cut
        if (m_UI_mode == UI_MASHUP) {
          if (wParam == 'h') {
            m_nMashPreset[m_nMashSlot] = m_nDirs + (rand() % (m_nPresets - m_nDirs));
            m_nLastMashChangeFrame[m_nMashSlot] = GetFrame() + MASH_APPLY_DELAY_FRAMES;  // causes instant apply
          } else {
            for (int mash = 0; mash < MASH_SLOTS; mash++) {
              m_nMashPreset[mash] = m_nDirs + (rand() % (m_nPresets - m_nDirs));
              m_nLastMashChangeFrame[mash] = GetFrame() + MASH_APPLY_DELAY_FRAMES;  // causes instant apply
            }
          }
        } else {
          NextPreset(0);
          m_fHardCutThresh *= 2.0f;  // make it a little less likely that a random hard cut follows soon.
        }
        return 0;  // we processed (or absorbed) the key
      case 'f':
      case 'F':
        m_pState->m_nVideoEchoOrientation = (m_pState->m_nVideoEchoOrientation + 1) % 4;
        return 0;  // we processed (or absorbed) the key
      case 'g':
        m_pState->m_fGammaAdj -= 0.1f;
        if (m_pState->m_fGammaAdj.eval(-1) < 0.0f) m_pState->m_fGammaAdj = 0.0f;
        return 0;  // we processed (or absorbed) the key
      case 'G':
        m_pState->m_fGammaAdj += 0.1f;
        // if (m_pState->m_fGammaAdj > 1.0f) m_pState->m_fGammaAdj = 1.0f;
        return 0;  // we processed (or absorbed) the key
      case 'j':
        m_pState->m_fWaveScale *= 0.9f;
        SendPresetWaveInfoToMilkwaveRemote();
        return 0;  // we processed (or absorbed) the key
      case 'J':
        m_pState->m_fWaveScale /= 0.9f;
        SendPresetWaveInfoToMilkwaveRemote();
        return 0;  // we processed (or absorbed) the key
      case 'k':
      case 'K': {
        USHORT mask = 1 << (sizeof(SHORT) * 8 - 1);  // we want the highest-order bit
        bool bShiftHeldDown = (GetKeyState(VK_SHIFT) & mask) != 0;

        if (bShiftHeldDown) {
          m_nNumericInputMode = NUMERIC_INPUT_MODE_SPRITE_KILL;
          SendMessageToMilkwaveRemote(L"STATUS=Sprite Mode set");
          PostMessageToMilkwaveRemote(WM_USER_SPRITE_MODE);
        } else if (m_nNumericInputMode == NUMERIC_INPUT_MODE_SPRITE) {
          m_nNumericInputMode = NUMERIC_INPUT_MODE_CUST_MSG;
          SendMessageToMilkwaveRemote(L"STATUS=Message Mode set");
          PostMessageToMilkwaveRemote(WM_USER_MESSAGE_MODE);
        } else {
          m_nNumericInputMode = NUMERIC_INPUT_MODE_SPRITE;
          SendMessageToMilkwaveRemote(L"STATUS=Sprite Mode set");
          PostMessageToMilkwaveRemote(WM_USER_SPRITE_MODE);
        }

        m_nNumericInputNum = 0;
        m_nNumericInputDigits = 0;
      }
        return 0;  // we processed (or absorbed) the key

        // row 3/misc. keys

      case '[':
        m_pState->m_fXPush -= 0.005f;
        return 0;  // we processed (or absorbed) the key
      case ']':
        m_pState->m_fXPush += 0.005f;
        return 0;  // we processed (or absorbed) the key
      case '{':
        m_pState->m_fYPush -= 0.005f;
        return 0;  // we processed (or absorbed) the key
      case '}':
        m_pState->m_fYPush += 0.005f;
        return 0;  // we processed (or absorbed) the key
      case '<':
        m_pState->m_fRot += 0.02f;
        return 0;  // we processed (or absorbed) the key
      case '>':
        m_pState->m_fRot -= 0.02f;
        return 0;  // we processed (or absorbed) the key

      case 's':  // SAVE PRESET
      case 'S':
        // SPOUT
        m_show_help = 0;
        if (m_UI_mode == UI_REGULAR) {
          bool isCtrlPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
          if (!isCtrlPressed) {
            // m_bPresetLockedByCode = true;
            m_UI_mode = UI_SAVEAS;

            // enter WaitString mode
            m_waitstring.bActive = true;
            m_waitstring.bFilterBadChars = true;
            m_waitstring.bDisplayAsCode = false;
            m_waitstring.nSelAnchorPos = -1;
            m_waitstring.nMaxLen = min(sizeof(m_waitstring.szText) - 1, MAX_PATH - lstrlenW(GetPresetDir()) - 6);  // 6 for the extension + null char.    We set this because win32 LoadFile, MoveFile, etc. barf if the path+filename+ext are > MAX_PATH chars.
            lstrcpyW(m_waitstring.szText, m_pState->m_szDesc);                                                     // initial string is the filename, minus the extension
            wasabiApiLangString(IDS_SAVE_AS, m_waitstring.szPrompt, 512);
            m_waitstring.szToolTip[0] = 0;
            m_waitstring.nCursorPos = lstrlenW(m_waitstring.szText);  // set the starting edit position
          }

          return 0;
        }
        break;

      case '`':
      case '~':
        m_bPresetLockedByUser = !m_bPresetLockedByUser;
        if (m_bPresetLockedByUser) {
          wchar_t buf[1024];
          swprintf(buf, 1024, L"Preset locked");
          AddNotification(buf);
        } else {
          wchar_t buf[1024];
          swprintf(buf, 1024, L"Preset unlocked");
          AddNotification(buf);
        }
        SendSettingsInfoToMilkwaveRemote();
        return 0;

      case 'l':  // LOAD PRESET
      case 'L':
        // SPOUT
        m_show_help = 0;

        if (m_UI_mode == UI_LOAD) {
          m_UI_mode = UI_REGULAR;
          return 0;  // we processed (or absorbed) the key

        } else if (
            m_UI_mode == UI_REGULAR ||
            m_UI_mode == UI_MENU) {
          UpdatePresetList(true);  // make sure list is completely ready
          m_UI_mode = UI_LOAD;
          m_bUserPagedUp = false;
          m_bUserPagedDown = false;
          return 0;  // we processed (or absorbed) the key
        }
        break;

      case 'm':
      case 'M':

        // SPOUT
        m_show_help = 0;

        if (m_UI_mode == UI_MENU)
          m_UI_mode = UI_REGULAR;
        else if (m_UI_mode == UI_REGULAR || m_UI_mode == UI_LOAD)
          m_UI_mode = UI_MENU;
        return 0;  // we processed (or absorbed) the key

      case '-':
        SetCurrentPresetRating(m_pState->m_fRating - 1.0f);
        return 0;  // we processed (or absorbed) the key
      case '+':
        SetCurrentPresetRating(m_pState->m_fRating + 1.0f);
        return 0;  // we processed (or absorbed) the key

      case '*':
        ReadCustomMessages();
        g_plugin.AddNotification(L"Messages reloaded");
        m_nNumericInputDigits = 0;
        m_nNumericInputNum = 0;
        return 0;
    }

  if (wParam == 'y' || wParam == 'Y')  // 'y' or 'Y'
  {
    // Milkwave: 'k' now toggles between sprite and message mode
    return 0;  // we processed (or absorbed) the key
  }

  return 1;
}

void CPlugin::SaveCurrentPresetToQuicksave(bool altDir) {
  // Find the last occurrence of the path separator ('\\') in the full path
  wchar_t* presetFilename = wcsrchr(m_szCurrentPresetFile, L'\\');
  if (presetFilename) {
    // Move past the '\\' to get the filename
    presetFilename++;
  } else {
    // If no '\\' is found, assume the full path is just the filename
    presetFilename = m_szCurrentPresetFile;
  }

  if (presetFilename[0] == L'\0') {  // Check if presetFilename is empty
    RemoveAngleBrackets(m_pState->m_szDesc);
    presetFilename = m_pState->m_szDesc;  // Default filename if empty
    // append ".milk" extension
    presetFilename = wcscat(presetFilename, L".milk");
  }

  // Get the executable's directory
  std::filesystem::path exeDir = std::filesystem::path(m_szBaseDir).parent_path();

  std::string quicksaveDir = "resources/presets/Quicksave";
  if (altDir) {
    quicksaveDir = "resources/presets/Quicksave2";
  }
  std::filesystem::path quicksavePresetPath = exeDir / quicksaveDir;
  std::filesystem::create_directories(quicksavePresetPath);

  quicksavePresetPath.append(presetFilename);
  // Convert std::filesystem::path to const wchar_t* before passing to Export
  if (!m_pState->Export(quicksavePresetPath.wstring().c_str())) {
    AddError(L"Quicksave failed", 5.0f, ERR_PRESET, true);
  } else {
    RemoveAngleBrackets(m_pState->m_szDesc);
    // lstrcpyW(m_pState->m_szDesc, m_szCurrentPresetFile);
    AddNotification(L"Quicksave successful");
  }
}

wchar_t* FormImageCacheSizeString(wchar_t* itemStr, UINT sizeID) {
  static wchar_t cacheBuf[128] = {0};
  StringCchPrintfW(cacheBuf, 128, L"%s %s", itemStr, wasabiApiLangString(sizeID));
  return cacheBuf;
}

//----------------------------------------------------------------------
