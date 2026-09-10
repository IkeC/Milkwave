// plugin_dx9.cpp - DirectX 9 resource allocation and cleanup
// Extracted from plugin.cpp

#include "plugin.h"
#include "plugin_internals.h"
#include "utility.h"
#include "support.h"
#include "resource.h"
#include "defines.h"
#include "shell_defines.h"
#include "wasabi.h"

int CPlugin::AllocateMyNonDx9Stuff() {
  // This gets called only once, when your plugin is actually launched.
  // If only the config panel is launched, this does NOT get called.
  //   (whereas MyPreInitialize() still does).
  // If anything fails here, return FALSE to safely exit the plugin,
  //   but only after displaying a messagebox giving the user some information
  //   about what went wrong.

  /*
  if (!m_hBlackBrush)
  m_hBlackBrush = CreateSolidBrush(RGB(0,0,0));
  */

  g_hThread = INVALID_HANDLE_VALUE;
  g_bThreadAlive = false;
  g_bThreadShouldQuit = false;
  InitializeCriticalSection(&g_cs);
  InitializeCriticalSection(&g_csRemoteMessage);

  // read in 'm_szShaderIncludeText'
  bool bSuccess = true;
  bSuccess = ReadFileToString(L"data\\include.fx", m_szShaderIncludeText, sizeof(m_szShaderIncludeText) - 4, false);
  if (!bSuccess) return false;
  StripComments(m_szShaderIncludeText);
  m_nShaderIncludeTextLen = lstrlen(m_szShaderIncludeText);
  bSuccess |= ReadFileToString(L"data\\warp_vs.fx", m_szDefaultWarpVShaderText, sizeof(m_szDefaultWarpVShaderText), true);
  if (!bSuccess) return false;
  bSuccess |= ReadFileToString(L"data\\warp_ps.fx", m_szDefaultWarpPShaderText, sizeof(m_szDefaultWarpPShaderText), true);
  if (!bSuccess) return false;
  bSuccess |= ReadFileToString(L"data\\comp_vs.fx", m_szDefaultCompVShaderText, sizeof(m_szDefaultCompVShaderText), true);
  if (!bSuccess) return false;
  bSuccess |= ReadFileToString(L"data\\comp_ps.fx", m_szDefaultCompPShaderText, sizeof(m_szDefaultCompPShaderText), true);
  if (!bSuccess) return false;
  bSuccess |= ReadFileToString(L"data\\blur_vs.fx", m_szBlurVS, sizeof(m_szBlurVS), true);
  if (!bSuccess) return false;
  bSuccess |= ReadFileToString(L"data\\blur1_ps.fx", m_szBlurPSX, sizeof(m_szBlurPSX), true);
  if (!bSuccess) return false;
  bSuccess |= ReadFileToString(L"data\\blur2_ps.fx", m_szBlurPSY, sizeof(m_szBlurPSY), true);
  if (!bSuccess) return false;

  BuildMenus();

  m_bMMX = CheckForMMX();
  // m_bSSE = CheckForSSE();

  m_pState->Default();
  m_pOldState->Default();
  m_pNewState->Default();

  // Initialize video capture
  m_pVideoCaptureTexture = nullptr;
  m_nVideoCaptureWidth = 640;  // Default video capture dimensions
  m_nVideoCaptureHeight = 480;
  m_fInputMixOpacity = 0.5f;
  m_bInputMixOnTop = true;
  m_bVideoInputEnabled = false;
  m_nVideoDeviceIndex = -1;

  // Initialize Spout input receiver
  m_pSpoutReceiver = nullptr;
  m_pSpoutInputTexture = nullptr;
  m_szSpoutSenderName[0] = '\0';
  m_nSpoutInputWidth = 0;
  m_nSpoutInputHeight = 0;
  m_bSpoutInputEnabled = false;

  // LoadRandomPreset(0.0f);   -avoid this here; causes some DX9 stuff to happen.

  return true;
}

//----------------------------------------------------------------------

void CPlugin::CleanUpMyNonDx9Stuff() {
  // This gets called only once, when your plugin exits.
  // Be sure to clean up any objects here that were
  //   created/initialized in AllocateMyNonDx9Stuff.

  // Clean up video capture
  if (m_pVideoCapture) {
    m_pVideoCapture->Stop();
    m_pVideoCapture->Release();
    delete m_pVideoCapture;
    m_pVideoCapture = nullptr;
  }

  // Clean up Spout input receiver
  if (m_pSpoutReceiver) {
    if (m_pSpoutInputTexture) {
      m_pSpoutInputTexture->Release();
      m_pSpoutInputTexture = nullptr;
    }
    m_pSpoutReceiver->ReleaseReceiver();
    delete m_pSpoutReceiver;
    m_pSpoutReceiver = nullptr;
  }

  // =========================================================
  // SPOUT cleanup on exit
  //
  spoutsender.ReleaseDX9sender();
  spoutsender.CloseDirectX9();

  // If Spout output or DirectX mode has been changed, save the config file
  // so it is started in the selected mode the next time
  if (bSpoutChanged) MyWriteConfig();
  // =========================================================

  // sound.Finish();

  // NOTE: DO NOT DELETE m_gdi_titlefont_doublesize HERE!!!

  DeleteCriticalSection(&g_cs);
  DeleteCriticalSection(&g_csRemoteMessage);

  CancelThread(1000);

  m_menuPreset.Finish();
  m_menuWave.Finish();
  m_menuAugment.Finish();
  m_menuCustomWave.Finish();
  m_menuCustomShape.Finish();
  m_menuMotion.Finish();
  m_menuPost.Finish();
  int i;
  for (i = 0; i < MAX_CUSTOM_WAVES; i++)
    m_menuWavecode[i].Finish();
  for (i = 0; i < MAX_CUSTOM_SHAPES; i++)
    m_menuShapecode[i].Finish();

  // dumpmsg("Finish: cleanup complete.");
}

//----------------------------------------------------------------------

int CPlugin::AllocateMyDX9Stuff() {
  // (...aka OnUserResizeWindow)
  // (...aka OnToggleFullscreen)

  // Allocate and initialize all your DX9 and D3DX stuff here: textures,
  //   surfaces, vertex/index buffers, D3DX fonts, and so on.
  // If anything fails here, return FALSE to safely exit the plugin,
  //   but only after displaying a messagebox giving the user some information
  //   about what went wrong.  If the error is NON-CRITICAL, you don't *have*
  //   to return; just make sure that the rest of the code will be still safely
  //   run (albeit with degraded features).
  // If you run out of video memory, you might want to show a short messagebox
  //   saying what failed to allocate and that the reason is a lack of video
  //   memory, and then call SuggestHowToFreeSomeMem(), which will show them
  //   a *second* messagebox that (intelligently) suggests how they can free up
  //   some video memory.
  // Don't forget to account for each object you create/allocate here by cleaning
  //   it up in CleanUpMyDX9Stuff!
  // IMPORTANT:
  //   Note that the code here isn't just run at program startup!
  //   When the user toggles between fullscreen and windowed modes
  //   or resizes the window, the base class calls this function before
  //   destroying & recreating the plugin window and DirectX object, and then
  //   calls AllocateMyDX9Stuff afterwards, to get your plugin running again.

  wchar_t buf[32768], title[64];

  m_nFramesSinceResize = 0;

  RecreateLyricsFont();
  RecreateLyricsMeasureFont(1.0f);

  int nNewCanvasStretch = (m_nCanvasStretch == 0) ? 100 : m_nCanvasStretch;

  DWORD PSVersion = GetCaps()->PixelShaderVersion & 0xFFFF;  // 0x0300, etc.
  if (PSVersion >= 0x0300)
    m_nMaxPSVersion_DX9 = MD2_PS_3_0;
  else if (PSVersion > 0x0200)
    m_nMaxPSVersion_DX9 = MD2_PS_2_X;
  else if (PSVersion >= 0x0200)
    m_nMaxPSVersion_DX9 = MD2_PS_2_0;
  else
    m_nMaxPSVersion_DX9 = MD2_PS_NONE;

  if (m_nMaxPSVersion_ConfigPanel == -1)
    m_nMaxPSVersion = m_nMaxPSVersion_DX9;
  else {
    // to still limit their choice by what HW reports:
    // m_nMaxPSVersion = min(m_nMaxPSVersion_DX9, m_nMaxPSVersion_ConfigPanel);

    // to allow them to override:
    m_nMaxPSVersion = m_nMaxPSVersion_ConfigPanel;
  }

  /*
     Auto mode: do a check against a few known, *SLOW* DX9/ps_2_0 cards to see
      if we should run them without pixel shaders instead.
     Here is valve's list of the cards they run DX8 on (mostly because they're too slow under DX9 + ps_2_0):
          NVIDIA GeForce FX 5200Â  31.12%
          ATI Radeon 9200Â Â Â Â Â Â Â Â  21.29%
          NVIDIA GeForce FX 5500Â  11.27%
          NVIDIA GeForce4Â Â Â Â Â Â Â Â Â  7.74%
          NVIDIA GeForce FX 5700Â Â  7.12%
          NVIDIA GeForce FX 5600Â Â  5.16%
          SiS 661FX_760_741Â Â Â Â Â Â Â  3.34%
          NVIDIA GeForce FX 5900Â Â  3.24%
          NVIDIA GeForce3Â Â Â Â Â Â Â Â Â  2.09%
          ATI Radeon 9000Â Â Â Â Â Â Â Â Â  1.98%
          otherÂ Â Â Â Â Â Â Â Â Â Â Â Â Â Â Â Â Â Â  5.66%
          [ from http://www.steampowered.com/status/survey.html ]
          see also:
              http://en.wikipedia.org/wiki/Radeon
              http://en.wikipedia.org/wiki/Geforce_fx
  */

  const char* szGPU = GetDriverDescription();
  /* known examples of this string:
      "ATI MOBILITY RADEON X600"
      "RADEON X800 Series   "     <- note the spaces at the end
      "Sapphire RADEON X700"
      "NVIDIA GeForce Go 6200  "  <- note the spaces at the end
      "NVIDIA GeForce 6800 GT"
      "Intel(R) 82865G Graphics Controller"
      "Mobile Intel(R) 915GM/GMS,910GML Express Chipset Family"

  might want to consider adding these to the list: [from http://www.intel.com/support/graphics/sb/cs-014257.htm ]
    (...they should support PS2.0, but not sure if they're fast...)
      "Mobile Intel(R) 945GM Express Chipset Family"
      "Mobile Intel(R) 915GM/GMS,910GML Express Chipset"
      "Intel(R) 945G Express Chipset"
      "Intel(R) 82915G/82910GL Express Chipset Family"

  or these, if they seem to be reporting that they do support ps_2_0, which would be very bogus info:
      "Intel(R) 82865G Graphics Controller"
      "Intel(R) 82852/82855 Graphics Controller Family"
      "Intel(R) 82845G Graphics Controller"
      "Intel(R) 82830M Graphics Controller"
      "Intel(R) 82815 Graphics Controller"
      "Intel(R) 82810 Graphics Controller"
  */

  // GREY LIST - slow ps_2_0 cards
  // In Canvas Stretch==Auto mode, for these cards, if they (claim to) run ps_2_0,
  //   we run at half-res (cuz they're slow).
  // THE GENERAL GUIDELINE HERE:
  //   It should be at least as fast as a GeForce FX 5700 or my GeForce 6200 (TC)
  //   if it's to run without stretch.
  if (m_nCanvasStretch == 0)  // && m_nMaxPSVersion_DX9 > 0)
  {
    // put cards on this list if you see them successfully run ps_2_0 (using override)
    // and they run well at a low resolution (512x512 or less).
    if (
        strstr(szGPU, "GeForce 4") ||      // probably not even ps_2_0
        strstr(szGPU, "GeForce FX 52") ||  // chip's computer (FX 5200) - does do ps_2_0, but slow
        strstr(szGPU, "GeForce FX 53") ||
        strstr(szGPU, "GeForce FX 54") ||
        strstr(szGPU, "GeForce FX 55") ||  // GeForce FX 5600 is 13 GB/s - 2.5x as fast as my 6200!
        strstr(szGPU, "GeForce FX 56") ||
        //...GeForce FX 5700 and up, we let those run at full-res on ps_2_0...
        strstr(szGPU, "GeForce FX 56") ||
        strstr(szGPU, "GeForce FX 56") ||
        strstr(szGPU, "SiS 300/305/630/540/730") ||  // mom's computer - just slow.
        strstr(szGPU, "Radeon 8") ||                 // no shader model 2.
        strstr(szGPU, "Radeon 90") ||                // from Valve.  no shader model 2.
        strstr(szGPU, "Radeon 91") ||                // no shader model 2.
        strstr(szGPU, "Radeon 92") ||                // from Valve.  no shader model 2.
        strstr(szGPU, "Radeon 93") ||                // no shader model 2.
        strstr(szGPU, "Radeon 94") ||                // no shader model 2.
        // guessing that 9500+ are ok - they're all ps_2_0 and the 9600 is like an FX 5900.
        strstr(szGPU, "Radeon 9550") ||  // *maybe* - kiv - super budget R200 chip.  def. ps_2_0 but possibly very slow.
        strstr(szGPU, "Radeon X300") ||  // *maybe* - kiv - super budget R200 chip   def. ps_2_0 but possibly very slow.
        0) {
      nNewCanvasStretch = 200;
    }
  }

  /*                           pix pipes
                             core    Fill(G)  membw(GB/s)
      Radeon 9600 Pro	        400	 4	1.6	     9.6
      Radeon 9600 XT	        500	 4	2.0	     9.6
      GeForce FX 5600 Ultra	400	 4	1.6	    12.8
      GeForce FX 5700 Ultra	475	 4	1.9	    14.4
      GeForce FX 5900 XT	    400	 4	1.6	    22.4
      GeForce FX 5900	        450	 4	1.8	    27.2
      GeForce FX 5950 Ultra 	475  4  2.9     30
      GeForce 6200 TC-32 	    350  4  1.1      5.6 (TurboDonkey)
      GeForce 6600 GT 	    500  8  2.0     16
      GeForce 6800 Ultra 	    400 16  6.4     35
      ATI Radeon X800 XT PE 	520 16  8.3     36
      ATI Radeon X850 XT PE   540 16  8.6     38

      Entry-level GPU 	5200, 5300, 5500
      Mid-Range GPU 	    5600, 5700
      High-end GPU 	    5800, 5900, 5950

      Entry-level GPU 	6200, 6500
      Mid-Range GPU 	    6600
      High-end GPU 	    6800

      Entry-level GPU
      Mid-Range GPU
      High-end GPU

      R200: only ps_1_4.  Radeon 8500-9250.
      R300: ps_2_0.       Radeon 9500-9800, X300-X600, X1050.  9600 fast enough (~FX5900).  9500/9700 ~ GeForce 4 Ti.
      R420: ps_2_0        Radeon X700-8750 - ALL FAST ENOUGH.  X700 is same speed as a GeForce 6600.

      6600		    ~	X700
      GeForce 4		<	X300 / X600 / 9600
      GeForce 4 Ti	>	Radeon 8500
      FX 5900		    =	Radeon 9600
      FX 5900 Ultra	<< [half]	Radeon 9800 Pro
      GeForce FX		<	Radeon 9700/9800
  */

  // BLACK LIST
  // In Pixel Shaders==Auto mode, for these cards, we avoid ps_2_0 completely.
  // There shouldn't be much on this list... feel free to put anything you KNOW doesn't do ps_2_0 (why not),
  // and to put anything that is slow to begin with, and HAS BUGGY DRIVERS (INTEL).
  if (m_nMaxPSVersion_ConfigPanel == -1) {
    if (strstr(szGPU, "GeForce2") ||  // from Valve
        strstr(szGPU, "GeForce3") ||  // from Valve
        strstr(szGPU, "GeForce4") ||  // from Valve
        strstr(szGPU, "Radeon 7") ||  // from Valve
        strstr(szGPU, "Radeon 8") ||
        strstr(szGPU, "SiS 661FX_760_741") ||  // from Valve
        // FOR NOW, FOR THESE, ASSUME INTEL EITHER DOESN'T DO PS_2_0,
        // OR DRIVERS SUCK AND IT WOULDN'T WORK ANYWAY!
        (strstr(szGPU, "Intel") && strstr(szGPU, "945G")) ||
        (strstr(szGPU, "Intel") && strstr(szGPU, "915G")) ||  // ben allison's laptop - snow, freezing when you try ps_2_0
        (strstr(szGPU, "Intel") && strstr(szGPU, "910G")) ||
        (strstr(szGPU, "Intel") && strstr(szGPU, "8291")) ||  // gonna guess that this supports ps_2_0 but is SLOW
        (strstr(szGPU, "Intel") && strstr(szGPU, "8281")) ||  // definitely DOESN'T support pixel shaders
        (strstr(szGPU, "Intel") && strstr(szGPU, "8283")) ||  // definitely DOESN'T support pixel shaders
        (strstr(szGPU, "Intel") && strstr(szGPU, "8284")) ||  // definitely DOESN'T support pixel shaders
        (strstr(szGPU, "Intel") && strstr(szGPU, "8285")) ||  // definitely DOESN'T support pixel shaders
        (strstr(szGPU, "Intel") && strstr(szGPU, "8286")) ||  // definitely DOESN'T support pixel shaders.  Ben Allison's desktop (865) - no image w/ps_2_0.  Plus Nes's desktop - no ps_2_0.
        0) {
      m_nMaxPSVersion = MD2_PS_NONE;
      // if (m_nCanvasStretch==0)
      //     nNewCanvasStretch = 100;
    }
  }

  /*char fname[512];
  sprintf(fname, "%s%s", GetPluginsDirPath(), TEXTURE_NAME);
  if (D3DXCreateTextureFromFile(GetDevice(), fname, &m_object_tex) != S_OK)
  {
      // just give a warning, and move on
      m_object_tex = NULL;    // (make sure pointer wasn't mangled by some bad driver)

      char msg[1024];
      sprintf(msg, "Unable to load texture:\r%s", fname);
      MessageBox(GetPluginWindow(), msg, "WARNING", MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
      //return false;
  }*/

  // Note: this code used to be in OnResizeGraphicsWindow().

  // SHADERS
  //-------------------------------------
  if (m_nMaxPSVersion > MD2_PS_NONE) {
    // Create vertex declarations (since we're not using FVF anymore)
    if (D3D_OK != GetDevice()->CreateVertexDeclaration(g_MyVertDecl, &m_pMyVertDecl)) {
      wasabiApiLangString(IDS_COULD_NOT_CREATE_MY_VERTEX_DECLARATION, buf, sizeof(buf));
      dumpmsg(buf);
      MessageBoxW(GetPluginWindow(), buf, wasabiApiLangString(IDS_MILKDROP_ERROR, title, sizeof(title)), MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
      return false;
    }
    if (D3D_OK != GetDevice()->CreateVertexDeclaration(g_WfVertDecl, &m_pWfVertDecl)) {
      wasabiApiLangString(IDS_COULD_NOT_CREATE_WF_VERTEX_DECLARATION, buf, sizeof(buf));
      dumpmsg(buf);
      MessageBoxW(GetPluginWindow(), buf, wasabiApiLangString(IDS_MILKDROP_ERROR, title, sizeof(title)), MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
      return false;
    }
    if (D3D_OK != GetDevice()->CreateVertexDeclaration(g_SpriteVertDecl, &m_pSpriteVertDecl)) {
      wasabiApiLangString(IDS_COULD_NOT_CREATE_SPRITE_VERTEX_DECLARATION, buf, sizeof(buf));
      dumpmsg(buf);
      MessageBoxW(GetPluginWindow(), buf, wasabiApiLangString(IDS_MILKDROP_ERROR, title, sizeof(title)), MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
      return false;
    }

    // Load the FALLBACK shaders...
    int PSVersion = m_IsAMD ? m_nMaxPSVersion_DX9 : 2;
    if (!RecompilePShader(m_szDefaultWarpPShaderText, &m_fallbackShaders_ps.warp, SHADER_WARP, true, PSVersion, false)) {
      wchar_t szSM[64];
      switch (m_nMaxPSVersion_DX9) {
        case MD2_PS_2_0:
        case MD2_PS_2_X:
          wasabiApiLangString(IDS_SHADER_MODEL_2, szSM, 64);
          break;
        case MD2_PS_3_0:
          wasabiApiLangString(IDS_SHADER_MODEL_3, szSM, 64);
          break;
        case MD2_PS_4_0:
          wasabiApiLangString(IDS_SHADER_MODEL_4, szSM, 64);
          break;
        default:
          swprintf(szSM, wasabiApiLangString(IDS_UKNOWN_CASE_X), m_nMaxPSVersion_DX9);
          break;
      }
      if (m_nMaxPSVersion_ConfigPanel >= MD2_PS_NONE && m_nMaxPSVersion_DX9 < m_nMaxPSVersion_ConfigPanel)
        swprintf(buf, wasabiApiLangString(IDS_FAILED_TO_COMPILE_PIXEL_SHADERS_USING_X), szSM, PSVersion);
      else
        swprintf(buf, wasabiApiLangString(IDS_FAILED_TO_COMPILE_PIXEL_SHADERS_HARDWARE_MIS_REPORT), szSM, PSVersion);
      dumpmsg(buf);
      MessageBoxW(GetPluginWindow(), buf, wasabiApiLangString(IDS_MILKDROP_ERROR, title, 64), MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
      return false;
    }
    if (!RecompileVShader(m_szDefaultWarpVShaderText, &m_fallbackShaders_vs.warp, SHADER_WARP, true, false)) {
      wasabiApiLangString(IDS_COULD_NOT_COMPILE_FALLBACK_WV_SHADER, buf, sizeof(buf));
      dumpmsg(buf);
      MessageBoxW(GetPluginWindow(), buf, wasabiApiLangString(IDS_MILKDROP_ERROR, title, 64), MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
      return false;
    }
    if (!RecompileVShader(m_szDefaultCompVShaderText, &m_fallbackShaders_vs.comp, SHADER_COMP, true, false)) {
      wasabiApiLangString(IDS_COULD_NOT_COMPILE_FALLBACK_CV_SHADER, buf, sizeof(buf));
      dumpmsg(buf);
      MessageBoxW(GetPluginWindow(), buf, wasabiApiLangString(IDS_MILKDROP_ERROR, title, 64), MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
      return false;
    }

    if (!RecompilePShader(m_szDefaultCompPShaderText, &m_fallbackShaders_ps.comp, SHADER_COMP, true, PSVersion, false)) {
      wasabiApiLangString(IDS_COULD_NOT_COMPILE_FALLBACK_CP_SHADER, buf, sizeof(buf));
      dumpmsg(buf);
      MessageBoxW(GetPluginWindow(), buf, wasabiApiLangString(IDS_MILKDROP_ERROR, title, 64), MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
      return false;
    }

    // Load the BLUR shaders...
    if (!RecompileVShader(m_szBlurVS, &m_BlurShaders[0].vs, SHADER_BLUR, true, false)) {
      wasabiApiLangString(IDS_COULD_NOT_COMPILE_BLUR1_VERTEX_SHADER, buf, sizeof(buf));
      dumpmsg(buf);
      MessageBoxW(GetPluginWindow(), buf, wasabiApiLangString(IDS_MILKDROP_ERROR, title, 64), MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
      return false;
    }

    if (!RecompilePShader(m_szBlurPSX, &m_BlurShaders[0].ps, SHADER_BLUR, true, PSVersion, false)) {
      wasabiApiLangString(IDS_COULD_NOT_COMPILE_BLUR1_PIXEL_SHADER, buf, sizeof(buf));
      dumpmsg(buf);
      MessageBoxW(GetPluginWindow(), buf, wasabiApiLangString(IDS_MILKDROP_ERROR, title, 64), MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
      return false;
    }
    if (!RecompileVShader(m_szBlurVS, &m_BlurShaders[1].vs, SHADER_BLUR, true, false)) {
      wasabiApiLangString(IDS_COULD_NOT_COMPILE_BLUR2_VERTEX_SHADER, buf, sizeof(buf));
      dumpmsg(buf);
      MessageBoxW(GetPluginWindow(), buf, wasabiApiLangString(IDS_MILKDROP_ERROR, title, 64), MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
      return false;
    }

    if (!RecompilePShader(m_szBlurPSY, &m_BlurShaders[1].ps, SHADER_BLUR, true, PSVersion, false)) {
      wasabiApiLangString(IDS_COULD_NOT_COMPILE_BLUR2_PIXEL_SHADER, buf, sizeof(buf));
      dumpmsg(buf);
      MessageBoxW(GetPluginWindow(), buf, wasabiApiLangString(IDS_MILKDROP_ERROR, title, 64), MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
      return false;
    }
  }

  // Compile custom input mix shader
  CompileInputMixShader();

  // create m_lpVS[2]
  {
    int log2texsize = GetNearestPow2Size(GetWidth(), GetHeight());

    // auto-guess texsize
    if (m_bTexSizeWasAutoExact) {
      // note: in windowed mode, the winamp window could be weird sizes,
      //        so the plugin shell now gives us a slightly enlarged size,
      //        which pads it out to the nearest 32x32 block size,
      //        and then on display, it intelligently crops the image.
      //       This is pretty likely to work on non-shitty GPUs.
      //        but some shitty ones will still only do powers of 2!
      //       So if we are running out of video memory here or experience
      //        other problems, though, we can make our VS's smaller;
      //        which will work, although it will lead to stretching.
      m_nTexSizeX = GetWidth();
      m_nTexSizeY = GetHeight();
    } else if (m_bTexSizeWasAutoPow2) {
      m_nTexSizeX = log2texsize;
      m_nTexSizeY = log2texsize;
    }

    // clip texsize by max. from caps
    if ((DWORD)m_nTexSizeX > GetCaps()->MaxTextureWidth && GetCaps()->MaxTextureWidth > 0)
      m_nTexSizeX = GetCaps()->MaxTextureWidth;
    if ((DWORD)m_nTexSizeY > GetCaps()->MaxTextureHeight && GetCaps()->MaxTextureHeight > 0)
      m_nTexSizeY = GetCaps()->MaxTextureHeight;

    // apply canvas stretch
    m_nTexSizeX = (m_nTexSizeX * 100) / nNewCanvasStretch;
    m_nTexSizeY = (m_nTexSizeY * 100) / nNewCanvasStretch;

    // re-compute closest power-of-2 size, now that we've factored in the stretching...
    log2texsize = GetNearestPow2Size(m_nTexSizeX, m_nTexSizeY);
    if (m_bTexSizeWasAutoPow2) {
      m_nTexSizeX = log2texsize;
      m_nTexSizeY = log2texsize;
    }

    // snap to 16x16 blocks
    m_nTexSizeX = ((m_nTexSizeX + 15) / 16) * 16;
    m_nTexSizeY = ((m_nTexSizeY + 15) / 16) * 16;

    // determine format for VS1/VS2
    D3DFORMAT fmt;
    switch (m_nTexBitsPerCh) {
      case 5:
        fmt = D3DFMT_R5G6B5;
        break;
      case 8:
        fmt = D3DFMT_X8R8G8B8;
        break;
      case 10:
        fmt = D3DFMT_A2R10G10B10;
        break;  // D3DFMT_A2W10V10U10 or D3DFMT_A2R10G10B10 or D3DFMT_A2B10G10R10
      case 16:
        fmt = D3DFMT_A16B16G16R16F;
        break;
      case 32:
        fmt = D3DFMT_A32B32G32R32F;
        break;  // FIXME
      default:
        fmt = D3DFMT_X8R8G8B8;
        break;
    }

    // reallocate
    bool bSuccess = false;
    DWORD vs_flags = D3DUSAGE_RENDERTARGET;  // | D3DUSAGE_AUTOGENMIPMAP;//FIXME! (make automipgen optional)
    bool bRevertedBitDepth = false;
    do {
      SafeRelease(m_lpVS[0]);
      SafeRelease(m_lpVS[1]);

      // create VS1
      bSuccess = (GetDevice()->CreateTexture(m_nTexSizeX, m_nTexSizeY, 1, vs_flags, fmt, D3DPOOL_DEFAULT, &m_lpVS[0], NULL) == D3D_OK);
      if (!bSuccess) {
        bSuccess = (GetDevice()->CreateTexture(m_nTexSizeX, m_nTexSizeY, 1, vs_flags, GetBackBufFormat(), D3DPOOL_DEFAULT, &m_lpVS[0], NULL) == D3D_OK);
        if (bSuccess)
          fmt = GetBackBufFormat();
      }

      // create VS2
      if (bSuccess)
        bSuccess = (GetDevice()->CreateTexture(m_nTexSizeX, m_nTexSizeY, 1, vs_flags, fmt, D3DPOOL_DEFAULT, &m_lpVS[1], NULL) == D3D_OK);

      if (!bSuccess) {
        if (m_bTexSizeWasAutoExact) {
          if (m_nTexSizeX > 256 || m_nTexSizeY > 256) {
            m_nTexSizeX /= 2;
            m_nTexSizeY /= 2;
            m_nTexSizeX = ((m_nTexSizeX + 15) / 16) * 16;
            m_nTexSizeY = ((m_nTexSizeY + 15) / 16) * 16;
          } else {
            m_nTexSizeX = log2texsize;
            m_nTexSizeY = log2texsize;
            m_bTexSizeWasAutoExact = false;
            m_bTexSizeWasAutoPow2 = true;
          }
        } else if (m_bTexSizeWasAutoPow2) {
          if (m_nTexSizeX > 256) {
            m_nTexSizeX /= 2;
            m_nTexSizeY /= 2;
          } else
            break;
        }
      }
    } while (!bSuccess);  // && m_nTexSizeX >= 256 && (m_bTexSizeWasAutoExact || m_bTexSizeWasAutoPow2));

    if (!bSuccess) {
      wchar_t buf[2048];
      UINT err_id = IDS_COULD_NOT_CREATE_INTERNAL_CANVAS_TEXTURE_NOT_ENOUGH_VID_MEM;

      if (!(m_bTexSizeWasAutoExact || m_bTexSizeWasAutoPow2))
        err_id = IDS_COULD_NOT_CREATE_INTERNAL_CANVAS_TEXTURE_NOT_ENOUGH_VID_MEM_RECOMMENDATION;

      wasabiApiLangString(err_id, buf, sizeof(buf));
      dumpmsg(buf);
      MessageBoxW(GetPluginWindow(), buf, wasabiApiLangString(IDS_MILKDROP_ERROR, title, 64), MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
      return false;
    } else {
      swprintf(buf, wasabiApiLangString(IDS_SUCCESSFULLY_CREATED_VS0_VS1), m_nTexSizeX, m_nTexSizeY, GetWidth(), GetHeight());
      dumpmsg(buf);
    }

    /*
      if (m_nTexSizeX != GetWidth() || m_nTexSizeY != GetHeight())
    {
          char buf[2048];
      sprintf(buf, "warning - canvas size adjusted from %dx%d to %dx%d.", GetWidth(), GetHeight(), m_nTexSizeX, m_nTexSizeY);
      dumpmsg(buf);
          AddError(buf, 3.2f, ERR_INIT, true);
    }/**/

    // create blur textures w/same format.  A complete mip chain costs 33% more video mem then 1 full-sized VS.
#if (NUM_BLUR_TEX > 0)
    int w = m_nTexSizeX;
    int h = m_nTexSizeY;
    DWORD blurtex_flags = D3DUSAGE_RENDERTARGET;  // | D3DUSAGE_AUTOGENMIPMAP;//FIXME! (make automipgen optional)
    for (int i = 0; i < NUM_BLUR_TEX; i++) {
      // main VS = 1024
      // blur0 = 512
      // blur1 = 256  <-  user sees this as "blur1"
      // blur2 = 128
      // blur3 = 128  <-  user sees this as "blur2"
      // blur4 =  64
      // blur5 =  64  <-  user sees this as "blur3"
      if (!(i & 1) || (i < 2)) {
        w = max(16, w / 2);
        h = max(16, h / 2);
      }
      int w2 = ((w + 3) / 16) * 16;
      int h2 = ((h + 3) / 4) * 4;
      bSuccess = (GetDevice()->CreateTexture(w2, h2, 1, blurtex_flags, fmt, D3DPOOL_DEFAULT, &m_lpBlur[i], NULL) == D3D_OK);
      m_nBlurTexW[i] = w2;
      m_nBlurTexH[i] = h2;
      if (!bSuccess) {
        m_nBlurTexW[i] = 1;
        m_nBlurTexH[i] = 1;
        MessageBoxW(GetPluginWindow(), wasabiApiLangString(IDS_ERROR_CREATING_BLUR_TEXTURES, buf, sizeof(buf)),
                    wasabiApiLangString(IDS_MILKDROP_WARNING, title, sizeof(title)), MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
        break;
      }

      // add it to m_textures[].
      TexInfo x;
      swprintf(x.texname, L"blur%d%s", i / 2 + 1, (i % 2) ? L"" : L"doNOTuseME");
      x.texptr = m_lpBlur[i];
      // x.texsize_param = NULL;
      x.w = w2;
      x.h = h2;
      x.d = 1;
      x.bEvictable = false;
      x.nAge = m_nPresetsLoadedTotal;
      x.nSizeInBytes = 0;
      m_textures.push_back(x);
    }
#endif
  }

  m_fAspectX = (m_nTexSizeY > m_nTexSizeX) ? m_nTexSizeX / (float)m_nTexSizeY : 1.0f;
  m_fAspectY = (m_nTexSizeX > m_nTexSizeY) ? m_nTexSizeY / (float)m_nTexSizeX : 1.0f;
  m_fInvAspectX = 1.0f / m_fAspectX;
  m_fInvAspectY = 1.0f / m_fAspectY;

  // Create video capture texture
  if (m_pVideoCaptureTexture) {
    m_pVideoCaptureTexture->Release();
    m_pVideoCaptureTexture = nullptr;
  }
  // Use the stored video capture dimensions, not the canvas size
  LPDIRECT3DDEVICE9EX pDev = GetDevice();
  if (pDev) {
    pDev->CreateTexture(m_nVideoCaptureWidth, m_nVideoCaptureHeight, 1, D3DUSAGE_DYNAMIC, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &m_pVideoCaptureTexture, NULL);

    // Create FFT spectrum texture (row0=smoothed, row1=peak hold)
    if (pDev->CreateTexture(m_nFFTShaderBins, 2, 1, D3DUSAGE_DYNAMIC, D3DFMT_R32F, D3DPOOL_DEFAULT, &m_lpFFTTexture, NULL) == D3D_OK)
      milkwave->LogInfo(L"FFT texture created successfully");
    else
      milkwave->LogInfo(L"Failed to create FFT texture (D3DFMT_R32F not supported?)");
  }

  // Restart video capture if it was enabled
  if (m_bVideoInputEnabled && m_pVideoCapture && m_nVideoDeviceIndex >= 0) {
    m_pVideoCapture->Stop();
    if (!m_pVideoCapture->Start()) {
      m_bVideoInputEnabled = false;
      milkwave->LogInfo(L"Failed to restart video capture after device reset");
    } else {
      milkwave->LogInfo(L"Successfully restarted video capture after device reset");
    }
  }

  // BUILD VERTEX LIST for final composite blit
  //   note the +0.5-texel offset!
  //   (otherwise, a 1-pixel-wide line of the image would wrap at the top and left edges).
  ZeroMemory(m_comp_verts, sizeof(MYVERTEX) * FCGSX * FCGSY);
  // float fOnePlusInvWidth  = 1.0f + 1.0f/(float)GetWidth();
  // float fOnePlusInvHeight = 1.0f + 1.0f/(float)GetHeight();
  float fHalfTexelW = 0.5f / (float)GetWidth();  // 2.5: 2 pixels bad @ bottom right
  float fHalfTexelH = 0.5f / (float)GetHeight();
  float fDivX = 1.0f / (float)(FCGSX - 2);
  float fDivY = 1.0f / (float)(FCGSY - 2);
  for (int j = 0; j < FCGSY; j++) {
    int j2 = j - j / (FCGSY / 2);
    float v = j2 * fDivY;
    v = SquishToCenter(v, 3.0f);
    float sy = -((v - fHalfTexelH) * 2 - 1);  // fOnePlusInvHeight*v*2-1;
    for (int i = 0; i < FCGSX; i++) {
      int i2 = i - i / (FCGSX / 2);
      float u = i2 * fDivX;
      u = SquishToCenter(u, 3.0f);
      float sx = (u - fHalfTexelW) * 2 - 1;  // fOnePlusInvWidth*u*2-1;
      MYVERTEX* p = &m_comp_verts[i + j * FCGSX];
      p->x = sx;
      p->y = sy;
      p->z = 0;
      float rad, ang;
      UvToMathSpace(u, v, &rad, &ang);
      // fix-ups:
      if (i == FCGSX / 2 - 1) {
        if (j < FCGSY / 2 - 1)
          ang = 3.1415926535898f * 1.5f;
        else if (j == FCGSY / 2 - 1)
          ang = 3.1415926535898f * 1.25f;
        else if (j == FCGSY / 2)
          ang = 3.1415926535898f * 0.75f;
        else
          ang = 3.1415926535898f * 0.5f;
      } else if (i == FCGSX / 2) {
        if (j < FCGSY / 2 - 1)
          ang = 3.1415926535898f * 1.5f;
        else if (j == FCGSY / 2 - 1)
          ang = 3.1415926535898f * 1.75f;
        else if (j == FCGSY / 2)
          ang = 3.1415926535898f * 0.25f;
        else
          ang = 3.1415926535898f * 0.5f;
      } else if (j == FCGSY / 2 - 1) {
        if (i < FCGSX / 2 - 1)
          ang = 3.1415926535898f * 1.0f;
        else if (i == FCGSX / 2 - 1)
          ang = 3.1415926535898f * 1.25f;
        else if (i == FCGSX / 2)
          ang = 3.1415926535898f * 1.75f;
        else
          ang = 3.1415926535898f * 2.0f;
      } else if (j == FCGSY / 2) {
        if (i < FCGSX / 2 - 1)
          ang = 3.1415926535898f * 1.0f;
        else if (i == FCGSX / 2 - 1)
          ang = 3.1415926535898f * 0.75f;
        else if (i == FCGSX / 2)
          ang = 3.1415926535898f * 0.25f;
        else
          ang = 3.1415926535898f * 0.0f;
      }
      p->tu = u;
      p->tv = v;
      // p->tu_orig = u;
      // p->tv_orig = v;
      p->rad = rad;
      p->ang = ang;
      p->Diffuse = 0xFFFFFFFF;
    }
  }

  // build index list for final composite blit -
  // order should be friendly for interpolation of 'ang' value!
  int* cur_index = &m_comp_indices[0];
  int y;
  for (y = 0; y < FCGSY - 1; y++) {
    if (y == FCGSY / 2 - 1)
      continue;
    for (int x = 0; x < FCGSX - 1; x++) {
      if (x == FCGSX / 2 - 1)
        continue;
      bool left_half = (x < FCGSX / 2);
      bool top_half = (y < FCGSY / 2);
      bool center_4 = ((x == FCGSX / 2 || x == FCGSX / 2 - 1) && (y == FCGSY / 2 || y == FCGSY / 2 - 1));

      if (((int)left_half + (int)top_half + (int)center_4) % 2) {
        *(cur_index + 0) = (y)*FCGSX + (x);
        *(cur_index + 1) = (y)*FCGSX + (x + 1);
        *(cur_index + 2) = (y + 1) * FCGSX + (x + 1);
        *(cur_index + 3) = (y + 1) * FCGSX + (x + 1);
        *(cur_index + 4) = (y + 1) * FCGSX + (x);
        *(cur_index + 5) = (y)*FCGSX + (x);
      } else {
        *(cur_index + 0) = (y + 1) * FCGSX + (x);
        *(cur_index + 1) = (y)*FCGSX + (x);
        *(cur_index + 2) = (y)*FCGSX + (x + 1);
        *(cur_index + 3) = (y)*FCGSX + (x + 1);
        *(cur_index + 4) = (y + 1) * FCGSX + (x + 1);
        *(cur_index + 5) = (y + 1) * FCGSX + (x);
      }
      cur_index += 6;
    }
  }

  // -----------------

  /*if (m_bFixSlowText && !m_bSeparateTextWindow)
  {
        if (D3DXCreateTexture(GetDevice(), GetWidth(), GetHeight(), 1, D3DUSAGE_RENDERTARGET, GetBackBufFormat(), D3DPOOL_DEFAULT, &m_lpDDSText) != D3D_OK)
    {
            char buf[2048];
      dumpmsg("Init: -WARNING-:");
      sprintf(buf, "WARNING: Not enough video memory to make a dedicated text surface; \rtext will still be drawn directly to the back buffer.\r\rTo avoid seeing this error again, uncheck the 'fix slow text' option.");
      dumpmsg(buf);
      if (!m_bWarningsDisabled)
        MessageBox(GetPluginWindow(), buf, "WARNING", MB_OK|MB_SETFOREGROUND|MB_TOPMOST );
      m_lpDDSText = NULL;
    }
  }*/

  // -----------------

  // reallocate the texture for font titles + custom msgs (m_lpDDSTitle)
  {
    m_nTitleTexSizeX = max(m_nTexSizeX, m_nTexSizeY);
    m_nTitleTexSizeY = m_nTitleTexSizeX / 4;

    // m_nTitleTexSizeX = max(m_nTexSizeX, m_nTexSizeY);
    // m_nTitleTexSizeY = m_nTitleTexSizeX / 4;

    // int sizeX = m_nTitleTexSizeX;
    // int sizeY = m_nTitleTexSizeY;

    // dumpmsg("Init: [re]allocating title surface");

    // [DEPRECATED as of transition to dx9:]
    // We could just create one title surface, but this is a problem because many
    // systems can only call DrawText on DDSCAPS_OFFSCREENPLAIN surfaces, and can NOT
    // draw text on a DDSCAPS_TEXTURE surface (it comes out garbled).
    // So, we create one of each; we draw the text to the DDSCAPS_OFFSCREENPLAIN surface
    // (m_lpDDSTitle[1]), then we blit that (once) to the DDSCAPS_TEXTURE surface
    // (m_lpDDSTitle[0]), which can then be drawn onto the screen on polys.

    HRESULT hr;
    for (int i = 0; i < NUM_SUPERTEXTS; i++) {
      m_nTitleTexSizeX = max(m_nTexSizeX, m_nTexSizeY);
      m_nTitleTexSizeY = m_nTitleTexSizeX / 4;
      do {
        hr = D3DXCreateTexture(GetDevice(), m_nTitleTexSizeX, m_nTitleTexSizeY, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_lpDDSTitle[i]);
        if (hr != D3D_OK) {
          if (m_nTitleTexSizeY < m_nTitleTexSizeX) {
            m_nTitleTexSizeY *= 2;
          } else {
            m_nTitleTexSizeX /= 2;
            m_nTitleTexSizeY /= 2;
          }
        }
      } while (hr != D3D_OK && m_nTitleTexSizeX > 16);
    }

    if (hr != D3D_OK) {
      // dumpmsg("Init: -WARNING-: Title texture could not be created!");
      for (int i = 0; i < NUM_SUPERTEXTS; i++) {
        m_lpDDSTitle[i] = NULL;
      }
      // SafeRelease(m_lpDDSTitle);
      // return true;
    } else {
      // sprintf(buf, "Init: title texture size is %dx%d (ideal size was %dx%d)", m_nTitleTexSizeX, m_nTitleTexSizeY, m_nTexSize, m_nTexSize/4);
      // dumpmsg(buf);
      for (int i = 0; i < NUM_SUPERTEXTS; i++) {
        if (m_supertexts[i].fStartTime != -1.0f) {
          m_supertexts[i].bRedrawSuperText = true;
        }
      }
    }
  }

  // -----------------

  // create 'm_gdi_title_font_doublesize'
  int songtitle_font_size = m_fontinfo[SONGTITLE_FONT].nSize * m_nTitleTexSizeX / 256;
  if (songtitle_font_size < 6) songtitle_font_size = 6;
  if (!(m_gdi_title_font_doublesize = CreateFontW(songtitle_font_size, 0, 0, 0, m_fontinfo[SONGTITLE_FONT].bBold ? 900 : 400,
                                                  m_fontinfo[SONGTITLE_FONT].bItalic, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, m_fontinfo[SONGTITLE_FONT].bAntiAliased ? ANTIALIASED_QUALITY : DEFAULT_QUALITY, DEFAULT_PITCH, m_fontinfo[SONGTITLE_FONT].szFace))) {
    MessageBoxW(NULL, wasabiApiLangString(IDS_ERROR_CREATING_DOUBLE_SIZED_GDI_TITLE_FONT),
                wasabiApiLangString(IDS_MILKDROP_ERROR, title, sizeof(title)),
                MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
    return false;
  }

  if (D3DXCreateFontW(GetDevice(),
                      songtitle_font_size,
                      0,
                      m_fontinfo[SONGTITLE_FONT].bBold ? 900 : 400,
                      1,
                      m_fontinfo[SONGTITLE_FONT].bItalic,
                      DEFAULT_CHARSET,
                      OUT_DEFAULT_PRECIS,
                      ANTIALIASED_QUALITY,  // DEFAULT_QUALITY,
                      DEFAULT_PITCH,
                      m_fontinfo[SONGTITLE_FONT].szFace,
                      &m_d3dx_title_font_doublesize) != D3D_OK) {
    MessageBoxW(GetPluginWindow(), wasabiApiLangString(IDS_ERROR_CREATING_DOUBLE_SIZED_D3DX_TITLE_FONT),
                wasabiApiLangString(IDS_MILKDROP_ERROR, title, sizeof(title)), MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
    return false;
  }

  // -----------------

  m_texmgr.Init(GetDevice());

  // dumpmsg("Init: mesh allocation");
  m_verts = new MYVERTEX[(m_nGridX + 1) * (m_nGridY + 1)];
  m_verts_temp = new MYVERTEX[(m_nGridX + 2) * 4];
  m_vertinfo = new td_vertinfo[(m_nGridX + 1) * (m_nGridY + 1)];
  m_indices_strip = new int[(m_nGridX + 2) * (m_nGridY * 2)];
  m_indices_list = new int[m_nGridX * m_nGridY * 6];
  if (!m_verts || !m_vertinfo) {
    swprintf(buf, L"couldn't allocate mesh - out of memory");
    dumpmsg(buf);
    MessageBoxW(GetPluginWindow(), buf, wasabiApiLangString(IDS_MILKDROP_ERROR, title, 64), MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
    return false;
  }

  int nVert = 0;
  float texel_offset_x = 0.5f / (float)m_nTexSizeX;
  float texel_offset_y = 0.5f / (float)m_nTexSizeY;
  for (y = 0; y <= m_nGridY; y++) {
    for (int x = 0; x <= m_nGridX; x++) {
      // precompute x,y,z
      m_verts[nVert].x = x / (float)m_nGridX * 2.0f - 1.0f;
      m_verts[nVert].y = y / (float)m_nGridY * 2.0f - 1.0f;
      m_verts[nVert].z = 0.0f;

      // precompute rad, ang, being conscious of aspect ratio
      if (m_bScreenDependentRenderMode)
        m_vertinfo[nVert].rad = sqrtf(m_verts[nVert].x * m_verts[nVert].x + m_verts[nVert].y * m_verts[nVert].y);
      else
        m_vertinfo[nVert].rad = sqrtf(m_verts[nVert].x * m_verts[nVert].x * m_fAspectX * m_fAspectX + m_verts[nVert].y * m_verts[nVert].y * m_fAspectY * m_fAspectY);

      if (y == m_nGridY / 2 && x == m_nGridX / 2)
        m_vertinfo[nVert].ang = 0.0f;
      else if (m_bScreenDependentRenderMode)
        m_vertinfo[nVert].ang = atan2f(m_verts[nVert].y, m_verts[nVert].x);
      else
        m_vertinfo[nVert].ang = atan2f(m_verts[nVert].y * m_fAspectY, m_verts[nVert].x * m_fAspectX);

      m_vertinfo[nVert].a = 1;
      m_vertinfo[nVert].c = 0;

      m_verts[nVert].rad = m_vertinfo[nVert].rad;
      m_verts[nVert].ang = m_vertinfo[nVert].ang;
      m_verts[nVert].tu_orig = m_verts[nVert].x * 0.5f + 0.5f + texel_offset_x;
      m_verts[nVert].tv_orig = -m_verts[nVert].y * 0.5f + 0.5f + texel_offset_y;

      nVert++;
    }
  }

  // .milk2: regenerate blend pattern after grid resize so the spatial blend survives resolution changes
  if (m_bMilk2PermanentBlend && m_nMilk2MixType >= 0) {
    int savedMixType = m_nMixType;
    m_nMixType = m_nMilk2MixType;
    srand(m_nMilk2PatternSeed);
    bool bSavedLoading = m_bLoadingMilk2;
    m_bLoadingMilk2 = true;  // Temporary flag so RandomizeBlendPattern reads the MILK2 variables correctly
    RandomizeBlendPattern();
    m_bLoadingMilk2 = bSavedLoading;
    srand((unsigned int)GetTickCount());
    m_nMixType = savedMixType;
  }

  // generate triangle strips for the 4 quadrants.
  // each quadrant has m_nGridY/2 strips.
  // each strip has m_nGridX+2 *points* in it, or m_nGridX/2 polygons.
  int xref, yref;
  int nVert_strip = 0;
  for (int quadrant = 0; quadrant < 4; quadrant++) {
    for (int slice = 0; slice < m_nGridY / 2; slice++) {
      for (int i = 0; i < m_nGridX + 2; i++) {
        // quadrants:	2 3
        //				0 1
        xref = i / 2;
        yref = (i % 2) + slice;

        if (quadrant & 1)
          xref = m_nGridX - xref;
        if (quadrant & 2)
          yref = m_nGridY - yref;

        int v = xref + (yref) * (m_nGridX + 1);

        m_indices_strip[nVert_strip++] = v;
      }
    }
  }

  // also generate triangle lists for drawing the main warp mesh.
  int nVert_list = 0;
  for (int quadrant = 0; quadrant < 4; quadrant++) {
    for (int slice = 0; slice < m_nGridY / 2; slice++) {
      for (int i = 0; i < m_nGridX / 2; i++) {
        // quadrants:	2 3
        //				0 1
        xref = i;
        yref = slice;

        if (quadrant & 1)
          xref = m_nGridX - 1 - xref;
        if (quadrant & 2)
          yref = m_nGridY - 1 - yref;

        int v = xref + (yref) * (m_nGridX + 1);

        m_indices_list[nVert_list++] = v;
        m_indices_list[nVert_list++] = v + 1;
        m_indices_list[nVert_list++] = v + m_nGridX + 1;
        m_indices_list[nVert_list++] = v + 1;
        m_indices_list[nVert_list++] = v + m_nGridX + 1;
        m_indices_list[nVert_list++] = v + m_nGridX + 1 + 1;
      }
    }
  }

  // GENERATED TEXTURES FOR SHADERS
  //-------------------------------------
  if (m_nMaxPSVersion > 0) {
    // Generate noise textures
    if (!AddNoiseTex(L"noise_lq", 256, 1)) return false;
    if (!AddNoiseTex(L"noise_lq_lite", 32, 1)) return false;
    if (!AddNoiseTex(L"noise_mq", 256, 4)) return false;
    if (!AddNoiseTex(L"noise_hq", 256, 8)) return false;

    if (!AddNoiseVol(L"noisevol_lq", 32, 1)) return false;
    if (!AddNoiseVol(L"noisevol_hq", 32, 4)) return false;
  }

  if (!m_bInitialPresetSelected) {
    UpdatePresetList(true);  //...just does its initial burst!
    if (m_bEnablePresetStartup && wcslen(m_szPresetStartup) > 0) {
      LoadPreset(m_szPresetStartup, 0.0f);

      std::wstring message(m_szPresetStartup);
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

      // try to set the current preset index
      for (size_t i = 0; i < m_presets.size(); i++) {
        if (wcscmp(m_presets[i].szFilename.c_str(), sFilename.c_str()) == 0) {
          m_nCurrentPreset = (int)i;
          break;
        }
      }
    } else {
      LoadRandomPreset(0.0f);
    }
    if (m_bAutoLockPresetWhenNoMusic)
      m_bPresetLockedByUser = false;
    m_bInitialPresetSelected = true;
  } else {
    LoadShaders(&m_shaders, m_pState, false, false);  // Also force-load the shaders - otherwise they'd only get compiled on a preset switch.

    // .milk2: restore permanent blend state after resize
    // CleanUpMyDX9Stuff() sets m_bBlending=false and clears m_OldShaders,
    // so we must re-arm the blend and reload the old preset's shaders.
    if (m_bMilk2PermanentBlend) {
      m_pState->m_bBlending = true;
      m_pState->m_fBlendProgress = m_fMilk2FrozenProgress;
      LoadShaders(&m_OldShaders, m_pOldState, false, false);

      // Re-launch milk2 sprites: CleanUpMyDX9Stuff() calls m_texmgr.Finish()
      // which destroys all sprite textures. Re-create them now.
      if (m_nMilk2SpriteCount > 0)
        LaunchMilk2Sprites();
    }
  }

  return true;
}

bool CPlugin::AddNoiseTex(const wchar_t* szTexName, int size, int zoom_factor) {
  // size = width & height of the texture;
  // zoom_factor = how zoomed-in the texture features should be.
  //           1 = random noise
  //           2 = smoothed (interp)
  //           4/8/16... = cubic interp.

  wchar_t buf[2048], title[64];

  // Synthesize noise texture(s)
  LPDIRECT3DTEXTURE9 pNoiseTex = NULL;
  // try twice - once with mips, once without.
  for (int i = 0; i < 2; i++) {
    if (D3D_OK != GetDevice()->CreateTexture(size, size, i, D3DUSAGE_DYNAMIC | (i ? 0 : D3DUSAGE_AUTOGENMIPMAP), D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &pNoiseTex, NULL)) {
      if (i == 1) {
        wasabiApiLangString(IDS_COULD_NOT_CREATE_NOISE_TEXTURE, buf, sizeof(buf));
        dumpmsg(buf);
        MessageBoxW(GetPluginWindow(), buf, wasabiApiLangString(IDS_MILKDROP_ERROR, title, 64), MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
        return false;
      }
    } else
      break;
  }

  D3DLOCKED_RECT r;
  if (D3D_OK != pNoiseTex->LockRect(0, &r, NULL, D3DLOCK_DISCARD)) {
    wasabiApiLangString(IDS_COULD_NOT_LOCK_NOISE_TEXTURE, buf, sizeof(buf));
    dumpmsg(buf);
    MessageBoxW(GetPluginWindow(), buf, wasabiApiLangString(IDS_MILKDROP_ERROR, title, 64), MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
    return false;
  }

  if (r.Pitch < size * 4) {
    wasabiApiLangString(IDS_NOISE_TEXTURE_BYTE_LAYOUT_NOT_RECOGNISED, buf, sizeof(buf));
    dumpmsg(buf);
    MessageBoxW(GetPluginWindow(), buf, wasabiApiLangString(IDS_MILKDROP_ERROR, title, 64), MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
    return false;
  }

  // write to the bits...
  DWORD* dst = (DWORD*)r.pBits;
  int dwords_per_line = r.Pitch / sizeof(DWORD);
  int RANGE = (zoom_factor > 1) ? 216 : 256;
  for (int y = 0; y < size; y++) {
    LARGE_INTEGER q;
    QueryPerformanceCounter(&q);
    srand(q.LowPart ^ q.HighPart ^ rand());
    int x;
    for (x = 0; x < size; x++) {
      dst[x] = (((DWORD)(rand() % RANGE) + RANGE / 2) << 24) |
               (((DWORD)(rand() % RANGE) + RANGE / 2) << 16) |
               (((DWORD)(rand() % RANGE) + RANGE / 2) << 8) |
               (((DWORD)(rand() % RANGE) + RANGE / 2));
    }
    // swap some pixels randomly, to improve 'randomness'
    for (x = 0; x < size; x++) {
      int x1 = (rand() ^ q.LowPart) % size;
      int x2 = (rand() ^ q.HighPart) % size;
      DWORD temp = dst[x2];
      dst[x2] = dst[x1];
      dst[x1] = temp;
    }
    dst += dwords_per_line;
  }

  // smoothing
  if (zoom_factor > 1) {
    // first go ACROSS, blending cubically on X, but only on the main lines.
    DWORD* dst = (DWORD*)r.pBits;
    for (int y = 0; y < size; y += zoom_factor)
      for (int x = 0; x < size; x++)
        if (x % zoom_factor) {
          int base_x = (x / zoom_factor) * zoom_factor + size;
          int base_y = y * dwords_per_line;
          DWORD y0 = dst[base_y + ((base_x - zoom_factor) % size)];
          DWORD y1 = dst[base_y + ((base_x) % size)];
          DWORD y2 = dst[base_y + ((base_x + zoom_factor) % size)];
          DWORD y3 = dst[base_y + ((base_x + zoom_factor * 2) % size)];

          float t = (x % zoom_factor) / (float)zoom_factor;

          DWORD result = dwCubicInterpolate(y0, y1, y2, y3, t);

          dst[y * dwords_per_line + x] = result;
        }

    // next go down, doing cubic interp along Y, on every line.
    for (int x = 0; x < size; x++)
      for (int y = 0; y < size; y++)
        if (y % zoom_factor) {
          int base_y = (y / zoom_factor) * zoom_factor + size;
          DWORD y0 = dst[((base_y - zoom_factor) % size) * dwords_per_line + x];
          DWORD y1 = dst[((base_y) % size) * dwords_per_line + x];
          DWORD y2 = dst[((base_y + zoom_factor) % size) * dwords_per_line + x];
          DWORD y3 = dst[((base_y + zoom_factor * 2) % size) * dwords_per_line + x];

          float t = (y % zoom_factor) / (float)zoom_factor;

          DWORD result = dwCubicInterpolate(y0, y1, y2, y3, t);

          dst[y * dwords_per_line + x] = result;
        }
  }

  // unlock texture
  pNoiseTex->UnlockRect(0);

  // add it to m_textures[].
  TexInfo x;
  lstrcpyW(x.texname, szTexName);
  x.texptr = pNoiseTex;
  // x.texsize_param = NULL;
  x.w = size;
  x.h = size;
  x.d = 1;
  x.bEvictable = false;
  x.nAge = m_nPresetsLoadedTotal;
  x.nSizeInBytes = 0;
  m_textures.push_back(x);

  return true;
}

bool CPlugin::AddNoiseVol(const wchar_t* szTexName, int size, int zoom_factor) {
  // size = width & height & depth of the texture;
  // zoom_factor = how zoomed-in the texture features should be.
  //           1 = random noise
  //           2 = smoothed (interp)
  //           4/8/16... = cubic interp.

  wchar_t buf[2048], title[64];

  // Synthesize noise texture(s)
  LPDIRECT3DVOLUMETEXTURE9 pNoiseTex = NULL;
  // try twice - once with mips, once without.
  // NO, TRY JUST ONCE - DX9 doesn't do auto mipgen w/volume textures.  (Debug runtime complains.)
  for (int i = 1; i < 2; i++) {
    if (D3D_OK != GetDevice()->CreateVolumeTexture(size, size, size, i, D3DUSAGE_DYNAMIC | (i ? 0 : D3DUSAGE_AUTOGENMIPMAP), D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &pNoiseTex, NULL)) {
      if (i == 1) {
        wasabiApiLangString(IDS_COULD_NOT_CREATE_3D_NOISE_TEXTURE, buf, sizeof(buf));
        dumpmsg(buf);
        MessageBoxW(GetPluginWindow(), buf, wasabiApiLangString(IDS_MILKDROP_ERROR, title, 64), MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
        return false;
      }
    } else
      break;
  }
  D3DLOCKED_BOX r;
  if (D3D_OK != pNoiseTex->LockBox(0, &r, NULL, D3DLOCK_DISCARD)) {
    wasabiApiLangString(IDS_COULD_NOT_LOCK_3D_NOISE_TEXTURE, buf, sizeof(buf));
    dumpmsg(buf);
    MessageBoxW(GetPluginWindow(), buf, wasabiApiLangString(IDS_MILKDROP_ERROR, title, 64), MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
    return false;
  }
  if (r.RowPitch < size * 4 || r.SlicePitch < size * size * 4) {
    wasabiApiLangString(IDS_3D_NOISE_TEXTURE_BYTE_LAYOUT_NOT_RECOGNISED, buf, sizeof(buf));
    dumpmsg(buf);
    MessageBoxW(GetPluginWindow(), buf, wasabiApiLangString(IDS_MILKDROP_ERROR, title, 64), MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
    return false;
  }
  // write to the bits...
  int dwords_per_slice = r.SlicePitch / sizeof(DWORD);
  int dwords_per_line = r.RowPitch / sizeof(DWORD);
  int RANGE = (zoom_factor > 1) ? 216 : 256;
  for (int z = 0; z < size; z++) {
    DWORD* dst = (DWORD*)r.pBits + z * dwords_per_slice;
    for (int y = 0; y < size; y++) {
      LARGE_INTEGER q;
      QueryPerformanceCounter(&q);
      srand(q.LowPart ^ q.HighPart ^ rand());
      int x;
      for (x = 0; x < size; x++) {
        dst[x] = (((DWORD)(rand() % RANGE) + RANGE / 2) << 24) |
                 (((DWORD)(rand() % RANGE) + RANGE / 2) << 16) |
                 (((DWORD)(rand() % RANGE) + RANGE / 2) << 8) |
                 (((DWORD)(rand() % RANGE) + RANGE / 2));
      }
      // swap some pixels randomly, to improve 'randomness'
      for (x = 0; x < size; x++) {
        int x1 = (rand() ^ q.LowPart) % size;
        int x2 = (rand() ^ q.HighPart) % size;
        DWORD temp = dst[x2];
        dst[x2] = dst[x1];
        dst[x1] = temp;
      }
      dst += dwords_per_line;
    }
  }

  // smoothing
  if (zoom_factor > 1) {
    // first go ACROSS, blending cubically on X, but only on the main lines.
    DWORD* dst = (DWORD*)r.pBits;
    int z;
    for (z = 0; z < size; z += zoom_factor)
      for (int y = 0; y < size; y += zoom_factor)
        for (int x = 0; x < size; x++)
          if (x % zoom_factor) {
            int base_x = (x / zoom_factor) * zoom_factor + size;
            int base_y = z * dwords_per_slice + y * dwords_per_line;
            DWORD y0 = dst[base_y + ((base_x - zoom_factor) % size)];
            DWORD y1 = dst[base_y + ((base_x) % size)];
            DWORD y2 = dst[base_y + ((base_x + zoom_factor) % size)];
            DWORD y3 = dst[base_y + ((base_x + zoom_factor * 2) % size)];

            float t = (x % zoom_factor) / (float)zoom_factor;

            DWORD result = dwCubicInterpolate(y0, y1, y2, y3, t);

            dst[z * dwords_per_slice + y * dwords_per_line + x] = result;
          }

    // next go down, doing cubic interp along Y, on the main slices.
    for (z = 0; z < size; z += zoom_factor)
      for (int x = 0; x < size; x++)
        for (int y = 0; y < size; y++)
          if (y % zoom_factor) {
            int base_y = (y / zoom_factor) * zoom_factor + size;
            int base_z = z * dwords_per_slice;
            DWORD y0 = dst[((base_y - zoom_factor) % size) * dwords_per_line + base_z + x];
            DWORD y1 = dst[((base_y) % size) * dwords_per_line + base_z + x];
            DWORD y2 = dst[((base_y + zoom_factor) % size) * dwords_per_line + base_z + x];
            DWORD y3 = dst[((base_y + zoom_factor * 2) % size) * dwords_per_line + base_z + x];

            float t = (y % zoom_factor) / (float)zoom_factor;

            DWORD result = dwCubicInterpolate(y0, y1, y2, y3, t);

            dst[y * dwords_per_line + base_z + x] = result;
          }

    // next go through, doing cubic interp along Z, everywhere.
    for (int x = 0; x < size; x++)
      for (int y = 0; y < size; y++)
        for (int z = 0; z < size; z++)
          if (z % zoom_factor) {
            int base_y = y * dwords_per_line;
            int base_z = (z / zoom_factor) * zoom_factor + size;
            DWORD y0 = dst[((base_z - zoom_factor) % size) * dwords_per_slice + base_y + x];
            DWORD y1 = dst[((base_z) % size) * dwords_per_slice + base_y + x];
            DWORD y2 = dst[((base_z + zoom_factor) % size) * dwords_per_slice + base_y + x];
            DWORD y3 = dst[((base_z + zoom_factor * 2) % size) * dwords_per_slice + base_y + x];

            float t = (z % zoom_factor) / (float)zoom_factor;

            DWORD result = dwCubicInterpolate(y0, y1, y2, y3, t);

            dst[z * dwords_per_slice + base_y + x] = result;
          }
  }

  // unlock texture
  pNoiseTex->UnlockBox(0);

  // add it to m_textures[].
  TexInfo x;
  lstrcpyW(x.texname, szTexName);
  x.texptr = pNoiseTex;
  // x.texsize_param = NULL;
  x.w = size;
  x.h = size;
  x.d = size;
  x.bEvictable = false;
  x.nAge = m_nPresetsLoadedTotal;
  x.nSizeInBytes = 0;
  m_textures.push_back(x);

  return true;
}

void CPlugin::RecreateLyricsFont(float scale) {
  SafeRelease(m_lyricsFontObject);
  if (scale < 0.0f) scale = m_lyricsCurrentFontScale >= 0.0f ? m_lyricsCurrentFontScale : 1.0f;
  if (scale < 0.1f) scale = 0.1f;
  if (scale > 10.0f) scale = 10.0f;
  m_lyricsCurrentFontScale = scale;
  // Round (not truncate) so a scale passed in as targetSize/sizePerScale maps
  // back to exactly targetSize — prevents off-by-one drift in the recreation
  // hysteresis used by the lyrics render loop.
  int lyricsFontSize = (int)(m_lyricsFontSize * scale * m_fRenderQuality + 0.5f);
  if (lyricsFontSize < 8) lyricsFontSize = 8;
  if (lyricsFontSize > 256) lyricsFontSize = 256;
  const wchar_t* lyricsFontFace = m_lyricsFont[0] ? m_lyricsFont : L"Segoe UI";
  const UINT fontWeight = m_lyricsFontBold ? FW_BOLD : FW_NORMAL;
  const BOOL italic = m_lyricsFontItalic ? TRUE : FALSE;
  const DWORD quality = m_lyricsFontAA ? ANTIALIASED_QUALITY : NONANTIALIASED_QUALITY;
  D3DXCreateFontW(GetDevice(), lyricsFontSize, lyricsFontSize * 4 / 10, fontWeight, 1, italic,
                  DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, quality, DEFAULT_PITCH,
                  lyricsFontFace, &m_lyricsFontObject);
}

void CPlugin::RecreateLyricsMeasureFont(float targetScale) {
  SafeRelease(m_lyricsMeasureFontObject);
  if (targetScale < 0.0f) targetScale = m_lyricsMeasureFontScale >= 0.0f ? m_lyricsMeasureFontScale : 1.0f;
  if (targetScale < 0.1f) targetScale = 0.1f;
  if (targetScale > 10.0f) targetScale = 10.0f;
  m_lyricsMeasureFontScale = targetScale;
  int lyricsFontSize = (int)(m_lyricsFontSize * targetScale * m_fRenderQuality + 0.5f);
  if (lyricsFontSize < 8) lyricsFontSize = 8;
  if (lyricsFontSize > 256) lyricsFontSize = 256;
  const wchar_t* lyricsFontFace = m_lyricsFont[0] ? m_lyricsFont : L"Segoe UI";
  const UINT fontWeight = m_lyricsFontBold ? FW_BOLD : FW_NORMAL;
  const BOOL italic = m_lyricsFontItalic ? TRUE : FALSE;
  const DWORD quality = m_lyricsFontAA ? ANTIALIASED_QUALITY : NONANTIALIASED_QUALITY;
  D3DXCreateFontW(GetDevice(), lyricsFontSize, lyricsFontSize * 4 / 10, fontWeight, 1, italic,
                  DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, quality, DEFAULT_PITCH,
                  lyricsFontFace, &m_lyricsMeasureFontObject);
}

void CPlugin::InvalidateLyricsWrapCache() {
  m_lyricsWrapCacheText.clear();
  m_lyricsWrapCacheLines.clear();
}

void CPlugin::CleanUpMyDX9Stuff(int final_cleanup) {
  // Clean up all your DX9 and D3DX textures, fonts, buffers, etc. here.
  // EVERYTHING CREATED IN ALLOCATEMYDX9STUFF() SHOULD BE CLEANED UP HERE.
  // The input parameter, 'final_cleanup', will be 0 if this is
  //   a routine cleanup (part of a window resize or switch between
  //   fullscr/windowed modes), or 1 if this is the final cleanup
  //   and the plugin is exiting.  Note that even if it is a routine
  //   cleanup, *you still have to release ALL your DirectX stuff,
  //   because the DirectX device is being destroyed and recreated!*
  // Also set all the pointers back to NULL;
  //   this is important because if we go to reallocate the DX9
  //   stuff later, and something fails, then CleanUp will get called,
  //   but it will then be trying to clean up invalid pointers.)
  // The SafeRelease() and SafeDelete() macros make your code prettier;
  //   they are defined here in utility.h as follows:
  //       #define SafeRelease(x) if (x) {x->Release(); x=NULL;}
  //       #define SafeDelete(x)  if (x) {delete x; x=NULL;}
  // IMPORTANT:
  //   This function ISN'T only called when the plugin exits!
  //   It is also called whenever the user toggles between fullscreen and
  //   windowed modes, or resizes the window.  Basically, on these events,
  //   the base class calls CleanUpMyDX9Stuff before Reset()ing the DirectX
  //   device, and then calls AllocateMyDX9Stuff afterwards.

  // One funky thing here: if we're switching between fullscreen and windowed,
  //  or doing any other thing that causes all this stuff to get reloaded in a second,
  //  then if we were blending 2 presets, jump fully to the new preset.
  // Otherwise the old preset wouldn't get all reloaded, and it app would crash
  //  when trying to use its stuff.
  if (m_nLoadingPreset != 0) {
    // finish up the pre-load & start the official blend
    m_nLoadingPreset = 8;
    LoadPresetTick();
  }
  // just force this:
  m_pState->m_bBlending = false;

  // Clean up video capture texture
  SafeRelease(m_pVideoCaptureTexture);

  // Clean up input mixing shader
  SafeRelease(m_lpPS_InputMix);

  size_t i;
  for (i = 0; i < m_textures.size(); i++)
    if (m_textures[i].texptr) {
      // notify all CShaderParams classes that we're releasing a bindable texture!!
      size_t N = global_CShaderParams_master_list.size();
      for (size_t j = 0; j < N; j++)
        global_CShaderParams_master_list[j]->OnTextureEvict(m_textures[i].texptr);

      SafeRelease(m_textures[i].texptr);
    }
  m_textures.clear();

  // DON'T RELEASE blur textures - they were already released because they're in m_textures[].
#if (NUM_BLUR_TEX > 0)
  for (i = 0; i < NUM_BLUR_TEX; i++)
    m_lpBlur[i] = NULL;  // SafeRelease(m_lpBlur[i]);
#endif

  // NOTE: not necessary; shell does this for us.
  /*if (GetDevice())
  {
      GetDevice()->SetTexture(0, NULL);
      GetDevice()->SetTexture(1, NULL);
  }*/

  SafeRelease(m_pSpriteVertDecl);
  SafeRelease(m_pWfVertDecl);
  SafeRelease(m_pMyVertDecl);

  m_shaders.comp.Clear();
  m_shaders.warp.Clear();
  m_OldShaders.comp.Clear();
  m_OldShaders.warp.Clear();
  m_NewShaders.comp.Clear();
  m_NewShaders.warp.Clear();
  m_fallbackShaders_vs.comp.Clear();
  m_fallbackShaders_ps.comp.Clear();
  m_fallbackShaders_vs.warp.Clear();
  m_fallbackShaders_ps.warp.Clear();
  m_BlurShaders[0].vs.Clear();
  m_BlurShaders[0].ps.Clear();
  m_BlurShaders[1].vs.Clear();
  m_BlurShaders[1].ps.Clear();
  /*
  SafeRelease( m_shaders.comp.ptr );
  SafeRelease( m_shaders.warp.ptr );
  SafeRelease( m_OldShaders.comp.ptr );
  SafeRelease( m_OldShaders.warp.ptr );
  SafeRelease( m_NewShaders.comp.ptr );
  SafeRelease( m_NewShaders.warp.ptr );
  SafeRelease( m_fallbackShaders_vs.comp.ptr );
  SafeRelease( m_fallbackShaders_ps.comp.ptr );
  SafeRelease( m_fallbackShaders_vs.warp.ptr );
  SafeRelease( m_fallbackShaders_ps.warp.ptr );
  */
  SafeRelease(m_pShaderCompileErrors);
  // SafeRelease( m_pCompiledFragments );
  // SafeRelease( m_pFragmentLinker );

  // 2. release stuff
  SafeRelease(m_lpFFTTexture);
  memset(m_fFFTSmoothed, 0, sizeof(m_fFFTSmoothed));
  SafeRelease(m_lpVS[0]);
  SafeRelease(m_lpVS[1]);

  for (int i = 0; i < NUM_SUPERTEXTS; i++) {
    SafeRelease(m_lpDDSTitle[i]);
  }

  SafeRelease(m_d3dx_title_font_doublesize);
  SafeRelease(m_lyricsFontObject);
  SafeRelease(m_lyricsMeasureFontObject);
  SafeRelease(m_lyricsTexture);
  SafeRelease(m_lyricsBurnTexture);
  m_lyricsTextureSizeX = m_lyricsTextureSizeY = 0;
  m_lyricsBurnTextureSizeX = m_lyricsBurnTextureSizeY = 0;
  m_lyricsTextureUseW = m_lyricsTextureUseH = 0;
  m_lyricsBurnTextureUseW = m_lyricsBurnTextureUseH = 0;

  // NOTE: THIS CODE IS IN THE RIGHT PLACE.
  if (m_gdi_title_font_doublesize) {
    DeleteObject(m_gdi_title_font_doublesize);
    m_gdi_title_font_doublesize = NULL;
  }

  m_texmgr.Finish();

  if (m_verts != NULL) {
    delete[] m_verts;
    m_verts = NULL;
  }

  if (m_verts_temp != NULL) {
    delete[] m_verts_temp;
    m_verts_temp = NULL;
  }

  if (m_vertinfo != NULL) {
    delete[] m_vertinfo;
    m_vertinfo = NULL;
  }

  if (m_indices_list != NULL) {
    delete[] m_indices_list;
    m_indices_list = NULL;
  }

  if (m_indices_strip != NULL) {
    delete[] m_indices_strip;
    m_indices_strip = NULL;
  }

  ClearErrors();
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------
