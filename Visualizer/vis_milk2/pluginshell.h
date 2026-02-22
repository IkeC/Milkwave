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

#ifndef __NULLSOFT_DX9_PLUGIN_SHELL_H__
#define __NULLSOFT_DX9_PLUGIN_SHELL_H__ 1

#include "shell_defines.h"
#include "dxcontext.h"
#include "dx12helpers.h"   // DX12Texture
#include "d3dx9compat.h"  // for LPDIRECT3DDEVICE9, D3DFORMAT etc. used by legacy DX9 code in Phase 2-4 TODO files
#include "fft.h"
#include "defines.h"
#include "textmgr.h"
#include <vector>

// SPOUT
#include <string> // For std::string
#include "io.h" // for _access

#define TIME_HIST_SLOTS 128     // # of slots used if fps > 60.  half this many if fps==30.
#define MAX_SONGS_PER_PAGE 40

typedef struct {
  wchar_t szFace[256];
  int nSize;  // size requested @ font creation time
  int bBold;
  int bItalic;
  int bAntiAliased;
  int R = 255;
  int G = 255;
  int B = 255;
} td_fontinfo;

typedef struct {
  float imm[2][3];                        // bass, mids, treble, no damping, for each channel (long-term average is 1)
  float avg[2][3];                        // bass, mids, treble, some damping, for each channel (long-term average is 1)
  float med_avg[2][3];                    // bass, mids, treble, more damping, for each channel (long-term average is 1)
  float long_avg[2][3];                   // bass, mids, treble, heavy damping, for each channel (long-term average is 1)
  float infinite_avg[2][3];               // bass, mids, treble: winamp's average output levels. (1)
  float fWaveform[2][576];                // Not all 576 are valid! - only NUM_WAVEFORM_SAMPLES samples are valid for each channel (note: NUM_WAVEFORM_SAMPLES is declared in shell_defines.h)
  float fSpectrum[2][NUM_FREQUENCIES];    // NUM_FREQUENCIES samples for each channel (note: NUM_FREQUENCIES is declared in shell_defines.h)
} td_soundinfo;                             // ...range is 0 Hz to 22050 Hz, evenly spaced.

class CPluginShell {
public:
  // GET METHODS
  // ------------------------------------------------------------
  int       GetFrame();          // returns current frame # (starts at zero)
  float     GetTime();           // returns current animation time (in seconds) (starts at zero) (updated once per frame)
  float     GetFps();            // returns current estimate of framerate (frames per second)
  HINSTANCE GetInstance();       // returns handle to the plugin DLL module; used for things like loading resources (dialogs, bitmaps, icons...) that are built into the plugin.
  wchar_t* GetPluginsDirPath(); // usually returns 'c:\\program files\\winamp\\plugins\\'
  wchar_t* GetConfigIniFile();  // usually returns 'c:\\program files\\winamp\\plugins\\something.ini' - filename is determined from identifiers in 'defines.h'
  char* GetConfigIniFileA();
  void SetVariableBackBuffer(int width, int height);

  float GetEffectiveRenderQuality(int width, int height);

  void ResetBufferAndFonts();
  void UpdateBackBufferTracking(int width, int height);

  // Back-buffer dimensions used by the plugin (may differ from window size for fixed Spout sizes)
  int m_backBufWidth;
  int m_backBufHeight;

  DXContext* m_lpDX;            // pointer to DXContext object

protected:

  // GET METHODS THAT ONLY WORK ONCE DIRECTX IS READY
  // ------------------------------------------------------------
  //  The following 'Get' methods are only available after DirectX has been initialized.
  //  If you call these from OverrideDefaults, MyPreInitialize, or MyReadConfig,
  //    they will return NULL (zero).
  // ------------------------------------------------------------
  HWND         GetPluginWindow();          // returns handle to the plugin window
  int          GetWidth();                 // returns back-buffer width in pixels
  int          GetHeight();                // returns back-buffer height in pixels
  int          GetBitDepth();              // always 32 (RGBA8 back buffer)

  // Phase 1 compatibility stub — milkdropfs.cpp and plugin.cpp still use DX9-typed device.
  // Returns nullptr in DX12; real DX9 device is gone. Code that calls DX9 methods on this
  // will crash at runtime but compile cleanly. Phase 2-4 will replace these call sites.
  LPDIRECT3DDEVICE9          GetDevice()        { return nullptr; }

  // DX12 accessors — use these for new Phase 2+ code.
  ID3D12Device*              GetDX12Device();
  ID3D12GraphicsCommandList* GetCommandList();  // returns current frame's command list

  // DX9 adapter/format stubs removed in DX12 migration. Phase 2 TODO: DXGI equivalents.
  const char*  GetDriverDescription() const { return ""; }
  D3DFORMAT    GetBackBufFormat()     const { return D3DFMT_X8R8G8B8; }

  // GetCaps() stub — returns a static D3DCAPS9 with values set to allow all capabilities.
  // Phase 1: DX9 caps are meaningless in DX12; callers that gate on caps will get
  //          generous limits so the rendering code continues to run.
  // Phase 2-4 TODO: replace each caps check with the appropriate DX12 query.
  D3DCAPS9* GetCaps() {
      static D3DCAPS9 s_caps = {};
      static bool s_init = false;
      if (!s_init) {
          s_init = true;
          // Pixel shader version — report PS 3.0 (0x0300) so shader paths are fully enabled
          s_caps.PixelShaderVersion  = D3DPS_VERSION(3, 0);
          s_caps.VertexShaderVersion = D3DVS_VERSION(3, 0);
          // Texture size — 0 means "no restriction" in the original caps check code
          s_caps.MaxTextureWidth     = 0;
          s_caps.MaxTextureHeight    = 0;
          // Primitive count — large enough to never be a bottleneck
          s_caps.MaxPrimitiveCount   = 0x7FFFFFFF;
          // Blend caps — report all blend modes as supported
          s_caps.SrcBlendCaps        = 0xFFFFFFFF;
          s_caps.DestBlendCaps       = 0xFFFFFFFF;
          s_caps.AlphaCmpCaps        = 0xFFFFFFFF;
      }
      return &s_caps;
  }

  // FONTS & TEXT (Phase 5: D3DX fonts replaced by DirectXTK12 SpriteFont)
  // ------------------------------------------------------------
public:
  // GetFont() returns a non-null sentinel encoding the font index.
  // CTextManager::DecodeFontIndex() extracts the index back.
  LPD3DXFONT   GetFont(eFontIndex idx) { return (LPD3DXFONT)(intptr_t)(idx + 1); }
  int          GetFontHeight(eFontIndex idx);
  CTextManager m_text;
  
  wchar_t      m_szBaseDir[MAX_PATH]; 

protected:

  // MISC
  // ------------------------------------------------------------
  td_soundinfo m_sound;                   // a structure always containing the most recent sound analysis information; defined in pluginshell.h.
  void         SuggestHowToFreeSomeMem(); // gives the user a 'smart' messagebox that suggests how they can free up some video memory.

  // CONFIG PANEL SETTINGS
  // ------------------------------------------------------------
  // *** only read/write these values during CPlugin::OverrideDefaults! ***
  int          m_start_fullscreen;        // 0 or 1
  int          m_start_desktop;           // 0 or 1
  int          m_fake_fullscreen_mode;    // 0 or 1
  int          m_max_fps_fs;              // 1-120, or 0 for 'unlimited'
  int          m_max_fps_dm;              // 1-120, or 0 for 'unlimited'
  int          m_max_fps_w;               // 1-120, or 0 for 'unlimited'
  int          m_show_press_f1_msg;       // 0 or 1
  int          m_allow_page_tearing_w;    // 0 or 1
  int          m_allow_page_tearing_fs;   // 0 or 1
  int          m_allow_page_tearing_dm;   // 0 or 1
  int          m_minimize_winamp;         // 0 or 1
  int          m_desktop_show_icons;      // 0 or 1
  int          m_desktop_textlabel_boxes; // 0 or 1
  int          m_desktop_manual_icon_scoot; // 0 or 1
  int          m_desktop_555_fix;         // 0 = 555, 1 = 565, 2 = 888
  int          m_dualhead_horz;           // 0 = both, 1 = left, 2 = right
  int          m_dualhead_vert;           // 0 = both, 1 = top, 2 = bottom
  int          m_save_cpu;                // 0 or 1
  int          m_skin;                    // 0 or 1
  int          m_fix_slow_text;           // 0 or 1
  td_fontinfo  m_fontinfo[NUM_BASIC_FONTS + NUM_EXTRA_FONTS];

  // Fullscreen target width/height (replaces D3DDISPLAYMODEEX)
  int m_disp_mode_fs_width;
  int m_disp_mode_fs_height;

  // PURE VIRTUAL FUNCTIONS (...must be implemented by derived classes)
  // ------------------------------------------------------------
  virtual void OverrideDefaults() = 0;
  virtual void MyPreInitialize() = 0;
  virtual void MyReadConfig() = 0;
  virtual void MyWriteConfig() = 0;
  virtual int  AllocateMyNonDx9Stuff() = 0;
  virtual void  CleanUpMyNonDx9Stuff() = 0;
  virtual int  AllocateMyDX9Stuff() = 0;   // renamed to DX12 in Phase 2; kept for now
  virtual void  CleanUpMyDX9Stuff(int final_cleanup) = 0;
  virtual void MyRenderFn(int redraw) = 0;
  virtual void MyRenderUI(int* upper_left_corner_y, int* upper_right_corner_y, int* lower_left_corner_y, int* lower_right_corner_y, int xL, int xR) = 0;
  virtual LRESULT MyWindowProc(HWND hWnd, unsigned uMsg, WPARAM wParam, LPARAM lParam) = 0;
  virtual void OnAltK() {}; // doesn't *have* to be implemented
  virtual void SetAMDFlag() = 0;

  int m_show_help;

  float m_frameFactor = 1.0f;
  float m_timeFactor = 1.0f;
  float m_fpsFactor = 1.0f;

  float m_VisIntensity = 1.0f;
  float m_VisShift = 0.0f;
  float m_VisVersion = 1.0f;

  float m_ColShiftHue = 0.0f;
  float m_ColShiftSaturation = 0.0f;
  float m_ColShiftBrightness = 0.0f;

  bool m_AutoHue = false;
  float m_AutoHueSeconds = 0.02f;
  float m_AutoHueTimeLastChange = 0.0f;

  float m_fRenderQuality = 1.0f;
  //=====================================================================================================================
private:

  // GENERAL PRIVATE STUFF
  int          m_frame;           // current frame #, starting at zero
  double        m_time;            // current animation time in seconds; starts at zero.
  float        m_fps;             // current estimate of frames per second
  HINSTANCE    m_hInstance;       // handle to application instance
  wchar_t      m_szPluginsDirPath[MAX_PATH];  // usually 'c:\\program files\\winamp\\plugins\\'
  wchar_t      m_szConfigIniFile[MAX_PATH];   // usually 'c:\\program files\\winamp\\plugins\\something.ini' - filename is determined from identifiers in 'defines.h'
  char         m_szConfigIniFileA[MAX_PATH];   // usually 'c:\\program files\\winamp\\plugins\\something.ini' - filename is determined from identifiers in 'defines.h'

  // FONTS (Phase 5: will be DirectXTK12 SpriteFont objects)
  // Placeholder font heights used for layout calculations
  int m_fontHeight[NUM_BASIC_FONTS + NUM_EXTRA_FONTS];
  HFONT        m_font[NUM_BASIC_FONTS + NUM_EXTRA_FONTS];
  HFONT        m_font_desktop;

  // PRIVATE CONFIG PANEL SETTINGS
  GUID m_adapter_guid_fullscreen;
  GUID m_adapter_guid_desktop;
  GUID m_adapter_guid_windowed;
  char m_adapter_devicename_fullscreen[256];
  char m_adapter_devicename_desktop[256];
  char m_adapter_devicename_windowed[256];

  // PRIVATE RUNTIME SETTINGS
  int m_lost_focus;     // ~mostly for fullscreen mode
  int m_hidden;         // ~mostly for windowed mode
  int m_resizing;       // ~mostly for windowed mode
  int m_show_playlist;
  int  m_playlist_pos;            // current selection on (plugin's) playlist menu
  int  m_playlist_pageups;        // can be + or -
  int  m_playlist_top_idx;        // used to track when our little playlist cache (m_playlist) needs updated.
  int  m_playlist_btm_idx;        // used to track when our little playlist cache (m_playlist) needs updated.
  int  m_playlist_width_pixels;   // considered invalid whenever 'm_playlist_top_idx' is -1.
  wchar_t m_playlist[MAX_SONGS_PER_PAGE][256];   // considered invalid whenever 'm_playlist_top_idx' is -1.
  int m_exiting;
  int m_upper_left_corner_y;
  int m_lower_left_corner_y;
  int m_upper_right_corner_y;
  int m_lower_right_corner_y;
  int m_left_edge;
  int m_right_edge;
  int m_force_accept_WM_WINDOWPOSCHANGING;
  int m_screen_pixels = -1;

  // PRIVATE - DESKTOP MODE STUFF
  bool                m_bClearVJWindow;

  // PRIVATE - MORE TIMEKEEPING
protected:
  double m_last_raw_time;
  LARGE_INTEGER m_high_perf_timer_freq;  // 0 if high-precision timer not available
private:
  float  m_time_hist[TIME_HIST_SLOTS];		// cumulative
  int    m_time_hist_pos;
  LARGE_INTEGER m_prev_end_of_frame;

  // PRIVATE AUDIO PROCESSING DATA
  FFT   m_fftobj;
  float m_oldwave[2][576];        // for wave alignment
  int   m_prev_align_offset[2];   // for wave alignment
  int   m_align_weights_ready;

public:
  CPluginShell();
  ~CPluginShell();

  // called by vis.cpp, on behalf of Winamp:
  int  PluginPreInitialize(HWND hWinampWnd, HINSTANCE hWinampInstance);

  // Initialize the plugin with a DX12 device, command queue, and DXGI factory.
  int PluginInitialize(
    ID3D12Device*       device,
    ID3D12CommandQueue* commandQueue,
    IDXGIFactory4*      factory,
    HWND                hwnd,
    int                 iWidth,
    int                 iHeight);

  int  PluginRender(unsigned char* pWaveL, unsigned char* pWaveR);
  void PluginQuit();

  void ToggleHelp();

  void READ_FONT(int n);
  void WRITE_FONT(int n);

  void ReadConfig();
  void WriteConfig();

  int  AllocateDX9Stuff();    // renamed to DX12 in Phase 2; kept for now
  DWORD GetFontColor(int fontIndex);

  bool IsSpoutActiveAndFixed();
  void OnUserResizeWindow();
  void CleanUpFonts();
  int  AllocateFonts(); // Phase 5: will use DirectXTK12 SpriteFont

  // config panel / windows messaging processes:
  static LRESULT CALLBACK WindowProc(HWND hWnd, unsigned uMsg, WPARAM wParam, LPARAM lParam);
  static LRESULT CALLBACK DesktopWndProc(HWND hWnd, unsigned uMsg, WPARAM wParam, LPARAM lParam);
  static LRESULT CALLBACK VJModeWndProc(HWND hWnd, unsigned uMsg, WPARAM wParam, LPARAM lParam);
  static INT_PTR CALLBACK ConfigDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
  static INT_PTR CALLBACK TabCtrlProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
  static INT_PTR CALLBACK FontDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
  static INT_PTR CALLBACK DesktopOptionsDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
  static INT_PTR CALLBACK DualheadDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
  void DrawAndDisplay(int redraw);
  void DoTime();
  void AnalyzeNewSound(unsigned char* pWaveL, unsigned char* pWaveR);
  void AlignWaves();

  int  InitDirectX(ID3D12Device* device, ID3D12CommandQueue* commandQueue, IDXGIFactory4* factory, HWND hwnd, int width, int height);

  void CleanUpDirectX();
  int  InitGDIStuff();
  void CleanUpGDIStuff();
  void CleanUpDX9Stuff(int final_cleanup);
  int  InitNondx9Stuff();
  void CleanUpNondx9Stuff();
  int  InitVJStuff(RECT* pClientRect = NULL);
  void CleanUpVJStuff();
  void AllocateTextSurface();
  void OnUserResizeTextWindow();
  void PrepareFor2DDrawing_B(int w, int h); // Phase 4: rewritten for DX12 command list
  void RenderBuiltInTextMsgs();
  bool CreateHelpTexture();
  void UpdateHelpTexture(int page);

  // Help overlay texture (GDI-rendered text uploaded to DX12, sized to window)
  DX12Texture m_helpTexture;
  ComPtr<ID3D12Resource> m_helpUploadBuffer;
  int m_helpTexturePage = 0;  // 0 = not rendered yet
  int  GetCanvasMarginX();     // returns the # of pixels that exist on the canvas, on each side, that the user will never see.  Mainly here for windowed mode, where sometimes, up to 15 pixels get cropped at edges of the screen.
  int  GetCanvasMarginY();     // returns the # of pixels that exist on the canvas, on each side, that the user will never see.  Mainly here for windowed mode, where sometimes, up to 15 pixels get cropped at edges of the screen.
public:
  void DrawDarkTranslucentBox(RECT* pr);

  void DrawDarkTranslucentBoxFullWindow();

protected:
  void RenderPlaylist();
  void StuffParams(DXCONTEXT_PARAMS* pParams);
  void EnforceMaxFPS();

  // SEPARATE TEXT WINDOW (FOR VJ MODE)
  int 		m_vj_mode;
  int       m_hidden_textwnd;
  int       m_resizing_textwnd;
protected:
  HWND		m_hTextWnd;
  // SPOUT
  HWND		m_hRenderWnd;
private:
  int		m_nTextWndWidth;
  int		m_nTextWndHeight;
  bool		m_bTextWindowClassRegistered;
  //HDC		m_memDC;		// memory device context
  //HBITMAP m_memBM, m_oldBM;
  //HBRUSH  m_hBlackBrush;

  // WINDOWPROC FUNCTIONS
public:
  LRESULT PluginShellWindowProc(HWND hWnd, unsigned uMsg, WPARAM wParam, LPARAM lParam);   // in windowproc.cpp
  LRESULT PluginShellDesktopWndProc(HWND hWnd, unsigned uMsg, WPARAM wParam, LPARAM lParam);
  LRESULT PluginShellVJModeWndProc(HWND hWnd, unsigned uMsg, WPARAM wParam, LPARAM lParam);

protected:
  // CONFIG PANEL FUNCTIONS:
  BOOL    PluginShellConfigDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
  BOOL    PluginShellConfigTab1Proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
  BOOL    PluginShellFontDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
  BOOL    PluginShellDesktopOptionsDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
  BOOL    PluginShellDualheadDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
  bool    InitConfig(HWND hDialogWnd);
  void    EndConfig();
  void    UpdateAdapters(int screenmode);
  void    UpdateFSAdapterDispModes();   // (fullscreen only)
  void    UpdateDispModeMultiSampling(int screenmode);
  void    UpdateMaxFps(int screenmode);
  int     GetCurrentlySelectedAdapter(int screenmode);
  void    SaveDisplayMode();
  void    SaveMultiSamp(int screenmode);
  void    SaveAdapter(int screenmode);
  void    SaveMaxFps(int screenmode);
  void    OnTabChanged(int nNewTab);
  ID3D12Device* GetTextDevice() { return m_lpDX ? m_lpDX->m_device.Get() : nullptr; }

  // CHANGES:
  friend class CShaderParams;

public:
  bool bSpoutOut; // Spout output on or off
  bool bSpoutFixedSize; // Use Spout output fixed size
  int nSpoutFixedWidth = 1280;
  int nSpoutFixedHeight = 720;
  bool bQualityAuto = false;
};

#endif