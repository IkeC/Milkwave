// plugin_render.cpp - Main frame rendering
// Extracted from plugin.cpp

#include "plugin.h"
#include "plugin_internals.h"
#include "utility.h"
#include "support.h"
#include "resource.h"
#include "defines.h"
#include "shell_defines.h"
#include "wasabi.h"

void CPlugin::MyRenderFn(int redraw) {
  EnterCriticalSection(&g_cs);

  // Render a frame of animation here.
  // This function is called each frame just AFTER BeginScene().
  // For timing information, call 'GetTime()' and 'GetFps()'.
  // The usual formula is like this (but doesn't have to be):
  //   1. take care of timing/other paperwork/etc. for new frame
  //   2. clear the background
  //   3. get ready for 3D drawing
  //   4. draw your 3D stuff
  //   5. call PrepareFor2DDrawing()
  //   6. draw your 2D stuff (overtop of your 3D scene)
  // If the 'redraw' flag is 1, you should try to redraw
  //   the last frame; GetTime, GetFps, and GetFrame should
  //   all return the same values as they did on the last
  //   call to MyRenderFn().  Otherwise, the redraw flag will
  //   be zero, and you can draw a new frame.  The flag is
  //   used to force the desktop to repaint itself when
  //   running in desktop mode and Winamp is paused or stopped.

  //   1. take care of timing/other paperwork/etc. for new frame
  if (!redraw) {
    float dt = GetTime() - m_prev_time;
    m_prev_time = GetTime();  // note: m_prev_time is not for general use!
    m_bPresetLockedByCode = (m_UI_mode != UI_REGULAR);
    if (m_bPresetLockedByUser || m_bPresetLockedByCode) {
      // to freeze time (at current preset time value) when menus are up or Scroll Lock is on:
      // m_fPresetStartTime += dt;
      // m_fNextPresetTime += dt;
      //  OR, to freeze time @ [preset] zero, so that when you exit menus,
      //    you don't run the risk of it changing the preset on you right away:
      m_fPresetStartTime = GetTime();
      m_fNextPresetTime = -1.0f;  // flags UpdateTime() to recompute this.
    }

    // if (!m_bPresetListReady)
    //     UpdatePresetList(true);//UpdatePresetRatings(); // read in a few each frame, til they're all in
  }

  m_bHadFocus = m_bHasFocus;

  HWND plugin = GetPluginWindow();
  HWND focus = GetFocus();
  HWND cur = plugin;

  timetick += 1 / GetFps();  // Now these timeticks variables are now became FPS-independent.
  timetick2 += 1 / GetFps();

  // HardCut Modes (controlled via F11 hotkey)
  if (HardcutMode == 2)  // Bass Blend
  {
    if (GetFps() > 1.0f && !m_bPresetLockedByUser && !m_bPresetLockedByCode) {
      if ((double)mysound.imm_rel[0] > 1.75 && timetick >= 0.99) {
        if (m_nLoadingPreset == 0)
          NextPreset(0.95f);
        timetick = 0;
      }
    }
  }
  if (HardcutMode == 3)  // Bass
  {
    if (GetFps() > 1.0f && !m_bPresetLockedByUser && !m_bPresetLockedByCode)
      if ((double)mysound.imm_rel[0] > 1.75) {
        if (m_nLoadingPreset == 0)
          NextPreset(0.0f);
      }
  }
  if (HardcutMode == 4)  // Middle
  {
    if (GetFps() > 1.0f && !m_bPresetLockedByUser && !m_bPresetLockedByCode)
      if ((double)mysound.imm_rel[1] > 1.75) {
        if (m_nLoadingPreset == 0)
          NextPreset(0.0f);
      }
  }
  if (HardcutMode == 5)  // Treble
  {
    if (GetFps() > 1.0f && !m_bPresetLockedByUser && !m_bPresetLockedByCode)
      if ((double)mysound.imm_rel[2] > 1.75) {
        if (m_nLoadingPreset == 0)
          NextPreset(0.0f);
      }
  }
  if (HardcutMode == 6)  // Bass Fast Blend
  {
    if (GetFps() > 1.0f && !m_bPresetLockedByUser && !m_bPresetLockedByCode)
      if ((double)mysound.imm_rel[0] > 1.75 && timetick >= 0.49) {
        if (m_nLoadingPreset == 0)
          NextPreset(0.4f);
        timetick = 0;
      }
  }
  if (HardcutMode == 7)  // Treble Fast Blend
  {
    if (GetFps() > 1.0f && !m_bPresetLockedByUser && !m_bPresetLockedByCode)
      if ((double)mysound.imm_rel[2] > 1.75 && timetick2 >= 0.49) {
        if (m_nLoadingPreset == 0)
          NextPreset(0.4f);
        timetick2 = 0;
      }
  }
  if (HardcutMode == 8)  // Bass Blend and Hard Cut Treble
  {
    if (GetFps() > 1.0f && !m_bPresetLockedByUser && !m_bPresetLockedByCode) {
      if ((double)mysound.imm_rel[0] > 1.75 && timetick >= 0.48) {
        if (m_nLoadingPreset == 0)
          NextPreset(0.24f);
        timetick = 0;
      }
      if ((double)mysound.imm_rel[2] > 1.75 && timetick2 >= 0.48) {
        if (m_nLoadingPreset == 0)
          NextPreset(0.0f);
        timetick2 = 0;
      }
    }
  }

  if (HardcutMode == 9)  // Rhythmic Hardcut
  {
    if (GetFps() > 1.0f && !m_bPresetLockedByUser && !m_bPresetLockedByCode) {
      if (((double)mysound.imm_rel[0] > 1.75 || (double)mysound.imm_rel[2] > 1.75) && timetick >= 0.23) {
        if (m_nLoadingPreset == 0)
          NextPreset(0.0f);
        timetick = 0;
      }
    }
  }

  if (HardcutMode == 10)  // 2 beats
  {
    if (GetFps() > 1.0f && !m_bPresetLockedByUser && !m_bPresetLockedByCode) {
      if (((double)mysound.imm_rel[0] > 2.05 && timetick >= 0.23)) {
        beatcount++;
        if (beatcount % 2 == 0) {
          if (m_nLoadingPreset == 0)
            NextPreset(0.0f);
        }
        timetick = 0;
      }
    }
    if (timetick >= 1)
      beatcount = -1;
  }

  if (HardcutMode == 11)  // 4 beats
  {
    if (GetFps() > 1.0f && !m_bPresetLockedByUser && !m_bPresetLockedByCode) {
      if (((double)mysound.imm_rel[0] > 2.05 && timetick >= 0.23)) {
        beatcount++;
        if (beatcount % 4 == 0) {
          if (m_nLoadingPreset == 0)
            NextPreset(0.0f);
        }
        timetick = 0;
      }
    }
    if (timetick >= 1)
      beatcount = -1;
  }

  if (HardcutMode == 12)  // Kinetronix (Vizikord) -- Probably we need BPM algorithm for getting in sync
  {
    if (GetFps() > 1.0f && !m_bPresetLockedByUser && !m_bPresetLockedByCode) {
      if (((double)mysound.imm_rel[0] > 2.05 && timetick >= 0.23)) {
        beatcount++;
        if (beatcount % 2 == 0) {
          if (m_nLoadingPreset == 0)
            NextPreset(0.0f);
        } else {
          if (m_nLoadingPreset == 0)
            PrevPreset(0.0f);
        }

        if (beatcount % 32 == 0) {
          {
            if (m_nLoadingPreset == 0)
              NextPreset(0.0f);
            if (m_nLoadingPreset == 0)
              NextPreset(0.0f);
          }
        }  // Double the Next Preset (basically a trick to load 2 presets at the same time)
        timetick = 0;
      }
    }
    if (timetick >= 1)
      beatcount = -1;
  }
  // END

  // Auto-Lock Preset when it's silence.
  if (m_bAutoLockPresetWhenNoMusic) {
    if (((double)mysound.imm_rel[0] + (double)mysound.imm_rel[1] + (double)mysound.imm_rel[2]) == 0) {
      if (TimeToAutoLockPreset <= 2.5)
        TimeToAutoLockPreset += 1 / GetFps();
      else {
        if (!AutoLockedPreset) {
          m_bPresetLockedByUser = true;
          AutoLockedPreset = true;
        }
      }
    }

    else if (((double)mysound.imm_rel[0] + (double)mysound.imm_rel[1] + (double)mysound.imm_rel[2]) != 0) {
      if (AutoLockedPreset) {
        m_bPresetLockedByUser = false;
        AutoLockedPreset = false;
      }
      TimeToAutoLockPreset = 0;
    }
  }
  // END

  if (m_bEnableMouseInteraction) {
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(GetPluginWindow(), &pt);

    RECT clientRect;
    GetClientRect(GetPluginWindow(), &clientRect);

    int clientW = clientRect.right - clientRect.left;
    int clientH = clientRect.bottom - clientRect.top;
    if (clientW <= 1) clientW = 1;
    if (clientH <= 1) clientH = 1;

    // Prefer renderer/backbuffer size if the renderer uses a different internal resolution
    int targetW = clientW;
    int targetH = clientH;
    if (g_plugin.d3dPp.BackBufferWidth > 0 && g_plugin.d3dPp.BackBufferHeight > 0) {
      targetW = static_cast<int>(g_plugin.d3dPp.BackBufferWidth);
      targetH = static_cast<int>(g_plugin.d3dPp.BackBufferHeight);
    } else if (g_plugin.m_WindowWidth > 0 && g_plugin.m_WindowHeight > 0) {
      targetW = g_plugin.m_WindowWidth;
      targetH = g_plugin.m_WindowHeight;
    }
    if (targetW <= 1) targetW = 1;
    if (targetH <= 1) targetH = 1;

    // Map client pixel coords to target pixel space (handles stretched/backbuffer-fixed modes)
    float sx = (static_cast<float>(pt.x) * targetW) / static_cast<float>(clientW);
    float sy = (static_cast<float>(pt.y) * targetH) / static_cast<float>(clientH);

    // Normalize [0 .. target-1] -> [0..1]
    float fx = sx / static_cast<float>(targetW - 1);
    float fy = sy / static_cast<float>(targetH - 1);

    // clamp to [0,1]
    fx = clamp(fx, 0.0f, 1.0f);
    fy = clamp(fy, 0.0f, 1.0f);

    // Convert to lower-left origin: (0,0)=lower-left, (1,1)=upper-right
    m_mouseX = fx;         // 0 = left, 1 = right
    m_mouseY = 1.0f - fy;  // 0 = bottom, 1 = top
  }

  // Duration of the click called from WM_LBUTTONDOWN
  if (m_mouseClicked > 0) {
    m_mouseClicked--;
  }

  // Don't show the help message again when the "Press F1 for help" message is finished.
  // Useful when I press CTRL + T or when it reaches 250000 seconds, it shows the message again, so I did.
  if (GetTime() >= PRESS_F1_DUR)
    m_show_press_f1_msg = 0;

  m_bHasFocus = false;
  do {
    m_bHasFocus = (focus == cur);
    if (m_bHasFocus)
      break;
    cur = GetParent(cur);
  } while (cur != NULL);

  if (m_hTextWnd && focus == m_hTextWnd)
    m_bHasFocus = 1;

  // if (m_bEnablePresetStartup)
  //     if (StartupPresetLoaded == false)
  //     {
  //         LoadPreset(m_szPresetStartup, 0.0f);
  //         StartupPresetLoaded = true;
  //     }  //The Preset Startup Implementation are reworked and moved to line 2560.

  if (GetFocus() == NULL)
    m_bHasFocus = 0;

  if (!redraw) {
    GetSongTitle(m_szSongTitle, sizeof(m_szSongTitle) - 1);
    if (wcscmp(m_szSongTitle, m_szSongTitlePrev)) {
      lstrcpynW(m_szSongTitlePrev, m_szSongTitle, 512);
      if (m_bSongTitleAnims)
        LaunchSongTitleAnim(-1);
    }

    if (m_AutoHue && m_AutoHueSeconds > 0) {
      if (GetTime() > m_AutoHueTimeLastChange + m_AutoHueSeconds) {
        m_AutoHueTimeLastChange = GetTime();
        m_ColShiftHue += 0.01f;
        if (m_ColShiftHue >= 1.0f) {
          m_ColShiftHue = -1.0f;
        }
        SendSettingsInfoToMilkwaveRemote();
      }
    }

    // Push the lyrics status and current line to the Remote whenever they
    // change so the Lyrics tab stays live. No-op when unchanged and nothing is
    // connected.
    if (m_lyricsDisplayEnabled || !m_lastSentLyricsStatus.empty() || !m_lastSentLyricsLine.empty()) {
      std::wstring lyricsStatus = ::milkwave.LyricsMonitorText(m_lyricsDisplayEnabled, m_lyricsOffsetMs);
      std::wstring lyricsLine = m_lyricsDisplayEnabled ? ::milkwave.CurrentLyricText(m_lyricsOffsetMs) : L"";
      for (auto& ch : lyricsStatus) {
        if (ch == L'|') ch = L' ';
      }
      for (auto& ch : lyricsLine) {
        if (ch == L'|') ch = L' ';
      }
      if (lyricsStatus != m_lastSentLyricsStatus) {
        m_lastSentLyricsStatus = lyricsStatus;
        SendMessageToMilkwaveRemote((L"LYRICSSTATUS=" + lyricsStatus).c_str(), true);
      }
      if (lyricsLine != m_lastSentLyricsLine) {
        m_lastSentLyricsLine = lyricsLine;
        SendMessageToMilkwaveRemote((L"LYRICSLINE=" + lyricsLine).c_str(), true);
      }
    }
  }

  // 2. Clear the background:
  // DWORD clear_color = (m_fog_enabled) ? FOG_COLOR : 0xFF000000;
  // GetDevice()->Clear(0, 0, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, clear_color, 1.0f, 0);

  // 5. switch to 2D drawing mode.  2D coord system:
  //         +--------+ Y=-1
  //         |        |
  //         | screen |             Z=0: front of scene
  //         |        |             Z=1: back of scene
  //         +--------+ Y=1
  //       X=-1      X=1
  PrepareFor2DDrawing(GetDevice());

  if (!redraw)
    DoCustomSoundAnalysis();  // emulates old pre-vms milkdrop sound analysis

  RenderFrame(redraw);  // see milkdropfs.cpp

  if (!redraw) {
    m_nFramesSinceResize++;
    if (m_nLoadingPreset > 0) {
      LoadPresetTick();
    }
  }

  LeaveCriticalSection(&g_cs);
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------

void CPlugin::DrawTooltip(wchar_t* str, int xR, int yB) {
  // draws a string in the lower-right corner of the screen.
  // note: ID3DXFont handles DT_RIGHT and DT_BOTTOM *very poorly*.
  //       it is best to calculate the size of the text first,
  //       then place it in the right spot.
  // note: use DT_WORDBREAK instead of DT_WORD_ELLIPSES, otherwise certain fonts'
  //       calcrect (for the dark box) will be wrong.

  RECT r, r2;
  SetRect(&r, 0, 0, xR - TEXT_MARGIN * 2, 2048);
  m_text.DrawTextW(GetFont(TOOLTIP_FONT), str, -1, &r, DT_CALCRECT, 0xFFFFFFFF, false);
  r2.bottom = yB - TEXT_MARGIN;
  r2.right = xR - TEXT_MARGIN;
  r2.left = r2.right - (r.right - r.left);
  r2.top = r2.bottom - (r.bottom - r.top);
  RECT r3 = r2;
  r3.left -= 4;
  r3.top -= 2;
  r3.right += 2;
  r3.bottom += 2;
  DrawDarkTranslucentBox(&r3);
  m_text.DrawTextW(GetFont(TOOLTIP_FONT), str, -1, &r2, 0, 0xFFFFFFFF, false);
}

void CPlugin::OnAltK() {
  AddNotification(wasabiApiLangString(IDS_PLEASE_EXIT_VIS_BEFORE_RUNNING_CONFIG_PANEL));
}

void CPlugin::AddNotification(wchar_t* szMsg) {
  g_plugin.AddError(szMsg, 3.0F, ERR_NOTIFY, m_fontinfo[SIMPLE_FONT].bBold);
}

void CPlugin::AddNotification(wchar_t* szMsg, float time) {
  g_plugin.AddError(szMsg, time, ERR_NOTIFY, m_fontinfo[SIMPLE_FONT].bBold);
}

void CPlugin::AddNotificationAudioDevice() {
  std::wstring statusMessage;
  if (m_szAudioDeviceDisplayName[0] != L'\0') {
    statusMessage = m_szAudioDeviceDisplayName;
  } else if (g_plugin.m_szAudioDeviceDisplayName[0] != L'\0') {
    statusMessage = g_plugin.m_szAudioDeviceDisplayName;
  } else if (g_plugin.m_szAudioDevice[0] != L'\0') {
    statusMessage = g_plugin.m_szAudioDevice;
  }

  int effectiveType = m_nAudioDeviceActiveType;
  if (effectiveType == 0) {
    effectiveType = m_nAudioDeviceRequestType;
  }

  const wchar_t* tag = nullptr;
  if (effectiveType == 1) {
    tag = L" [In]";
  } else if (effectiveType == 2) {
    tag = L" [Out]";
  }

  if (!statusMessage.empty() && tag != nullptr) {
    if (statusMessage.find(tag) == std::wstring::npos) {
      statusMessage += tag;
    }
  }

  if (!statusMessage.empty()) {
    AddNotification(statusMessage.data());
  } else {
    AddNotification(g_plugin.m_szAudioDeviceDisplayName);
  }
}

void CPlugin::AddError(wchar_t* szMsg, float fDuration, int category, bool bBold) {
  OutputDebugStringW(szMsg);
  if (category == ERR_NOTIFY)
    ClearErrors(category);

  assert(category != ERR_ALL);
  ErrorMsg x;
  x.msg = szMsg;
  x.birthTime = GetTime();
  x.expireTime = GetTime() + fDuration;
  x.category = category;
  x.bBold = bBold;
  x.bSentToRemote = false;  // not sent to remote yet
  m_errors.push_back(x);
}

void CPlugin::ClearErrors(int category)  // 0=all categories
{
  int N = m_errors.size();
  for (int i = 0; i < N; i++)
    if (category == ERR_ALL || m_errors[i].category == category) {
      m_errors.erase(m_errors.begin() + i);
      i--;
      N--;
    }
}
