/*
  LICENSE
  -------
Copyright 2005-2013 Nullsoft, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

  * Neither the name of Nullsoft nor the names of its contributors may be used to
    endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
    TO DO
    -----
    -done/v1.06:
        -(nothing yet)
        -
        -
    -to do/v1.06:
        -FFT: high freq. data kinda sucks because of the 8-bit samples we get in;
            look for justin to put 16-bit vis data into wa5.
        -make an 'advanced view' button on config panel; hide complicated stuff
            til they click that.
        -put an asterisk(*) next to the 'max framerate' values that
            are ideal (given the current windows display mode or selected FS dispmode).
        -or add checkbox: "smart sync"
            -> matches FPS limit to nearest integer divisor of refresh rate.
        -debug.txt/logging support!
        -audio: make it a DSP plugin? then we could get the complete, continuous waveform
            and overlap our waveform windows, so we'd never miss a brief high note.
        -bugs:
            -vms plugins sometimes freeze after a several-minute pause; I've seen it
                with most of them.  hard to repro, though.
            -running FS on monitor 2, hit ALT-TAB -> minimizes!!!
                -but only if you let go of TAB first.  Let go of ALT first and it's fine!
                -> means it's related to the keyup...
            -fix delayloadhelper leak; one for each launch to config panel/plugin.
            -also, delayload(d3d9.dll) still leaks, if plugin has error initializing and
                quits by returning false from PluginInitialize().
        -add config panel option to ignore fake-fullscreen tips
            -"tip" boxes in dxcontext.cpp
            -"notice" box on WM_ACTIVATEAPP?
        -desktop mode:
            -icon context menus: 'send to', 'cut', and 'copy' links do nothing.
                -http://netez.com/2xExplorer/shellFAQ/bas_context.html
            -create a 2nd texture to render all icon text labels into
                (they're the sole reason that desktop mode is slow)
            -in UpdateIconBitmaps, don't read the whole bitmap and THEN
                realize it's a dupe; try to compare icon filename+index or somethign?
            -DRAG AND DROP.  COMPLICATED; MANY DETAILS.
                -http://netez.com/2xExplorer/shellFAQ/adv_drag.html
                -http://www.codeproject.com/shell/explorerdragdrop.asp
                -hmm... you can't drag icons between the 2 desktops (ugh)
            -multiple delete/open/props/etc
            -delete + enter + arrow keys.
            -try to solve mysteries w/ShellExecuteEx() and desktop *shortcuts* (*.lnk).
            -(notice that when icons are selected, they get modulated by the
                highlight color, when they should be blended 50% with that color.)

    ---------------------------
    final touches:
        -Tests:
            -make sure desktop still functions/responds properly when winamp paused
            -desktop mode + multimon:
                -try desktop mode on all monitors
                -try moving taskbar around; make sure icons are in the
                    right place, that context menus (general & for
                    specific icons) pop up in the right place, and that
                    text-off-left-edge is ok.
                -try setting the 2 monitors to different/same resolutions
        -check tab order of config panel controls!
        -Clean All
        -build in release mode to include in the ZIP
        -leave only one file open in workspace: README.TXT.
        -TEMPORARILY "ATTRIB -R" ALL FILES BEFORE ZIPPING THEM!

    ---------------------------
    KEEP IN VIEW:
        -EMBEDWND:
            -kiv: on resize of embedwnd, it's out of our control; winamp
                resizes the child every time the mouse position changes,
                and we have to cleanup & reallocate everything, b/c we
                can't tell when the resize begins & ends.
                [justin said he'd fix in wa5, though]
            -kiv: with embedded windows of any type (plugin, playlist, etc.)
                you can't place the winamp main wnd over them.
            -kiv: embedded windows are child windows and don't get the
                WM_SETFOCUS or WM_KILLFOCUS messages when they get or lose
                the focus.  (For a workaround, see milkdrop & scroll lock key.)
            -kiv: tiny bug (IGNORE): when switching between embedwnd &
                no-embedding, the window gets scooted a tiny tiny bit.
        -kiv: fake fullscreen mode w/multiple monitors: there is no way
            to keep the taskbar from popping up [potentially overtop of
            the plugin] when you click on something besides the plugin.
            To get around this, use true fullscreen mode.
        -kiv: max_fps implementation assumptions:
            -that most computers support high-precision timer
            -that no computers [regularly] sleep for more than 1-2 ms
                when you call Sleep(1) after timeBeginPeriod(1).
        -reminder: if vms_desktop.dll's interface needs changed,
            it will have to be renamed!  (version # upgrades are ok
            as long as it won't break on an old version; if the
            new functionality is essential, rename the DLL.)

    ---------------------------
    REMEMBER:
        -GF2MX + GF4 have icon scooting probs in desktop mode
            (when taskbar is on upper or left edge of screen)
        -Radeon is the one w/super slow text probs @ 1280x1024.
            (it goes unstable after you show playlist AND helpscr; -> ~1 fps)
        -Mark's win98 machine has hidden cursor (in all modes),
            but no one else seems to have this problem.
        -links:
            -win2k-only-style desktop mode: (uses VirtualAllocEx, vs. DLL Injection)
                http://www.digiwar.com/scripts/renderpage.php?section=2&subsection=2
            -http://www.experts-exchange.com/Programming/Programming_Platforms/Win_Prog/Q_20096218.html
*/
#include "pluginshell.h"
#include "utility.h"
#include "defines.h"
#include "shell_defines.h"
#include "resource.h"
#include "wasabi.h"
#include "support.h"       // WFVERTEX, SPRITEVERTEX
#include "dx12pipeline.h"  // PSO enum
#include <multimon.h>
#include "AutoCharFn.h"
#include <mmsystem.h>
#pragma comment(lib,"winmm.lib")    // for timeGetTime
#pragma comment(lib,"user32.lib")  // ensure GetSystemMetrics (user32) is linked

#define clamp(value, min, max) ((value) < (min) ? (min) : ((value) > (max) ? (max) : (value)))

// STATE VALUES & VERTEX FORMATS FOR HELP SCREEN TEXTURE:
#define TEXT_SURFACE_NOT_READY  0
#define TEXT_SURFACE_REQUESTED  1
#define TEXT_SURFACE_READY      2
#define TEXT_SURFACE_ERROR      3
typedef struct _HELPVERTEX {
  float x, y;      // screen position
  float z;         // Z-buffer depth
  DWORD Diffuse;   // diffuse color. also acts as filler; aligns struct to 16 bytes (good for random access/indexed prims)
  float tu, tv;    // texture coordinates for texture #0
} HELPVERTEX, * LPHELPVERTEX;
#define HELP_VERTEX_FORMAT (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1)
typedef struct _SIMPLEVERTEX {
  float x, y;      // screen position
  float z;         // Z-buffer depth
  DWORD Diffuse;   // diffuse color. also acts as filler; aligns struct to 16 bytes (good for random access/indexed prims)
} SIMPLEVERTEX, * LPSIMPLEVERTEX;
#define SIMPLE_VERTEX_FORMAT (D3DFVF_XYZ | D3DFVF_DIFFUSE)

extern wchar_t* g_szHelp;
extern wchar_t* g_szHelp_Page2;
extern int g_szHelp_W;

// resides in vms_desktop.dll/lib:
//void getItemData(int x);

// LJ DEBUG
static int bdn = 0;
static BOOL CALLBACK GetWindowNames(HWND h, LPARAM l) {
  char search_window_name[MAX_PATH];

  if (h == NULL) {
    printf("GetWindowNames - null handle\n");
    return FALSE;
  }

  if (IsWindow(h) && IsWindowVisible(h)) {
    GetWindowTextA(h, search_window_name, MAX_PATH);
    if (search_window_name[0]) {
      // printf("GetWindowNames - %s (%x)\n", search_window_name, h);
      // Does the search window name contain "Milkwave" ?
      if (strstr(search_window_name, "Milkwave Visualizer") != NULL) {
        // printf("Found BeatDrop (%d)\n", bdn);
        bdn++;
      }
    }
  }
  return TRUE;
}



CPluginShell::CPluginShell() {
  // this should remain empty!
}

CPluginShell::~CPluginShell() {
  // this should remain empty!
}

int       CPluginShell::GetFrame() {
  //return (int)m_frame * m_frameFactor;
  return m_frame;
};
float     CPluginShell::GetTime() {
  return m_time * m_timeFactor;
};
float     CPluginShell::GetFps() {
  return m_fps * m_fpsFactor;
};

HWND      CPluginShell::GetPluginWindow() {
  if (m_lpDX) return m_lpDX->GetHwnd();       else return NULL;
};
int       CPluginShell::GetWidth() {
  if (m_lpDX) {
    if (IsSpoutActiveAndFixed()) {
      return m_lpDX->m_backbuffer_width;
    }
    else {
      return m_lpDX->m_client_width;
    }
  }
  else return 0;
};

int       CPluginShell::GetHeight() {
  if (m_lpDX) {
    if (IsSpoutActiveAndFixed()) {
      return m_lpDX->m_backbuffer_height;
    }
    else {
      return m_lpDX->m_client_height;
    }
  }
}

int       CPluginShell::GetCanvasMarginX() {
  if (m_lpDX) return (m_lpDX->m_client_width - m_lpDX->m_REAL_client_width) / 2;
  else return 0;
};
int       CPluginShell::GetCanvasMarginY() {
  if (m_lpDX) return (m_lpDX->m_client_height - m_lpDX->m_REAL_client_height) / 2;
  else return 0;
};
HINSTANCE CPluginShell::GetInstance() {
  return m_hInstance;
};
wchar_t* CPluginShell::GetPluginsDirPath() {
  return m_szPluginsDirPath;
};
wchar_t* CPluginShell::GetConfigIniFile() {
  return m_szConfigIniFile;
};
char* CPluginShell::GetConfigIniFileA() {
  return m_szConfigIniFileA;
}
int  CPluginShell::GetFontHeight(eFontIndex idx) {
  if (idx >= 0 && idx < NUM_BASIC_FONTS + NUM_EXTRA_FONTS) {
    if (IsSpoutActiveAndFixed()) {
      return (int)m_fontinfo[idx].nSize;
    }
    else {
      return (int)m_fontinfo[idx].nSize * m_fRenderQuality;
    }
  }
  else return 0;
};
int CPluginShell::GetBitDepth() {
  return m_lpDX->GetBitDepth();
};

// GetDevice() is declared inline in pluginshell.h returning LPDIRECT3DDEVICE9 = nullptr.
// GetDX12Device() returns the real ID3D12Device* for Phase 2+ DX12 code.
ID3D12Device* CPluginShell::GetDX12Device() {
  return m_lpDX ? m_lpDX->m_device.Get() : nullptr;
}

ID3D12GraphicsCommandList* CPluginShell::GetCommandList() {
  return m_lpDX ? m_lpDX->m_commandList.Get() : nullptr;
}

int CPluginShell::InitNondx9Stuff() {
  timeBeginPeriod(1);
  m_fftobj.Init(576, NUM_FREQUENCIES);
  if (!InitGDIStuff()) return false;
  return AllocateMyNonDx9Stuff();
}

void CPluginShell::CleanUpNondx9Stuff() {
  timeEndPeriod(1);
  CleanUpMyNonDx9Stuff();
  CleanUpGDIStuff();
  m_fftobj.CleanUp();
}

int CPluginShell::InitGDIStuff() {
  wchar_t title[64];
  // note: messagebox parent window should be NULL here, because lpDX is still NULL!
  for (int i = 0; i < NUM_BASIC_FONTS + NUM_EXTRA_FONTS; i++) {
    if (!(m_font[i] = CreateFontW(m_fontinfo[i].nSize, 0, 0, 0, m_fontinfo[i].bBold ? 900 : 400, m_fontinfo[i].bItalic, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, m_fontinfo[i].bAntiAliased ? ANTIALIASED_QUALITY : DEFAULT_QUALITY, DEFAULT_PITCH, m_fontinfo[i].szFace))) {
      MessageBoxW(NULL, wasabiApiLangString(IDS_ERROR_CREATING_GDI_FONTS),
        wasabiApiLangString(IDS_MILKDROP_ERROR, title, 64),
        MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
      return false;
    }
  }

  return true;
}

void CPluginShell::CleanUpGDIStuff() {
  for (int i = 0; i < NUM_BASIC_FONTS + NUM_EXTRA_FONTS; i++) {
    if (m_font[i]) {
      DeleteObject(m_font[i]);
      m_font[i] = NULL;
    }
  }
}

int CPluginShell::InitVJStuff(RECT* pClientRect) {
  // VJ mode (secondary DX9 window) not yet implemented in DX12 migration.
  // Phase 5 TODO: DX12 swap chain + DirectXTK12 SpriteFont for VJ text window.
  return true;
}

#if 0 // InitVJStuff DX9 body — preserved for Phase 5 reference, does not compile
int CPluginShell::InitVJStuff_DX9_REMOVED(RECT* pClientRect) {
  wchar_t title[64];

  // Init VJ mode (second window for text):
  if (m_vj_mode) {
    DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_SYSMENU;
    POINT upper_left_corner;
    upper_left_corner.x = 0;
    upper_left_corner.y = 0;

    // Create direct 3d & get some infos
    if (!(m_vjd3d9 = Direct3DCreate9(D3D_SDK_VERSION))) {
      MessageBoxW(NULL, wasabiApiLangString(IDS_ERROR_CREATING_DIRECT3D_DEVICE_FOR_VJ_MODE),
        wasabiApiLangString(IDS_MILKDROP_ERROR, title, 64), MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
      return false;
    }

    // Get ordinal adapter # for the currently-selected Windowed Mode display adapter
    int ordinal_adapter = D3DADAPTER_DEFAULT;
    int nAdapters = m_vjd3d9->GetAdapterCount();
    for (int i = 0; i < nAdapters; i++) {
      D3DADAPTER_IDENTIFIER9 temp;
      if ((m_vjd3d9->GetAdapterIdentifier(i, /*D3DENUM_NO_WHQL_LEVEL*/ 0, &temp) == D3D_OK) &&
        (memcmp(&temp.DeviceIdentifier, &m_adapter_guid_windowed, sizeof(GUID)) == 0)) {
        ordinal_adapter = i;
        break;
      }
    }

    // Get current display mode for windowed-mode adapter:
    D3DDISPLAYMODE dm;
    if (D3D_OK != m_vjd3d9->GetAdapterDisplayMode(ordinal_adapter, &dm)) {
      MessageBoxW(NULL, wasabiApiLangString(IDS_VJ_MODE_INIT_ERROR),
        wasabiApiLangString(IDS_MILKDROP_ERROR, title, 64),
        MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
      return false;
    }

    // And get the upper-left corner of the monitor for it:
    HMONITOR hMon = m_vjd3d9->GetAdapterMonitor(ordinal_adapter);
    if (hMon) {
      MONITORINFO mi;
      mi.cbSize = sizeof(mi);
      //if (GetMonitorInfo(hMon, &mi))
      //{
      //	upper_left_corner.x = mi.rcWork.left;
      //	upper_left_corner.y = mi.rcWork.top;
      //}
    }

    // CREATE THE WINDOW

    RECT rect;
    if (pClientRect) {
      rect = *pClientRect;
      AdjustWindowRect(&rect, dwStyle, 0); // convert client->wnd
    }
    else {
      // SPOUT - make help screen wider
      // SetRect(&rect, 0, 0, 384, 384);
      SetRect(&rect, 0, 0, 720, 720);
      AdjustWindowRect(&rect, dwStyle, 0); // convert client->wnd

      rect.right -= rect.left;
      rect.left = 0;
      rect.bottom -= rect.top;
      rect.top = 0;

      rect.top += upper_left_corner.y + 32;
      rect.left += upper_left_corner.x + 32;
      rect.right += upper_left_corner.x + 32;
      rect.bottom += upper_left_corner.y + 32;
    }

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = VJModeWndProc;				// our window procedure
    wc.hInstance = GetInstance();	// hInstance of DLL
    wc.hIcon = LoadIcon(GetInstance(), MAKEINTRESOURCE(IDI_PLUGIN_ICON));
    wc.lpszClassName = TEXT_WINDOW_CLASSNAME;			// our window class name
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS; // CS_DBLCLKS lets the window receive WM_LBUTTONDBLCLK, for toggling fullscreen mode...
    wc.cbWndExtra = sizeof(DWORD);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

    if (!RegisterClass(&wc)) {
      MessageBoxW(NULL, wasabiApiLangString(IDS_ERROR_REGISTERING_WINDOW_CLASS_FOR_TEXT_WINDOW),
        wasabiApiLangString(IDS_MILKDROP_ERROR, title, 64),
        MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
      return false;
    }
    m_bTextWindowClassRegistered = true;

    //DWORD nThreadID;
    //CreateThread(NULL, 0, TextWindowThread, &rect, 0, &nThreadID);

    // =====================================
    // SPOUT
    // The render window title will have been modified
    // to show the number of BeatDrop instances.
    // Copy this number to the VJ text window title
    // The render window handle is already saved
    char consoletitle[64];
    strcpy_s(consoletitle, 64, TEXT_WINDOW_CLASSNAME); // Default is the class name re-used
    char temp[64];
    int nc = GetWindowTextA(m_hRenderWnd, temp, 64);
    // The return value is the number of characters
    // Default is "Milkwave Visualizer" (20 chars)
    // (see Milkdrop2PcmVisualzer.cpp)
    if (nc > 20) {
      // Get the _01 - _02 etc.. appended for multiple instances
      std::string str1 = temp;
      std::string str2 = str1.substr(20);
      // Append to the text window title
      strcat_s(consoletitle, 64, str2.c_str());
    }
    // =====================================

    // Create the text window
    m_hTextWnd = CreateWindowEx(
      0,
      TEXT_WINDOW_CLASSNAME,				// our window class name
      // SPOUT
// ===============
consoletitle,
// TEXT_WINDOW_CLASSNAME,				// use description for a window title
// ===============
dwStyle,
rect.left, rect.top,								// screen position (read from config)
rect.right - rect.left, rect.bottom - rect.top,  // width & height of window (need to adjust client area later)
NULL,								// parent window (winamp main window)
NULL,								// no menu
GetInstance(),						// hInstance of DLL
NULL
); // no window creation data

    if (!m_hTextWnd) {
      MessageBoxW(NULL, wasabiApiLangString(IDS_ERROR_CREATING_VJ_WINDOW),
        wasabiApiLangString(IDS_MILKDROP_ERROR, title, 64),
        MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
      return false;
    }

    SetWindowLongPtr(m_hTextWnd, GWLP_USERDATA, (LONG_PTR)this);

    // SPOUT - remove close button
    //EnableMenuItem(GetSystemMenu(m_hTextWnd, FALSE), SC_CLOSE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

    // SPOUT
    // LJ
    // Hide VJ window if NestImmersion UI is open
    // For compile option set m_vj_mode = 0 in pluginshell.cpp
    if (FindWindowA(NULL, "BeatDropUI") != NULL)
      ShowWindow(m_hTextWnd, SW_HIDE);
    else if (IsIconic(m_hTextWnd))
      ShowWindow(m_hTextWnd, SW_RESTORE);

    GetClientRect(m_hTextWnd, &rect);
    m_nTextWndWidth = rect.right - rect.left;
    m_nTextWndHeight = rect.bottom - rect.top;


    // Create the device
    D3DPRESENT_PARAMETERS pres_param;
    ZeroMemory(&pres_param, sizeof(pres_param));
    pres_param.BackBufferCount = 0;
    pres_param.BackBufferFormat = dm.Format;
    pres_param.BackBufferWidth = rect.right - rect.left;
    pres_param.BackBufferHeight = rect.bottom - rect.top;
    pres_param.hDeviceWindow = m_hTextWnd;
    pres_param.AutoDepthStencilFormat = D3DFMT_D16;
    pres_param.EnableAutoDepthStencil = FALSE;
    pres_param.SwapEffect = D3DSWAPEFFECT_DISCARD;
    pres_param.MultiSampleType = D3DMULTISAMPLE_NONE;
    pres_param.Flags = 0;
    pres_param.FullScreen_RefreshRateInHz = 0;
    pres_param.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;//D3DPRESENT_INTERVAL_ONE;//D3DPRESENT_INTERVAL_IMMEDIATE;//m_current_mode.allow_page_tearing ? D3DPRESENT_INTERVAL_IMMEDIATE : D3DPRESENT_INTERVAL_ONE;//D3DPRESENT_INTERVAL_IMMEDIATE;//D3DPRESENT_INTERVAL_ONE;
    //pres_param.FullScreen_PresentationInterval = 0;
    pres_param.Windowed = TRUE;

    HRESULT hr;
    if (D3D_OK != (hr = m_vjd3d9->CreateDevice(ordinal_adapter,//D3DADAPTER_DEFAULT,
      D3DDEVTYPE_HAL,
      m_hTextWnd,
      D3DCREATE_SOFTWARE_VERTEXPROCESSING,
      &pres_param,
      &m_vjd3d9_device))) {
      m_vjd3d9_device = NULL;
      MessageBoxW(m_lpDX->GetHwnd(), wasabiApiLangString(IDS_ERROR_CREATING_D3D_DEVICE_FOR_VJ_MODE),
        wasabiApiLangString(IDS_MILKDROP_ERROR, title, 64),
        MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
      return false;
    }

    if (!AllocateFonts())
      return false;

    if (m_fix_slow_text)
      AllocateTextSurface();

    m_text.Finish();
    m_text.Init(nullptr, nullptr, 0); // VJ mode: secondary DX9 device removed in DX12 migration

    m_bClearVJWindow = true;
  }

  return true;
}
#endif // InitVJStuff DX9 body

void CPluginShell::CleanUpVJStuff() {
  // VJ secondary DX9 device removed in DX12 migration.
  // GPU work is flushed in CleanUpDX9Stuff via WaitForGpu() before resource cleanup.
  // VJ DX9 device removed in DX12 migration — texture cleanup no longer needed here.

  if (!m_vj_mode)
    return;

  // clean up VJ mode (VJ secondary DX9 device removed in DX12 migration)
  {
    CleanUpFonts();

    if (m_hTextWnd) {
      //dumpmsg("Finish: destroying text window");
      DestroyWindow(m_hTextWnd);
      m_hTextWnd = NULL;
      //dumpmsg("Finish: text window destroyed");
    }

    if (m_bTextWindowClassRegistered) {
      //dumpmsg("Finish: unregistering text window class");
      UnregisterClass(TEXT_WINDOW_CLASSNAME, GetInstance()); // unregister window class
      m_bTextWindowClassRegistered = false;
      //dumpmsg("Finish: text window class unregistered");
    }
  }
}

int CPluginShell::AllocateFonts() {
  // Phase 5 TODO: replace with DirectXTK12 SpriteFont.
  // For now, record expected font heights from GDI font metrics for layout.
  for (int i = 0; i < NUM_BASIC_FONTS + NUM_EXTRA_FONTS; i++) {
    int fSize = m_fontinfo[i].nSize;
    if (!IsSpoutActiveAndFixed())
      fSize = (int)(fSize * m_fRenderQuality);
    m_fontHeight[i] = fSize; // approximate height — GDI metric
  }
  return true;
}

void CPluginShell::CleanUpFonts() {
  // Phase 5 TODO: release SpriteFont objects here.
  for (int i = 0; i < NUM_BASIC_FONTS + NUM_EXTRA_FONTS; i++)
    m_fontHeight[i] = 0;
}

void CPluginShell::AllocateTextSurface() {
  // Phase 5 TODO: create the DX12 text render target resource here.
  // For now this is a no-op.
}

int CPluginShell::AllocateDX9Stuff() {
  if (!m_vj_mode) {
    AllocateFonts();
    if (m_fix_slow_text)
      AllocateTextSurface();
  }

  int ret = AllocateMyDX9Stuff();

  // invalidate various 'caches' here:
  m_playlist_top_idx = -1;    // invalidating playlist cache forces recompute of playlist width
  //m_icon_list.clear();      // clear desktop mode icon list, so it has to read the bitmaps back in

  if (!m_vj_mode) {
    m_text.Finish();
    m_text.Init(GetDX12Device(), nullptr, 1); // Phase 5: text surface replaced by DX12 resource
  }

  return ret;
}

void CPluginShell::CleanUpDX9Stuff(int final_cleanup) {
  // In DX12, wait for GPU idle before releasing resources
  if (m_lpDX) {
    m_lpDX->WaitForGpu();
  }

  // ALWAYS unbind the textures before releasing textures,
  // otherwise they might still have a hanging reference!
  if (!m_vj_mode) {
    CleanUpFonts();
    // Release help overlay texture (will be re-created lazily on next F1)
    m_helpTexture.Reset();
    m_helpUploadBuffer.Reset();
    m_helpTexturePage = 0;
  }

  CleanUpMyDX9Stuff(final_cleanup);
}

void CPluginShell::OnUserResizeTextWindow() {
  // Update window properties
  RECT w, c;
  GetWindowRect(m_hTextWnd, &w);
  GetClientRect(m_hTextWnd, &c);

  WINDOWPLACEMENT wp;
  ZeroMemory(&wp, sizeof(wp));
  wp.length = sizeof(wp);
  GetWindowPlacement(m_hTextWnd, &wp);

  // convert client rect from client coords to screen coords:
  // (window rect is already in screen coords...)
  POINT p;
  p.x = c.left;
  p.y = c.top;
  if (ClientToScreen(m_hTextWnd, &p)) {
    c.left += p.x;
    c.right += p.x;
    c.top += p.y;
    c.bottom += p.y;
  }

  if (wp.showCmd != SW_SHOWMINIMIZED) {
    if (m_nTextWndWidth != c.right - c.left ||
      m_nTextWndHeight != c.bottom - c.top) {
      CleanUpVJStuff();
      if (!InitVJStuff(&c)) {
        SuggestHowToFreeSomeMem();
        m_lpDX->m_ready = false;   // flag to exit
        return;
      }
    }

    // save the new window position:
    //if (wp.showCmd==SW_SHOWNORMAL)
    //    SaveTextWindowPos();
  }
}

void CPluginShell::OnUserResizeWindow() {

  // Update window properties
  RECT w, c;
  GetWindowRect(m_lpDX->GetHwnd(), &w);
  GetClientRect(m_lpDX->GetHwnd(), &c);

  WINDOWPLACEMENT wp;
  ZeroMemory(&wp, sizeof(wp));
  wp.length = sizeof(wp);
  GetWindowPlacement(m_lpDX->GetHwnd(), &wp);

  // convert client rect from client coords to screen coords:
  // (window rect is already in screen coords...)
  POINT p;
  p.x = c.left;
  p.y = c.top;
  if (ClientToScreen(m_lpDX->GetHwnd(), &p)) {
    c.left += p.x;
    c.right += p.x;
    c.top += p.y;
    c.bottom += p.y;
  }

  if (wp.showCmd != SW_SHOWMINIMIZED) {
    int new_REAL_client_w = c.right - c.left;
    int new_REAL_client_h = c.bottom - c.top;

    // kiv: could we just resize when the *snapped* w/h changes?  slightly more ideal...
    if (m_lpDX->m_REAL_client_width != new_REAL_client_w || m_lpDX->m_REAL_client_height != new_REAL_client_h) {

      //if (true) {
        //CleanUpVJStuff();

      if (true) {

        //if (m_lpDX->m_REAL_client_width != new_REAL_client_w || m_lpDX->m_REAL_client_height != new_REAL_client_h) {
        CleanUpDX9Stuff(0);
        //}
        if (!m_lpDX->OnUserResizeWindow(&w, &c, false)) {
          // note: a basic warning messagebox will have already been given.
          // now suggest specific advice on how to regain more video memory:
          SuggestHowToFreeSomeMem();
          return;
        }
        int newW = c.right - c.left;
        int newH = c.bottom - c.top;
        if (IsSpoutActiveAndFixed()) {
          newW = nSpoutFixedWidth;
          newH = nSpoutFixedHeight;
        }
        SetVariableBackBuffer(newW, newH);
        UpdateBackBufferTracking(newW, newH);
        // DX12: resize the swap chain instead of resetting the device
        m_lpDX->ResizeSwapChain(newW, newH);
      }
      //if (m_lpDX->m_REAL_client_width != new_REAL_client_w || m_lpDX->m_REAL_client_height != new_REAL_client_h) {
      if (!AllocateDX9Stuff()) {
        m_lpDX->m_ready = false;   // flag to exit
        return;
      }
      //}
      /*if (!InitVJStuff())
      {
          m_lpDX->m_ready = false;   // flag to exit
          return;
      }*/
    }

    // save the new window position:
    if (wp.showCmd == SW_SHOWNORMAL)
      m_lpDX->SaveWindow();
  }
}

void CPluginShell::StuffParams(DXCONTEXT_PARAMS* pParams) {
  // display_mode (D3DDISPLAYMODEEX) removed in DX12 migration
  pParams->nbackbuf = 1;
  pParams->m_dualhead_horz = m_dualhead_horz;
  pParams->m_dualhead_vert = m_dualhead_vert;
  pParams->m_skin = m_skin;
  pParams->allow_page_tearing = m_allow_page_tearing_w;
  pParams->adapter_guid = m_adapter_guid_windowed;
  // multisamp removed in DX12 migration (MSAA configured via PSO desc instead)
  strcpy(pParams->adapter_devicename, m_adapter_devicename_windowed);
  pParams->parent_window = NULL;
}

int CPluginShell::InitDirectX(
    ID3D12Device*       device,
    ID3D12CommandQueue* commandQueue,
    IDXGIFactory4*      factory,
    HWND                hwnd,
    int                 width,
    int                 height)
{
  if (!device || !commandQueue || !factory) {
    wchar_t title[64];
    MessageBoxW(NULL, wasabiApiLangString(IDS_UNABLE_TO_INIT_DXCONTEXT),
      wasabiApiLangString(IDS_MILKDROP_ERROR, title, 64),
      MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
    return FALSE;
  }

  m_lpDX = new DXContext(device, commandQueue, factory, hwnd, width, height, m_szConfigIniFile);

  if (!m_lpDX) {
    wchar_t title[64];
    MessageBoxW(NULL, wasabiApiLangString(IDS_UNABLE_TO_INIT_DXCONTEXT),
      wasabiApiLangString(IDS_MILKDROP_ERROR, title, 64),
      MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
    return FALSE;
  }

  if (m_lpDX->m_lastErr != S_OK) {
    delete m_lpDX;
    m_lpDX = nullptr;
    return FALSE;
  }

  DXCONTEXT_PARAMS params;
  StuffParams(&params);
  m_lpDX->StartOrRestartDevice(&params);

  return TRUE;
}

void CPluginShell::CleanUpDirectX() {
  SafeDelete(m_lpDX);
}

int CPluginShell::PluginPreInitialize(HWND hWinampWnd, HINSTANCE hWinampInstance) {

  // PROTECTED CONFIG PANEL SETTINGS (also see 'private' settings, below)
  m_start_fullscreen = 0;
  m_start_desktop = 0;
  m_fake_fullscreen_mode = 0;
  m_max_fps_fs = 144;
  m_max_fps_dm = 144;
  m_max_fps_w = 144;
  m_show_press_f1_msg = 1;
  m_allow_page_tearing_w = 1;
  m_allow_page_tearing_fs = 0;
  m_allow_page_tearing_dm = 0;
  m_minimize_winamp = 1;
  m_desktop_show_icons = 1;
  m_desktop_textlabel_boxes = 1;
  m_desktop_manual_icon_scoot = 0;
  m_desktop_555_fix = 2;
  m_dualhead_horz = 2;
  m_dualhead_vert = 1;
  m_save_cpu = 1;
  m_skin = 1;
  m_fix_slow_text = 0;

  // initialize font settings:
  wcscpy(m_fontinfo[SIMPLE_FONT].szFace, SIMPLE_FONT_DEFAULT_FACE);
  m_fontinfo[SIMPLE_FONT].nSize = SIMPLE_FONT_DEFAULT_SIZE;
  m_fontinfo[SIMPLE_FONT].bBold = SIMPLE_FONT_DEFAULT_BOLD;
  m_fontinfo[SIMPLE_FONT].bItalic = SIMPLE_FONT_DEFAULT_ITAL;
  m_fontinfo[SIMPLE_FONT].bAntiAliased = SIMPLE_FONT_DEFAULT_AA;

  wcscpy(m_fontinfo[DECORATIVE_FONT].szFace, DECORATIVE_FONT_DEFAULT_FACE);
  m_fontinfo[DECORATIVE_FONT].nSize = DECORATIVE_FONT_DEFAULT_SIZE;
  m_fontinfo[DECORATIVE_FONT].bBold = DECORATIVE_FONT_DEFAULT_BOLD;
  m_fontinfo[DECORATIVE_FONT].bItalic = DECORATIVE_FONT_DEFAULT_ITAL;
  m_fontinfo[DECORATIVE_FONT].bAntiAliased = DECORATIVE_FONT_DEFAULT_AA;

  wcscpy(m_fontinfo[HELPSCREEN_FONT].szFace, HELPSCREEN_FONT_DEFAULT_FACE);
  m_fontinfo[HELPSCREEN_FONT].nSize = HELPSCREEN_FONT_DEFAULT_SIZE;
  m_fontinfo[HELPSCREEN_FONT].bBold = HELPSCREEN_FONT_DEFAULT_BOLD;
  m_fontinfo[HELPSCREEN_FONT].bItalic = HELPSCREEN_FONT_DEFAULT_ITAL;
  m_fontinfo[HELPSCREEN_FONT].bAntiAliased = HELPSCREEN_FONT_DEFAULT_AA;

  wcscpy(m_fontinfo[PLAYLIST_FONT].szFace, PLAYLIST_FONT_DEFAULT_FACE);
  m_fontinfo[PLAYLIST_FONT].nSize = PLAYLIST_FONT_DEFAULT_SIZE;
  m_fontinfo[PLAYLIST_FONT].bBold = PLAYLIST_FONT_DEFAULT_BOLD;
  m_fontinfo[PLAYLIST_FONT].bItalic = PLAYLIST_FONT_DEFAULT_ITAL;
  m_fontinfo[PLAYLIST_FONT].bAntiAliased = PLAYLIST_FONT_DEFAULT_AA;

#if (NUM_EXTRA_FONTS >= 1)
  wcscpy(m_fontinfo[NUM_BASIC_FONTS + 0].szFace, EXTRA_FONT_1_DEFAULT_FACE);
  m_fontinfo[NUM_BASIC_FONTS + 0].nSize = EXTRA_FONT_1_DEFAULT_SIZE;
  m_fontinfo[NUM_BASIC_FONTS + 0].bBold = EXTRA_FONT_1_DEFAULT_BOLD;
  m_fontinfo[NUM_BASIC_FONTS + 0].bItalic = EXTRA_FONT_1_DEFAULT_ITAL;
  m_fontinfo[NUM_BASIC_FONTS + 0].bAntiAliased = EXTRA_FONT_1_DEFAULT_AA;
#endif
#if (NUM_EXTRA_FONTS >= 2)
  wcscpy(m_fontinfo[NUM_BASIC_FONTS + 1].szFace, EXTRA_FONT_2_DEFAULT_FACE);
  m_fontinfo[NUM_BASIC_FONTS + 1].nSize = EXTRA_FONT_2_DEFAULT_SIZE;
  m_fontinfo[NUM_BASIC_FONTS + 1].bBold = EXTRA_FONT_2_DEFAULT_BOLD;
  m_fontinfo[NUM_BASIC_FONTS + 1].bItalic = EXTRA_FONT_2_DEFAULT_ITAL;
  m_fontinfo[NUM_BASIC_FONTS + 1].bAntiAliased = EXTRA_FONT_2_DEFAULT_AA;
#endif
#if (NUM_EXTRA_FONTS >= 3)
  wcscpy(m_fontinfo[NUM_BASIC_FONTS + 2].szFace, EXTRA_FONT_3_DEFAULT_FACE);
  m_fontinfo[NUM_BASIC_FONTS + 2].nSize = EXTRA_FONT_3_DEFAULT_SIZE;
  m_fontinfo[NUM_BASIC_FONTS + 2].bBold = EXTRA_FONT_3_DEFAULT_BOLD;
  m_fontinfo[NUM_BASIC_FONTS + 2].bItalic = EXTRA_FONT_3_DEFAULT_ITAL;
  m_fontinfo[NUM_BASIC_FONTS + 2].bAntiAliased = EXTRA_FONT_3_DEFAULT_AA;
#endif
#if (NUM_EXTRA_FONTS >= 4)
  strcpy(m_fontinfo[NUM_BASIC_FONTS + 3].szFace, EXTRA_FONT_4_DEFAULT_FACE);
  m_fontinfo[NUM_BASIC_FONTS + 3].nSize = EXTRA_FONT_4_DEFAULT_SIZE;
  m_fontinfo[NUM_BASIC_FONTS + 3].bBold = EXTRA_FONT_4_DEFAULT_BOLD;
  m_fontinfo[NUM_BASIC_FONTS + 3].bItalic = EXTRA_FONT_4_DEFAULT_ITAL;
  m_fontinfo[NUM_BASIC_FONTS + 3].bAntiAliased = EXTRA_FONT_4_DEFAULT_AA;
#endif
#if (NUM_EXTRA_FONTS >= 5)
  strcpy(m_fontinfo[NUM_BASIC_FONTS + 4].szFace, EXTRA_FONT_5_DEFAULT_FACE);
  m_fontinfo[NUM_BASIC_FONTS + 4].nSize = EXTRA_FONT_5_DEFAULT_SIZE;
  m_fontinfo[NUM_BASIC_FONTS + 4].bBold = EXTRA_FONT_5_DEFAULT_BOLD;
  m_fontinfo[NUM_BASIC_FONTS + 4].bItalic = EXTRA_FONT_5_DEFAULT_ITAL;
  m_fontinfo[NUM_BASIC_FONTS + 4].bAntiAliased = EXTRA_FONT_5_DEFAULT_AA;
#endif

  m_disp_mode_fs_width  = DEFAULT_FULLSCREEN_WIDTH;
  m_disp_mode_fs_height = DEFAULT_FULLSCREEN_HEIGHT;
  // Use current display settings as default if available (RefreshRate/Format not used in DX12):
  DEVMODE dm;
  dm.dmSize = sizeof(dm);
  dm.dmDriverExtra = 0;
  if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm)) {
    m_disp_mode_fs_width  = dm.dmPelsWidth;
    m_disp_mode_fs_height = dm.dmPelsHeight;
  }

  // PROTECTED STRUCTURES/POINTERS
  for (int i = 0; i < NUM_BASIC_FONTS + NUM_EXTRA_FONTS; i++)
    m_fontHeight[i] = 0;
  ZeroMemory(&m_sound, sizeof(td_soundinfo));
  for (int ch = 0; ch < 2; ch++)
    for (int i = 0; i < 3; i++) {
      m_sound.infinite_avg[ch][i] = m_sound.avg[ch][i] = m_sound.med_avg[ch][i] = m_sound.long_avg[ch][i] = 1.0f;
    }

  // GENERAL PRIVATE STUFF
  //m_screenmode: set at end (derived setting)
  m_frame = 0;
  m_time = 0;
  m_fps = 60;
  m_hInstance = hWinampInstance;
  m_lpDX = NULL;
  m_szPluginsDirPath[0] = 0;  // will be set further down
  m_szConfigIniFile[0] = 0;  // will be set further down

  wcscpy(m_szPluginsDirPath, m_szBaseDir);

  wchar_t* p = m_szPluginsDirPath + wcslen(m_szPluginsDirPath);
  while (p >= m_szPluginsDirPath && *p != L'\\') p--;
  if (++p >= m_szPluginsDirPath) *p = 0;

  swprintf(m_szConfigIniFile, L"%s%s", m_szPluginsDirPath, INIFILE);
  lstrcpyn(m_szConfigIniFileA, AutoCharFn(m_szConfigIniFile), MAX_PATH);

  // PRIVATE CONFIG PANEL SETTINGS
  // m_multisample_* removed in DX12 migration (MSAA configured via PSO instead)
  ZeroMemory(&m_adapter_guid_fullscreen, sizeof(GUID));
  ZeroMemory(&m_adapter_guid_desktop, sizeof(GUID));
  ZeroMemory(&m_adapter_guid_windowed, sizeof(GUID));
  m_adapter_devicename_windowed[0] = 0;
  m_adapter_devicename_fullscreen[0] = 0;
  m_adapter_devicename_desktop[0] = 0;


  // PRIVATE RUNTIME SETTINGS
  m_lost_focus = 0;
  m_hidden = 0;
  m_resizing = 0;
  m_show_help = 0;
  m_show_playlist = 0;
  m_playlist_pos = 0;
  m_playlist_pageups = 0;
  m_playlist_top_idx = -1;
  m_playlist_btm_idx = -1;
  // m_playlist_width_pixels will be considered invalid whenever 'm_playlist_top_idx' is -1.
  // m_playlist[256][256] will be considered invalid whenever 'm_playlist_top_idx' is -1.
  m_exiting = 0;
  m_upper_left_corner_y = 0;
  m_lower_left_corner_y = 0;
  m_upper_right_corner_y = 0;
  m_lower_right_corner_y = 0;
  m_left_edge = 0;
  m_right_edge = 0;
  m_force_accept_WM_WINDOWPOSCHANGING = 0;

  // PRIVATE - GDI STUFF
  for (i = 0; i < NUM_BASIC_FONTS + NUM_EXTRA_FONTS; i++)
    m_font[i] = NULL;
  m_font_desktop = NULL;

  // PRIVATE - MORE TIMEKEEPING
  m_last_raw_time = 0;
  memset(m_time_hist, 0, sizeof(m_time_hist));
  m_time_hist_pos = 0;
  if (!QueryPerformanceFrequency(&m_high_perf_timer_freq))
    m_high_perf_timer_freq.QuadPart = 0;
  m_prev_end_of_frame.QuadPart = 0;

  // PRIVATE AUDIO PROCESSING DATA
  //(m_fftobj needs no init)
  memset(m_oldwave[0], 0, sizeof(float) * 576);
  memset(m_oldwave[1], 0, sizeof(float) * 576);
  m_prev_align_offset[0] = 0;
  m_prev_align_offset[1] = 0;
  m_align_weights_ready = 0;

  // SEPARATE TEXT WINDOW (FOR VJ MODE)
  // SPOUT
  m_vj_mode = 0; // 0;
  m_hidden_textwnd = 0;
  m_resizing_textwnd = 0;
  m_hTextWnd = NULL;
  m_nTextWndWidth = 0;
  m_nTextWndHeight = 0;
  m_bTextWindowClassRegistered = false;

  //-----

  OverrideDefaults();
  ReadConfig();
  MyPreInitialize();
  MyReadConfig();
  SetAMDFlag();

  //-----

  return TRUE;
}

int CPluginShell::PluginInitialize(
    ID3D12Device*       device,
    ID3D12CommandQueue* commandQueue,
    IDXGIFactory4*      factory,
    HWND                hwnd,
    int                 iWidth,
    int                 iHeight)
{
  if (!InitDirectX(device, commandQueue, factory, hwnd, iWidth, iHeight)) return FALSE;

  m_lpDX->m_client_width       = iWidth;
  m_lpDX->m_client_height      = iHeight;
  m_lpDX->m_REAL_client_width  = iWidth;
  m_lpDX->m_REAL_client_height = iHeight;
  UpdateBackBufferTracking(iWidth, iHeight);

  if (!InitNondx9Stuff()) return FALSE;
  if (!AllocateDX9Stuff()) return FALSE;

  m_hRenderWnd = hwnd;
  if (!InitVJStuff()) return FALSE;

  return TRUE;
}

void CPluginShell::PluginQuit() {
  CleanUpVJStuff();
  CleanUpDX9Stuff(1);
  CleanUpNondx9Stuff();
  CleanUpDirectX();
}

wchar_t* BuildSettingName(wchar_t* name, int number) {
  static wchar_t temp[64];
  swprintf(temp, L"%s%d", name, number);
  return temp;
}

void CPluginShell::READ_FONT(int n) {
  int iniIndex = n + 1;
  GetPrivateProfileStringW(L"Fonts", BuildSettingName(L"FontFace", iniIndex), m_fontinfo[n].szFace, m_fontinfo[n].szFace, sizeof(m_fontinfo[n].szFace), m_szConfigIniFile);
  m_fontinfo[n].nSize = GetPrivateProfileIntW(L"Fonts", BuildSettingName(L"FontSize", iniIndex), m_fontinfo[n].nSize, m_szConfigIniFile);

  m_fontinfo[n].bBold = GetPrivateProfileIntW(L"Fonts", BuildSettingName(L"FontBold", iniIndex), m_fontinfo[n].bBold, m_szConfigIniFile);
  m_fontinfo[n].bItalic = GetPrivateProfileIntW(L"Fonts", BuildSettingName(L"FontItalic", iniIndex), m_fontinfo[n].bItalic, m_szConfigIniFile);
  m_fontinfo[n].bAntiAliased = GetPrivateProfileIntW(L"Fonts", BuildSettingName(L"FontAA", iniIndex), m_fontinfo[n].bItalic, m_szConfigIniFile);

  if (n == SIMPLE_FONT) {
    m_fontinfo[n].R = SIMPLE_FONT_DEFAULT_COLOR_R;
    m_fontinfo[n].G = SIMPLE_FONT_DEFAULT_COLOR_G;
    m_fontinfo[n].B = SIMPLE_FONT_DEFAULT_COLOR_B;
  }
  else if (n == DECORATIVE_FONT) {
    m_fontinfo[n].R = DECORATIVE_FONT_DEFAULT_COLOR_R;
    m_fontinfo[n].G = DECORATIVE_FONT_DEFAULT_COLOR_G;
    m_fontinfo[n].B = DECORATIVE_FONT_DEFAULT_COLOR_B;
  }
  else if (n == EXTRA_1) {
    m_fontinfo[n].R = EXTRA_FONT_1_DEFAULT_COLOR_R;
    m_fontinfo[n].G = EXTRA_FONT_1_DEFAULT_COLOR_G;
    m_fontinfo[n].B = EXTRA_FONT_1_DEFAULT_COLOR_B;
  }
  else if (n == EXTRA_2) {
    m_fontinfo[n].R = EXTRA_FONT_2_DEFAULT_COLOR_R;
    m_fontinfo[n].G = EXTRA_FONT_2_DEFAULT_COLOR_G;
    m_fontinfo[n].B = EXTRA_FONT_2_DEFAULT_COLOR_B;
  }
  else if (n == EXTRA_3) {
    m_fontinfo[n].R = EXTRA_FONT_3_DEFAULT_COLOR_R;
    m_fontinfo[n].G = EXTRA_FONT_3_DEFAULT_COLOR_G;
    m_fontinfo[n].B = EXTRA_FONT_3_DEFAULT_COLOR_B;
  }

  m_fontinfo[n].R = GetPrivateProfileIntW(L"Fonts", BuildSettingName(L"FontColorR", iniIndex), m_fontinfo[n].R, m_szConfigIniFile);
  m_fontinfo[n].G = GetPrivateProfileIntW(L"Fonts", BuildSettingName(L"FontColorG", iniIndex), m_fontinfo[n].G, m_szConfigIniFile);
  m_fontinfo[n].B = GetPrivateProfileIntW(L"Fonts", BuildSettingName(L"FontColorB", iniIndex), m_fontinfo[n].B, m_szConfigIniFile);
}

void CPluginShell::ReadConfig() {
  int old_ver = GetPrivateProfileIntW(L"Settings", L"version", -1, m_szConfigIniFile);
  int old_subver = GetPrivateProfileIntW(L"Settings", L"subversion", -1, m_szConfigIniFile);

  // nuke old settings from prev. version:
  if (old_ver < INT_VERSION)
    return;
  else if (old_subver < INT_SUBVERSION)
    return;

  // m_multisample_* removed in DX12 migration (MSAA configured via PSO instead)

  //GUID m_adapter_guid_fullscreen
  //GUID m_adapter_guid_desktop
  //GUID m_adapter_guid_windowed
  char str[256];
  GetPrivateProfileString("settings", "adapter_guid_fullscreen", "", str, sizeof(str) - 1, m_szConfigIniFileA);
  TextToGuid(str, &m_adapter_guid_fullscreen);
  GetPrivateProfileString("settings", "adapter_guid_desktop", "", str, sizeof(str) - 1, m_szConfigIniFileA);
  TextToGuid(str, &m_adapter_guid_desktop);
  GetPrivateProfileString("settings", "adapter_guid_windowed", "", str, sizeof(str) - 1, m_szConfigIniFileA);
  TextToGuid(str, &m_adapter_guid_windowed);
  GetPrivateProfileString("settings", "adapter_devicename_fullscreen", "", m_adapter_devicename_fullscreen, sizeof(m_adapter_devicename_fullscreen) - 1, m_szConfigIniFileA);
  GetPrivateProfileString("settings", "adapter_devicename_desktop", "", m_adapter_devicename_desktop, sizeof(m_adapter_devicename_desktop) - 1, m_szConfigIniFileA);
  GetPrivateProfileString("settings", "adapter_devicename_windowed", "", m_adapter_devicename_windowed, sizeof(m_adapter_devicename_windowed) - 1, m_szConfigIniFileA);

  // FONTS
  READ_FONT(0);
  READ_FONT(1);
  READ_FONT(2);
  READ_FONT(3);
#if (NUM_EXTRA_FONTS >= 1)
  READ_FONT(4);
#endif
#if (NUM_EXTRA_FONTS >= 2)
  READ_FONT(5);
#endif
#if (NUM_EXTRA_FONTS >= 3)
  READ_FONT(6);
#endif
#if (NUM_EXTRA_FONTS >= 4)
  READ_FONT(7);
#endif
#if (NUM_EXTRA_FONTS >= 5)
  READ_FONT(8);
#endif

  m_start_fullscreen = GetPrivateProfileIntW(L"Settings", L"start_fullscreen", m_start_fullscreen, m_szConfigIniFile);
  m_start_desktop = GetPrivateProfileIntW(L"Settings", L"start_desktop", m_start_desktop, m_szConfigIniFile);
  m_fake_fullscreen_mode = GetPrivateProfileIntW(L"Settings", L"fake_fullscreen_mode", m_fake_fullscreen_mode, m_szConfigIniFile);
  m_max_fps_fs = GetPrivateProfileIntW(L"Settings", L"max_fps_fs", m_max_fps_fs, m_szConfigIniFile);
  m_max_fps_dm = GetPrivateProfileIntW(L"Settings", L"max_fps_dm", m_max_fps_dm, m_szConfigIniFile);
  m_max_fps_w = GetPrivateProfileIntW(L"Settings", L"max_fps_w", m_max_fps_w, m_szConfigIniFile);
  m_show_press_f1_msg = GetPrivateProfileIntW(L"Settings", L"show_press_f1_msg", m_show_press_f1_msg, m_szConfigIniFile);
  m_allow_page_tearing_w = GetPrivateProfileIntW(L"Settings", L"allow_page_tearing_w", m_allow_page_tearing_w, m_szConfigIniFile);
  m_allow_page_tearing_fs = GetPrivateProfileIntW(L"Settings", L"allow_page_tearing_fs", m_allow_page_tearing_fs, m_szConfigIniFile);
  m_allow_page_tearing_dm = GetPrivateProfileIntW(L"Settings", L"allow_page_tearing_dm", m_allow_page_tearing_dm, m_szConfigIniFile);
  m_minimize_winamp = GetPrivateProfileIntW(L"Settings", L"minimize_winamp", m_minimize_winamp, m_szConfigIniFile);
  m_desktop_show_icons = GetPrivateProfileIntW(L"Settings", L"desktop_show_icons", m_desktop_show_icons, m_szConfigIniFile);
  m_desktop_textlabel_boxes = GetPrivateProfileIntW(L"Settings", L"desktop_textlabel_boxes", m_desktop_textlabel_boxes, m_szConfigIniFile);
  m_desktop_manual_icon_scoot = GetPrivateProfileIntW(L"Settings", L"desktop_manual_icon_scoot", m_desktop_manual_icon_scoot, m_szConfigIniFile);
  m_desktop_555_fix = GetPrivateProfileIntW(L"Settings", L"desktop_555_fix", m_desktop_555_fix, m_szConfigIniFile);
  m_dualhead_horz = GetPrivateProfileIntW(L"Settings", L"dualhead_horz", m_dualhead_horz, m_szConfigIniFile);
  m_dualhead_vert = GetPrivateProfileIntW(L"Settings", L"dualhead_vert", m_dualhead_vert, m_szConfigIniFile);
  m_save_cpu = GetPrivateProfileIntW(L"Settings", L"save_cpu", m_save_cpu, m_szConfigIniFile);
  m_skin = GetPrivateProfileIntW(L"Settings", L"skin", m_skin, m_szConfigIniFile);
  m_fix_slow_text = GetPrivateProfileIntW(L"Settings", L"fix_slow_text", m_fix_slow_text, m_szConfigIniFile);
  m_vj_mode = GetPrivateProfileBoolW(L"Settings", L"vj_mode", m_vj_mode, m_szConfigIniFile);


  //D3DDISPLAYMODE m_fs_disp_mode
  m_disp_mode_fs_width  = GetPrivateProfileIntW(L"Settings", L"disp_mode_fs_w", m_disp_mode_fs_width,  m_szConfigIniFile);
  m_disp_mode_fs_height = GetPrivateProfileIntW(L"Settings", L"disp_mode_fs_h", m_disp_mode_fs_height, m_szConfigIniFile);
  // RefreshRate and Format not persisted in DX12 migration

  // note: we don't call MyReadConfig() yet, because we
  // want to completely finish CPluginShell's preinit (and ReadConfig)
  // before calling CPlugin's preinit and ReadConfig.
}

void CPluginShell::WRITE_FONT(int n) {
  WritePrivateProfileStringW(L"Settings", BuildSettingName(L"szFontFace", n), m_fontinfo[n].szFace, m_szConfigIniFile);
  WritePrivateProfileIntW(m_fontinfo[n].bBold, BuildSettingName(L"bFontBold", n), m_szConfigIniFile, L"Settings");
  WritePrivateProfileIntW(m_fontinfo[n].bItalic, BuildSettingName(L"bFontItalic", n), m_szConfigIniFile, L"Settings");
  WritePrivateProfileIntW(m_fontinfo[n].nSize, BuildSettingName(L"nFontSize", n), m_szConfigIniFile, L"Settings");
  WritePrivateProfileIntW(m_fontinfo[n].bAntiAliased, BuildSettingName(L"bFontAA", n), m_szConfigIniFile, L"Settings");
}

void CPluginShell::WriteConfig() {
  // m_multisample_* removed in DX12 migration (MSAA configured via PSO instead)

  //GUID m_adapter_guid_fullscreen
  //GUID m_adapter_guid_desktop
  //GUID m_adapter_guid_windowed
  char str[256];
  GuidToText(&m_adapter_guid_fullscreen, str, sizeof(str));
  WritePrivateProfileString("settings", "adapter_guid_fullscreen", str, m_szConfigIniFileA);
  GuidToText(&m_adapter_guid_desktop, str, sizeof(str));
  WritePrivateProfileString("settings", "adapter_guid_desktop", str, m_szConfigIniFileA);
  GuidToText(&m_adapter_guid_windowed, str, sizeof(str));
  WritePrivateProfileString("settings", "adapter_guid_windowed", str, m_szConfigIniFileA);
  WritePrivateProfileString("settings", "adapter_devicename_fullscreen", m_adapter_devicename_fullscreen, m_szConfigIniFileA);
  WritePrivateProfileString("settings", "adapter_devicename_desktop", m_adapter_devicename_desktop, m_szConfigIniFileA);
  WritePrivateProfileString("settings", "adapter_devicename_windowed", m_adapter_devicename_windowed, m_szConfigIniFileA);

  // FONTS
  WRITE_FONT(0);
  WRITE_FONT(1);
  WRITE_FONT(2);
  WRITE_FONT(3);
#if (NUM_EXTRA_FONTS >= 1)
  WRITE_FONT(4);
#endif
#if (NUM_EXTRA_FONTS >= 2)
  WRITE_FONT(5);
#endif
#if (NUM_EXTRA_FONTS >= 3)
  WRITE_FONT(6);
#endif
#if (NUM_EXTRA_FONTS >= 4)
  WRITE_FONT(7);
#endif
#if (NUM_EXTRA_FONTS >= 5)
  WRITE_FONT(8);
#endif

  WritePrivateProfileIntW(m_start_fullscreen, L"start_fullscreen", m_szConfigIniFile, L"Settings");
  WritePrivateProfileIntW(m_start_desktop, L"start_desktop", m_szConfigIniFile, L"Settings");
  WritePrivateProfileIntW(m_fake_fullscreen_mode, L"fake_fullscreen_mode", m_szConfigIniFile, L"Settings");
  WritePrivateProfileIntW(m_max_fps_fs, L"max_fps_fs", m_szConfigIniFile, L"Settings");
  WritePrivateProfileIntW(m_max_fps_dm, L"max_fps_dm", m_szConfigIniFile, L"Settings");
  WritePrivateProfileIntW(m_max_fps_w, L"max_fps_w", m_szConfigIniFile, L"Settings");
  WritePrivateProfileIntW(m_show_press_f1_msg, L"show_press_f1_msg", m_szConfigIniFile, L"Settings");
  WritePrivateProfileIntW(m_allow_page_tearing_w, L"allow_page_tearing_w", m_szConfigIniFile, L"Settings");
  WritePrivateProfileIntW(m_allow_page_tearing_fs, L"allow_page_tearing_fs", m_szConfigIniFile, L"Settings");
  WritePrivateProfileIntW(m_allow_page_tearing_dm, L"allow_page_tearing_dm", m_szConfigIniFile, L"Settings");
  WritePrivateProfileIntW(m_minimize_winamp, L"minimize_winamp", m_szConfigIniFile, L"Settings");
  WritePrivateProfileIntW(m_desktop_show_icons, L"desktop_show_icons", m_szConfigIniFile, L"Settings");
  WritePrivateProfileIntW(m_desktop_textlabel_boxes, L"desktop_textlabel_boxes", m_szConfigIniFile, L"Settings");
  WritePrivateProfileIntW(m_desktop_manual_icon_scoot, L"desktop_manual_icon_scoot", m_szConfigIniFile, L"Settings");
  WritePrivateProfileIntW(m_desktop_555_fix, L"desktop_555_fix", m_szConfigIniFile, L"Settings");
  WritePrivateProfileIntW(m_dualhead_horz, L"dualhead_horz", m_szConfigIniFile, L"Settings");
  WritePrivateProfileIntW(m_dualhead_vert, L"dualhead_vert", m_szConfigIniFile, L"Settings");
  WritePrivateProfileIntW(m_save_cpu, L"save_cpu", m_szConfigIniFile, L"Settings");
  WritePrivateProfileIntW(m_skin, L"skin", m_szConfigIniFile, L"Settings");
  WritePrivateProfileIntW(m_fix_slow_text, L"fix_slow_text", m_szConfigIniFile, L"Settings");
  WritePrivateProfileIntW(m_vj_mode, L"vj_mode", m_szConfigIniFile, L"Settings");

  //D3DDISPLAYMODE m_fs_disp_mode
  WritePrivateProfileIntW(m_disp_mode_fs_width,  L"disp_mode_fs_w", m_szConfigIniFile, L"Settings");
  WritePrivateProfileIntW(m_disp_mode_fs_height, L"disp_mode_fs_h", m_szConfigIniFile, L"Settings");
  // RefreshRate and Format not persisted in DX12 migration

  WritePrivateProfileIntW(INT_VERSION, L"version", m_szConfigIniFile, L"Settings");
  WritePrivateProfileIntW(INT_SUBVERSION, L"subversion", m_szConfigIniFile, L"Settings");

  // finally, save the plugin's unique settings:
  MyWriteConfig();
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------

int CPluginShell::PluginRender(unsigned char* pWaveL, unsigned char* pWaveR)//, unsigned char *pSpecL, unsigned char *pSpecR)
{
  // return FALSE here to tell Winamp to terminate the plugin

  if (!m_lpDX || !m_lpDX->m_ready) {
    // note: 'm_ready' will go false when a device reset fatally fails
    //       (for example, when user resizes window, or toggles fullscreen.)
    m_exiting = 1;
    return false;   // EXIT THE PLUGIN
  }

  if (m_hTextWnd)
    m_lost_focus = ((GetFocus() != GetPluginWindow()) && (GetFocus() != m_hTextWnd));
  else
    m_lost_focus = (GetFocus() != GetPluginWindow());

  // SPOUT
  // Allow render when minimized
  // if (m_hidden || m_resizing)
  if (m_resizing) {
    Sleep(30);
    return true;
  }

  // DX12 does not have TestCooperativeLevel() or device-lost in the DX9 sense.
  // DXGI Present returns DXGI_ERROR_DEVICE_REMOVED/RESET in catastrophic cases,
  // which are handled inside DXContext::EndFrame() and surfaced via m_lastErr.
  if (m_lpDX->m_lastErr != S_OK) {
    // Unrecoverable device error — exit plugin
    return false;
  }

  DoTime();
  AnalyzeNewSound(pWaveL, pWaveR);
  AlignWaves();

  DrawAndDisplay(0);

  EnforceMaxFPS();

  // m_frame++;
  m_frame += (int)m_frameFactor;

  return true;
}

void CPluginShell::DrawAndDisplay(int redraw) {
  int cx = m_lpDX->m_client_width;
  int cy = m_lpDX->m_client_height;

  int textMargin = TEXT_MARGIN;
  if (IsSpoutActiveAndFixed()) {
    cx = nSpoutFixedWidth;
    cy = nSpoutFixedHeight;
  }
  else {
    float q = GetEffectiveRenderQuality(cx, cy);
    cx = (int)(cx * q);
    cy = (int)(cy * q);
    textMargin *= q;
  }

  int marginTop = textMargin + GetCanvasMarginY();
  int marginBottom = cy - textMargin - GetCanvasMarginY();
  int marginLeft = textMargin + GetCanvasMarginX();
  int marginRight = cx - textMargin - GetCanvasMarginX();

  m_upper_left_corner_y = marginTop;
  m_upper_right_corner_y = marginTop;
  m_lower_left_corner_y = marginBottom;
  m_lower_right_corner_y = marginBottom;

  m_left_edge = marginLeft;
  m_right_edge = marginRight;

  if (m_fRenderQuality < 0.1) {
    // stop trying to display texts properly when quality is too low anyways
    m_left_edge = m_right_edge = -99999;
  }

  // DX12 frame: open command list, record all draw calls, then execute + present.
  if (m_lpDX->BeginFrame()) {
    // Set viewport on the command list
    D3D12_VIEWPORT vp = { 0.f, 0.f,
        (float)m_lpDX->m_client_width, (float)m_lpDX->m_client_height,
        0.f, 1.f };
    D3D12_RECT scissor = { 0, 0, m_lpDX->m_client_width, m_lpDX->m_client_height };
    m_lpDX->m_commandList->RSSetViewports(1, &vp);
    m_lpDX->m_commandList->RSSetScissorRects(1, &scissor);

    // Clear the back buffer
    {
      float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
      D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_lpDX->m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
      rtvHandle.ptr += (SIZE_T)m_lpDX->m_frameIndex * m_lpDX->m_rtvDescriptorSize;
      m_lpDX->m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    }

    // The main visualizer render function records its commands into m_lpDX->m_commandList
    MyRenderFn(redraw);

    PrepareFor2DDrawing_B(GetWidth(), GetHeight());

    RenderBuiltInTextMsgs();

    MyRenderUI(&m_upper_left_corner_y, &m_upper_right_corner_y,
      &m_lower_left_corner_y, &m_lower_right_corner_y,
      m_left_edge, m_right_edge);
    RenderPlaylist();

    m_text.DrawNow();

    // EndFrame: transitions back buffer to PRESENT, executes command list, calls Present()
    m_lpDX->EndFrame();
  }

}

void CPluginShell::EnforceMaxFPS() {
  int max_fps = m_max_fps_w;

  if (max_fps <= 0)
    return;

  float fps_lo = (float)max_fps;
  float fps_hi = (float)max_fps;

  if (m_save_cpu) {
    // Find the optimal lo/hi bounds for the fps
    // that will result in a maximum difference,
    // in the time for a single frame, of 0.003 seconds -
    // the assumed granularity for Sleep(1) -

    // Using this range of acceptable fps
    // will allow us to do (sloppy) fps limiting
    // using only Sleep(1), and never the
    // second half of it: Sleep(0) in a tight loop,
    // which sucks up the CPU (whereas Sleep(1)
    // leaves it idle).

    // The original equation:
    //   1/(max_fps*t1) = 1/(max*fps/t1) - 0.003
    // where:
    //   t1 > 0
    //   max_fps*t1 is the upper range for fps
    //   max_fps/t1 is the lower range for fps

    float a = 1;
    float b = -0.003f * max_fps;
    float c = -1.0f;
    float det = b * b - 4 * a * c;
    if (det > 0) {
      float t1 = (-b + sqrtf(det)) / (2 * a);
      //float t2 = (-b - sqrtf(det)) / (2*a);

      if (t1 > 1.0f) {
        fps_lo = max_fps / t1;
        fps_hi = max_fps * t1;
        // verify: now [1.0f/fps_lo - 1.0f/fps_hi] should equal 0.003 seconds.
        // note: allowing tolerance to go beyond these values for
        // fps_lo and fps_hi would gain nothing.
      }
    }
  }

  if (m_high_perf_timer_freq.QuadPart > 0) {
    LARGE_INTEGER t;
    QueryPerformanceCounter(&t);

    if (m_prev_end_of_frame.QuadPart != 0) {
      int ticks_to_wait_lo = (int)((float)m_high_perf_timer_freq.QuadPart / (float)fps_hi);
      int ticks_to_wait_hi = (int)((float)m_high_perf_timer_freq.QuadPart / (float)fps_lo);
      int done = 0;
      int loops = 0;
      do {
        QueryPerformanceCounter(&t);

        __int64 t2 = t.QuadPart - m_prev_end_of_frame.QuadPart;
        if (t2 > 2147483000)
          done = 1;
        if (t.QuadPart < m_prev_end_of_frame.QuadPart)    // time wrap
          done = 1;

        // this is sloppy - if your freq. is high, this can overflow (to a (-) int) in just a few minutes
        // but it's ok, we have protection for that above.
        int ticks_passed = (int)(t.QuadPart - m_prev_end_of_frame.QuadPart);
        if (ticks_passed >= ticks_to_wait_lo)
          done = 1;

        if (!done) {
          // if > 0.01s left, do Sleep(1), which will actually sleep some
          //   steady amount of up to 3 ms (depending on the OS),
          //   and do so in a nice way (cpu meter drops; laptop battery spared).
          // otherwise, do a few Sleep(0)'s, which just give up the timeslice,
          //   but don't really save cpu or battery, but do pass a tiny
          //   amount of time.

          //if (ticks_left > (int)m_high_perf_timer_freq.QuadPart/500)
          if (ticks_to_wait_hi - ticks_passed > (int)m_high_perf_timer_freq.QuadPart / 100)
            Sleep(5);
          else if (ticks_to_wait_hi - ticks_passed > (int)m_high_perf_timer_freq.QuadPart / 1000)
            Sleep(1);
          else
            for (int i = 0; i < 10; i++)
              Sleep(0);  // causes thread to give up its timeslice
        }
      } while (!done);
    }

    m_prev_end_of_frame = t;
  }
  else {
    Sleep(1000 / max_fps);
  }
}

void CPluginShell::DoTime() {
  if (m_frame == 0) {
    m_fps = 60;
    m_time = 0;
    m_time_hist_pos = 0;
  }

  double new_raw_time;
  float elapsed;

  if (m_high_perf_timer_freq.QuadPart != 0) {
    // get high-precision time
    // precision: usually from 1..6 us (MICROseconds), depending on the cpu speed.
    // (higher cpu speeds tend to have better precision here)
    LARGE_INTEGER t;
    if (!QueryPerformanceCounter(&t)) {
      m_high_perf_timer_freq.QuadPart = 0;   // something went wrong (exception thrown) -> revert to crappy timer
    }
    else {
      new_raw_time = (double)t.QuadPart;
      elapsed = (float)((new_raw_time - m_last_raw_time) / (double)m_high_perf_timer_freq.QuadPart);
    }
  }

  if (m_high_perf_timer_freq.QuadPart == 0) {
    // get low-precision time
    // precision: usually 1 ms (MILLIsecond) for win98, and 10 ms for win2k.
    new_raw_time = (double)(timeGetTime() * 0.001);
    elapsed = (float)(new_raw_time - m_last_raw_time);
  }

  m_last_raw_time = new_raw_time;
  int slots_to_look_back = (m_high_perf_timer_freq.QuadPart == 0) ? TIME_HIST_SLOTS : TIME_HIST_SLOTS / 2;

  m_time += 1.0f / m_fps;
  if (m_time >= 250000)
    m_time = 0; // Reset the time variable after 250000 seconds.

  if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) && (GetAsyncKeyState('T') & 0x8000))
    m_time = 0;

  // timekeeping goals:
  //    1. keep 'm_time' increasing SMOOTHLY: (smooth animation depends on it)
  //          m_time += 1.0f/m_fps;     // where m_fps is a bit damped
  //    2. keep m_time_hist[] 100% accurate (except for filtering out pauses),
  //       so that when we look take the difference between two entries,
  //       we get the real amount of time that passed between those 2 frames.
  //          m_time_hist[i] = m_last_raw_time + elapsed_corrected;

  if (m_frame > TIME_HIST_SLOTS) {
    if (m_fps < 60.0f)
      slots_to_look_back = (int)(slots_to_look_back * (0.1f + 0.9f * (m_fps / 60.0f)));

    if (elapsed > 5.0f / m_fps || elapsed > 1.0f || elapsed < 0)
      elapsed = 1.0f / 30.0f;

    float old_hist_time = m_time_hist[(m_time_hist_pos - slots_to_look_back + TIME_HIST_SLOTS) % TIME_HIST_SLOTS];
    float new_hist_time = m_time_hist[(m_time_hist_pos - 1 + TIME_HIST_SLOTS) % TIME_HIST_SLOTS]
      + elapsed;

    m_time_hist[m_time_hist_pos] = new_hist_time;
    m_time_hist_pos = (m_time_hist_pos + 1) % TIME_HIST_SLOTS;

    float new_fps = slots_to_look_back / (float)(new_hist_time - old_hist_time);
    float damping = (m_high_perf_timer_freq.QuadPart == 0) ? 0.93f : 0.87f;

    // damp heavily, so that crappy timer precision doesn't make animation jerky
    if (fabsf(m_fps - new_fps) > 3.0f)
      m_fps = new_fps;
    else
      m_fps = damping * m_fps + (1 - damping) * new_fps;
  }
  else {
    float damping = (m_high_perf_timer_freq.QuadPart == 0) ? 0.8f : 0.6f;

    if (m_frame < 2)
      elapsed = 1.0f / 30.0f;
    else if (elapsed > 1.0f || elapsed < 0)
      elapsed = 1.0f / m_fps;

    float old_hist_time = m_time_hist[0];
    float new_hist_time = m_time_hist[(m_time_hist_pos - 1 + TIME_HIST_SLOTS) % TIME_HIST_SLOTS]
      + elapsed;

    m_time_hist[m_time_hist_pos] = new_hist_time;
    m_time_hist_pos = (m_time_hist_pos + 1) % TIME_HIST_SLOTS;

    if (m_frame > 0) {
      float new_fps = (m_frame) / (new_hist_time - old_hist_time);
      m_fps = damping * m_fps + (1 - damping) * new_fps;
    }
  }
}

void CPluginShell::AnalyzeNewSound(unsigned char* pWaveL, unsigned char* pWaveR) {
  // we get 576 samples in from winamp.
  // the output of the fft has 'num_frequencies' samples,
  //   and represents the frequency range 0 hz - 22,050 hz.
  // usually, plugins only use half of this output (the range 0 hz - 11,025 hz),
  //   since >10 khz doesn't usually contribute much.

  int i;

  float temp_wave[2][576];

  int old_i = 0;
  for (i = 0; i < 576; i++) {
    m_sound.fWaveform[0][i] = (float)((pWaveL[i] ^ 128) - 128);
    m_sound.fWaveform[1][i] = (float)((pWaveR[i] ^ 128) - 128);

    // simulating single frequencies from 200 to 11,025 Hz:
    //float freq = 1.0f + 11050*(GetFrame() % 100)*0.01f;
    //m_sound.fWaveform[0][i] = 10*sinf(i*freq*6.28f/44100.0f);

    // damp the input into the FFT a bit, to reduce high-frequency noise:
    temp_wave[0][i] = 0.5f * (m_sound.fWaveform[0][i] + m_sound.fWaveform[0][old_i]);
    temp_wave[1][i] = 0.5f * (m_sound.fWaveform[1][i] + m_sound.fWaveform[1][old_i]);
    old_i = i;
  }

  m_fftobj.time_to_frequency_domain(temp_wave[0], m_sound.fSpectrum[0]);
  m_fftobj.time_to_frequency_domain(temp_wave[1], m_sound.fSpectrum[1]);

  // sum (left channel) spectrum up into 3 bands
  // [note: the new ranges do it so that the 3 bands are equally spaced, pitch-wise]
  float min_freq = 20.0f;
  float max_freq = 20000.0f;
  float net_octaves = (logf(max_freq / min_freq) / logf(2.0f));     // 5.7846348455575205777914165223593
  float octaves_per_band = net_octaves / 3.0f;                    // 1.9282116151858401925971388407864
  float mult = powf(2.0f, octaves_per_band); // each band's highest freq. divided by its lowest freq.; 3.805831305510122517035102576162
  // [to verify: min_freq * mult * mult * mult should equal max_freq.]
  for (int ch = 0; ch < 2; ch++) {
    for (i = 0; i < 3; i++) {
      // old guesswork code for this:
      //   float exp = 2.1f;
      //   int start = (int)(NUM_FREQUENCIES*0.5f*powf(i/3.0f, exp));
      //   int end   = (int)(NUM_FREQUENCIES*0.5f*powf((i+1)/3.0f, exp));
      // results:
      //          old range:      new range (ideal):
      //   bass:  0-1097          200-761
      //   mids:  1097-4705       761-2897
      //   treb:  4705-11025      2897-11025
      int start = (int)(NUM_FREQUENCIES * min_freq * powf(mult, (float)i) / max_freq);
      int end = (int)(NUM_FREQUENCIES * min_freq * powf(mult, (float)(i + 1)) / max_freq);
      if (start < 0) start = 0;
      if (end > NUM_FREQUENCIES) end = NUM_FREQUENCIES;

      m_sound.imm[ch][i] = 0;
      for (int j = start; j < end; j++)
        m_sound.imm[ch][i] += m_sound.fSpectrum[ch][j];
      m_sound.imm[ch][i] /= (float)(end - start);
    }
  }

  // multiply by long-term, empirically-determined inverse averages:
  // (for a trial of 244 songs, 10 seconds each, somewhere in the 2nd or 3rd minute,
  //  the average levels were: 0.326781557	0.38087377	0.199888934
  for (ch = 0; ch < 2; ch++) {
    m_sound.imm[ch][0] /= 0.326781557f;//0.270f;
    m_sound.imm[ch][1] /= 0.380873770f;//0.343f;
    m_sound.imm[ch][2] /= 0.199888934f;//0.295f;
  }

  // do temporal blending to create attenuated and super-attenuated versions
  for (ch = 0; ch < 2; ch++) {
    for (i = 0; i < 3; i++) {
      // m_sound.avg[i]
      {
        float avg_mix;
        if (m_sound.imm[ch][i] > m_sound.avg[ch][i])
          avg_mix = AdjustRateToFPS(0.2f, 14.0f, m_fps);
        else
          avg_mix = AdjustRateToFPS(0.5f, 14.0f, m_fps);
        m_sound.avg[ch][i] = m_sound.avg[ch][i] * avg_mix + m_sound.imm[ch][i] * (1 - avg_mix);
      }

      // m_sound.med_avg[i]
      // m_sound.long_avg[i]
      {
        float med_mix = 0.91f;//0.800f + 0.11f*powf(t, 0.4f);    // primarily used for velocity_damping
        float long_mix = 0.96f;//0.800f + 0.16f*powf(t, 0.2f);    // primarily used for smoke plumes
        med_mix = AdjustRateToFPS(med_mix, 14.0f, m_fps);
        long_mix = AdjustRateToFPS(long_mix, 14.0f, m_fps);
        m_sound.med_avg[ch][i] = m_sound.med_avg[ch][i] * (med_mix)+m_sound.imm[ch][i] * (1 - med_mix);
        m_sound.long_avg[ch][i] = m_sound.long_avg[ch][i] * (long_mix)+m_sound.imm[ch][i] * (1 - long_mix);
      }
    }
  }
}

void CPluginShell::PrepareFor2DDrawing_B(int w, int h) {
  // Phase 4 TODO: set up a 2D orthographic PSO on the command list for UI overlay rendering.
  // In DX12 all render state is baked into Pipeline State Objects, so there are no
  // individual SetRenderState() / SetTransform() calls here.
  // The viewport/scissor are already set in DrawAndDisplay before calling MyRenderFn.
  (void)w; (void)h;
}

void CPluginShell::DrawDarkTranslucentBox(RECT* pr) {
  // Phase 4 TODO: render a translucent dark quad over the given rect using DX12 command list.
  // Requires a dedicated alpha-blended PSO and a simple vertex buffer.
  (void)pr;
}

void CPluginShell::DrawDarkTranslucentBoxFullWindow() {
  // Phase 4 TODO: render a full-window translucent dark overlay using DX12 command list.
  if (!m_lpDX) return;
}

bool CPluginShell::CreateHelpTexture() {
  if (!m_lpDX || !m_lpDX->m_device) return false;

  auto* device = m_lpDX->m_device.Get();
  UINT w = (UINT)m_lpDX->m_client_width;
  UINT h = (UINT)m_lpDX->m_client_height;
  if (w == 0 || h == 0) return false;

  // Create DEFAULT-heap texture (SRV-only, no render target needed)
  D3D12_RESOURCE_DESC desc = {};
  desc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
  desc.Width              = w;
  desc.Height             = h;
  desc.DepthOrArraySize   = 1;
  desc.MipLevels          = 1;
  desc.Format             = DXGI_FORMAT_B8G8R8A8_UNORM;
  desc.SampleDesc.Count   = 1;
  desc.Flags              = D3D12_RESOURCE_FLAG_NONE;

  D3D12_HEAP_PROPERTIES heapProps = {};
  heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

  HRESULT hr = device->CreateCommittedResource(
      &heapProps, D3D12_HEAP_FLAG_NONE, &desc,
      D3D12_RESOURCE_STATE_COPY_DEST,
      nullptr, IID_PPV_ARGS(&m_helpTexture.resource));
  if (FAILED(hr)) return false;

  m_helpTexture.width  = w;
  m_helpTexture.height = h;
  m_helpTexture.format = DXGI_FORMAT_B8G8R8A8_UNORM;
  m_helpTexture.currentState = D3D12_RESOURCE_STATE_COPY_DEST;

  // Allocate SRV descriptor
  D3D12_CPU_DESCRIPTOR_HANDLE srvCpu = m_lpDX->AllocateSrvCpu();
  m_helpTexture.srvIndex = m_lpDX->m_nextFreeSrvSlot;

  D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
  srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  srvDesc.Format                  = DXGI_FORMAT_B8G8R8A8_UNORM;
  srvDesc.ViewDimension           = D3D12_SRV_DIMENSION_TEXTURE2D;
  srvDesc.Texture2D.MipLevels     = 1;
  device->CreateShaderResourceView(m_helpTexture.resource.Get(), &srvDesc, srvCpu);
  m_lpDX->AllocateSrvGpu();

  // Create 16-entry binding block for texture binding
  m_lpDX->CreateBindingBlockForTexture(m_helpTexture);

  // Create upload buffer (row-pitch aligned for CopyTextureRegion)
  UINT rowPitch = (w * 4 + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1)
                  & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1);
  UINT64 uploadSize = (UINT64)rowPitch * h;

  D3D12_HEAP_PROPERTIES uploadHeapProps = {};
  uploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

  D3D12_RESOURCE_DESC bufDesc = {};
  bufDesc.Dimension        = D3D12_RESOURCE_DIMENSION_BUFFER;
  bufDesc.Width            = uploadSize;
  bufDesc.Height           = 1;
  bufDesc.DepthOrArraySize = 1;
  bufDesc.MipLevels        = 1;
  bufDesc.Format           = DXGI_FORMAT_UNKNOWN;
  bufDesc.SampleDesc.Count = 1;
  bufDesc.Layout           = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

  hr = device->CreateCommittedResource(
      &uploadHeapProps, D3D12_HEAP_FLAG_NONE, &bufDesc,
      D3D12_RESOURCE_STATE_GENERIC_READ,
      nullptr, IID_PPV_ARGS(&m_helpUploadBuffer));
  if (FAILED(hr)) {
    m_helpTexture.Reset();
    return false;
  }

  m_helpTexturePage = 0;
  return true;
}

void CPluginShell::UpdateHelpTexture(int page) {
  if (!m_helpTexture.IsValid() || !m_helpUploadBuffer || !m_lpDX) return;

  UINT w = m_helpTexture.width;
  UINT h = m_helpTexture.height;
  UINT rowPitch = (w * 4 + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1)
                  & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1);

  // Select help text for the requested page.
  // g_szHelp_W == 1 means Unicode; 0 means ANSI bytes cast to wchar_t*.
  void* helpText = (page == 1) ? (void*)g_szHelp : (void*)g_szHelp_Page2;
  if (!helpText) return;
  bool isUnicode = (g_szHelp_W != 0);

  // Create GDI DIB section for text rendering
  BITMAPINFO bmi = {};
  bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth       = w;
  bmi.bmiHeader.biHeight      = -(int)h;  // top-down
  bmi.bmiHeader.biPlanes      = 1;
  bmi.bmiHeader.biBitCount    = 32;
  bmi.bmiHeader.biCompression = BI_RGB;

  void* dibBits = nullptr;
  HDC memDC = CreateCompatibleDC(nullptr);
  HBITMAP hBmp = CreateDIBSection(memDC, &bmi, DIB_RGB_COLORS, &dibBits, nullptr, 0);
  if (!hBmp || !dibBits) {
    DeleteDC(memDC);
    return;
  }

  HGDIOBJ oldBmp = SelectObject(memDC, hBmp);

  // Clear to fully transparent black
  memset(dibBits, 0, (size_t)w * h * 4);

  // Scale font to window height — large enough for accessibility.
  // Compensate for DPI: the memory DC inherits the display's DPI, so GDI
  // inflates font sizes by dpi/96. Divide out so we get true pixel sizes.
  int dpiY = GetDeviceCaps(memDC, LOGPIXELSY);
  if (dpiY <= 0) dpiY = 96;
  int fontSize = max(22, (int)h / 32);
  int fontRequest = MulDiv(fontSize, 96, dpiY);  // actual pixels, DPI-neutral
  LONG pad = max(20L, (LONG)h / 40);

  HFONT hFont = CreateFontW(
      -fontRequest, 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE,
      DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
      ANTIALIASED_QUALITY, FIXED_PITCH | FF_MODERN, L"Consolas");
  HGDIOBJ oldFont = SelectObject(memDC, hFont);

  // White text on transparent background
  SetBkMode(memDC, TRANSPARENT);
  SetTextColor(memDC, RGB(255, 255, 255));

  RECT textRect = { pad, pad, (LONG)w - pad, (LONG)h - pad };
  if (isUnicode)
    ::DrawTextW(memDC, (const wchar_t*)helpText, -1, &textRect,
                DT_LEFT | DT_TOP | DT_EXPANDTABS);
  else
    ::DrawTextA(memDC, (const char*)helpText, -1, &textRect,
                DT_LEFT | DT_TOP | DT_EXPANDTABS);

  SelectObject(memDC, oldFont);
  DeleteObject(hFont);
  SelectObject(memDC, oldBmp);
  DeleteDC(memDC);

  // Post-process: convert GDI grayscale antialiasing to proper alpha
  // GDI renders white text (R=G=B=intensity) but leaves A=0.
  // Convert: text pixels → BGRA(255,255,255, intensity), bg → BGRA(0,0,0,0)
  BYTE* pixels = (BYTE*)dibBits;
  for (UINT y = 0; y < h; y++) {
    for (UINT x = 0; x < w; x++) {
      UINT idx = (y * w + x) * 4;
      BYTE b = pixels[idx + 0];
      BYTE g = pixels[idx + 1];
      BYTE r = pixels[idx + 2];
      BYTE intensity = (r > g) ? ((r > b) ? r : b) : ((g > b) ? g : b);
      if (intensity > 0) {
        pixels[idx + 0] = 255;       // B
        pixels[idx + 1] = 255;       // G
        pixels[idx + 2] = 255;       // R
        pixels[idx + 3] = intensity; // A = text coverage
      }
    }
  }

  // Copy DIB to upload buffer with row-pitch alignment
  BYTE* uploadPtr = nullptr;
  m_helpUploadBuffer->Map(0, nullptr, (void**)&uploadPtr);
  if (uploadPtr) {
    for (UINT y = 0; y < h; y++) {
      memcpy(uploadPtr + y * rowPitch, pixels + y * w * 4, w * 4);
    }
    m_helpUploadBuffer->Unmap(0, nullptr);
  }

  DeleteObject(hBmp);

  // Record copy + transition on the current command list
  auto* cmdList = m_lpDX->m_commandList.Get();

  m_lpDX->TransitionResource(m_helpTexture, D3D12_RESOURCE_STATE_COPY_DEST);

  D3D12_TEXTURE_COPY_LOCATION src = {};
  src.pResource = m_helpUploadBuffer.Get();
  src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
  src.PlacedFootprint.Offset = 0;
  src.PlacedFootprint.Footprint.Format   = DXGI_FORMAT_B8G8R8A8_UNORM;
  src.PlacedFootprint.Footprint.Width    = w;
  src.PlacedFootprint.Footprint.Height   = h;
  src.PlacedFootprint.Footprint.Depth    = 1;
  src.PlacedFootprint.Footprint.RowPitch = rowPitch;

  D3D12_TEXTURE_COPY_LOCATION dst = {};
  dst.pResource = m_helpTexture.resource.Get();
  dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
  dst.SubresourceIndex = 0;

  cmdList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

  m_lpDX->TransitionResource(m_helpTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

  m_helpTexturePage = page;
}

void CPluginShell::RenderBuiltInTextMsgs() {
  if (!m_lpDX || m_show_help == 0) return;

  UINT curW = (UINT)m_lpDX->m_client_width;
  UINT curH = (UINT)m_lpDX->m_client_height;

  // Recreate if window size changed (fullscreen toggle, resize)
  if (m_helpTexture.IsValid() &&
      (m_helpTexture.width != curW || m_helpTexture.height != curH)) {
    m_helpTexture.Reset();
    m_helpUploadBuffer.Reset();
    m_helpTexturePage = 0;
  }

  // Lazy-create help texture on first use
  if (!m_helpTexture.IsValid()) {
    if (!CreateHelpTexture()) return;
  }

  // Re-render if page changed
  if (m_helpTexturePage != m_show_help) {
    UpdateHelpTexture(m_show_help);
  }

  auto* cmdList = m_lpDX->m_commandList.Get();

  // Help overlay covers most of the screen with margins
  float marginX = 0.06f;
  float marginY = 0.06f;
  float x0 = -1.0f + marginX * 2.0f;
  float y0 =  1.0f - marginY * 2.0f;
  float x1 =  1.0f - marginX * 2.0f;
  float y1 = -1.0f + marginY * 2.0f;

  // Pass 1: Dark semi-transparent background quad
  WFVERTEX bgVerts[6];
  DWORD bgColor = 0xC0000000; // 75% opaque black
  bgVerts[0] = { x0, y0, 0, bgColor };
  bgVerts[1] = { x1, y0, 0, bgColor };
  bgVerts[2] = { x0, y1, 0, bgColor };
  bgVerts[3] = { x0, y1, 0, bgColor };
  bgVerts[4] = { x1, y0, 0, bgColor };
  bgVerts[5] = { x1, y1, 0, bgColor };

  cmdList->SetPipelineState(m_lpDX->m_PSOs[PSO_ALPHABLEND_WFVERTEX].Get());
  m_lpDX->DrawVertices(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, bgVerts, 6, sizeof(WFVERTEX));

  // Pass 2: Text texture overlay (white text on transparent background)
  m_lpDX->TransitionResource(m_helpTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

  D3D12_GPU_DESCRIPTOR_HANDLE srvHandle = m_lpDX->GetBindingBlockGpuHandle(m_helpTexture);
  cmdList->SetGraphicsRootDescriptorTable(1, srvHandle);

  SPRITEVERTEX textVerts[6];
  DWORD white = 0xFFFFFFFF;
  textVerts[0] = { x0, y0, 0, white, 0, 0 };
  textVerts[1] = { x1, y0, 0, white, 1, 0 };
  textVerts[2] = { x0, y1, 0, white, 0, 1 };
  textVerts[3] = { x0, y1, 0, white, 0, 1 };
  textVerts[4] = { x1, y0, 0, white, 1, 0 };
  textVerts[5] = { x1, y1, 0, white, 1, 1 };

  cmdList->SetPipelineState(m_lpDX->m_PSOs[PSO_TEXTURED_SPRITEVERTEX].Get());
  m_lpDX->DrawVertices(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, textVerts, 6, sizeof(SPRITEVERTEX));
}

void CPluginShell::RenderPlaylist() {
  // Phase 5 TODO: replace D3DX font playlist rendering with DirectXTK12 SpriteFont.
  // For now playlist display is disabled until SpriteFont migration is complete.
  if (m_show_playlist) {
    int nSongs = 0;
    if (nSongs <= 0) {
      m_show_playlist = 0;
    }
    else if (false) {
      int playlist_vert_pixels = m_lower_left_corner_y - m_upper_left_corner_y;
      int disp_lines = min(MAX_SONGS_PER_PAGE, (playlist_vert_pixels - PLAYLIST_INNER_MARGIN * 2) / GetFontHeight(PLAYLIST_FONT));
      int total_pages = (nSongs) / disp_lines;

      if (disp_lines <= 0)
        return;

      // apply PgUp/PgDn keypresses since last time
      m_playlist_pos -= m_playlist_pageups * disp_lines;
      m_playlist_pageups = 0;

      if (m_playlist_pos < 0)
        m_playlist_pos = 0;
      if (m_playlist_pos >= nSongs)
        m_playlist_pos = nSongs - 1;

      // NOTE: 'dwFlags' is used for both DDRAW and DX9
      DWORD dwFlags = DT_SINGLELINE;// | DT_NOPREFIX | DT_WORD_ELLIPSIS;
      DWORD color;

      int cur_page = (m_playlist_pos) / disp_lines;
      int cur_line = (m_playlist_pos + disp_lines - 1) % disp_lines;
      int new_top_idx = cur_page * disp_lines;
      int new_btm_idx = new_top_idx + disp_lines;
      wchar_t buf[1024] = { 0 };

      // ask winamp for the song names, but DO IT BEFORE getting the DC,
      // otherwise vaio will crash (~DDRAW port).
      if (m_playlist_top_idx != new_top_idx ||
        m_playlist_btm_idx != new_btm_idx) {
        for (int i = 0; i < disp_lines; i++) {
          int j = new_top_idx + i;
          if (j < nSongs) {
            // clip max len. of song name to 240 chars, to prevent overflows
            //lstrcpynW(buf, (wchar_t*)SendMessage(m_hWndWinamp, WM_USER, j, IPC_GETPLAYLISTTITLEW), 240);
            //wsprintfW(m_playlist[i], L"%d. %s ", j+1, buf);  // leave an extra space @ end, so italicized fonts don't get clipped
          }
        }
      }

      // update playlist cache, if necessary:
      if (m_playlist_top_idx != new_top_idx ||
        m_playlist_btm_idx != new_btm_idx) {
        m_playlist_top_idx = new_top_idx;
        m_playlist_btm_idx = new_btm_idx;
        m_playlist_width_pixels = 0;

        int max_w = min(m_right_edge - m_left_edge, m_lpDX->m_client_width - TEXT_MARGIN * 2 - PLAYLIST_INNER_MARGIN * 2);

        for (int i = 0; i < disp_lines; i++) {
          int j = new_top_idx + i;
          if (j < nSongs) {
            // clip max len. of song name to 240 chars, to prevent overflows
            //strcpy(buf, (char*)SendMessage(m_hWndWinamp, WM_USER, j, 212));
            //buf[240] = 0;
            //sprintf(m_playlist[i], "%d. %s ", j+1, buf);  // leave an extra space @ end, so italicized fonts don't get clipped

            // Phase 5 TODO: measure text width with SpriteFont
            // m_d3dx_font[PLAYLIST_FONT]->DrawTextW(...) -> SpriteFont equivalent
            int w = 0; // placeholder
            if (w > 0)
              m_playlist_width_pixels = max(m_playlist_width_pixels, w);
          }
          else {
            m_playlist[i][0] = 0;
          }
        }

        if (m_playlist_width_pixels == 0 ||
          m_playlist_width_pixels > max_w)
          m_playlist_width_pixels = max_w;
      }

      int start = max(0, (cur_page)*disp_lines);
      int end = min(nSongs, (cur_page + 1) * disp_lines);

      // draw dark box around where the playlist will go:

      RECT r;
      r.top = m_upper_left_corner_y;
      r.left = m_left_edge;
      r.right = m_left_edge + m_playlist_width_pixels + PLAYLIST_INNER_MARGIN * 2;
      r.bottom = m_upper_left_corner_y + (end - start) * GetFontHeight(PLAYLIST_FONT) + PLAYLIST_INNER_MARGIN * 2;
      DrawDarkTranslucentBox(&r);

      //m_d3dx_font[PLAYLIST_FONT]->Begin();

      // draw playlist text
      const int now_playing = -1; // Winamp now_playing removed; Phase 5 TODO: wire up actual playback position
      int y = m_upper_left_corner_y + PLAYLIST_INNER_MARGIN;
      for (int i = start; i < end; i++) {
        SetRect(&r, m_left_edge + PLAYLIST_INNER_MARGIN, y, m_left_edge + PLAYLIST_INNER_MARGIN + m_playlist_width_pixels, y + GetFontHeight(PLAYLIST_FONT));

        if (m_lpDX->GetBitDepth() == 8)
          color = (i == m_playlist_pos) ?
          (i == now_playing ? 0xFFFFFFFF : 0xFFFFFFFF) :
          (i == now_playing ? 0xFFFFFFFF : 0xFF707070);
        else
          color = (i == m_playlist_pos) ?
          (i == now_playing ? PLAYLIST_COLOR_BOTH : PLAYLIST_COLOR_HILITE_TRACK) :
          (i == now_playing ? PLAYLIST_COLOR_PLAYING_TRACK : PLAYLIST_COLOR_NORMAL);

        // Phase 5 TODO: draw playlist entry with SpriteFont
        y += GetFontHeight(PLAYLIST_FONT); // placeholder advance
      }

      //m_d3dx_font[PLAYLIST_FONT]->End();
    }
  }
}

void CPluginShell::SuggestHowToFreeSomeMem() {
  // This function is called when the plugin runs out of video memory;
  //   it lets you show a messagebox to the user so you can (intelligently)
  //   suggest how to free up some video memory, based on what settings
  //   they've chosen.

  wchar_t str[1024];

  // DX12 has no multisampling setting equivalent here
  wasabiApiLangString(IDS_TO_FREE_UP_VIDEO_MEMORY, str, 2048);

  MessageBoxW(m_lpDX->GetHwnd(), str, wasabiApiLangString(IDS_MILKDROP_SUGGESTION), MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
}

LRESULT CALLBACK CPluginShell::WindowProc(HWND hWnd, unsigned uMsg, WPARAM wParam, LPARAM lParam) {
  //if (uMsg==WM_GETDLGCODE)
  //    return DLGC_WANTALLKEYS|DLGC_WANTCHARS|DLGC_WANTMESSAGE;    // this tells the embedwnd that we want keypresses to flow through to our client wnd.

  if (uMsg == WM_CREATE) {
    CREATESTRUCT* create = (CREATESTRUCT*)lParam;
    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)create->lpCreateParams);
  }

  CPluginShell* p = (CPluginShell*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
  if (p)
    return p->PluginShellWindowProc(hWnd, uMsg, wParam, lParam);
  else
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

LRESULT CPluginShell::PluginShellWindowProc(HWND hWnd, unsigned uMsg, WPARAM wParam, LPARAM lParam) {
  USHORT mask = 1 << (sizeof(SHORT) * 8 - 1);
  //bool bShiftHeldDown = (GetKeyState(VK_SHIFT) & mask) != 0;
  bool bCtrlHeldDown = (GetKeyState(VK_CONTROL) & mask) != 0;
  //bool bAltHeldDown: most keys come in under WM_SYSKEYDOWN when ALT is depressed.
  RECT rect;

  switch (uMsg) {
  case WM_ERASEBKGND:
    // Repaint window when song is paused and image needs to be repainted:
    if (m_lpDX && m_lpDX->m_ready && GetFrame() > 0) {
      // DX12: re-present the last frame when window is erased (no explicit Present needed;
      // the swap chain already holds the last rendered buffer).
      return 0;
    }
    break;

  case WM_WINDOWPOSCHANGING:
    if (m_lpDX && m_lpDX->m_ready && m_lpDX->m_current_mode.m_skin)
      m_lpDX->SaveWindow();
    break;

  case WM_DESTROY:
    // note: don't post quit message here if the window is being destroyed
    // and re-created on a switch between windowed & FAKE fullscreen modes.
    if (!m_lpDX->TempIgnoreDestroyMessages()) {
      // this is a final exit, and not just destroy-then-recreate-the-window.
      // so, flag DXContext so it knows that someone else
      // will take care of destroying the window!
      m_lpDX->OnTrulyExiting();
      PostQuitMessage(0);
    }
    return FALSE;
    break;

    // benski> a little hack to get the window size correct. it seems to work
  case WM_USER + 555:
    if (m_lpDX && m_lpDX->m_ready && !m_resizing) {
      OnUserResizeWindow();
      m_lpDX->SaveWindow();
    }
    break;

  case WM_SIZE:

    // clear or set activity flag to reflect focus
    if (m_lpDX && m_lpDX->m_ready && !m_resizing) {
      m_hidden = (SIZE_MAXHIDE == wParam || SIZE_MINIMIZED == wParam) ? TRUE : FALSE;
      // SPOUT DEBUG
      // Allow restore from minimize without reset of the window
      if (SIZE_MAXIMIZED == wParam || SIZE_RESTORED == wParam) // the window has been maximized or restored
        // if (SIZE_MAXIMIZED == wParam ) // the window has been maximized
        OnUserResizeWindow();
    }

    break;

  case WM_ENTERSIZEMOVE:
    m_resizing = 1;
    break;

  case WM_EXITSIZEMOVE:
    // SPOUT
    // Find out whether the window has been resized or just moved
    GetClientRect(hWnd, &rect);
    if ((rect.right - rect.left) != 1280
      || (rect.bottom - rect.top) != 720) {
      if (m_lpDX && m_lpDX->m_ready)
        OnUserResizeWindow();
    }
    m_resizing = 0;
    break;

  case WM_GETMINMAXINFO:
  {
    // don't let the window get too small
    MINMAXINFO* p = (MINMAXINFO*)lParam;
    if (p->ptMinTrackSize.x < 64)
      p->ptMinTrackSize.x = 64;
    p->ptMinTrackSize.y = p->ptMinTrackSize.x * 3 / 4;
  }
  return 0;

  case WM_SETFOCUS:
    // note: this msg never comes in when embedwnd is used, but that's ok, because that's only
    // in Windowed mode, and m_lost_focus only makes us sleep when fullscreen.
    m_lost_focus = 0;
    break;

  case WM_KILLFOCUS:
    // note: this msg never comes in when embedwnd is used, but that's ok, because that's only
    // in Windowed mode, and m_lost_focus only makes us sleep when fullscreen.
    m_lost_focus = 1;
    break;

  case WM_COMMAND:
  {
    // then allow the plugin to override any command:
    if (MyWindowProc(hWnd, uMsg, wParam, lParam) == 0)
      return 0;
  }
  break;

  case WM_SYSCHAR:
    if ((wParam == 'k' || wParam == 'K')) {
      OnAltK();
      return 0;
    }
    break;

  case WM_CHAR:
    // if playlist is showing, steal p/j keys from the plugin:
    if (m_show_playlist) {
      /* resync m_playlist_pos */
    }

    // then allow the plugin to override any keys:
    if (MyWindowProc(hWnd, uMsg, wParam, lParam) == 0)
      return 0;

    switch (wParam) {
      // WINAMP PLAYBACK CONTROL KEYS:
    case 'x':
    case 'X':
      /* play */
      return 0;
    case 'c':
    case 'C':
      /* pause */
      return 0;
    case 'v':
    case 'V':
      /* stop */
      return 0;
    case 'b':
    case 'B':
      /* next */
      return 0;
    case 's':
    case 'S':
      /* shuffle */
      return 0;
    case 'r':
    case 'R':
      /* repeat */
      return 0;
    }

    return 0;

  case WM_KEYUP:

    // allow the plugin to override any keys:
    if (MyWindowProc(hWnd, uMsg, wParam, lParam) == 0)
      return 0;

    return 0;
    break;

  case WM_KEYDOWN:

    // SPOUT DEBUG : BeatDrop help changed from F12
    // Special case to pass the key code on to plugin
    // so that the ui mode is set back to regular
    // and any existing mode is cancelled and text is cleared
    if (wParam == VK_F1) {
      MyWindowProc(hWnd, uMsg, wParam, lParam);
      return 0;
    }

    if (m_show_playlist) {
      switch (wParam) {
      case VK_UP:
      {
        int nRepeat = lParam & 0xFFFF;
        if (GetKeyState(VK_SHIFT) & mask)
          m_playlist_pos -= 10 * nRepeat;
        else
          m_playlist_pos -= nRepeat;
      }
      return 0;

      case VK_DOWN:
      {
        int nRepeat = lParam & 0xFFFF;
        if (GetKeyState(VK_SHIFT) & mask)
          m_playlist_pos += 10 * nRepeat;
        else
          m_playlist_pos += nRepeat;
      }
      return 0;

      case VK_HOME:
        m_playlist_pos = 0;
        return 0;

      case VK_END:
        return 0;

      case VK_PRIOR:
        if (GetKeyState(VK_SHIFT) & mask)
          m_playlist_pageups += 10;
        else
          m_playlist_pageups++;
        return 0;

      case VK_NEXT:
        if (GetKeyState(VK_SHIFT) & mask)
          m_playlist_pageups -= 10;
        else
          m_playlist_pageups--;
        return 0;

      case VK_RETURN:
        /* set playlist selection, and play */
        return 0;
      }
    }

    // allow the plugin to override any keys:
    // Note from plugin.cpp
    // handle non - character keys(virtual keys) and return 0.
    //         if we don't handle them, return 1, and the shell will
    //         (passing some to the shell's key bindings, some to Winamp,
    //          and some to DefWindowProc)
    if (MyWindowProc(hWnd, uMsg, wParam, lParam) == 0)
      return 0;

    switch (wParam) {
      // SPOUT : hide/show render window
    case VK_F12:
      if (IsWindowVisible(GetPluginWindow()))
        ShowWindow(GetPluginWindow(), SW_HIDE);
      else
        ShowWindow(GetPluginWindow(), SW_SHOW);
      return 0;

    case VK_ESCAPE:
      if (m_show_help)
        ToggleHelp();
      return 0;

    case VK_LEFT:
    case VK_RIGHT:
    {
      /* rewind 5 seconds, ff 5 seconds */
    }
    return 0;
    }
    return 0;
  }
  if (uMsg == WM_WINDOWPOSCHANGING || uMsg == WM_WINDOWPOSCHANGED) {
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
  }
  return MyWindowProc(hWnd, uMsg, wParam, lParam);
}

void CPluginShell::ToggleHelp() {
  if (m_show_help == 0) {
    m_show_help = 1;
  }
  else if (m_show_help == 1) {
    m_show_help = 2;
  }
  else if (m_show_help == 2) {
    m_show_help = 0;
  }
}

LRESULT CALLBACK CPluginShell::DesktopWndProc(HWND hWnd, unsigned uMsg, WPARAM wParam, LPARAM lParam) {
  CPluginShell* p = (CPluginShell*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
  if (p)
    return p->PluginShellDesktopWndProc(hWnd, uMsg, wParam, lParam);
  else
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CPluginShell::PluginShellDesktopWndProc(HWND hWnd, unsigned uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
  case WM_KEYDOWN:
  case WM_KEYUP:
  case WM_CHAR:
  case WM_SYSCHAR:
  case WM_SYSKEYDOWN:
  case WM_SYSKEYUP:
    //PostMessage(GetPluginWindow(), uMsg, wParam, lParam);
    PluginShellWindowProc(GetPluginWindow(), uMsg, wParam, lParam);
    return 0;
    break;
  }

  return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void CPluginShell::AlignWaves() {
  // align waves, using recursive (mipmap-style) least-error matching
  // note: NUM_WAVEFORM_SAMPLES must be between 32 and 576.

  int align_offset[2] = { 0, 0 };

#if (NUM_WAVEFORM_SAMPLES < 576) // [don't let this code bloat our DLL size if it's not going to be used]

  int nSamples = NUM_WAVEFORM_SAMPLES;

#define MAX_OCTAVES 10

  int octaves = (int)floorf(logf((float)(576 - nSamples)) / logf(2.0f));
  if (octaves < 4)
    return;
  if (octaves > MAX_OCTAVES)
    octaves = MAX_OCTAVES;

  for (int ch = 0; ch < 2; ch++) {
    // only worry about matching the lower 'nSamples' samples
    float temp_new[MAX_OCTAVES][576];
    float temp_old[MAX_OCTAVES][576];
    static float temp_weight[MAX_OCTAVES][576];
    static int   first_nonzero_weight[MAX_OCTAVES];
    static int   last_nonzero_weight[MAX_OCTAVES];
    int spls[MAX_OCTAVES];
    int space[MAX_OCTAVES];

    memcpy(temp_new[0], m_sound.fWaveform[ch], sizeof(float) * 576);
    memcpy(temp_old[0], &m_oldwave[ch][m_prev_align_offset[ch]], sizeof(float) * nSamples);
    spls[0] = 576;
    space[0] = 576 - nSamples;

    // potential optimization: could reuse (instead of recompute) mip levels for m_oldwave[2][]?
    for (int octave = 1; octave < octaves; octave++) {
      spls[octave] = spls[octave - 1] / 2;
      space[octave] = space[octave - 1] / 2;
      for (int n = 0; n < spls[octave]; n++) {
        temp_new[octave][n] = 0.5f * (temp_new[octave - 1][n * 2] + temp_new[octave - 1][n * 2 + 1]);
        temp_old[octave][n] = 0.5f * (temp_old[octave - 1][n * 2] + temp_old[octave - 1][n * 2 + 1]);
      }
    }

    if (!m_align_weights_ready) {
      m_align_weights_ready = 1;
      for (octave = 0; octave < octaves; octave++) {
        int compare_samples = spls[octave] - space[octave];
        for (int n = 0; n < compare_samples; n++) {
          // start with pyramid-shaped pdf, from 0..1..0
          if (n < compare_samples / 2)
            temp_weight[octave][n] = n * 2 / (float)compare_samples;
          else
            temp_weight[octave][n] = (compare_samples - 1 - n) * 2 / (float)compare_samples;

          // TWEAK how much the center matters, vs. the edges:
          temp_weight[octave][n] = (temp_weight[octave][n] - 0.8f) * 5.0f + 0.8f;

          // clip:
          if (temp_weight[octave][n] > 1) temp_weight[octave][n] = 1;
          if (temp_weight[octave][n] < 0) temp_weight[octave][n] = 0;
        }

        n = 0;
        while (temp_weight[octave][n] == 0 && n < compare_samples)
          n++;
        first_nonzero_weight[octave] = n;

        n = compare_samples - 1;
        while (temp_weight[octave][n] == 0 && n >= 0)
          n--;
        last_nonzero_weight[octave] = n;
      }
    }

    int n1 = 0;
    int n2 = space[octaves - 1];
    for (octave = octaves - 1; octave >= 0; octave--) {
      // for example:
      //  space[octave] == 4
      //  spls[octave] == 36
      //  (so we test 32 samples, w/4 offsets)
      int compare_samples = spls[octave] - space[octave];

      int lowest_err_offset = -1;
      float lowest_err_amount = 0;
      for (int n = n1; n < n2; n++) {
        float err_sum = 0;
        //for (int i=0; i<compare_samples; i++)
        for (int i = first_nonzero_weight[octave]; i <= last_nonzero_weight[octave]; i++) {
          float x = (temp_new[octave][i + n] - temp_old[octave][i]) * temp_weight[octave][i];
          if (x > 0)
            err_sum += x;
          else
            err_sum -= x;
        }

        if (lowest_err_offset == -1 || err_sum < lowest_err_amount) {
          lowest_err_offset = n;
          lowest_err_amount = err_sum;
        }
      }

      // now use 'lowest_err_offset' to guide bounds of search in next octave:
      //  space[octave] == 8
      //  spls[octave] == 72
      //     -say 'lowest_err_offset' was 2
      //     -that corresponds to samples 4 & 5 of the next octave
      //     -also, expand about this by 2 samples?  YES.
      //  (so we'd test 64 samples, w/8->4 offsets)
      if (octave > 0) {
        n1 = lowest_err_offset * 2 - 1;
        n2 = lowest_err_offset * 2 + 2 + 1;
        if (n1 < 0) n1 = 0;
        if (n2 > space[octave - 1]) n2 = space[octave - 1];
      }
      else
        align_offset[ch] = lowest_err_offset;
    }
  }
#endif
  memcpy(m_oldwave[0], m_sound.fWaveform[0], sizeof(float) * 576);
  memcpy(m_oldwave[1], m_sound.fWaveform[1], sizeof(float) * 576);
  m_prev_align_offset[0] = align_offset[0];
  m_prev_align_offset[1] = align_offset[1];

  // finally, apply the results: modify m_sound.fWaveform[2][0..576]
  // by scooting the aligned samples so that they start at m_sound.fWaveform[2][0].
  for (ch = 0; ch < 2; ch++)
    if (align_offset[ch] > 0) {
      for (int i = 0; i < nSamples; i++)
        m_sound.fWaveform[ch][i] = m_sound.fWaveform[ch][i + align_offset[ch]];
      // zero the rest out, so it's visually evident that these samples are now bogus:
      memset(&m_sound.fWaveform[ch][nSamples], 0, (576 - nSamples) * sizeof(float));
    }
}

LRESULT CALLBACK CPluginShell::VJModeWndProc(HWND hWnd, unsigned uMsg, WPARAM wParam, LPARAM lParam) {
  CPluginShell* p = (CPluginShell*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
  if (p)
    return p->PluginShellVJModeWndProc(hWnd, uMsg, wParam, lParam);
  else
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CPluginShell::PluginShellVJModeWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message) {
  case WM_KEYDOWN:
  case WM_KEYUP:
  case WM_CHAR:
  case WM_SYSKEYDOWN:
  case WM_SYSKEYUP:
  case WM_SYSCHAR:
    // pass keystrokes on to plugin!
    return PluginShellWindowProc(GetPluginWindow(), message, wParam, lParam);

  case WM_ERASEBKGND:
    // VJ DX9 Present removed in DX12 migration; Phase 5 TODO: DX12 VJ window present.
    break;

  case WM_CLOSE:
    // if they close the VJ window (by some means other than ESC key),
    // this will make the graphics window close, too.
    m_exiting = 1;
    if (GetPluginWindow())
      PostMessage(GetPluginWindow(), WM_CLOSE, 0, 0);
    break;

  case WM_GETMINMAXINFO:
  {
    // don't let the window get too small
    MINMAXINFO* p = (MINMAXINFO*)lParam;
    if (p->ptMinTrackSize.x < 64)
      p->ptMinTrackSize.x = 64;
    p->ptMinTrackSize.y = p->ptMinTrackSize.x * 3 / 4;
  }
  return 0;

  case WM_SIZE:
    // VJ DX9 device removed in DX12 migration; Phase 5 TODO: DX12 VJ window resize.
    break;

  case WM_ENTERSIZEMOVE:
    m_resizing_textwnd = 1;
    break;

  case WM_EXITSIZEMOVE:
    // VJ DX9 device removed; Phase 5 TODO: DX12 VJ window resize.
    m_resizing_textwnd = 0;
    break;
  }

  return DefWindowProc(hwnd, message, wParam, lParam);
}

DWORD CPluginShell::GetFontColor(int fontIndex) {
  DWORD cr = m_fontinfo[fontIndex].R;
  DWORD cg = m_fontinfo[fontIndex].G;
  DWORD cb = m_fontinfo[fontIndex].B;
  DWORD alpha = 255;
  alpha = 255;
  DWORD z = (alpha << 24) | (cr << 16) | (cg << 8) | cb;
  return z;
}

bool CPluginShell::IsSpoutActiveAndFixed() {
  return (bSpoutOut && bSpoutFixedSize);
}

void CPluginShell::SetVariableBackBuffer(int width, int height) {
  if (IsSpoutActiveAndFixed() || width == 0 || height == 0) return;
  float q = GetEffectiveRenderQuality(width, height);
  m_backBufWidth  = (int)(width  * q);
  m_backBufHeight = (int)(height * q);
}

void CPluginShell::UpdateBackBufferTracking(int width, int height) {
  if (!m_lpDX) return;
  m_lpDX->m_backbuffer_width = width;
  m_lpDX->m_backbuffer_height = height;
}

float CPluginShell::GetEffectiveRenderQuality(int width, int height) {
  float q = clamp(m_fRenderQuality, 0.01, 1);
  if (bQualityAuto) {
    // adjust quality based on window/screen ratio
    // Use runtime GetProcAddress to avoid depending on an import thunk (xGetSystemMetrics)
    // which can cause unresolved externals with certain SDK / linker setups.
    if (m_screen_pixels == -1) {
      HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
      typedef int(WINAPI* PFN_GetSystemMetrics)(int);
      PFN_GetSystemMetrics pGetSystemMetrics = hUser32 ? (PFN_GetSystemMetrics)GetProcAddress(hUser32, "GetSystemMetrics") : NULL;
      int cxScreen = 0, cyScreen = 0;
      if (pGetSystemMetrics) {
        cxScreen = pGetSystemMetrics(SM_CXSCREEN);
        cyScreen = pGetSystemMetrics(SM_CYSCREEN);
      }
      else {
        // fallback: assume 1920x1080 to avoid divide-by-zero if lookup fails
        cxScreen = 1920;
        cyScreen = 1080;
      }
      m_screen_pixels = cxScreen * cyScreen;
    }
    float avg_pixels = m_screen_pixels / 4;
    float window_pixels = (float)width * (float)height;
    q = q * sqrt(avg_pixels / window_pixels);
  }
  return clamp(q, 0.01, 1);
}

void CPluginShell::ResetBufferAndFonts() {
  RECT w, c;
  GetWindowRect(m_lpDX->GetHwnd(), &w);
  GetClientRect(m_lpDX->GetHwnd(), &c);
  m_lpDX->OnUserResizeWindow(&w, &c, false);

  int newW = m_lpDX->m_client_width;
  int newH = m_lpDX->m_client_height;
  SetVariableBackBuffer(newW, newH);
  UpdateBackBufferTracking(newW, newH);
  // DX12: resize swap chain instead of device reset
  m_lpDX->ResizeSwapChain(newW, newH);

  if (newW != 0 && newH != 0) {
    CleanUpFonts();
    AllocateFonts();
  }
}