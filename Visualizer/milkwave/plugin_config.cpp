// plugin_config.cpp - Configuration, initialization, and menu building
// Extracted from plugin.cpp

#include "plugin.h"
#include "plugin_internals.h"
#include "utility.h"
#include "support.h"
#include "resource.h"
#include "defines.h"
#include "shell_defines.h"
#include "wasabi.h"

void CPlugin::OverrideDefaults() {
  // Here, you have the option of overriding the "default defaults"
  //   for the stuff on tab 1 of the config panel, replacing them
  //   with custom defaults for your plugin.
  // To override any of the defaults, just uncomment the line
  //   and change the value.
  // DO NOT modify these values from any function but this one!

  // This example plugin only changes the default width/height
  //   for fullscreen mode; the "default defaults" are just
  //   640 x 480.
  // If your plugin is very dependent on smooth animation and you
  //   wanted it plugin to have the 'save cpu' option OFF by default,
  //   for example, you could set 'm_save_cpu' to 0 here.

  // m_start_fullscreen      = 0;       // 0 or 1
  // m_start_desktop         = 0;       // 0 or 1
  // m_fake_fullscreen_mode  = 0;       // 0 or 1
  // m_max_fps_fs            = 0;      // 1-120, or 0 for 'unlimited'
  // m_max_fps_dm            = 0;      // 1-120, or 0 for 'unlimited'
  // m_max_fps_w             = 0;      // 1-120, or 0 for 'unlimited'
  // m_show_press_f1_msg     = 1;       // 0 or 1
  m_allow_page_tearing_w = 0;  // 0 or 1
  // m_allow_page_tearing_fs = 0;       // 0 or 1
  // m_allow_page_tearing_dm = 1;       // 0 or 1
  // m_minimize_winamp       = 1;       // 0 or 1
  // m_desktop_textlabel_boxes = 1;     // 0 or 1
  // m_save_cpu              = 0;       // 0 or 1

  // lstrcpy(m_fontinfo[0].szFace, "Trebuchet MS"); // system font
  // m_fontinfo[0].nSize     = 18;
  // m_fontinfo[0].bBold     = 0;
  // m_fontinfo[0].bItalic   = 0;
  // lstrcpy(m_fontinfo[1].szFace, "Times New Roman"); // decorative font
  // m_fontinfo[1].nSize     = 24;
  // m_fontinfo[1].bBold     = 0;
  // m_fontinfo[1].bItalic   = 1;

  // Don't override default FS mode here; shell is now smart and sets it to match
  // the current desktop display mode, by default.

  // m_disp_mode_fs.Width    = 1024;             // normally 640
  // m_disp_mode_fs.Height   = 768;              // normally 480
  //  use either D3DFMT_X8R8G8B8 or D3DFMT_R5G6B5.
  //  The former will match to any 32-bit color format available,
  //  and the latter will match to any 16-bit color available,
  //  if that exact format can't be found.
  // m_disp_mode_fs.Format   = D3DFMT_UNKNOWN; //<- this tells config panel & visualizer to use current display mode as a default!!   //D3DFMT_X8R8G8B8;
  //  m_disp_mode_fs.RefreshRate = 60;
}

//----------------------------------------------------------------------

void CPlugin::MyPreInitialize() {
  // Initialize EVERY data member you've added to CPlugin here;
  //   these will be the default values.
  // If you want to initialize any of your variables with random values
  //   (using rand()), be sure to seed the random number generator first!
  // (If you want to change the default values for settings that are part of
  //   the plugin shell (framework), do so from OverrideDefaults() above.)

  // =========================================================
  // SPOUT initialisation
  //
  spoutDX9 spoutsender;

  // Error logging to AppData
  // EnableSpoutLogFile("SpoutBeatdrop.log");
  // For debugging
  // EnableSpoutLog(); // Shows Spout logs on the console
  // OpenSpoutConsole(); // Empty console

  sprintf(WinampSenderName, "Milkwave");
  bInitialized = false;
  bSpoutOut = true;       // User on/off toggle
  bSpoutChanged = false;  // set to write config on exit
  // DirectX 11 mode uses a format that is incompatible with DirectX 9 receivers
  // DirectX9 mode can fail with some drivers. Noted on Intel/NVIDIA laptop.
  g_Width = 0;
  g_Height = 0;
  g_hwnd = NULL;
  g_hdc = NULL;

  // seed the system's random number generator w/the current system time:
  // srand((unsigned)time(NULL));  -don't - let winamp do it

  // attempt to load a unicode F1 help message otherwise revert to the ansi version
  g_szHelp = (wchar_t*)GetTextResource(IDR_TEXT2, 1);
  if (!g_szHelp)
    g_szHelp = (wchar_t*)GetTextResource(IDR_TEXT1, 0);
  else
    g_szHelp_W = 1;
  g_szHelp_Page2 = (wchar_t*)GetTextResource(IDR_TEXT2_PAGE2, 1);
  if (!g_szHelp_Page2) g_szHelp_Page2 = (wchar_t*)GetTextResource(IDR_TEXT1_PAGE2, 0);

  // CONFIG PANEL SETTINGS THAT WE'VE ADDED (TAB #2)
  m_bFirstRun = true;
  m_bInitialPresetSelected = false;
  m_fBlendTimeUser = 1.7f;
  m_fBlendTimeAuto = 2.7f;
  m_fTimeBetweenPresets = 60.0f;
  m_fTimeBetweenPresetsRand = 10.0f;
  m_bSequentialPresetOrder = true;
  m_bHardCutsDisabled = true;
  m_fHardCutLoudnessThresh = 2.5f;
  m_fHardCutHalflife = 60.0f;
  // m_nWidth			= 1024;
  // m_nHeight			= 768;
  // m_nDispBits		= 16;
  m_nCanvasStretch = 100;
  m_nTexSizeX = -1;  // -1 means "auto"
  m_nTexSizeY = -1;  // -1 means "auto"
  m_nTexBitsPerCh = 8;
  m_nGridX = 64;  // 32;
  m_nGridY = 48;  // 24;

  // m_bShowPressF1ForHelp = true;
  // lstrcpy(m_szMonitorName, "[don't use multimon]");
  m_bShowMenuToolTips = true;  // NOTE: THIS IS CURRENTLY HARDWIRED TO TRUE - NO OPTION TO CHANGE
  m_n16BitGamma = 2;
  m_bAutoGamma = true;
  // m_nFpsLimit			= -1;
  m_bEnableRating = true;
  // m_bInstaScan            = false;
  m_bSongTitleAnims = true;
  m_fSongTitleAnimDuration = 1.7f;
  m_fTimeBetweenRandomSongTitles = -1.0f;
  m_fTimeBetweenRandomCustomMsgs = -1.0f;
  m_nSongTitlesSpawned = 0;
  m_nCustMsgsSpawned = 0;
  m_nFramesSinceResize = 0;

  // Input mixing initial settings
  m_bVideoInputEnabled = false;
  m_nVideoDeviceIndex = 0;
  m_bSpoutInputEnabled = false;
  m_szSpoutSenderName[0] = L'\0';
  m_fInputMixOpacity = 0.5f;
  m_cInputMixTint = D3DCOLOR_XRGB(255, 255, 255);
  m_bInputMixLumaActive = false;
  m_fInputMixLumakeyThreshold = 0.5f;
  m_fInputMixLumakeySoftness = 0.1f;
  m_bInputMixOnTop = true;
  m_pVideoCapture = nullptr;
  m_pVideoCaptureTexture = nullptr;
  m_pSpoutReceiver = nullptr;
  m_pSpoutInputTexture = nullptr;
  m_lpPS_InputMix = nullptr;
  m_nVideoCaptureWidth = 0;
  m_nVideoCaptureHeight = 0;

  // m_bAlways3D		  	    = false;
  // m_fStereoSep            = 1.0f;
  // m_bAlwaysOnTop		= false;
  // m_bFixSlowText          = true;
  // m_bWarningsDisabled     = false;
  m_bWarningsDisabled2 = false;
  // m_bAnisotropicFiltering = true;
  m_bPresetLockOnAtStartup = true;
  m_bPreventScollLockHandling = false;
  m_nMaxPSVersion_ConfigPanel = -1;  // -1 = auto, 0 = disable shaders, 2 = ps_2_0, 3 = ps_3_0
  m_nMaxPSVersion_DX9 = -1;          // 0 = no shader support, 2 = ps_2_0, 3 = ps_3_0
  m_nMaxPSVersion = -1;              // this one will be the ~min of the other two.  0/2/3.
  m_nMaxImages = 2048;
  m_nMaxBytes = 2000000000;

#ifdef _DEBUG
  m_dwShaderFlags = D3DXSHADER_DEBUG | (1 << 16);
#else
  m_dwShaderFlags = (1 << 16);  // D3DXSHADER_SKIPOPTIMIZATION|D3DXSHADER_NO_PRESHADER;
#endif
  // m_pFragmentLinker = NULL;
  // m_pCompiledFragments = NULL;
  m_pShaderCompileErrors = NULL;
  // m_vs_warp = NULL;
  // m_ps_warp = NULL;
  // m_vs_comp = NULL;
  // m_ps_comp = NULL;
  ZeroMemory(&m_shaders, sizeof(PShaderSet));
  ZeroMemory(&m_OldShaders, sizeof(PShaderSet));
  ZeroMemory(&m_NewShaders, sizeof(PShaderSet));
  ZeroMemory(&m_fallbackShaders_vs, sizeof(VShaderSet));
  ZeroMemory(&m_fallbackShaders_ps, sizeof(PShaderSet));
  ZeroMemory(m_BlurShaders, sizeof(m_BlurShaders));
  m_bWarpShaderLock = false;
  m_bCompShaderLock = false;
  m_bNeedRescanTexturesDir = true;

  // vertex declarations:
  m_pSpriteVertDecl = NULL;
  m_pWfVertDecl = NULL;
  m_pMyVertDecl = NULL;

  m_gdi_title_font_doublesize = NULL;
  m_d3dx_title_font_doublesize = NULL;

  // RUNTIME SETTINGS THAT WE'VE ADDED
  m_prev_time = GetTime() - 0.0333f;  // note: this will be updated each frame, at bottom of MyRenderFn.
  m_bTexSizeWasAutoPow2 = false;
  m_bTexSizeWasAutoExact = false;
  // m_bPresetLockedByUser = false;  NOW SET IN DERIVED SETTINGS
  m_bPresetLockedByCode = false;
  m_fStartTime = 0.0f;
  m_fPresetStartTime = 0.0f;
  m_fNextPresetTime = -1.0f;  // negative value means no time set (...it will be auto-set on first call to UpdateTime)
  m_nLoadingPreset = 0;
  m_nPresetsLoadedTotal = 0;
  m_fSnapPoint = 0.5f;
  m_pState = &m_state_DO_NOT_USE[0];
  m_pOldState = &m_state_DO_NOT_USE[1];
  m_pNewState = &m_state_DO_NOT_USE[2];
  m_pMilk2OldState = &m_state_DO_NOT_USE[3];
  ZeroMemory(&m_Milk2OldShaders, sizeof(PShaderSet));
  m_UI_mode = UI_REGULAR;
  m_bShowShaderHelp = false;

  m_nMashSlot = 0;  // 0..MASH_SLOTS-1
  for (int mash = 0; mash < MASH_SLOTS; mash++)
    m_nLastMashChangeFrame[mash] = 0;

  // m_nTrackPlaying	= 0;
  // m_nSongPosMS      = 0;
  // m_nSongLenMS      = 0;
  m_bUserPagedUp = false;
  m_bUserPagedDown = false;
  m_fMotionVectorsTempDx = 0.0f;
  m_fMotionVectorsTempDy = 0.0f;

  m_waitstring.bActive = false;
  m_waitstring.bOvertypeMode = false;
  m_waitstring.szClipboard[0] = 0;

  m_nPresets = 0;
  m_nDirs = 0;
  m_nPresetListCurPos = 0;
  m_nCurrentPreset = -1;
  m_szCurrentPresetFile[0] = 0;
  m_szLoadingPreset[0] = 0;
  // m_szPresetDir[0] = 0; // will be set @ end of this function
  m_bPresetListReady = false;
  m_szUpdatePresetMask[0] = 0;
  // m_nRatingReadProgress = -1;

  myfft.Init(576, MY_FFT_SAMPLES, -1);
  m_fftShader.Init(m_nFFTShaderInput, m_nFFTShaderBins, 0, 1.0f);  // Hann¹ window: tighter main lobe vs Hann³, giving stable per-bin amplitude especially at high frequencies
  memset(&mysound, 0, sizeof(mysound));

  int i;
  for (i = 0; i < PRESET_HIST_LEN; i++)
    m_presetHistory[i] = L"";
  m_presetHistoryPos = 0;
  m_presetHistoryBackFence = 0;
  m_presetHistoryFwdFence = 0;

  // m_nTextHeightPixels = -1;
  // m_nTextHeightPixels_Fancy = -1;
  m_bShowFPS = false;
  m_bShowRating = false;
  m_bShowPresetInfo = false;
  m_bShowDebugInfo = false;
  m_bShowSongTitle = false;
  m_bShowSongTime = false;
  m_bShowSongLen = false;
  m_fShowRatingUntilThisTime = -1.0f;
  ClearErrors();
  m_szDebugMessage[0] = 0;
  m_szSongTitle[0] = 0;
  m_szSongTitlePrev[0] = 0;

  m_lpVS[0] = NULL;
  m_lpVS[1] = NULL;
#if (NUM_BLUR_TEX > 0)
  for (i = 0; i < NUM_BLUR_TEX; i++)
    m_lpBlur[i] = NULL;
#endif

  for (i = 0; i < NUM_SUPERTEXTS; i++) {
    m_lpDDSTitle[i] = NULL;
  }

  m_nTitleTexSizeX = 0;
  m_nTitleTexSizeY = 0;
  m_verts = NULL;
  m_verts_temp = NULL;
  m_vertinfo = NULL;
  m_indices_list = NULL;
  m_indices_strip = NULL;

  m_bMMX = false;
  m_bHasFocus = true;
  m_bHadFocus = false;
  m_bOrigScrollLockState = GetKeyState(VK_SCROLL) & 1;
  // m_bMilkdropScrollLockState is derived at end of MyReadConfig()

  m_nNumericInputMode = NUMERIC_INPUT_MODE_SPRITE;
  m_nNumericInputNum = 0;
  m_nNumericInputDigits = 0;
  // td_custom_msg_font   m_CustomMessageFont[MAX_CUSTOM_MESSAGE_FONTS];
  // td_custom_msg        m_CustomMessage[MAX_CUSTOM_MESSAGES];

  texmgr m_texmgr;  // for user sprites
  KillAllSupertexts();
  // --------------------other init--------------------

  g_bDebugOutput = false;
  g_bDumpFileCleared = false;

  std::wstring::size_type pos = std::wstring(m_szBaseDir).find_last_of(L"\\/");
  std::wstring dir = std::wstring(m_szBaseDir).substr(0, pos);

  swprintf(m_szMilkdrop2Path, L"%s\\%s", dir.c_str(), SUBDIR);
  swprintf(m_szPresetDir, L"%spresets\\", m_szMilkdrop2Path);

  // note that the config dir can be under Program Files or Application Data!!
  wchar_t szConfigDir[MAX_PATH] = {0};
  lstrcpyW(szConfigDir, GetConfigIniFile());
  wchar_t* p = wcsrchr(szConfigDir, L'\\');
  if (p) *(p + 1) = 0;
  swprintf(m_szMsgIniFile, L"%s%s", szConfigDir, MSG_INIFILE);
  swprintf(m_szImgIniFile, L"%s%s", szConfigDir, IMG_INIFILE);
}

//----------------------------------------------------------------------

void CPlugin::MyReadConfig() {
  // Read the user's settings from the .INI file.
  // If you've added any controls to the config panel, read their value in
  //   from the .INI file here.

  // use this function         declared in   to read a value of this type:
  // -----------------         -----------   ----------------------------
  // GetPrivateProfileInt      Win32 API     int
  // GetPrivateProfileBool     utility.h     bool
  // GetPrivateProfileBOOL     utility.h     BOOL
  // GetPrivateProfileFloat    utility.h     float
  // GetPrivateProfileString   Win32 API     string

  // ex: m_fog_enabled = GetPrivateProfileInt("settings","fog_enabled"       ,m_fog_enabled       ,GetConfigIniFile());

  int n = 0;
  wchar_t* pIni = GetConfigIniFile();

  // ======================================
  // SPOUT - save whether in DirectX11 (true) or DirectX 9 (false) mode, default true
  bSpoutOut = GetPrivateProfileBoolW(L"Settings", L"bSpoutOut", bSpoutOut, pIni);
  bQualityAuto = GetPrivateProfileBoolW(L"Settings", L"bQualityAuto", bQualityAuto, pIni);
  bSpoutFixedSize = GetPrivateProfileBoolW(L"Settings", L"bSpoutFixedSize", bSpoutFixedSize, pIni);
  nSpoutFixedWidth = GetPrivateProfileIntW(L"Settings", L"nSpoutFixedWidth", nSpoutFixedWidth, pIni);
  nSpoutFixedHeight = GetPrivateProfileIntW(L"Settings", L"nSpoutFixedHeight", nSpoutFixedHeight, pIni);
  // ======================================
  m_fRenderQuality = GetPrivateProfileFloatW(L"Settings", L"fRenderQuality", m_fRenderQuality, pIni);

  m_bFirstRun = !GetPrivateProfileBoolW(L"Settings", L"bConfigured", false, pIni);
  m_bEnableRating = GetPrivateProfileBoolW(L"Settings", L"bEnableRating", m_bEnableRating, pIni);
  m_bEnableMouseInteraction = GetPrivateProfileBoolW(L"Settings", L"bEnableMouseInteraction", m_bEnableMouseInteraction, pIni);

  // m_bInstaScan    = GetPrivateProfileBool("settings","bInstaScan",m_bInstaScan,pIni);
  m_bHardCutsDisabled = GetPrivateProfileBoolW(L"Settings", L"bHardCutsDisabled", m_bHardCutsDisabled, pIni);
  g_bDebugOutput = GetPrivateProfileBoolW(L"Settings", L"bDebugOutput", g_bDebugOutput, pIni);
  // m_bShowSongInfo = GetPrivateProfileBool("settings","bShowSongInfo",m_bShowSongInfo,pIni);
  // m_bShowPresetInfo=GetPrivateProfileBool("settings","bShowPresetInfo",m_bShowPresetInfo,pIni);
  //  m_bShowPressF1ForHelp = GetPrivateProfileBoolW(L"Settings", L"bShowPressF1ForHelp", m_bShowPressF1ForHelp, pIni);
  // m_bShowMenuToolTips = GetPrivateProfileBool("settings","bShowMenuToolTips",m_bShowMenuToolTips,pIni);
  m_bSongTitleAnims = GetPrivateProfileBoolW(L"Settings", L"bSongTitleAnims", m_bSongTitleAnims, pIni);
  m_bEnablePresetStartup = GetPrivateProfileBoolW(L"Settings", L"bEnablePresetStartup", m_bEnablePresetStartup, pIni);
  m_bEnableAudioCapture = GetPrivateProfileBoolW(L"Settings", L"bEnableAudioCapture", m_bEnableAudioCapture, pIni);
  m_bEnablePresetStartupSavingOnClose = GetPrivateProfileBoolW(L"Settings", L"bEnablePresetStartupSavingOnClose", m_bEnablePresetStartupSavingOnClose, pIni);

  m_bAutoLockPresetWhenNoMusic = GetPrivateProfileBoolW(L"Settings", L"bAutoLockPresetWhenNoMusic", m_bAutoLockPresetWhenNoMusic, pIni);
  m_bScreenDependentRenderMode = GetPrivateProfileBoolW(L"Settings", L"bScreenDependentRenderMode", m_bScreenDependentRenderMode, pIni);

  m_bShowFPS = GetPrivateProfileBoolW(L"Settings", L"bShowFPS", m_bShowFPS, pIni);
  m_bShowRating = GetPrivateProfileBoolW(L"Settings", L"bShowRating", m_bShowRating, pIni);
  m_bShowPresetInfo = GetPrivateProfileBoolW(L"Settings", L"bShowPresetInfo", m_bShowPresetInfo, pIni);
  // m_bShowDebugInfo	= GetPrivateProfileBool("settings","bShowDebugInfo", m_bShowDebugInfo	,pIni);
  m_bShowSongTitle = GetPrivateProfileBoolW(L"Settings", L"bShowSongTitle", m_bShowSongTitle, pIni);
  m_bShowSongTime = GetPrivateProfileBoolW(L"Settings", L"bShowSongTime", m_bShowSongTime, pIni);
  m_bShowSongLen = GetPrivateProfileBoolW(L"Settings", L"bShowSongLen", m_bShowSongLen, pIni);

  // m_bFixPinkBug		= GetPrivateProfileBool("settings","bFixPinkBug",m_bFixPinkBug,pIni);
  int nTemp = GetPrivateProfileBoolW(L"Settings", L"bFixPinkBug", -1, pIni);
  if (nTemp == 0)
    m_n16BitGamma = 0;
  else if (nTemp == 1)
    m_n16BitGamma = 2;
  m_n16BitGamma = GetPrivateProfileIntW(L"Settings", L"n16BitGamma", m_n16BitGamma, pIni);
  m_bAutoGamma = GetPrivateProfileBoolW(L"Settings", L"bAutoGamma", m_bAutoGamma, pIni);
  // m_bAlways3D				= GetPrivateProfileBool("settings","bAlways3D",m_bAlways3D,pIni);
  // m_fStereoSep            = GetPrivateProfileFloat("settings","fStereoSep",m_fStereoSep,pIni);
  // m_bFixSlowText          = GetPrivateProfileBool("settings","bFixSlowText",m_bFixSlowText,pIni);
  // m_bAlwaysOnTop		= GetPrivateProfileBool("settings","bAlwaysOnTop",m_bAlwaysOnTop,pIni);
  // m_bWarningsDisabled		= GetPrivateProfileBool("settings","bWarningsDisabled",m_bWarningsDisabled,pIni);
  m_bWarningsDisabled2 = GetPrivateProfileBoolW(L"Settings", L"bWarningsDisabled2", m_bWarningsDisabled2, pIni);
  // m_bAnisotropicFiltering = GetPrivateProfileBool("settings","bAnisotropicFiltering",m_bAnisotropicFiltering,pIni);
  m_bPresetLockOnAtStartup = GetPrivateProfileBoolW(L"Settings", L"bPresetLockOnAtStartup", m_bPresetLockOnAtStartup, pIni);
  m_bSequentialPresetOrder = GetPrivateProfileBoolW(L"Settings", L"bSequentialPresetOrder", m_bSequentialPresetOrder, pIni);

  m_bPreventScollLockHandling = GetPrivateProfileBoolW(L"Settings", L"m_bPreventScollLockHandling", m_bPreventScollLockHandling, pIni);

  m_nCanvasStretch = 100;  // GetPrivateProfileIntW(L"Settings",L"nCanvasStretch"    ,m_nCanvasStretch,pIni);
  m_nTexSizeX = -1;        // GetPrivateProfileIntW(L"Settings",L"nTexSize"    ,m_nTexSizeX   ,pIni);
  m_nTexSizeY = -1;        // m_nTexSizeX;
  m_bTexSizeWasAutoPow2 = (m_nTexSizeX == -2);
  m_bTexSizeWasAutoExact = (m_nTexSizeX == -1);
  m_nTexBitsPerCh = GetPrivateProfileIntW(L"Settings", L"nTexBitsPerCh", m_nTexBitsPerCh, pIni);
  m_nGridX = GetPrivateProfileIntW(L"Settings", L"nMeshSize", m_nGridX, pIni);
  m_nGridY = m_nGridX * 3 / 4;

  m_nMaxPSVersion_ConfigPanel = GetPrivateProfileIntW(L"Settings", L"MaxPSVersion", m_nMaxPSVersion_ConfigPanel, pIni);
  m_nMaxImages = GetPrivateProfileIntW(L"Settings", L"MaxImages", m_nMaxImages, pIni);
  m_nMaxBytes = GetPrivateProfileIntW(L"Settings", L"MaxBytes", m_nMaxBytes, pIni);
  m_nBassStart = GetPrivateProfileIntW(L"Settings", L"BassStart", m_nBassStart, pIni);
  m_nBassEnd = GetPrivateProfileIntW(L"Settings", L"BassEnd", m_nBassEnd, pIni);
  m_nMidStart = GetPrivateProfileIntW(L"Settings", L"MidStart", m_nMidStart, pIni);
  m_nMidEnd = GetPrivateProfileIntW(L"Settings", L"MidEnd", m_nMidEnd, pIni);
  m_nTrebStart = GetPrivateProfileIntW(L"Settings", L"TrebStart", m_nTrebStart, pIni);
  m_nTrebEnd = GetPrivateProfileIntW(L"Settings", L"TrebEnd", m_nTrebEnd, pIni);

  m_fBlendTimeUser = GetPrivateProfileFloatW(L"Settings", L"fBlendTimeUser", m_fBlendTimeUser, pIni);
  m_fBlendTimeAuto = GetPrivateProfileFloatW(L"Settings", L"fBlendTimeAuto", m_fBlendTimeAuto, pIni);
  m_fTimeBetweenPresets = GetPrivateProfileFloatW(L"Settings", L"fTimeBetweenPresets", m_fTimeBetweenPresets, pIni);
  m_fTimeBetweenPresetsRand = GetPrivateProfileFloatW(L"Settings", L"fTimeBetweenPresetsRand", m_fTimeBetweenPresetsRand, pIni);

  m_fHardCutLoudnessThresh = GetPrivateProfileFloatW(L"Settings", L"fHardCutLoudnessThresh", m_fHardCutLoudnessThresh, pIni);
  m_fHardCutHalflife = GetPrivateProfileFloatW(L"Settings", L"fHardCutHalflife", m_fHardCutHalflife, pIni);
  m_fSongTitleAnimDuration = GetPrivateProfileFloatW(L"Settings", L"fSongTitleAnimDuration", m_fSongTitleAnimDuration, pIni);
  m_fTimeBetweenRandomSongTitles = GetPrivateProfileFloatW(L"Settings", L"fTimeBetweenRandomSongTitles", m_fTimeBetweenRandomSongTitles, pIni);
  m_fTimeBetweenRandomCustomMsgs = GetPrivateProfileFloatW(L"Settings", L"fTimeBetweenRandomCustomMsgs", m_fTimeBetweenRandomCustomMsgs, pIni);
  m_adapterId = GetPrivateProfileIntW(L"Settings", L"nVideoAdapterIndex", 0, pIni);

  // Input Mix Settings (Unified)
  m_fInputMixOpacity = GetPrivateProfileFloatW(L"Milkwave", L"InputMixOpacity", 0.5f, pIni);
  m_fInputMixLumakeyThreshold = GetPrivateProfileFloatW(L"Milkwave", L"InputMixLumakeyThreshold", 0.0f, pIni);
  m_fInputMixLumakeySoftness = GetPrivateProfileFloatW(L"Milkwave", L"InputMixLumakeySoftness", 0.1f, pIni);
  m_bInputMixOnTop = GetPrivateProfileBoolW(L"Milkwave", L"InputMixOnTop", true, pIni);
  m_cInputMixTint = (D3DCOLOR)GetPrivateProfileIntW(L"Milkwave", L"InputMixTint", 0xFFFFFFFF, pIni);

  m_fEQAttackGlobal = GetPrivateProfileFloatW(L"Milkwave", L"EQAttack", GetPrivateProfileFloatW(L"Milkwave", L"FFTAttack", 0.5f, pIni), pIni);
  m_fEQDecayGlobal = GetPrivateProfileFloatW(L"Milkwave", L"EQDecay", GetPrivateProfileFloatW(L"Milkwave", L"FFTDecay", 0.7f, pIni), pIni);
  m_fEQBoostGlobal = GetPrivateProfileFloatW(L"Milkwave", L"EQBoost", GetPrivateProfileFloatW(L"Milkwave", L"FFTBoost", 1.0f, pIni), pIni);

  // Shader FFT window size (must be power of 2, clamped to MIN_FFT_SHADER_INPUT..MAX_FFT_SHADER_INPUT)
  {
    int fftSize = GetPrivateProfileIntW(L"Milkwave", L"FFTSize", DEFAULT_FFT_SHADER_INPUT, pIni);
    if (fftSize < MIN_FFT_SHADER_INPUT) fftSize = MIN_FFT_SHADER_INPUT;
    if (fftSize > MAX_FFT_SHADER_INPUT) fftSize = MAX_FFT_SHADER_INPUT;
    // Round down to nearest power of 2
    int p = 1;
    while (p * 2 <= fftSize) p *= 2;
    m_nFFTShaderInput = p;
    m_nFFTShaderBins = p / 2;
  }

  // --------

  GetPrivateProfileStringW(L"Settings", L"szPresetDir", m_szPresetDir, m_szPresetDir, sizeof(m_szPresetDir), pIni);
  GetPrivateProfileStringW(L"Settings", L"szPresetStartup", m_szPresetStartup, m_szPresetStartup, sizeof(m_szPresetStartup), pIni);

  // Milkwave
  GetPrivateProfileStringW(L"Milkwave", L"AudioDevice", m_szAudioDevice, m_szAudioDevice, sizeof(m_szAudioDevice), pIni);
  m_nAudioDeviceRequestType = GetPrivateProfileIntW(L"Milkwave", L"AudioDeviceRequestType", m_nAudioDeviceRequestType, pIni);
  m_SongInfoPollingEnabled = GetPrivateProfileBoolW(L"Milkwave", L"SongInfoPollingEnabled", m_SongInfoPollingEnabled, pIni);
  const int legacyLyricsEnabled = GetPrivateProfileIntW(L"Milkwave", L"LyricsDisplayEnabled", -1, pIni);
  m_lyricsDisplayEnabled = GetPrivateProfileBoolW(L"Lyrics", L"LyricsEnabled", legacyLyricsEnabled < 0 ? true : legacyLyricsEnabled != 0, pIni);
  m_bLyricsAutoLoad = GetPrivateProfileBoolW(L"Lyrics", L"LyricsAutoLoad", true, pIni);
  ::milkwave.SetLyricsAutoLoad(m_bLyricsAutoLoad);
  m_SongInfoDisplayCorner = GetPrivateProfileIntW(L"Milkwave", L"SongInfoDisplayCorner", m_SongInfoDisplayCorner, pIni);
  GetPrivateProfileStringW(L"Milkwave", L"SongInfoFormat", L"Artist;Title;Album", m_SongInfoFormat, sizeof(m_SongInfoFormat), pIni);
  m_ChangePresetWithSong = GetPrivateProfileBoolW(L"Milkwave", L"ChangePresetWithSong", m_ChangePresetWithSong, pIni);
  m_SongInfoDisplaySeconds = GetPrivateProfileFloatW(L"Milkwave", L"SongInfoDisplaySeconds", m_SongInfoDisplaySeconds, pIni);
  m_DisplayCover = GetPrivateProfileBoolW(L"Milkwave", L"DisplayCover", m_DisplayCover, pIni);
  m_DisplayCoverWhenPressingB = GetPrivateProfileBoolW(L"Milkwave", L"DisplayCoverWhenPressingB", m_DisplayCoverWhenPressingB, pIni);
  m_HideNotificationsWhenRemoteActive = GetPrivateProfileBoolW(L"Milkwave", L"HideNotificationsWhenRemoteActive", m_HideNotificationsWhenRemoteActive, pIni);

  m_ShowTabPreset = GetPrivateProfileBoolW(L"Milkwave", L"ShowTabPreset", m_ShowTabPreset, pIni);
  m_ShowTabMessage = GetPrivateProfileBoolW(L"Milkwave", L"ShowTabMessage", m_ShowTabMessage, pIni);
  m_ShowTabInOut = GetPrivateProfileBoolW(L"Milkwave", L"ShowTabInOut", m_ShowTabInOut, pIni);
  m_ShowTabSettings = GetPrivateProfileBoolW(L"Milkwave", L"ShowTabSettings", m_ShowTabSettings, pIni);
  m_ShowTabLyrics = GetPrivateProfileBoolW(L"Milkwave", L"ShowTabLyrics", m_ShowTabLyrics, pIni);
  m_ShowTabFonts = GetPrivateProfileBoolW(L"Milkwave", L"ShowTabFonts", m_ShowTabFonts, pIni);
  m_ShowTabMidi = GetPrivateProfileBoolW(L"Milkwave", L"ShowTabMidi", m_ShowTabMidi, pIni);
  m_ShowTabWave = GetPrivateProfileBoolW(L"Milkwave", L"ShowTabWave", m_ShowTabWave, pIni);
  m_ShowTabShader = GetPrivateProfileBoolW(L"Milkwave", L"ShowTabShader", m_ShowTabShader, pIni);

  m_lyricsBurnTime = GetPrivateProfileFloatW(L"Lyrics", L"LyricsBurnTime", m_lyricsBurnTime, pIni);
  m_lyricsBurnType = GetPrivateProfileIntW(L"Lyrics", L"LyricsBurnType", m_lyricsBurnType, pIni);
  if (m_lyricsBurnType < 0) m_lyricsBurnType = 0;
  if (m_lyricsBurnType > 3) m_lyricsBurnType = 3;
  m_lyricsAutoScale = GetPrivateProfileBoolW(L"Lyrics", L"LyricsAutoScale", m_lyricsAutoScale, pIni);
  m_lyricsPositionX = GetPrivateProfileFloatW(L"Lyrics", L"LyricsPositionX", m_lyricsPositionX, pIni);
  m_lyricsPositionY = GetPrivateProfileFloatW(L"Lyrics", L"LyricsPositionY", m_lyricsPositionY, pIni);
  m_lyricsStartX = GetPrivateProfileFloatW(L"Lyrics", L"LyricsStartX", m_lyricsStartX, pIni);
  m_lyricsStartY = GetPrivateProfileFloatW(L"Lyrics", L"LyricsStartY", m_lyricsStartY, pIni);
  m_lyricsZoom = GetPrivateProfileFloatW(L"Lyrics", L"LyricsZoom", m_lyricsZoom, pIni);
  m_lyricsMaxWidth = GetPrivateProfileFloatW(L"Lyrics", L"LyricsMaxWidth", m_lyricsMaxWidth, pIni);
  GetPrivateProfileStringW(L"Lyrics", L"LyricsFont", m_lyricsFont, m_lyricsFont, _countof(m_lyricsFont), pIni);
  m_lyricsFontSize = GetPrivateProfileIntW(L"Lyrics", L"LyricsFontSize", m_lyricsFontSize, pIni);
  m_lyricsFontBold = GetPrivateProfileBoolW(L"Lyrics", L"LyricsFontBold", m_lyricsFontBold, pIni);
  m_lyricsFontItalic = GetPrivateProfileBoolW(L"Lyrics", L"LyricsFontItalic", m_lyricsFontItalic, pIni);
  m_lyricsFontAA = GetPrivateProfileBoolW(L"Lyrics", L"LyricsFontAA", m_lyricsFontAA, pIni);
  m_lyricsColorR = GetPrivateProfileIntW(L"Lyrics", L"LyricsColorR", m_lyricsColorR, pIni);
  m_lyricsColorG = GetPrivateProfileIntW(L"Lyrics", L"LyricsColorG", m_lyricsColorG, pIni);
  m_lyricsColorB = GetPrivateProfileIntW(L"Lyrics", L"LyricsColorB", m_lyricsColorB, pIni);
  m_lyricsShadow = GetPrivateProfileIntW(L"Lyrics", L"LyricsShadow", m_lyricsShadow, pIni);
  // LyricsOffsetSeconds is stored in seconds (float); internally the offset is kept in milliseconds.
  m_lyricsOffsetMs = static_cast<std::int64_t>(GetPrivateProfileFloatW(L"Lyrics", L"LyricsOffsetSeconds", (float)m_lyricsOffsetMs / 1000.0f, pIni) * 1000.0f);
  m_lyricsFade = GetPrivateProfileFloatW(L"Lyrics", L"LyricsFade", m_lyricsFade, pIni);
  if (m_lyricsFade < 0.0f) m_lyricsFade = 0.0f;
  GetPrivateProfileStringW(L"Lyrics", L"LyricsApiUrl", m_lyricsApiUrl, m_lyricsApiUrl, _countof(m_lyricsApiUrl), pIni);
  if (m_lyricsApiUrl[0] == L'\0') lstrcpyW(m_lyricsApiUrl, kDefaultLyricsApiUrl);
  ::milkwave.SetLyricsApiUrl(m_lyricsApiUrl);

  m_ShowLockSymbol = GetPrivateProfileBoolW(L"Milkwave", L"ShowLockSymbol", m_ShowLockSymbol, pIni);
  m_ShaderCaching = GetPrivateProfileBoolW(L"Milkwave", L"ShaderCaching", m_ShaderCaching, pIni);
  m_ShaderPrecompileOnStartup = GetPrivateProfileBoolW(L"Milkwave", L"ShaderPrecompileOnStartup", m_ShaderPrecompileOnStartup, pIni);
  m_CheckDirectXOnStartup = GetPrivateProfileBoolW(L"Milkwave", L"CheckDirectXOnStartup", m_CheckDirectXOnStartup, pIni);
  m_LogLevel = GetPrivateProfileIntW(L"Milkwave", L"LogLevel", m_LogLevel, pIni);

  m_blackmode = GetPrivateProfileBoolW(L"Milkwave", L"BlackMode", m_blackmode, pIni);
  m_AMDDetectionMode = GetPrivateProfileIntW(L"Milkwave", L"AMDDetectionMode", m_AMDDetectionMode, pIni);

  m_MessageDefaultBurnTime = GetPrivateProfileFloatW(L"Milkwave", L"MessageDefaultBurnTime", m_MessageDefaultBurnTime, pIni);
  m_MessageDefaultFadeinTime = GetPrivateProfileFloatW(L"Milkwave", L"MessageDefaultFadeinTime", m_MessageDefaultFadeinTime, pIni);
  m_MessageDefaultFadeoutTime = GetPrivateProfileFloatW(L"Milkwave", L"MessageDefaultFadeoutTime", m_MessageDefaultFadeoutTime, pIni);

  // We'll put these in the settings section since other MilkDrop forks use similar settings
  m_MinPSVersionConfig = GetPrivateProfileIntW(L"Settings", L"MinPSVersion", m_MinPSVersionConfig, pIni);
  if (m_MinPSVersionConfig < 0) m_MinPSVersionConfig = 2;
  m_MaxPSVersionConfig = GetPrivateProfileIntW(L"Settings", L"MaxPSVersion", m_MaxPSVersionConfig, pIni);
  if (m_MaxPSVersionConfig < 0) m_MaxPSVersionConfig = 4;
  m_nMixType = GetPrivateProfileIntW(L"Settings", L"Mixtype", m_nMixType, pIni);

  m_ShowUpArrowInDescriptionIfPSMinVersionForced = GetPrivateProfileBoolW(L"Milkwave", L"ShowUpArrowInDescriptionIfPSMinVersionForced", m_ShowUpArrowInDescriptionIfPSMinVersionForced, pIni);

  // Network (TCP server)
  m_TcpEnabled = GetPrivateProfileBoolW(L"Network", L"TcpEnabled", m_TcpEnabled, pIni);
  m_TcpPort = GetPrivateProfileIntW(L"Network", L"TcpPort", m_TcpPort, pIni);
  GetPrivateProfileStringW(L"Network", L"TcpPIN", L"", m_szTcpPIN, _countof(m_szTcpPIN), pIni);
  GetPrivateProfileStringW(L"Network", L"PinHash", L"", m_szTcpPinHash, _countof(m_szTcpPinHash), pIni);

  m_WindowBorderless = GetPrivateProfileBoolW(L"Milkwave", L"WindowBorderless", m_WindowBorderless, pIni);
  m_bAlwaysOnTop = GetPrivateProfileBoolW(L"Milkwave", L"WindowAlwaysOnTop", m_bAlwaysOnTop, pIni);

  fOpacity = GetPrivateProfileFloatW(L"Milkwave", L"WindowOpacity", fOpacity, pIni);
  m_WindowWatermarkModeOpacity = GetPrivateProfileFloatW(L"Milkwave", L"WindowWatermarkModeOpacity", m_WindowWatermarkModeOpacity, pIni);
  m_WindowX = GetPrivateProfileIntW(L"Milkwave", L"WindowX", m_WindowX, pIni);
  m_WindowY = GetPrivateProfileIntW(L"Milkwave", L"WindowY", m_WindowY, pIni);
  m_WindowWidth = GetPrivateProfileIntW(L"Milkwave", L"WindowWidth", m_WindowWidth, pIni);
  m_WindowHeight = GetPrivateProfileIntW(L"Milkwave", L"WindowHeight", m_WindowHeight, pIni);
  m_WindowFixedWidth = GetPrivateProfileIntW(L"Milkwave", L"WindowFixedWidth", m_WindowFixedWidth, pIni);
  m_WindowFixedHeight = GetPrivateProfileIntW(L"Milkwave", L"WindowFixedHeight", m_WindowFixedHeight, pIni);

  ReadCustomMessages();

  // bounds-checking:
  if (m_nGridX > MAX_GRID_X)
    m_nGridX = MAX_GRID_X;
  if (m_nGridY > MAX_GRID_Y)
    m_nGridY = MAX_GRID_Y;
  if (m_fTimeBetweenPresetsRand < 0)
    m_fTimeBetweenPresetsRand = 0;
  if (m_fTimeBetweenPresets < 0.1f)
    m_fTimeBetweenPresets = 0.1f;

  // DERIVED SETTINGS
  m_bPresetLockedByUser = m_bPresetLockOnAtStartup;
  // m_bMilkdropScrollLockState = m_bPresetLockOnAtStartup;

  // Re-init shader FFT with configured size (MyPreInitialize used defaults)
  m_fftShader.Init(m_nFFTShaderInput, m_nFFTShaderBins, 0, 1.0f);
}

//----------------------------------------------------------------------

void CPlugin::MyWriteConfig() {
  // Write the user's settings to the .INI file.
  // This gets called only when the user runs the config panel and hits OK.
  // If you've added any controls to the config panel, write their value out
  //   to the .INI file here.

  // use this function         declared in   to write a value of this type:
  // -----------------         -----------   ----------------------------
  // WritePrivateProfileInt    Win32 API     int
  // WritePrivateProfileInt    utility.h     bool
  // WritePrivateProfileInt    utility.h     BOOL
  // WritePrivateProfileFloat  utility.h     float
  // WritePrivateProfileString Win32 API     string

  // ex: WritePrivateProfileInt(m_fog_enabled       ,"fog_enabled"       ,GetConfigIniFile(),"settings");

  wchar_t* pIni = GetConfigIniFile();

  // constants:
  WritePrivateProfileStringW(L"Settings", L"bConfigured", L"1", pIni);

  // note: m_szPresetDir is not written here; it is written manually, whenever it changes.

  wchar_t szSectionName[] = L"Settings";

  // ================================
  // SPOUT
  WritePrivateProfileIntW(bSpoutOut, L"bSpoutOut", pIni, L"Settings");
  WritePrivateProfileIntW(bQualityAuto, L"bQualityAuto", pIni, L"Settings");
  WritePrivateProfileIntW(bSpoutFixedSize, L"bSpoutFixedSize", pIni, L"Settings");
  WritePrivateProfileIntW(nSpoutFixedWidth, L"nSpoutFixedWidth", pIni, L"Settings");
  WritePrivateProfileIntW(nSpoutFixedHeight, L"nSpoutFixedHeight", pIni, L"Settings");
  // ================================
  WritePrivateProfileFloatW(m_fRenderQuality, L"fRenderQuality", pIni, L"Settings");

  WritePrivateProfileIntW(m_bSongTitleAnims, L"bSongTitleAnims", pIni, L"Settings");
  WritePrivateProfileIntW(m_bHardCutsDisabled, L"bHardCutsDisabled", pIni, L"Settings");
  WritePrivateProfileIntW(m_bEnableRating, L"bEnableRating", pIni, L"Settings");
  WritePrivateProfileIntW(m_bEnableMouseInteraction, L"bEnableMouseInteraction", pIni, L"Settings");

  // WritePrivateProfileIntW(m_bInstaScan,            "bInstaScan",		    pIni, "settings");
  WritePrivateProfileIntW(g_bDebugOutput, L"bDebugOutput", pIni, L"Settings");

  // itePrivateProfileInt(m_bShowPresetInfo, 	    "bShowPresetInfo",		pIni, "settings");
  // itePrivateProfileInt(m_bShowSongInfo, 		"bShowSongInfo",        pIni, "settings");
  // itePrivateProfileInt(m_bFixPinkBug, 		    "bFixPinkBug",			pIni, "settings");

  // WritePrivateProfileIntW(m_bShowPressF1ForHelp, L"bShowPressF1ForHelp", pIni, L"Settings");
  // itePrivateProfileInt(m_bShowMenuToolTips, 	"bShowMenuToolTips",    pIni, "settings");
  WritePrivateProfileIntW(m_n16BitGamma, L"n16BitGamma", pIni, L"Settings");
  WritePrivateProfileIntW(m_bAutoGamma, L"bAutoGamma", pIni, L"Settings");

  // WritePrivateProfileIntW(m_bAlways3D, 			"bAlways3D",			pIni, "settings");
  // WritePrivateProfileFloat(m_fStereoSep,          "fStereoSep",           pIni, "settings");
  // WritePrivateProfileIntW(m_bFixSlowText,		    "bFixSlowText",			pIni, "settings");
  // itePrivateProfileInt(m_bAlwaysOnTop,		    "bAlwaysOnTop",			pIni, "settings");
  // WritePrivateProfileIntW(m_bWarningsDisabled,	    "bWarningsDisabled",	pIni, "settings");
  WritePrivateProfileIntW(m_bWarningsDisabled2, L"bWarningsDisabled2", pIni, L"Settings");
  // WritePrivateProfileIntW(m_bAnisotropicFiltering,	"bAnisotropicFiltering",pIni, "settings");
  WritePrivateProfileIntW(m_bPresetLockOnAtStartup, L"bPresetLockOnAtStartup", pIni, L"Settings");
  WritePrivateProfileIntW(m_bSequentialPresetOrder, L"bSequentialPresetOrder", pIni, L"Settings");

  WritePrivateProfileIntW(m_bPreventScollLockHandling, L"m_bPreventScollLockHandling", pIni, L"Settings");
  // note: this is also written @ exit of the visualizer
  WritePrivateProfileIntW(m_bEnablePresetStartup, L"bEnablePresetStartup", pIni, L"Settings");
  WritePrivateProfileIntW(m_bAutoLockPresetWhenNoMusic, L"bAutoLockPresetWhenNoMusic", pIni, L"Settings");
  WritePrivateProfileIntW(m_bScreenDependentRenderMode, L"bScreenDependentRenderMode", pIni, L"Settings");

  WritePrivateProfileIntW(m_nCanvasStretch, L"nCanvasStretch", pIni, L"Settings");
  // WritePrivateProfileIntW(m_nTexSizeX,			    L"nTexSize",				pIni, L"Settings");
  WritePrivateProfileIntW(m_nTexBitsPerCh, L"nTexBitsPerCh", pIni, L"Settings");
  WritePrivateProfileIntW(m_nGridX, L"nMeshSize", pIni, L"Settings");
  WritePrivateProfileIntW(m_nMaxPSVersion_ConfigPanel, L"MaxPSVersion", pIni, L"Settings");
  WritePrivateProfileIntW(m_nMaxImages, L"MaxImages", pIni, L"Settings");
  WritePrivateProfileIntW(m_nMaxBytes, L"MaxBytes", pIni, L"Settings");
  WritePrivateProfileIntW(m_nBassStart, L"BassStart", pIni, L"Settings");
  WritePrivateProfileIntW(m_nBassEnd, L"BassEnd", pIni, L"Settings");
  WritePrivateProfileIntW(m_nMidStart, L"MidStart", pIni, L"Settings");
  WritePrivateProfileIntW(m_nMidEnd, L"MidEnd", pIni, L"Settings");
  WritePrivateProfileIntW(m_nTrebStart, L"TrebStart", pIni, L"Settings");
  WritePrivateProfileIntW(m_nTrebEnd, L"TrebEnd", pIni, L"Settings");

  WritePrivateProfileFloatW(m_fBlendTimeAuto, L"fBlendTimeAuto", pIni, L"Settings");
  WritePrivateProfileFloatW(m_fBlendTimeUser, L"fBlendTimeUser", pIni, L"Settings");
  WritePrivateProfileFloatW(m_fTimeBetweenPresets, L"fTimeBetweenPresets", pIni, L"Settings");
  WritePrivateProfileFloatW(m_fTimeBetweenPresetsRand, L"fTimeBetweenPresetsRand", pIni, L"Settings");
  WritePrivateProfileFloatW(m_fHardCutLoudnessThresh, L"fHardCutLoudnessThresh", pIni, L"Settings");
  WritePrivateProfileFloatW(m_fHardCutHalflife, L"fHardCutHalflife", pIni, L"Settings");
  WritePrivateProfileFloatW(m_fSongTitleAnimDuration, L"fSongTitleAnimDuration", pIni, L"Settings");
  WritePrivateProfileFloatW(m_fTimeBetweenRandomSongTitles, L"fTimeBetweenRandomSongTitles", pIni, L"Settings");
  WritePrivateProfileFloatW(m_fTimeBetweenRandomCustomMsgs, L"fTimeBetweenRandomCustomMsgs", pIni, L"Settings");

  WritePrivateProfileIntW(m_adapterId, L"nVideoAdapterIndex", pIni, L"Settings");
  WritePrivateProfileIntW(m_bPresetLockedByUser, L"bPresetLockOnAtStartup", GetConfigIniFile(), L"Settings");
  if (m_bEnablePresetStartupSavingOnClose) {
    WritePrivateProfileStringW(L"Settings", L"szPresetStartup", m_szCurrentPresetFile, pIni);
  }
  WritePrivateProfileIntW(m_bShowFPS, L"bShowFPS", GetConfigIniFile(), L"Settings");
  WritePrivateProfileIntW(m_bShowRating, L"bShowRating", GetConfigIniFile(), L"Settings");
  WritePrivateProfileIntW(m_bShowPresetInfo, L"bShowPresetInfo", GetConfigIniFile(), L"Settings");
  WritePrivateProfileIntW(m_show_press_f1_msg, L"show_press_f1_msg", GetConfigIniFile(), L"Settings");

  // Milkwave
  WritePrivateProfileStringW(L"Milkwave", L"AudioDevice", m_szAudioDevice, pIni);
  WritePrivateProfileIntW(m_nAudioDeviceRequestType, L"AudioDeviceRequestType", pIni, L"Milkwave");
  WritePrivateProfileIntW(m_SongInfoPollingEnabled, L"SongInfoPollingEnabled", pIni, L"Milkwave");
  WritePrivateProfileIntW(m_SongInfoDisplayCorner, L"SongInfoDisplayCorner", pIni, L"Milkwave");
  WritePrivateProfileIntW(m_ChangePresetWithSong, L"ChangePresetWithSong", pIni, L"Milkwave");
  WritePrivateProfileIntW(m_DisplayCover, L"DisplayCover", pIni, L"Milkwave");
  // WritePrivateProfileIntW(m_DisplayCoverWhenPressingB, L"mDisplayCoverWhenPressingB", pIni, L"Milkwave");
  WritePrivateProfileIntW(m_ShowTabPreset, L"ShowTabPreset", pIni, L"Milkwave");
  WritePrivateProfileIntW(m_ShowTabMessage, L"ShowTabMessage", pIni, L"Milkwave");
  WritePrivateProfileIntW(m_ShowTabInOut, L"ShowTabInOut", pIni, L"Milkwave");
  WritePrivateProfileIntW(m_ShowTabSettings, L"ShowTabSettings", pIni, L"Milkwave");
  WritePrivateProfileIntW(m_ShowTabLyrics, L"ShowTabLyrics", pIni, L"Milkwave");
  WritePrivateProfileIntW(m_ShowTabFonts, L"ShowTabFonts", pIni, L"Milkwave");
  WritePrivateProfileIntW(m_ShowTabMidi, L"ShowTabMidi", pIni, L"Milkwave");
  WritePrivateProfileIntW(m_ShowTabWave, L"ShowTabWave", pIni, L"Milkwave");
  WritePrivateProfileIntW(m_ShowTabShader, L"ShowTabShader", pIni, L"Milkwave");
  WritePrivateProfileIntW(m_blackmode, L"BlackMode", pIni, L"Milkwave");
  WritePrivateProfileIntW(m_CheckDirectXOnStartup, L"CheckDirectXOnStartup", pIni, L"Milkwave");

  WritePrivateProfileIntW(m_lyricsDisplayEnabled, L"LyricsEnabled", pIni, L"Lyrics");
  WritePrivateProfileIntW(m_bLyricsAutoLoad, L"LyricsAutoLoad", pIni, L"Lyrics");
  WritePrivateProfileIntW(m_lyricsAutoScale, L"LyricsAutoScale", pIni, L"Lyrics");
  WritePrivateProfileFloatW(m_lyricsBurnTime, L"LyricsBurnTime", pIni, L"Lyrics");
  WritePrivateProfileIntW(m_lyricsBurnType, L"LyricsBurnType", pIni, L"Lyrics");
  WritePrivateProfileFloatW(m_lyricsPositionX, L"LyricsPositionX", pIni, L"Lyrics");
  WritePrivateProfileFloatW(m_lyricsPositionY, L"LyricsPositionY", pIni, L"Lyrics");
  WritePrivateProfileFloatW(m_lyricsStartX, L"LyricsStartX", pIni, L"Lyrics");
  WritePrivateProfileFloatW(m_lyricsStartY, L"LyricsStartY", pIni, L"Lyrics");
  WritePrivateProfileFloatW(m_lyricsZoom, L"LyricsZoom", pIni, L"Lyrics");
  WritePrivateProfileFloatW(m_lyricsMaxWidth, L"LyricsMaxWidth", pIni, L"Lyrics");
  WritePrivateProfileStringW(L"Lyrics", L"LyricsFont", m_lyricsFont, pIni);
  WritePrivateProfileIntW(m_lyricsFontSize, L"LyricsFontSize", pIni, L"Lyrics");
  WritePrivateProfileIntW(m_lyricsFontBold, L"LyricsFontBold", pIni, L"Lyrics");
  WritePrivateProfileIntW(m_lyricsFontItalic, L"LyricsFontItalic", pIni, L"Lyrics");
  WritePrivateProfileIntW(m_lyricsFontAA, L"LyricsFontAA", pIni, L"Lyrics");
  WritePrivateProfileIntW(m_lyricsColorR, L"LyricsColorR", pIni, L"Lyrics");
  WritePrivateProfileIntW(m_lyricsColorG, L"LyricsColorG", pIni, L"Lyrics");
  WritePrivateProfileIntW(m_lyricsColorB, L"LyricsColorB", pIni, L"Lyrics");
  WritePrivateProfileIntW(m_lyricsShadow, L"LyricsShadow", pIni, L"Lyrics");
  WritePrivateProfileFloatW((float)m_lyricsOffsetMs / 1000.0f, L"LyricsOffsetSeconds", pIni, L"Lyrics");
  WritePrivateProfileFloatW(m_lyricsFade, L"LyricsFade", pIni, L"Lyrics");
  WritePrivateProfileStringW(L"Lyrics", L"LyricsApiUrl", m_lyricsApiUrl, pIni);

  WritePrivateProfileIntW(m_WindowBorderless, L"WindowBorderless", pIni, L"Milkwave");
  WritePrivateProfileIntW(m_bAlwaysOnTop, L"WindowAlwaysOnTop", pIni, L"Milkwave");

  WritePrivateProfileFloatW(m_WindowWatermarkModeOpacity, L"WindowWatermarkModeOpacity", pIni, L"Milkwave");
  WritePrivateProfileFloatW(fOpacity, L"WindowOpacity", pIni, L"Milkwave");
  WritePrivateProfileIntW(m_WindowX, L"WindowX", pIni, L"Milkwave");
  WritePrivateProfileIntW(m_WindowY, L"WindowY", pIni, L"Milkwave");
  WritePrivateProfileIntW(m_WindowWidth, L"WindowWidth", pIni, L"Milkwave");
  WritePrivateProfileIntW(m_WindowHeight, L"WindowHeight", pIni, L"Milkwave");

  WritePrivateProfileFloatW(m_fInputMixOpacity, L"InputMixOpacity", pIni, L"Milkwave");
  WritePrivateProfileFloatW(m_fInputMixLumakeyThreshold, L"InputMixLumakeyThreshold", pIni, L"Milkwave");
  WritePrivateProfileFloatW(m_fInputMixLumakeySoftness, L"InputMixLumakeySoftness", pIni, L"Milkwave");
  WritePrivateProfileIntW(m_bInputMixOnTop, L"InputMixOnTop", pIni, L"Milkwave");
  WritePrivateProfileIntW((int)m_cInputMixTint, L"InputMixTint", pIni, L"Milkwave");

  WritePrivateProfileFloatW(m_fEQAttackGlobal, L"EQAttack", pIni, L"Milkwave");
  WritePrivateProfileFloatW(m_fEQDecayGlobal, L"EQDecay", pIni, L"Milkwave");
  WritePrivateProfileFloatW(m_fEQBoostGlobal, L"EQBoost", pIni, L"Milkwave");
  WritePrivateProfileStringW(L"Milkwave", L"FFTAttack", nullptr, pIni);
  WritePrivateProfileStringW(L"Milkwave", L"FFTDecay", nullptr, pIni);
  WritePrivateProfileStringW(L"Milkwave", L"FFTBoost", nullptr, pIni);

  // Network (TCP server)
  WritePrivateProfileIntW(m_TcpEnabled, L"TcpEnabled", pIni, L"Network");
  WritePrivateProfileIntW(m_TcpPort, L"TcpPort", pIni, L"Network");

  ::milkwave.LogInfo(L"Configuration written: " + std::wstring(pIni));
}

void CPlugin::SaveWindowSizeAndPosition(HWND hwnd) {
  RECT rect;
  if (GetWindowRect(hwnd, &rect)) {
    m_WindowX = rect.left;
    m_WindowY = rect.top;
    m_WindowWidth = rect.right - rect.left;
    m_WindowHeight = rect.bottom - rect.top;
  }
}

//----------------------------------------------------------------------

void CPlugin::Randomize() {
  srand((int)(GetTime() * 100));
  // m_fAnimTime		= (rand() % 51234L)*0.01f;
  m_fRandStart[0] = (rand() % 64841L) * 0.01f;
  m_fRandStart[1] = (rand() % 53751L) * 0.01f;
  m_fRandStart[2] = (rand() % 42661L) * 0.01f;
  m_fRandStart[3] = (rand() % 31571L) * 0.01f;

  // CState temp;
  // temp.Randomize(rand() % NUM_MODES);
  // m_pState->StartBlend(&temp, m_fAnimTime, m_fBlendTimeUser);
}

//----------------------------------------------------------------------

void CPlugin::SetMenusForPresetVersion(int WarpPSVersion, int CompPSVersion) {
  int MaxPSVersion = max(WarpPSVersion, CompPSVersion);

  m_menuPreset.EnableItem(wasabiApiLangString(IDS_MENU_EDIT_WARP_SHADER), WarpPSVersion > 0);
  m_menuPreset.EnableItem(wasabiApiLangString(IDS_MENU_EDIT_COMPOSITE_SHADER), CompPSVersion > 0);
  m_menuPost.EnableItem(wasabiApiLangString(IDS_MENU_SUSTAIN_LEVEL), WarpPSVersion == 0);
  m_menuPost.EnableItem(wasabiApiLangString(IDS_MENU_TEXTURE_WRAP), WarpPSVersion == 0);
  m_menuPost.EnableItem(wasabiApiLangString(IDS_MENU_GAMMA_ADJUSTMENT), CompPSVersion == 0);
  m_menuPost.EnableItem(wasabiApiLangString(IDS_MENU_HUE_SHADER), CompPSVersion == 0);
  m_menuPost.EnableItem(wasabiApiLangString(IDS_MENU_VIDEO_ECHO_ALPHA), CompPSVersion == 0);
  m_menuPost.EnableItem(wasabiApiLangString(IDS_MENU_VIDEO_ECHO_ZOOM), CompPSVersion == 0);
  m_menuPost.EnableItem(wasabiApiLangString(IDS_MENU_VIDEO_ECHO_ORIENTATION), CompPSVersion == 0);
  m_menuPost.EnableItem(wasabiApiLangString(IDS_MENU_FILTER_INVERT), CompPSVersion == 0);
  m_menuPost.EnableItem(wasabiApiLangString(IDS_MENU_FILTER_BRIGHTEN), CompPSVersion == 0);
  m_menuPost.EnableItem(wasabiApiLangString(IDS_MENU_FILTER_DARKEN), CompPSVersion == 0);
  m_menuPost.EnableItem(wasabiApiLangString(IDS_MENU_FILTER_SOLARIZE), CompPSVersion == 0);
  m_menuPost.EnableItem(wasabiApiLangString(IDS_MENU_BLUR1_EDGE_DARKEN_AMOUNT), MaxPSVersion > 0);
  m_menuPost.EnableItem(wasabiApiLangString(IDS_MENU_BLUR1_MIN_COLOR_VALUE), MaxPSVersion > 0);
  m_menuPost.EnableItem(wasabiApiLangString(IDS_MENU_BLUR1_MAX_COLOR_VALUE), MaxPSVersion > 0);
  m_menuPost.EnableItem(wasabiApiLangString(IDS_MENU_BLUR2_MIN_COLOR_VALUE), MaxPSVersion > 0);
  m_menuPost.EnableItem(wasabiApiLangString(IDS_MENU_BLUR2_MAX_COLOR_VALUE), MaxPSVersion > 0);
  m_menuPost.EnableItem(wasabiApiLangString(IDS_MENU_BLUR3_MIN_COLOR_VALUE), MaxPSVersion > 0);
  m_menuPost.EnableItem(wasabiApiLangString(IDS_MENU_BLUR3_MAX_COLOR_VALUE), MaxPSVersion > 0);
}

void CPlugin::BuildMenus() {
  wchar_t buf[1024];

  m_pCurMenu = &m_menuPreset;  //&m_menuMain;

  m_menuPreset.Init(wasabiApiLangString(IDS_EDIT_CURRENT_PRESET));
  m_menuMotion.Init(wasabiApiLangString(IDS_MOTION));
  m_menuCustomShape.Init(wasabiApiLangString(IDS_DRAWING_CUSTOM_SHAPES));
  m_menuCustomWave.Init(wasabiApiLangString(IDS_DRAWING_CUSTOM_WAVES));
  m_menuWave.Init(wasabiApiLangString(IDS_DRAWING_SIMPLE_WAVEFORM));
  m_menuAugment.Init(wasabiApiLangString(IDS_DRAWING_BORDERS_MOTION_VECTORS));
  m_menuPost.Init(wasabiApiLangString(IDS_POST_PROCESSING_MISC));
  int i;
  for (i = 0; i < MAX_CUSTOM_WAVES; i++) {
    swprintf(buf, wasabiApiLangString(IDS_CUSTOM_WAVE_X), i + 1);
    m_menuWavecode[i].Init(buf);
  }
  for (i = 0; i < MAX_CUSTOM_SHAPES; i++) {
    swprintf(buf, wasabiApiLangString(IDS_CUSTOM_SHAPE_X), i + 1);
    m_menuShapecode[i].Init(buf);
  }

  //-------------------------------------------

  // MAIN MENU / menu hierarchy

  m_menuPreset.AddChildMenu(&m_menuMotion);
  m_menuPreset.AddChildMenu(&m_menuCustomShape);
  m_menuPreset.AddChildMenu(&m_menuCustomWave);
  m_menuPreset.AddChildMenu(&m_menuWave);
  m_menuPreset.AddChildMenu(&m_menuAugment);
  m_menuPreset.AddChildMenu(&m_menuPost);

  for (i = 0; i < MAX_CUSTOM_SHAPES; i++)
    m_menuCustomShape.AddChildMenu(&m_menuShapecode[i]);
  for (i = 0; i < MAX_CUSTOM_WAVES; i++)
    m_menuCustomWave.AddChildMenu(&m_menuWavecode[i]);

  // NOTE: all of the eval menuitems use a CALLBACK function to register the user's changes (see last param)
  m_menuPreset.AddItem(wasabiApiLangString(IDS_MENU_EDIT_PRESET_INIT_CODE),
                       &m_pState->m_szPerFrameInit, MENUITEMTYPE_STRING,
                       wasabiApiLangString(IDS_MENU_EDIT_PRESET_INIT_CODE_TT, buf, 1024),
                       256, 0, &OnUserEditedPresetInit, sizeof(m_pState->m_szPerFrameInit), 0);

  m_menuPreset.AddItem(wasabiApiLangString(IDS_MENU_EDIT_PER_FRAME_EQUATIONS),
                       &m_pState->m_szPerFrameExpr, MENUITEMTYPE_STRING,
                       wasabiApiLangString(IDS_MENU_EDIT_PER_FRAME_EQUATIONS_TT, buf, 1024),
                       256, 0, &OnUserEditedPerFrame, sizeof(m_pState->m_szPerFrameExpr), 0);

  m_menuPreset.AddItem(wasabiApiLangString(IDS_MENU_EDIT_PER_VERTEX_EQUATIONS),
                       &m_pState->m_szPerPixelExpr, MENUITEMTYPE_STRING,
                       wasabiApiLangString(IDS_MENU_EDIT_PER_VERTEX_EQUATIONS_TT, buf, 1024),
                       256, 0, &OnUserEditedPerPixel, sizeof(m_pState->m_szPerPixelExpr), 0);

  m_menuPreset.AddItem(wasabiApiLangString(IDS_MENU_EDIT_WARP_SHADER),
                       &m_pState->m_szWarpShadersText, MENUITEMTYPE_STRING,
                       wasabiApiLangString(IDS_MENU_EDIT_WARP_SHADER_TT, buf, 1024),
                       256, 0, &OnUserEditedWarpShaders, sizeof(m_pState->m_szWarpShadersText), 0);

  m_menuPreset.AddItem(wasabiApiLangString(IDS_MENU_EDIT_COMPOSITE_SHADER),
                       &m_pState->m_szCompShadersText, MENUITEMTYPE_STRING,
                       wasabiApiLangString(IDS_MENU_EDIT_COMPOSITE_SHADER_TT, buf, 1024),
                       256, 0, &OnUserEditedCompShaders, sizeof(m_pState->m_szCompShadersText), 0);

  m_menuPreset.AddItem(wasabiApiLangString(IDS_MENU_EDIT_UPGRADE_PRESET_PS_VERSION),
                       (void*)UI_UPGRADE_PIXEL_SHADER, MENUITEMTYPE_UIMODE,
                       wasabiApiLangString(IDS_MENU_EDIT_UPGRADE_PRESET_PS_VERSION_TT, buf, 1024),
                       0, 0, NULL, UI_UPGRADE_PIXEL_SHADER, 0);

  m_menuPreset.AddItem(wasabiApiLangString(IDS_MENU_EDIT_DO_A_PRESET_MASH_UP),
                       (void*)UI_MASHUP, MENUITEMTYPE_UIMODE,
                       wasabiApiLangString(IDS_MENU_EDIT_DO_A_PRESET_MASH_UP_TT, buf, 1024),
                       0, 0, NULL, UI_MASHUP, 0);

  //-------------------------------------------

// menu items
#define MEN_T(id) wasabiApiLangString(id)
#define MEN_TT(id) wasabiApiLangString(id, buf, 1024)

  m_menuWave.AddItem(MEN_T(IDS_MENU_WAVE_TYPE), &m_pState->m_nWaveMode, MENUITEMTYPE_INT, MEN_TT(IDS_MENU_WAVE_TYPE_TT), 0, NUM_WAVES - 1);
  m_menuWave.AddItem(MEN_T(IDS_MENU_SIZE), &m_pState->m_fWaveScale, MENUITEMTYPE_LOGBLENDABLE, MEN_TT(IDS_MENU_SIZE_TT));
  m_menuWave.AddItem(MEN_T(IDS_MENU_SMOOTH), &m_pState->m_fWaveSmoothing, MENUITEMTYPE_BLENDABLE, MEN_TT(IDS_MENU_SMOOTH_TT), 0.0f, 0.9f);
  m_menuWave.AddItem(MEN_T(IDS_MENU_MYSTERY_PARAMETER), &m_pState->m_fWaveParam, MENUITEMTYPE_BLENDABLE, MEN_TT(IDS_MENU_MYSTERY_PARAMETER_TT), -1.0f, 1.0f);
  m_menuWave.AddItem(MEN_T(IDS_MENU_POSITION_X), &m_pState->m_fWaveX, MENUITEMTYPE_BLENDABLE, MEN_TT(IDS_MENU_POSITION_X_TT), 0, 1);
  m_menuWave.AddItem(MEN_T(IDS_MENU_POSITION_Y), &m_pState->m_fWaveY, MENUITEMTYPE_BLENDABLE, MEN_TT(IDS_MENU_POSITION_Y_TT), 0, 1);
  m_menuWave.AddItem(MEN_T(IDS_MENU_COLOR_RED), &m_pState->m_fWaveR, MENUITEMTYPE_BLENDABLE, MEN_TT(IDS_MENU_COLOR_RED_TT), 0, 1);
  m_menuWave.AddItem(MEN_T(IDS_MENU_COLOR_GREEN), &m_pState->m_fWaveG, MENUITEMTYPE_BLENDABLE, MEN_TT(IDS_MENU_COLOR_GREEN_TT), 0, 1);
  m_menuWave.AddItem(MEN_T(IDS_MENU_COLOR_BLUE), &m_pState->m_fWaveB, MENUITEMTYPE_BLENDABLE, MEN_TT(IDS_MENU_COLOR_BLUE_TT), 0, 1);
  m_menuWave.AddItem(MEN_T(IDS_MENU_OPACITY), &m_pState->m_fWaveAlpha, MENUITEMTYPE_LOGBLENDABLE, MEN_TT(IDS_MENU_OPACITY_TT), 0.001f, 100.0f);
  m_menuWave.AddItem(MEN_T(IDS_MENU_USE_DOTS), &m_pState->m_bWaveDots, MENUITEMTYPE_BOOL, MEN_TT(IDS_MENU_USE_DOTS_TT));
  m_menuWave.AddItem(MEN_T(IDS_MENU_DRAW_THICK), &m_pState->m_bWaveThick, MENUITEMTYPE_BOOL, MEN_TT(IDS_MENU_DRAW_THICK_TT));
  m_menuWave.AddItem(MEN_T(IDS_MENU_MODULATE_OPACITY_BY_VOLUME), &m_pState->m_bModWaveAlphaByVolume, MENUITEMTYPE_BOOL, MEN_TT(IDS_MENU_MODULATE_OPACITY_BY_VOLUME_TT));
  m_menuWave.AddItem(MEN_T(IDS_MENU_MODULATION_TRANSPARENT_VOLUME), &m_pState->m_fModWaveAlphaStart, MENUITEMTYPE_BLENDABLE, MEN_TT(IDS_MENU_MODULATION_TRANSPARENT_VOLUME_TT), 0.0f, 2.0f);
  m_menuWave.AddItem(MEN_T(IDS_MENU_MODULATION_OPAQUE_VOLUME), &m_pState->m_fModWaveAlphaEnd, MENUITEMTYPE_BLENDABLE, MEN_TT(IDS_MENU_MODULATION_OPAQUE_VOLUME_TT), 0.0f, 2.0f);
  m_menuWave.AddItem(MEN_T(IDS_MENU_ADDITIVE_DRAWING), &m_pState->m_bAdditiveWaves, MENUITEMTYPE_BOOL, MEN_TT(IDS_MENU_ADDITIVE_DRAWING_TT));
  m_menuWave.AddItem(MEN_T(IDS_MENU_COLOR_BRIGHTENING), &m_pState->m_bMaximizeWaveColor, MENUITEMTYPE_BOOL, MEN_TT(IDS_MENU_COLOR_BRIGHTENING_TT));

  m_menuAugment.AddItem(MEN_T(IDS_MENU_OUTER_BORDER_THICKNESS), &m_pState->m_fOuterBorderSize, MENUITEMTYPE_BLENDABLE, MEN_TT(IDS_MENU_OUTER_BORDER_THICKNESS_TT), 0, 0.5f);
  m_menuAugment.AddItem(MEN_T(IDS_MENU_COLOR_RED_OUTER), &m_pState->m_fOuterBorderR, MENUITEMTYPE_BLENDABLE, MEN_TT(IDS_MENU_COLOR_RED_OUTER_TT), 0, 1);
  m_menuAugment.AddItem(MEN_T(IDS_MENU_COLOR_GREEN_OUTER), &m_pState->m_fOuterBorderG, MENUITEMTYPE_BLENDABLE, MEN_TT(IDS_MENU_COLOR_GREEN_OUTER_TT), 0, 1);
  m_menuAugment.AddItem(MEN_T(IDS_MENU_COLOR_BLUE_OUTER), &m_pState->m_fOuterBorderB, MENUITEMTYPE_BLENDABLE, MEN_TT(IDS_MENU_COLOR_BLUE_OUTER_TT), 0, 1);
  m_menuAugment.AddItem(MEN_T(IDS_MENU_OPACITY_OUTER), &m_pState->m_fOuterBorderA, MENUITEMTYPE_BLENDABLE, MEN_TT(IDS_MENU_OPACITY_OUTER_TT), 0, 1);
  m_menuAugment.AddItem(MEN_T(IDS_MENU_INNER_BORDER_THICKNESS), &m_pState->m_fInnerBorderSize, MENUITEMTYPE_BLENDABLE, MEN_TT(IDS_MENU_INNER_BORDER_THICKNESS_TT), 0, 0.5f);
  m_menuAugment.AddItem(MEN_T(IDS_MENU_COLOR_RED_OUTER), &m_pState->m_fInnerBorderR, MENUITEMTYPE_BLENDABLE, MEN_TT(IDS_MENU_COLOR_RED_INNER_TT), 0, 1);
  m_menuAugment.AddItem(MEN_T(IDS_MENU_COLOR_GREEN_OUTER), &m_pState->m_fInnerBorderG, MENUITEMTYPE_BLENDABLE, MEN_TT(IDS_MENU_COLOR_GREEN_INNER_TT), 0, 1);
  m_menuAugment.AddItem(MEN_T(IDS_MENU_COLOR_BLUE_OUTER), &m_pState->m_fInnerBorderB, MENUITEMTYPE_BLENDABLE, MEN_TT(IDS_MENU_COLOR_BLUE_INNER_TT), 0, 1);
  m_menuAugment.AddItem(MEN_T(IDS_MENU_OPACITY_OUTER), &m_pState->m_fInnerBorderA, MENUITEMTYPE_BLENDABLE, MEN_TT(IDS_MENU_OPACITY_INNER_TT), 0, 1);
  m_menuAugment.AddItem(MEN_T(IDS_MENU_MOTION_VECTOR_OPACITY), &m_pState->m_fMvA, MENUITEMTYPE_BLENDABLE, MEN_TT(IDS_MENU_MOTION_VECTOR_OPACITY_TT), 0, 1);
  m_menuAugment.AddItem(MEN_T(IDS_MENU_NUM_MOT_VECTORS_X), &m_pState->m_fMvX, MENUITEMTYPE_BLENDABLE, MEN_TT(IDS_MENU_NUM_MOT_VECTORS_X_TT), 0, 64);
  m_menuAugment.AddItem(MEN_T(IDS_MENU_NUM_MOT_VECTORS_Y), &m_pState->m_fMvY, MENUITEMTYPE_BLENDABLE, MEN_TT(IDS_MENU_NUM_MOT_VECTORS_Y_TT), 0, 48);
  m_menuAugment.AddItem(MEN_T(IDS_MENU_OFFSET_X), &m_pState->m_fMvDX, MENUITEMTYPE_BLENDABLE, MEN_TT(IDS_MENU_OFFSET_X_TT), -1, 1);
  m_menuAugment.AddItem(MEN_T(IDS_MENU_OFFSET_Y), &m_pState->m_fMvDY, MENUITEMTYPE_BLENDABLE, MEN_TT(IDS_MENU_OFFSET_Y_TT), -1, 1);
  m_menuAugment.AddItem(MEN_T(IDS_MENU_TRAIL_LENGTH), &m_pState->m_fMvL, MENUITEMTYPE_BLENDABLE, MEN_TT(IDS_MENU_TRAIL_LENGTH_TT), 0, 5);
  m_menuAugment.AddItem(MEN_T(IDS_MENU_COLOR_RED_OUTER), &m_pState->m_fMvR, MENUITEMTYPE_BLENDABLE, MEN_TT(IDS_MENU_COLOR_RED_MOTION_VECTOR_TT), 0, 1);
  m_menuAugment.AddItem(MEN_T(IDS_MENU_COLOR_GREEN_OUTER), &m_pState->m_fMvG, MENUITEMTYPE_BLENDABLE, MEN_TT(IDS_MENU_COLOR_GREEN_MOTION_VECTOR_TT), 0, 1);
  m_menuAugment.AddItem(MEN_T(IDS_MENU_COLOR_BLUE_OUTER), &m_pState->m_fMvB, MENUITEMTYPE_BLENDABLE, MEN_TT(IDS_MENU_COLOR_BLUE_MOTION_VECTOR_TT), 0, 1);

  m_menuMotion.AddItem(MEN_T(IDS_MENU_ZOOM_AMOUNT), &m_pState->m_fZoom, MENUITEMTYPE_LOGBLENDABLE, MEN_TT(IDS_MENU_ZOOM_AMOUNT_TT));
  m_menuMotion.AddItem(MEN_T(IDS_MENU_ZOOM_EXPONENT), &m_pState->m_fZoomExponent, MENUITEMTYPE_LOGBLENDABLE, MEN_TT(IDS_MENU_ZOOM_EXPONENT_TT));
  m_menuMotion.AddItem(MEN_T(IDS_MENU_WARP_AMOUNT), &m_pState->m_fWarpAmount, MENUITEMTYPE_LOGBLENDABLE, MEN_TT(IDS_MENU_WARP_AMOUNT_TT));
  m_menuMotion.AddItem(MEN_T(IDS_MENU_WARP_SCALE), &m_pState->m_fWarpScale, MENUITEMTYPE_LOGBLENDABLE, MEN_TT(IDS_MENU_WARP_SCALE_TT));
  m_menuMotion.AddItem(MEN_T(IDS_MENU_WARP_SPEED), &m_pState->m_fWarpAnimSpeed, MENUITEMTYPE_LOGFLOAT, MEN_TT(IDS_MENU_WARP_SPEED_TT));
  m_menuMotion.AddItem(MEN_T(IDS_MENU_ROTATION_AMOUNT), &m_pState->m_fRot, MENUITEMTYPE_BLENDABLE, MEN_TT(IDS_MENU_ROTATION_AMOUNT_TT), -1.00f, 1.00f);
  m_menuMotion.AddItem(MEN_T(IDS_MENU_ROTATION_CENTER_OF_X), &m_pState->m_fRotCX, MENUITEMTYPE_BLENDABLE, MEN_TT(IDS_MENU_ROTATION_CENTER_OF_X_TT), -1.0f, 2.0f);
  m_menuMotion.AddItem(MEN_T(IDS_MENU_ROTATION_CENTER_OF_Y), &m_pState->m_fRotCY, MENUITEMTYPE_BLENDABLE, MEN_TT(IDS_MENU_ROTATION_CENTER_OF_Y_TT), -1.0f, 2.0f);
  m_menuMotion.AddItem(MEN_T(IDS_MENU_TRANSLATION_X), &m_pState->m_fXPush, MENUITEMTYPE_BLENDABLE, MEN_TT(IDS_MENU_TRANSLATION_X_TT), -1.0f, 1.0f);
  m_menuMotion.AddItem(MEN_T(IDS_MENU_TRANSLATION_Y), &m_pState->m_fYPush, MENUITEMTYPE_BLENDABLE, MEN_TT(IDS_MENU_TRANSLATION_Y_TT), -1.0f, 1.0f);
  m_menuMotion.AddItem(MEN_T(IDS_MENU_SCALING_X), &m_pState->m_fStretchX, MENUITEMTYPE_LOGBLENDABLE, MEN_TT(IDS_MENU_SCALING_X_TT));
  m_menuMotion.AddItem(MEN_T(IDS_MENU_SCALING_Y), &m_pState->m_fStretchY, MENUITEMTYPE_LOGBLENDABLE, MEN_TT(IDS_MENU_SCALING_Y_TT));

  m_menuPost.AddItem(MEN_T(IDS_MENU_SUSTAIN_LEVEL), &m_pState->m_fDecay, MENUITEMTYPE_BLENDABLE, MEN_TT(IDS_MENU_SUSTAIN_LEVEL_TT), 0.50f, 1.0f);
  m_menuPost.AddItem(MEN_T(IDS_MENU_DARKEN_CENTER), &m_pState->m_bDarkenCenter, MENUITEMTYPE_BOOL, MEN_TT(IDS_MENU_DARKEN_CENTER_TT));
  m_menuPost.AddItem(MEN_T(IDS_MENU_GAMMA_ADJUSTMENT), &m_pState->m_fGammaAdj, MENUITEMTYPE_BLENDABLE, MEN_TT(IDS_MENU_GAMMA_ADJUSTMENT_TT), 1.0f, 8.0f);
  m_menuPost.AddItem(MEN_T(IDS_MENU_HUE_SHADER), &m_pState->m_fShader, MENUITEMTYPE_BLENDABLE, MEN_TT(IDS_MENU_HUE_SHADER_TT), 0.0f, 1.0f);
  m_menuPost.AddItem(MEN_T(IDS_MENU_VIDEO_ECHO_ALPHA), &m_pState->m_fVideoEchoAlpha, MENUITEMTYPE_BLENDABLE, MEN_TT(IDS_MENU_VIDEO_ECHO_ALPHA_TT), 0.0f, 1.0f);
  m_menuPost.AddItem(MEN_T(IDS_MENU_VIDEO_ECHO_ZOOM), &m_pState->m_fVideoEchoZoom, MENUITEMTYPE_LOGBLENDABLE, MEN_TT(IDS_MENU_VIDEO_ECHO_ZOOM_TT));
  m_menuPost.AddItem(MEN_T(IDS_MENU_VIDEO_ECHO_ORIENTATION), &m_pState->m_nVideoEchoOrientation, MENUITEMTYPE_INT, MEN_TT(IDS_MENU_VIDEO_ECHO_ORIENTATION_TT), 0.0f, 3.0f);
  m_menuPost.AddItem(MEN_T(IDS_MENU_TEXTURE_WRAP), &m_pState->m_bTexWrap, MENUITEMTYPE_BOOL, MEN_TT(IDS_MENU_TEXTURE_WRAP_TT));
  // m_menuPost.AddItem("stereo 3D",               &m_pState->m_bRedBlueStereo,        MENUITEMTYPE_BOOL, "displays the image in stereo 3D; you need 3D glasses (with red and blue lenses) for this.");
  m_menuPost.AddItem(MEN_T(IDS_MENU_FILTER_INVERT), &m_pState->m_bInvert, MENUITEMTYPE_BOOL, MEN_TT(IDS_MENU_FILTER_INVERT_TT));
  m_menuPost.AddItem(MEN_T(IDS_MENU_FILTER_BRIGHTEN), &m_pState->m_bBrighten, MENUITEMTYPE_BOOL, MEN_TT(IDS_MENU_FILTER_BRIGHTEN_TT));
  m_menuPost.AddItem(MEN_T(IDS_MENU_FILTER_DARKEN), &m_pState->m_bDarken, MENUITEMTYPE_BOOL, MEN_TT(IDS_MENU_FILTER_DARKEN_TT));
  m_menuPost.AddItem(MEN_T(IDS_MENU_FILTER_SOLARIZE), &m_pState->m_bSolarize, MENUITEMTYPE_BOOL, MEN_TT(IDS_MENU_FILTER_SOLARIZE_TT));
  m_menuPost.AddItem(MEN_T(IDS_MENU_BLUR1_EDGE_DARKEN_AMOUNT), &m_pState->m_fBlur1EdgeDarken, MENUITEMTYPE_FLOAT, MEN_TT(IDS_MENU_BLUR1_EDGE_DARKEN_AMOUNT_TT), 0.0f, 1.0f);
  m_menuPost.AddItem(MEN_T(IDS_MENU_BLUR1_MIN_COLOR_VALUE), &m_pState->m_fBlur1Min, MENUITEMTYPE_FLOAT, MEN_TT(IDS_MENU_BLUR1_MIN_COLOR_VALUE_TT), 0.0f, 1.0f);
  m_menuPost.AddItem(MEN_T(IDS_MENU_BLUR1_MAX_COLOR_VALUE), &m_pState->m_fBlur1Max, MENUITEMTYPE_FLOAT, MEN_TT(IDS_MENU_BLUR1_MAX_COLOR_VALUE_TT), 0.0f, 1.0f);
  m_menuPost.AddItem(MEN_T(IDS_MENU_BLUR2_MIN_COLOR_VALUE), &m_pState->m_fBlur2Min, MENUITEMTYPE_FLOAT, MEN_TT(IDS_MENU_BLUR2_MIN_COLOR_VALUE_TT), 0.0f, 1.0f);
  m_menuPost.AddItem(MEN_T(IDS_MENU_BLUR2_MAX_COLOR_VALUE), &m_pState->m_fBlur2Max, MENUITEMTYPE_FLOAT, MEN_TT(IDS_MENU_BLUR2_MAX_COLOR_VALUE_TT), 0.0f, 1.0f);
  m_menuPost.AddItem(MEN_T(IDS_MENU_BLUR3_MIN_COLOR_VALUE), &m_pState->m_fBlur3Min, MENUITEMTYPE_FLOAT, MEN_TT(IDS_MENU_BLUR3_MIN_COLOR_VALUE_TT), 0.0f, 1.0f);
  m_menuPost.AddItem(MEN_T(IDS_MENU_BLUR3_MAX_COLOR_VALUE), &m_pState->m_fBlur3Max, MENUITEMTYPE_FLOAT, MEN_TT(IDS_MENU_BLUR3_MAX_COLOR_VALUE_TT), 0.0f, 1.0f);

  for (i = 0; i < MAX_CUSTOM_WAVES; i++) {
    // blending: do both; fade opacities in/out (w/exagerrated weighting)
    m_menuWavecode[i].AddItem(MEN_T(IDS_MENU_ENABLED), &m_pState->m_wave[i].enabled, MENUITEMTYPE_BOOL, MEN_TT(IDS_MENU_ENABLED_TT));                             // bool
    m_menuWavecode[i].AddItem(MEN_T(IDS_MENU_NUMBER_OF_SAMPLES), &m_pState->m_wave[i].samples, MENUITEMTYPE_INT, MEN_TT(IDS_MENU_NUMBER_OF_SAMPLES_TT), 2, 512);  // 0-512
    m_menuWavecode[i].AddItem(MEN_T(IDS_MENU_L_R_SEPARATION), &m_pState->m_wave[i].sep, MENUITEMTYPE_INT, MEN_TT(IDS_MENU_L_R_SEPARATION_TT), 0, 256);            // 0-512
    m_menuWavecode[i].AddItem(MEN_T(IDS_MENU_SCALING), &m_pState->m_wave[i].scaling, MENUITEMTYPE_LOGFLOAT, MEN_TT(IDS_MENU_SCALING_TT));
    m_menuWavecode[i].AddItem(MEN_T(IDS_MENU_SMOOTH), &m_pState->m_wave[i].smoothing, MENUITEMTYPE_FLOAT, MEN_TT(IDS_MENU_SMOOTHING_TT), 0, 1);
    m_menuWavecode[i].AddItem(MEN_T(IDS_MENU_COLOR_RED), &m_pState->m_wave[i].r, MENUITEMTYPE_FLOAT, MEN_TT(IDS_MENU_COLOR_RED_TT), 0, 1);
    m_menuWavecode[i].AddItem(MEN_T(IDS_MENU_COLOR_GREEN), &m_pState->m_wave[i].g, MENUITEMTYPE_FLOAT, MEN_TT(IDS_MENU_COLOR_GREEN_TT), 0, 1);
    m_menuWavecode[i].AddItem(MEN_T(IDS_MENU_COLOR_BLUE), &m_pState->m_wave[i].b, MENUITEMTYPE_FLOAT, MEN_TT(IDS_MENU_COLOR_BLUE_TT), 0, 1);
    m_menuWavecode[i].AddItem(MEN_T(IDS_MENU_OPACITY), &m_pState->m_wave[i].a, MENUITEMTYPE_FLOAT, MEN_TT(IDS_MENU_OPACITY_WAVE_TT), 0, 1);
    m_menuWavecode[i].AddItem(MEN_T(IDS_MENU_USE_SPECTRUM), &m_pState->m_wave[i].bSpectrum, MENUITEMTYPE_BOOL, MEN_TT(IDS_MENU_USE_SPECTRUM_TT));               // 0-5 [0=wave left, 1=wave center, 2=wave right; 3=spectrum left, 4=spec center, 5=spec right]
    m_menuWavecode[i].AddItem(MEN_T(IDS_MENU_USE_DOTS), &m_pState->m_wave[i].bUseDots, MENUITEMTYPE_BOOL, MEN_TT(IDS_MENU_USE_DOTS_WAVE_TT));                   // bool
    m_menuWavecode[i].AddItem(MEN_T(IDS_MENU_DRAW_THICK), &m_pState->m_wave[i].bDrawThick, MENUITEMTYPE_BOOL, MEN_TT(IDS_MENU_DRAW_THICK_WAVE_TT));             // bool
    m_menuWavecode[i].AddItem(MEN_T(IDS_MENU_ADDITIVE_DRAWING), &m_pState->m_wave[i].bAdditive, MENUITEMTYPE_BOOL, MEN_TT(IDS_MENU_ADDITIVE_DRAWING_WAVE_TT));  // bool
    m_menuWavecode[i].AddItem(MEN_T(IDS_MENU_EXPORT_TO_FILE), (void*)UI_EXPORT_WAVE, MENUITEMTYPE_UIMODE, MEN_TT(IDS_MENU_EXPORT_TO_FILE_TT), 0, 0, NULL, UI_EXPORT_WAVE, i);
    m_menuWavecode[i].AddItem(MEN_T(IDS_MENU_IMPORT_FROM_FILE), (void*)UI_IMPORT_WAVE, MENUITEMTYPE_UIMODE, MEN_TT(IDS_MENU_IMPORT_FROM_FILE_TT), 0, 0, NULL, UI_IMPORT_WAVE, i);
    m_menuWavecode[i].AddItem(MEN_T(IDS_MENU_EDIT_INIT_CODE), &m_pState->m_wave[i].m_szInit, MENUITEMTYPE_STRING, MEN_TT(IDS_MENU_EDIT_INIT_CODE_TT), 256, 0, &OnUserEditedWavecodeInit, sizeof(m_pState->m_wave[i].m_szInit), 0);
    m_menuWavecode[i].AddItem(MEN_T(IDS_MENU_EDIT_PER_FRAME_CODE), &m_pState->m_wave[i].m_szPerFrame, MENUITEMTYPE_STRING, MEN_TT(IDS_MENU_EDIT_PER_FRAME_CODE_TT), 256, 0, &OnUserEditedWavecode, sizeof(m_pState->m_wave[i].m_szPerFrame), 0);
    m_menuWavecode[i].AddItem(MEN_T(IDS_MENU_EDIT_PER_POINT_CODE), &m_pState->m_wave[i].m_szPerPoint, MENUITEMTYPE_STRING, MEN_TT(IDS_MENU_EDIT_PER_POINT_CODE_TT), 256, 0, &OnUserEditedWavecode, sizeof(m_pState->m_wave[i].m_szPerPoint), 0);
  }

  for (i = 0; i < MAX_CUSTOM_SHAPES; i++) {
    // blending: do both; fade opacities in/out (w/exagerrated weighting)
    m_menuShapecode[i].AddItem(MEN_T(IDS_MENU_ENABLED), &m_pState->m_shape[i].enabled, MENUITEMTYPE_BOOL, MEN_TT(IDS_MENU_ENABLED_SHAPE_TT));  // bool
    m_menuShapecode[i].AddItem(MEN_T(IDS_MENU_NUMBER_OF_INSTANCES), &m_pState->m_shape[i].instances, MENUITEMTYPE_INT, MEN_TT(IDS_MENU_NUMBER_OF_INSTANCES_TT), 1, 1024);
    m_menuShapecode[i].AddItem(MEN_T(IDS_MENU_NUMBER_OF_SIDES), &m_pState->m_shape[i].sides, MENUITEMTYPE_INT, MEN_TT(IDS_MENU_NUMBER_OF_SIDES_TT), 3, 100);
    m_menuShapecode[i].AddItem(MEN_T(IDS_MENU_DRAW_THICK), &m_pState->m_shape[i].thickOutline, MENUITEMTYPE_BOOL, MEN_TT(IDS_MENU_DRAW_THICK_SHAPE_TT));          // bool
    m_menuShapecode[i].AddItem(MEN_T(IDS_MENU_ADDITIVE_DRAWING), &m_pState->m_shape[i].additive, MENUITEMTYPE_BOOL, MEN_TT(IDS_MENU_ADDITIVE_DRAWING_SHAPE_TT));  // bool
    m_menuShapecode[i].AddItem(MEN_T(IDS_MENU_X_POSITION), &m_pState->m_shape[i].x, MENUITEMTYPE_FLOAT, MEN_TT(IDS_MENU_X_POSITION_TT), 0, 1);
    m_menuShapecode[i].AddItem(MEN_T(IDS_MENU_Y_POSITION), &m_pState->m_shape[i].y, MENUITEMTYPE_FLOAT, MEN_TT(IDS_MENU_Y_POSITION_TT), 0, 1);
    m_menuShapecode[i].AddItem(MEN_T(IDS_MENU_RADIUS), &m_pState->m_shape[i].rad, MENUITEMTYPE_LOGFLOAT, MEN_TT(IDS_MENU_RADIUS_TT));
    m_menuShapecode[i].AddItem(MEN_T(IDS_MENU_ANGLE), &m_pState->m_shape[i].ang, MENUITEMTYPE_FLOAT, MEN_TT(IDS_MENU_ANGLE_TT), 0, 3.1415927f * 2.0f);
    m_menuShapecode[i].AddItem(MEN_T(IDS_MENU_TEXTURED), &m_pState->m_shape[i].textured, MENUITEMTYPE_BOOL, MEN_TT(IDS_MENU_TEXTURED_TT));                                  // bool
    m_menuShapecode[i].AddItem(MEN_T(IDS_MENU_TEXTURE_ZOOM), &m_pState->m_shape[i].tex_zoom, MENUITEMTYPE_LOGFLOAT, MEN_TT(IDS_MENU_TEXTURE_ZOOM_TT));                      // bool
    m_menuShapecode[i].AddItem(MEN_T(IDS_MENU_TEXTURE_ANGLE), &m_pState->m_shape[i].tex_ang, MENUITEMTYPE_FLOAT, MEN_TT(IDS_MENU_TEXTURE_ANGLE_TT), 0, 3.1415927f * 2.0f);  // bool
    m_menuShapecode[i].AddItem(MEN_T(IDS_MENU_INNER_COLOR_RED), &m_pState->m_shape[i].r, MENUITEMTYPE_FLOAT, MEN_TT(IDS_MENU_INNER_COLOR_RED_TT), 0, 1);
    m_menuShapecode[i].AddItem(MEN_T(IDS_MENU_INNER_COLOR_GREEN), &m_pState->m_shape[i].g, MENUITEMTYPE_FLOAT, MEN_TT(IDS_MENU_INNER_COLOR_GREEN_TT), 0, 1);
    m_menuShapecode[i].AddItem(MEN_T(IDS_MENU_INNER_COLOR_BLUE), &m_pState->m_shape[i].b, MENUITEMTYPE_FLOAT, MEN_TT(IDS_MENU_INNER_COLOR_BLUE_TT), 0, 1);
    m_menuShapecode[i].AddItem(MEN_T(IDS_MENU_INNER_OPACITY), &m_pState->m_shape[i].a, MENUITEMTYPE_FLOAT, MEN_TT(IDS_MENU_INNER_OPACITY_TT), 0, 1);
    m_menuShapecode[i].AddItem(MEN_T(IDS_MENU_OUTER_COLOR_RED), &m_pState->m_shape[i].r2, MENUITEMTYPE_FLOAT, MEN_TT(IDS_MENU_OUTER_COLOR_RED_TT), 0, 1);
    m_menuShapecode[i].AddItem(MEN_T(IDS_MENU_OUTER_COLOR_GREEN), &m_pState->m_shape[i].g2, MENUITEMTYPE_FLOAT, MEN_TT(IDS_MENU_OUTER_COLOR_GREEN_TT), 0, 1);
    m_menuShapecode[i].AddItem(MEN_T(IDS_MENU_OUTER_COLOR_BLUE), &m_pState->m_shape[i].b2, MENUITEMTYPE_FLOAT, MEN_TT(IDS_MENU_OUTER_COLOR_BLUE_TT), 0, 1);
    m_menuShapecode[i].AddItem(MEN_T(IDS_MENU_OUTER_OPACITY), &m_pState->m_shape[i].a2, MENUITEMTYPE_FLOAT, MEN_TT(IDS_MENU_OUTER_OPACITY_TT), 0, 1);
    m_menuShapecode[i].AddItem(MEN_T(IDS_MENU_BORDER_COLOR_RED), &m_pState->m_shape[i].border_r, MENUITEMTYPE_FLOAT, MEN_TT(IDS_MENU_BORDER_COLOR_RED_TT), 0, 1);
    m_menuShapecode[i].AddItem(MEN_T(IDS_MENU_BORDER_COLOR_GREEN), &m_pState->m_shape[i].border_g, MENUITEMTYPE_FLOAT, MEN_TT(IDS_MENU_BORDER_COLOR_GREEN_TT), 0, 1);
    m_menuShapecode[i].AddItem(MEN_T(IDS_MENU_BORDER_COLOR_BLUE), &m_pState->m_shape[i].border_b, MENUITEMTYPE_FLOAT, MEN_TT(IDS_MENU_BORDER_COLOR_BLUE_TT), 0, 1);
    m_menuShapecode[i].AddItem(MEN_T(IDS_MENU_BORDER_OPACITY), &m_pState->m_shape[i].border_a, MENUITEMTYPE_FLOAT, MEN_TT(IDS_MENU_BORDER_OPACITY_TT), 0, 1);
    m_menuShapecode[i].AddItem(MEN_T(IDS_MENU_EXPORT_TO_FILE), NULL, MENUITEMTYPE_UIMODE, MEN_TT(IDS_MENU_EXPORT_TO_FILE_SHAPE_TT), 0, 0, NULL, UI_EXPORT_SHAPE, i);
    m_menuShapecode[i].AddItem(MEN_T(IDS_MENU_IMPORT_FROM_FILE), NULL, MENUITEMTYPE_UIMODE, MEN_TT(IDS_MENU_IMPORT_FROM_FILE_SHAPE_TT), 0, 0, NULL, UI_IMPORT_SHAPE, i);
    m_menuShapecode[i].AddItem(MEN_T(IDS_MENU_EDIT_INIT_CODE), &m_pState->m_shape[i].m_szInit, MENUITEMTYPE_STRING, MEN_TT(IDS_MENU_EDIT_INIT_CODE_SHAPE_TT), 256, 0, &OnUserEditedShapecodeInit, sizeof(m_pState->m_shape[i].m_szInit), 0);
    m_menuShapecode[i].AddItem(MEN_T(IDS_MENU_EDIT_PER_FRAME_INSTANCE_CODE), &m_pState->m_shape[i].m_szPerFrame, MENUITEMTYPE_STRING, MEN_TT(IDS_MENU_EDIT_PER_FRAME_INSTANCE_CODE_TT), 256, 0, &OnUserEditedShapecode, sizeof(m_pState->m_shape[i].m_szPerFrame), 0);
    // m_menuShapecode[i].AddItem("[ edit per-point code ]",&m_pState->m_shape[i].m_szPerPoint,  MENUITEMTYPE_STRING, "IN: sample [0..1]; value1 [left ch], value2 [right ch], plus all vars for per-frame code / OUT: x,y; r,g,b,a; t1-t8", 256, 0, &OnUserEditedWavecode);
  }
}

void CPlugin::WriteRealtimeConfig() {
  // WritePrivateProfileIntW(m_bShowSongTitle, L"bShowSongTitle", GetConfigIniFile(), L"Settings");
  // WritePrivateProfileIntW(m_bShowSongTime, L"bShowSongTime", GetConfigIniFile(), L"Settings");
  // WritePrivateProfileIntW(m_bShowSongLen, L"bShowSongLen", GetConfigIniFile(), L"Settings");
}

void CPlugin::dumpmsg(wchar_t* s) {
#if _DEBUG
  OutputDebugStringW(s);
  if (s[0]) {
    int len = lstrlenW(s);
    if (s[len - 1] != L'\n')
      OutputDebugStringW(L"\n");
  }
#endif
}
