// plugin_shaders.cpp - Shader compilation, parameter caching, and texture management
// Extracted from plugin.cpp

#include "plugin.h"
#include "plugin_internals.h"
#include "utility.h"
#include "support.h"
#include "resource.h"
#include "defines.h"
#include "shell_defines.h"
#include "wasabi.h"
#include <cstdint>
#include <filesystem>
#include <mutex>

void VShaderInfo::Clear() {
  SafeRelease(ptr);
  SafeRelease(CT);
  params.Clear();
}
void PShaderInfo::Clear() {
  SafeRelease(ptr);
  SafeRelease(CT);
  params.Clear();
}

// global_CShaderParams_master_list: a master list of all CShaderParams classes in existence.
//   ** when we evict a texture, we need to NULL out any texptrs these guys have! **
CShaderParamsList global_CShaderParams_master_list;
CShaderParams::CShaderParams() {
  if (global_CShaderParams_master_list.size() > 0)
    global_CShaderParams_master_list.push_back(this);
}

CShaderParams::~CShaderParams() {
  auto first = global_CShaderParams_master_list.begin();

  int N = global_CShaderParams_master_list.size();
  for (int i = 0; i < N; i++)
    if (global_CShaderParams_master_list[i] == this)
      global_CShaderParams_master_list.erase(first + i);
  texsize_params.clear();
}

void CShaderParams::OnTextureEvict(LPDIRECT3DBASETEXTURE9 texptr) {
  for (int i = 0; i < sizeof(m_texture_bindings) / sizeof(m_texture_bindings[0]); i++)
    if (m_texture_bindings[i].texptr == texptr)
      m_texture_bindings[i].texptr = NULL;
}

void CShaderParams::Clear() {
  // float4 handles:
  rand_frame = NULL;
  rand_preset = NULL;
  luma_params = NULL;

  ZeroMemory(rot_mat, sizeof(rot_mat));
  ZeroMemory(const_handles, sizeof(const_handles));
  ZeroMemory(q_const_handles, sizeof(q_const_handles));
  texsize_params.clear();

  // sampler stages for various PS texture bindings:
  for (int i = 0; i < sizeof(m_texture_bindings) / sizeof(m_texture_bindings[0]); i++) {
    m_texture_bindings[i].texptr = NULL;
    m_texcode[i] = TEX_DISK;
  }
}

bool CPlugin::EvictSomeTexture() {
  // note: this won't evict a texture whose age is zero,
  //       or whose reported size is zero!

#if _DEBUG
  {
    int nEvictableFiles = 0;
    int nEvictableBytes = 0;
    int N = m_textures.size();
    for (int i = 0; i < N; i++)
      if (m_textures[i].bEvictable && m_textures[i].texptr) {
        nEvictableFiles++;
        nEvictableBytes += m_textures[i].nSizeInBytes;
      }
    char buf[1024];
    sprintf(buf, "evicting at %d textures, %.1f MB\n", nEvictableFiles, nEvictableBytes * 0.000001f);
    OutputDebugString(buf);
  }
#endif

  int N = m_textures.size();

  // find age gap
  int newest = 99999999;
  int oldest = 0;
  bool bAtLeastOneFound = false;
  int i;
  for (i = 0; i < N; i++)
    if (m_textures[i].bEvictable && m_textures[i].nSizeInBytes > 0 && m_textures[i].nAge < m_nPresetsLoadedTotal - 1)  // note: -1 here keeps images around for the blend-from preset, too...
    {
      newest = min(newest, m_textures[i].nAge);
      oldest = max(oldest, m_textures[i].nAge);
      bAtLeastOneFound = true;
    }
  if (!bAtLeastOneFound)
    return false;

  // find the "biggest" texture, but dilate things so that the newest textures
  // are HALF as big as the oldest textures, and thus, less likely to get booted.
  int biggest_bytes = 0;
  int biggest_index = -1;
  for (i = 0; i < N; i++)
    if (m_textures[i].bEvictable && m_textures[i].nSizeInBytes > 0 && m_textures[i].nAge < m_nPresetsLoadedTotal - 1)  // note: -1 here keeps images around for the blend-from preset, too...
    {
      float size_mult = 1.0f + (m_textures[i].nAge - newest) / (float)(oldest - newest);
      int bytes = (int)(m_textures[i].nSizeInBytes * size_mult);
      if (bytes > biggest_bytes) {
        biggest_bytes = bytes;
        biggest_index = i;
      }
    }
  if (biggest_index == -1)
    return false;

  // evict that sucker
  assert(m_textures[biggest_index].texptr);

  // notify all CShaderParams classes that we're releasing a bindable texture!!
  N = global_CShaderParams_master_list.size();
  for (i = 0; i < N; i++)
    global_CShaderParams_master_list[i]->OnTextureEvict(m_textures[biggest_index].texptr);

  // 2. erase the texture itself
  SafeRelease(m_textures[biggest_index].texptr);
  m_textures.erase(m_textures.begin() + biggest_index);

  return true;
}

std::wstring texture_exts[] = {L"jpg", L"jpeg", L"jfif", L"dds", L"png", L"tga", L"bmp", L"dib"};
const wchar_t szExtsWithSlashes[] = L".jpg|.png|.dds|etc.";
typedef std::vector<std::wstring> StringVec;
bool PickRandomTexture(const wchar_t* prefix, wchar_t* szRetTextureFilename)  // should be MAX_PATH chars
{
  static StringVec texfiles;
  static DWORD texfiles_timestamp = 0;  // update this a max of every ~2 seconds or so

  // if it's been more than a few seconds since the last textures dir scan, redo it.
  // (..just enough to make sure we don't do it more than once per preset load)
  // DWORD t = timeGetTime(); // in milliseconds
  // if (abs(t - texfiles_timestamp) > 2000)
  if (g_plugin.m_bNeedRescanTexturesDir) {
    g_plugin.m_bNeedRescanTexturesDir = false;  // texfiles_timestamp = t;
    texfiles.clear();

    wchar_t szMask[MAX_PATH];
    swprintf(szMask, L"%stextures\\*.*", g_plugin.m_szMilkdrop2Path);

    WIN32_FIND_DATAW ffd = {0};

    HANDLE hFindFile = INVALID_HANDLE_VALUE;
    if ((hFindFile = FindFirstFileW(szMask, &ffd)) == INVALID_HANDLE_VALUE)  // note: returns filename -without- path
      return false;

    // first, count valid texture files
    do {
      if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        continue;

      wchar_t* ext = wcsrchr(ffd.cFileName, L'.');
      if (!ext)
        continue;

      for (int i = 0; i < sizeof(texture_exts) / sizeof(texture_exts[0]); i++)
        if (!_wcsicmp(texture_exts[i].c_str(), ext + 1)) {
          // valid texture found - add it to the list.  ("heart.jpg", for example)
          texfiles.push_back(ffd.cFileName);
          continue;
        }
    } while (FindNextFileW(hFindFile, &ffd));
    FindClose(hFindFile);
  }

  if (texfiles.size() == 0)
    return false;

  // then randomly pick one
  if (prefix == NULL || prefix[0] == 0) {
    // pick randomly from entire list
    int i = rand() % texfiles.size();
    lstrcpyW(szRetTextureFilename, texfiles[i].c_str());
  } else {
    // only pick from files w/the right prefix
    StringVec temp_list;
    int N = texfiles.size();
    int len = lstrlenW(prefix);
    int i;
    for (i = 0; i < N; i++)
      if (!_wcsnicmp(prefix, texfiles[i].c_str(), len))
        temp_list.push_back(texfiles[i]);
    N = temp_list.size();
    if (N == 0)
      return false;
    // pick randomly from the subset
    i = rand() % temp_list.size();
    lstrcpyW(szRetTextureFilename, temp_list[i].c_str());
  }
  return true;
}

void CShaderParams::CacheParams(LPD3DXCONSTANTTABLE pCT, bool bHardErrors) {
  Clear();

  if (!pCT)
    return;

  D3DXCONSTANTTABLE_DESC d;
  pCT->GetDesc(&d);

  D3DXCONSTANT_DESC cd;

#define MAX_RAND_TEX 16
  std::wstring RandTexName[MAX_RAND_TEX];

  // pass 1: find all the samplers (and texture bindings).
  UINT i;
  for (i = 0; i < d.Constants; i++) {
    D3DXHANDLE h = pCT->GetConstant(NULL, i);
    unsigned int count = 1;
    pCT->GetConstantDesc(h, &cd, &count);

    // cd.Name          = VS_Sampler
    // cd.RegisterSet   = D3DXRS_SAMPLER
    // cd.RegisterIndex = 3
    if (cd.RegisterSet == D3DXRS_SAMPLER && cd.RegisterIndex >= 0 && cd.RegisterIndex < sizeof(m_texture_bindings) / sizeof(m_texture_bindings[0])) {
      assert(m_texture_bindings[cd.RegisterIndex].texptr == NULL);

      // remove "sampler_" prefix to create root file name.  could still have "FW_" prefix or something like that.
      wchar_t szRootName[MAX_PATH];
      if (!strncmp(cd.Name, "sampler_", 8))
        lstrcpyW(szRootName, AutoWide(&cd.Name[8]));
      else
        lstrcpyW(szRootName, AutoWide(cd.Name));

      // also peel off "XY_" prefix, if it's there, to specify filtering & wrap mode.
      bool bBilinear = true;
      bool bWrap = true;
      bool bWrapFilterSpecified = false;
      if (lstrlenW(szRootName) > 3 && szRootName[2] == L'_') {
        wchar_t temp[3];
        temp[0] = szRootName[0];
        temp[1] = szRootName[1];
        temp[2] = 0;
        // convert to uppercase
        if (temp[0] >= L'a' && temp[0] <= L'z')
          temp[0] -= L'a' - L'A';
        if (temp[1] >= L'a' && temp[1] <= L'z')
          temp[1] -= L'a' - L'A';

        if (!wcscmp(temp, L"FW")) {
          bWrapFilterSpecified = true;
          bBilinear = true;
          bWrap = true;
        } else if (!wcscmp(temp, L"FC")) {
          bWrapFilterSpecified = true;
          bBilinear = true;
          bWrap = false;
        } else if (!wcscmp(temp, L"PW")) {
          bWrapFilterSpecified = true;
          bBilinear = false;
          bWrap = true;
        } else if (!wcscmp(temp, L"PC")) {
          bWrapFilterSpecified = true;
          bBilinear = false;
          bWrap = false;
        }
        // also allow reverses:
        else if (!wcscmp(temp, L"WF")) {
          bWrapFilterSpecified = true;
          bBilinear = true;
          bWrap = true;
        } else if (!wcscmp(temp, L"CF")) {
          bWrapFilterSpecified = true;
          bBilinear = true;
          bWrap = false;
        } else if (!wcscmp(temp, L"WP")) {
          bWrapFilterSpecified = true;
          bBilinear = false;
          bWrap = true;
        } else if (!wcscmp(temp, L"CP")) {
          bWrapFilterSpecified = true;
          bBilinear = false;
          bWrap = false;
        }

        // peel off the prefix
        int i = 0;
        while (szRootName[i + 3]) {
          szRootName[i] = szRootName[i + 3];
          i++;
        }
        szRootName[i] = 0;
      }
      m_texture_bindings[cd.RegisterIndex].bWrap = bWrap;
      m_texture_bindings[cd.RegisterIndex].bBilinear = bBilinear;

      // if <szFileName> is "main", map it to the VS...
      if (!wcscmp(L"main", szRootName)) {
        m_texture_bindings[cd.RegisterIndex].texptr = NULL;
        m_texcode[cd.RegisterIndex] = TEX_VS;
      }
#if (NUM_BLUR_TEX >= 2)
      else if (!wcscmp(L"blur1", szRootName)) {
        m_texture_bindings[cd.RegisterIndex].texptr = g_plugin.m_lpBlur[1];
        m_texcode[cd.RegisterIndex] = TEX_BLUR1;
        if (!bWrapFilterSpecified) {  // when sampling blur textures, default is CLAMP
          m_texture_bindings[cd.RegisterIndex].bWrap = false;
          m_texture_bindings[cd.RegisterIndex].bBilinear = true;
        }
      }
#endif
#if (NUM_BLUR_TEX >= 4)
      else if (!wcscmp(L"blur2", szRootName)) {
        m_texture_bindings[cd.RegisterIndex].texptr = g_plugin.m_lpBlur[3];
        m_texcode[cd.RegisterIndex] = TEX_BLUR2;
        if (!bWrapFilterSpecified) {  // when sampling blur textures, default is CLAMP
          m_texture_bindings[cd.RegisterIndex].bWrap = false;
          m_texture_bindings[cd.RegisterIndex].bBilinear = true;
        }
      }
#endif
#if (NUM_BLUR_TEX >= 6)
      else if (!wcscmp(L"blur3", szRootName)) {
        m_texture_bindings[cd.RegisterIndex].texptr = g_plugin.m_lpBlur[5];
        m_texcode[cd.RegisterIndex] = TEX_BLUR3;
        if (!bWrapFilterSpecified) {  // when sampling blur textures, default is CLAMP
          m_texture_bindings[cd.RegisterIndex].bWrap = false;
          m_texture_bindings[cd.RegisterIndex].bBilinear = true;
        }
      }
#endif
#if (NUM_BLUR_TEX >= 8)
      else if (!wcscmp("blur4", szRootName)) {
        m_texture_bindings[cd.RegisterIndex].texptr = g_plugin.m_lpBlur[7];
        m_texcode[cd.RegisterIndex] = TEX_BLUR4;
        if (!bWrapFilterSpecified) {  // when sampling blur textures, default is CLAMP
          m_texture_bindings[cd.RegisterIndex].bWrap = false;
          m_texture_bindings[cd.RegisterIndex].bBilinear = true;
        }
      }
#endif
#if (NUM_BLUR_TEX >= 10)
      else if (!wcscmp("blur5", szRootName)) {
        m_texture_bindings[cd.RegisterIndex].texptr = g_plugin.m_lpBlur[9];
        m_texcode[cd.RegisterIndex] = TEX_BLUR5;
        if (!bWrapFilterSpecified) {  // when sampling blur textures, default is CLAMP
          m_texture_bindings[cd.RegisterIndex].bWrap = false;
          m_texture_bindings[cd.RegisterIndex].bBilinear = true;
        }
      }
#endif
#if (NUM_BLUR_TEX >= 12)
      else if (!wcscmp("blur6", szRootName)) {
        m_texture_bindings[cd.RegisterIndex].texptr = g_plugin.m_lpBlur[11];
        m_texcode[cd.RegisterIndex] = TEX_BLUR6;
        if (!bWrapFilterSpecified) {  // when sampling blur textures, default is CLAMP
          m_texture_bindings[cd.RegisterIndex].bWrap = false;
          m_texture_bindings[cd.RegisterIndex].bBilinear = true;
        }
      }
#endif
      else if (!wcscmp(L"fft", szRootName)) {
        m_texture_bindings[cd.RegisterIndex].texptr = g_plugin.m_lpFFTTexture;
        m_texcode[cd.RegisterIndex] = TEX_FFT;
        if (!bWrapFilterSpecified) {
          m_texture_bindings[cd.RegisterIndex].bWrap = false;     // clamp
          m_texture_bindings[cd.RegisterIndex].bBilinear = true;  // linear interpolation between bins
        }
      } else {
        m_texcode[cd.RegisterIndex] = TEX_DISK;

        // check for request for random texture.
        if (!wcsncmp(L"rand", szRootName, 4) &&
            IsNumericChar(szRootName[4]) &&
            IsNumericChar(szRootName[5]) &&
            (szRootName[6] == 0 || szRootName[6] == '_')) {
          int rand_slot = -1;

          // peel off filename prefix ("rand13_smalltiled", for example)
          wchar_t prefix[MAX_PATH];
          if (szRootName[6] == L'_')
            lstrcpyW(prefix, &szRootName[7]);
          else
            prefix[0] = 0;
          szRootName[6] = 0;

          swscanf(&szRootName[4], L"%d", &rand_slot);
          if (rand_slot >= 0 && rand_slot <= 15)  // otherwise, not a special filename - ignore it
          {
            if (!PickRandomTexture(prefix, szRootName)) {
              if (prefix[0])
                swprintf(szRootName, L"[rand%02d] %s*", rand_slot, prefix);
              else
                swprintf(szRootName, L"[rand%02d] *", rand_slot);
            } else {
              // chop off extension
              wchar_t* p = wcsrchr(szRootName, L'.');
              if (p)
                *p = 0;
            }

            RandTexName[rand_slot] = szRootName;  // we'll need to remember this for texsize_ params!
          }
        }

        // see if <szRootName>.tga or .jpg has already been loaded.
        //   (if so, grab a pointer to it)
        //   (if NOT, create & load it).
        int N = g_plugin.m_textures.size();
        for (int n = 0; n < N; n++) {
          if (!wcscmp(g_plugin.m_textures[n].texname, szRootName)) {
            // found a match - texture was already loaded
            m_texture_bindings[cd.RegisterIndex].texptr = g_plugin.m_textures[n].texptr;
            // also bump its age down to zero! (for cache mgmt)
            g_plugin.m_textures[n].nAge = g_plugin.m_nPresetsLoadedTotal;
            break;
          }
        }
        // if still not found, load it up / make a new texture
        if (!m_texture_bindings[cd.RegisterIndex].texptr) {
          TexInfo x;
          wcsncpy(x.texname, szRootName, 254);
          x.texptr = NULL;
          // x.texsize_param = NULL;

          // check if we need to evict anything from the cache,
          // due to our own cache constraints...
          while (1) {
            int nTexturesCached = 0;
            int nBytesCached = 0;
            int N = g_plugin.m_textures.size();
            for (int i = 0; i < N; i++)
              if (g_plugin.m_textures[i].bEvictable && g_plugin.m_textures[i].texptr) {
                nBytesCached += g_plugin.m_textures[i].nSizeInBytes;
                nTexturesCached++;
              }
            if (nTexturesCached < g_plugin.m_nMaxImages &&
                nBytesCached < g_plugin.m_nMaxBytes)
              break;
            // otherwise, evict now - and loop until we are within the constraints
            if (!g_plugin.EvictSomeTexture())
              break;  // or if there was nothing to evict, just give up
          }

          // load the texture
          wchar_t szFilename[MAX_PATH];
          for (int z = 0; z < sizeof(texture_exts) / sizeof(texture_exts[0]); z++) {
            swprintf(szFilename, L"%stextures\\%s.%s", g_plugin.m_szMilkdrop2Path, szRootName, texture_exts[z].c_str());
            if (GetFileAttributesW(szFilename) == 0xFFFFFFFF) {
              // try again, but in presets dir
              swprintf(szFilename, L"%s%s.%s", g_plugin.m_szPresetDir, szRootName, texture_exts[z].c_str());
              if (GetFileAttributesW(szFilename) == 0xFFFFFFFF)
                continue;
            }
            D3DXIMAGE_INFO desc;

            // keep trying to load it - if it fails due to memory, evict something and try again.
            while (1) {
              HRESULT hr = D3DXCreateTextureFromFileExW(g_plugin.GetDevice(),
                                                        szFilename,
                                                        D3DX_DEFAULT_NONPOW2,  // w
                                                        D3DX_DEFAULT_NONPOW2,  // h
                                                        D3DX_DEFAULT,          // # mip levels to gen - all
                                                        0,                     // usage flags
                                                        D3DFMT_UNKNOWN,
                                                        D3DPOOL_DEFAULT,
                                                        D3DX_DEFAULT,  // filter
                                                        D3DX_DEFAULT,  // mipfilter
                                                        0,             // color key
                                                        &desc,
                                                        NULL,  // palette
                                                        (IDirect3DTexture9**)&x.texptr);
              if (hr == D3DERR_OUTOFVIDEOMEMORY || hr == E_OUTOFMEMORY) {
                // out of memory - try evicting something old and/or big
                if (g_plugin.EvictSomeTexture())
                  continue;
              }

              if (hr == D3D_OK) {
                x.w = desc.Width;
                x.h = desc.Height;
                x.d = desc.Depth;
                x.bEvictable = true;
                x.nAge = g_plugin.m_nPresetsLoadedTotal;
                int nPixels = desc.Width * desc.Height * max(1, desc.Depth);
                int BitsPerPixel = GetDX9TexFormatBitsPerPixel(desc.Format);
                x.nSizeInBytes = nPixels * BitsPerPixel / 8 + 16384;  // plus some overhead
              }
              break;
            }
          }

          if (!x.texptr) {
            wchar_t buf[2048], title[64];
            swprintf(buf, wasabiApiLangString(IDS_COULD_NOT_LOAD_TEXTURE_X), szRootName, szExtsWithSlashes);
            g_plugin.dumpmsg(buf);
            if (bHardErrors)
              MessageBoxW(g_plugin.GetPluginWindow(), buf, wasabiApiLangString(IDS_MILKDROP_ERROR, title, 64), MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
            else {
              g_plugin.AddError(buf, 6.0f, ERR_PRESET, true);
            }
            return;
          }

          g_plugin.m_textures.push_back(x);
          m_texture_bindings[cd.RegisterIndex].texptr = x.texptr;
        }
      }
    }
  }

  // pass 2: bind all the float4's.  "texsize_XYZ" params will be filled out via knowledge of loaded texture sizes.
  for (i = 0; i < d.Constants; i++) {
    D3DXHANDLE h = pCT->GetConstant(NULL, i);
    unsigned int count = 1;
    pCT->GetConstantDesc(h, &cd, &count);

    if (cd.RegisterSet == D3DXRS_FLOAT4) {
      if (cd.Class == D3DXPC_MATRIX_COLUMNS) {
        if (!strcmp(cd.Name, "rot_s1"))
          rot_mat[0] = h;
        else if (!strcmp(cd.Name, "rot_s2"))
          rot_mat[1] = h;
        else if (!strcmp(cd.Name, "rot_s3"))
          rot_mat[2] = h;
        else if (!strcmp(cd.Name, "rot_s4"))
          rot_mat[3] = h;
        else if (!strcmp(cd.Name, "rot_d1"))
          rot_mat[4] = h;
        else if (!strcmp(cd.Name, "rot_d2"))
          rot_mat[5] = h;
        else if (!strcmp(cd.Name, "rot_d3"))
          rot_mat[6] = h;
        else if (!strcmp(cd.Name, "rot_d4"))
          rot_mat[7] = h;
        else if (!strcmp(cd.Name, "rot_f1"))
          rot_mat[8] = h;
        else if (!strcmp(cd.Name, "rot_f2"))
          rot_mat[9] = h;
        else if (!strcmp(cd.Name, "rot_f3"))
          rot_mat[10] = h;
        else if (!strcmp(cd.Name, "rot_f4"))
          rot_mat[11] = h;
        else if (!strcmp(cd.Name, "rot_vf1"))
          rot_mat[12] = h;
        else if (!strcmp(cd.Name, "rot_vf2"))
          rot_mat[13] = h;
        else if (!strcmp(cd.Name, "rot_vf3"))
          rot_mat[14] = h;
        else if (!strcmp(cd.Name, "rot_vf4"))
          rot_mat[15] = h;
        else if (!strcmp(cd.Name, "rot_uf1"))
          rot_mat[16] = h;
        else if (!strcmp(cd.Name, "rot_uf2"))
          rot_mat[17] = h;
        else if (!strcmp(cd.Name, "rot_uf3"))
          rot_mat[18] = h;
        else if (!strcmp(cd.Name, "rot_uf4"))
          rot_mat[19] = h;
        else if (!strcmp(cd.Name, "rot_rand1"))
          rot_mat[20] = h;
        else if (!strcmp(cd.Name, "rot_rand2"))
          rot_mat[21] = h;
        else if (!strcmp(cd.Name, "rot_rand3"))
          rot_mat[22] = h;
        else if (!strcmp(cd.Name, "rot_rand4"))
          rot_mat[23] = h;
      } else if (cd.Class == D3DXPC_VECTOR) {
        if (!strcmp(cd.Name, "rand_frame"))
          rand_frame = h;
        else if (!strcmp(cd.Name, "rand_preset"))
          rand_preset = h;
        else if (!strcmp(cd.Name, "luma_params"))
          luma_params = h;
        else if (!strncmp(cd.Name, "texsize_", 8)) {
          // remove "texsize_" prefix to find root file name.
          wchar_t szRootName[MAX_PATH];
          if (!strncmp(cd.Name, "texsize_", 8))
            lstrcpyW(szRootName, AutoWide(&cd.Name[8]));
          else
            lstrcpyW(szRootName, AutoWide(cd.Name));

          // check for request for random texture.
          // it should be a previously-seen random index - just fetch/reuse the name.
          if (!wcsncmp(L"rand", szRootName, 4) &&
              IsNumericChar(szRootName[4]) &&
              IsNumericChar(szRootName[5]) &&
              (szRootName[6] == 0 || szRootName[6] == L'_')) {
            int rand_slot = -1;

            // ditch filename prefix ("rand13_smalltiled", for example)
            // and just go by the slot
            if (szRootName[6] == L'_')
              szRootName[6] = 0;

            swscanf(&szRootName[4], L"%d", &rand_slot);
            if (rand_slot >= 0 && rand_slot <= 15)  // otherwise, not a special filename - ignore it
              if (RandTexName[rand_slot].size() > 0)
                lstrcpyW(szRootName, RandTexName[rand_slot].c_str());
          }

          // see if <szRootName>.tga or .jpg has already been loaded.
          bool bTexFound = false;
          int N = g_plugin.m_textures.size();
          for (int n = 0; n < N; n++) {
            if (!wcscmp(g_plugin.m_textures[n].texname, szRootName)) {
              // found a match - texture was loaded
              TexSizeParamInfo y;
              y.texname = szRootName;  // for debugging
              y.texsize_param = h;
              y.w = g_plugin.m_textures[n].w;
              y.h = g_plugin.m_textures[n].h;
              texsize_params.push_back(y);

              bTexFound = true;
              break;
            }
          }

          if (!bTexFound) {
            wchar_t buf[1024];
            swprintf(buf, wasabiApiLangString(IDS_UNABLE_TO_RESOLVE_TEXSIZE_FOR_A_TEXTURE_NOT_IN_USE), cd.Name);
            g_plugin.AddError(buf, 6.0f, ERR_PRESET, true);
          }
        } else if (cd.Name[0] == '_' && cd.Name[1] == 'c') {
          int z;
          if (sscanf(&cd.Name[2], "%d", &z) == 1)
            if (z >= 0 && z < sizeof(const_handles) / sizeof(const_handles[0]))
              const_handles[z] = h;
        } else if (cd.Name[0] == '_' && cd.Name[1] == 'q') {
          int z = cd.Name[2] - 'a';
          if (z >= 0 && z < sizeof(q_const_handles) / sizeof(q_const_handles[0]))
            q_const_handles[z] = h;
        }
      }
    }
  }
}

//----------------------------------------------------------------------

bool CPlugin::RecompileVShader(const char* szShadersText, VShaderInfo* si, int shaderType, bool bHardErrors, bool bCompileOnly) {
  SafeRelease(si->ptr);
  ZeroMemory(si, sizeof(VShaderInfo));

  char ver[16];
  if (m_IsAMD)
    lstrcpy(ver, "vs_3_0");
  else
    lstrcpy(ver, "vs_1_1");

  // LOAD SHADER
  if (!LoadShaderFromMemory(szShadersText, "VS", ver, &si->CT, (void**)&si->ptr, shaderType, bHardErrors, bCompileOnly))
    return false;

  if (!bCompileOnly) {
    // Track down texture & float4 param bindings for this shader.
    // Also loads any textures that need loaded.
    si->params.CacheParams(si->CT, bHardErrors);
  }

  return true;
}

bool CPlugin::RecompilePShader(const char* szShadersText, PShaderInfo* si, int shaderType, bool bHardErrors, int PSVersion,
                               bool bCompileOnly, bool bSkipIfCached) {
  assert(m_nMaxPSVersion > 0);

  SafeRelease(si->ptr);
  ZeroMemory(si, sizeof(PShaderInfo));

  // LOAD SHADER
  // note: ps_1_4 required for dependent texture lookups.
  //       ps_2_0 required for tex2Dbias.
  char ver[16];
  lstrcpy(ver, "ps_0_0");
  switch (PSVersion) {
    case MD2_PS_NONE:
      // Even though the PRESET doesn't use shaders, if MilkDrop is running where it CAN do shaders,
      //   we run all the old presets through (shader) emulation.
      // This way, during a MilkDrop session, we are always calling either WarpedBlit() or WarpedBlit_NoPixelShaders(),
      //   and blending always works.
      lstrcpy(ver, "ps_2_0");
      break;
    case MD2_PS_2_0:
      lstrcpy(ver, "ps_2_0");
      break;
    case MD2_PS_2_X:
      lstrcpy(ver, "ps_2_a");
      break;  // we'll try ps_2_a first, LoadShaderFromMemory will try ps_2_b if compilation fails
    case MD2_PS_3_0:
      lstrcpy(ver, "ps_3_0");
      break;
    case MD2_PS_4_0:
      lstrcpy(ver, "ps_4_0");
      break;
    default:
      assert(0);
      break;
  }

  if (!LoadShaderFromMemory(szShadersText, "PS", ver, &si->CT, (void**)&si->ptr, shaderType, bHardErrors, bCompileOnly,
                            bSkipIfCached))
    return false;

  if (!bCompileOnly) {
    // Track down texture & float4 param bindings for this shader.
    // Also loads any textures that need loaded.
    si->params.CacheParams(si->CT, bHardErrors);
  }
  return true;
}

bool CPlugin::LoadShaders(PShaderSet* sh, CState* pState, bool bTick, bool bCompileOnly, bool bSkipIfCached) {
  if (m_nMaxPSVersion <= 0)
    return true;

  // A background preset compile can be aborted from the UI (ESC). Give up between shaders -
  // D3DXCompileShader itself cannot be interrupted, so this is the earliest safe checkpoint.
  if (bCompileOnly && m_nPresetCompileCancel)
    return false;

  // load one of the pixel shaders
  if (!sh->warp.ptr && pState->m_nWarpPSVersion > 0) {
    bool bOK = RecompilePShader(pState->m_szWarpShadersText, &sh->warp, SHADER_WARP, false, pState->m_nWarpPSVersion,
                                bCompileOnly, bSkipIfCached);
    if (!bOK) {
      // switch to fallback shader
      m_fallbackShaders_ps.warp.ptr->AddRef();
      m_fallbackShaders_ps.warp.CT->AddRef();
      memcpy(&sh->warp, &m_fallbackShaders_ps.warp, sizeof(PShaderInfo));
      // cancel any slow-preset-load
      // m_nLoadingPreset = 1000;
    }

    if (bTick)
      return true;
    if (bCompileOnly && m_nPresetCompileCancel)
      return false;
  }

  if (!sh->comp.ptr && pState->m_nCompPSVersion > 0) {
    bool bOK = RecompilePShader(pState->m_szCompShadersText, &sh->comp, SHADER_COMP, false, pState->m_nCompPSVersion,
                                bCompileOnly, bSkipIfCached);
    if (!bOK) {
      // switch to fallback shader
      m_fallbackShaders_ps.comp.ptr->AddRef();
      m_fallbackShaders_ps.comp.CT->AddRef();
      memcpy(&sh->comp, &m_fallbackShaders_ps.comp, sizeof(PShaderInfo));
      // cancel any slow-preset-load
      // m_nLoadingPreset = 1000;
    }
  }

  return true;
}

bool CPlugin::LoadShaderFromMemory(const char* szOrigShaderText, char* szFn, char* szProfile,
                                   LPD3DXCONSTANTTABLE* ppConstTable, void** ppShader, int shaderType, bool bHardErrors,
                                   bool compileOnly, bool skipIfCached) {
  const char szWarpDefines[] =
      "#define rad _rad_ang.x\n"
      "#define ang _rad_ang.y\n"
      "#define uv _uv.xy\n"
      "#define uv_orig _uv.zw\n";
  const char szCompDefines[] =
      "#define rad _rad_ang.x\n"
      "#define ang _rad_ang.y\n"
      "#define uv _uv.xy\n"
      "#define uv_orig _uv.xy\n"  //[sic]
      "#define hue_shader _vDiffuse.xyz\n";
  const char szWarpParams[] = "float4 _vDiffuse : COLOR, float4 _uv : TEXCOORD0, float2 _rad_ang : TEXCOORD1, out float4 _return_value : COLOR0";
  const char szCompParams[] = "float4 _vDiffuse : COLOR, float2 _uv : TEXCOORD0, float2 _rad_ang : TEXCOORD1, out float4 _return_value : COLOR0";
  const char szFirstLine[] = "    float3 ret = 0;";

  char szWhichShader[64];
  switch (shaderType) {
    case SHADER_WARP:
      lstrcpy(szWhichShader, "warp");
      break;
    case SHADER_COMP:
      lstrcpy(szWhichShader, "composite");
      break;
    case SHADER_BLUR:
      lstrcpy(szWhichShader, "blur");
      break;
    case SHADER_OTHER:
      lstrcpy(szWhichShader, "(other)");
      break;
    default:
      lstrcpy(szWhichShader, "(unknown)");
      break;
  }

  LPD3DXBUFFER pShaderByteCode = NULL;
  wchar_t title[64];

  *ppShader = NULL;
  *ppConstTable = NULL;

  char szShaderText[128000];
  char temp[128000];
  int writePos = 0;

  // paste the universal #include
  lstrcpy(&szShaderText[writePos], m_szShaderIncludeText);  // first, paste in the contents of 'inputs.fx' before the actual shader text.  Has 13's and 10's.
  writePos += m_nShaderIncludeTextLen;

  bool bIsPs20 = (strcmp(szProfile, "ps_2_0") == 0);

  // paste in luma_params global for composite shaders (must be outside function signature for compatibility)
  if (shaderType == SHADER_COMP && szProfile[0] == 'p' && !bIsPs20) {
    const char szLumaUniform[] = "uniform float4 luma_params;\r\n";
    lstrcpy(&szShaderText[writePos], szLumaUniform);
    writePos += lstrlen(szLumaUniform);
  }

  // paste in any custom #defines for this shader type
  if (shaderType == SHADER_WARP && szProfile[0] == 'p') {
    lstrcpy(&szShaderText[writePos], szWarpDefines);
    writePos += lstrlen(szWarpDefines);
  } else if (shaderType == SHADER_COMP && szProfile[0] == 'p') {
    lstrcpy(&szShaderText[writePos], szCompDefines);
    writePos += lstrlen(szCompDefines);
  }

  // paste in the shader itself - converting LCC's to 13+10's.
  // avoid lstrcpy b/c it might not handle the linefeed stuff...?
  int shaderStartPos = writePos;
  {
    const char* s = szOrigShaderText;
    char* d = &szShaderText[writePos];
    while (*s) {
      if (*s == LINEFEED_CONTROL_CHAR) {
        *d++ = 13;
        writePos++;
        *d++ = 10;
        writePos++;
      } else {
        *d++ = *s;
        writePos++;
      }
      s++;
    }
    *d = 0;
    writePos++;
  }

  // strip out all comments - but cheat a little - start at the shader test.
  // (the include file was already stripped of comments)
  StripComments(&szShaderText[shaderStartPos]);

  // note: only do this stuff if type is WARP or COMP shader... not for blur, etc!
  // FIXME - hints on the inputs / output / samplers etc.
  //    can go in the menu header, NOT the preset!  =)
  // then update presets
  //   -> be sure to update the presets on disk AND THE DEFAULT SHADERS (for loading MD1 presets)
  // FIXME - then update auth. guide w/new examples,
  //    and a list of the invisible inputs (and one output) to each shader!
  //    warp: float2 uv, float2 uv_orig, rad, ang
  //    comp: float2 uv, rad, ang, float3 hue_shader
  //  test all this string code in Debug mode - make sure nothing bad is happening

  /*
  1. paste warp or comp #defines
  2. search for "void" + whitespace + szFn + [whitespace] + '('
  3. insert params
  4. search for [whitespace] + ')'.
  5. search for final '}' (strrchr)
  6. back up one char, insert the Last Line, and add '}' and that's it.
  */
  if ((shaderType == SHADER_WARP || shaderType == SHADER_COMP) && szProfile[0] == 'p') {
    char* p = &szShaderText[shaderStartPos];

    // seek to 'shader_body' and replace it with spaces
    while (*p && strncmp(p, "shader_body", 11))
      p++;
    if (p) {
      for (int i = 0; i < 11; i++)
        *p++ = ' ';
    }

    if (p) {
      // insert "void PS(...params...)\n"
      lstrcpy(temp, p);
      const char* params = (shaderType == SHADER_WARP) ? szWarpParams : szCompParams;

      sprintf(p, "void %s( %s )\n", szFn, params);
      p += lstrlen(p);
      lstrcpy(p, temp);

      // find the starting curly brace
      p = strchr(p, '{');
      if (p) {
        // skip over it
        p++;
        // then insert "float3 ret = 0;"
        lstrcpy(temp, p);
        sprintf(p, "%s\n", szFirstLine);
        p += lstrlen(p);
        lstrcpy(p, temp);

        // find the ending curly brace
        p = strrchr(p, '}');
        // add the last line with optional lumakey support (COMP only)
        if (p) {
          if (shaderType == SHADER_COMP && !bIsPs20) {
            char szLastLine[] =
                "    float luma_v = dot(ret.xyz, float3(0.299, 0.587, 0.114));\n"
                "    float luma_a = (luma_params.w > 0.5) ? saturate((luma_v - luma_params.x) / max(0.0001, luma_params.y)) : 1.0;\n"
                "    _return_value = float4(shiftHSV(ret.xyz), luma_a * _vDiffuse.w);";

            sprintf(p, " %s\n}\n", szLastLine);
          } else {
            char szLastLine[] = "    _return_value = float4(shiftHSV(ret.xyz), _vDiffuse.w);";
            sprintf(p, " %s\n}\n", szLastLine);
          }
        }
      }
    }

    if (!p) {
      wchar_t temp[512];
      swprintf(temp, wasabiApiLangString(IDS_ERROR_PARSING_X_X_SHADER), szProfile, szWhichShader);
      dumpmsg(temp);
      AddError(temp, 8.0f, ERR_PRESET, true);
      return false;
    }
  }

  // now really try to compile it.

  bool failed = false;
  int len = lstrlen(szShaderText);

  std::wstring wideShaderText = std::wstring(szShaderText, szShaderText + strlen(szShaderText));
  wchar_t tempBuffer[32768];                           // Ensure the buffer size is sufficient for the content.
  wcsncpy(tempBuffer, wideShaderText.c_str(), 32767);  // Copy the content safely.
  tempBuffer[32767] = L'\0';                           // Null-terminate to avoid overflow.
  dumpmsg(tempBuffer);                                 // Pass the non-const buffer to dumpmsg.

  bool bLoadedFromCache = false;
  uint32_t checksum = crc32(szShaderText, len);
  if (m_ShaderCaching) {
    pShaderByteCode = LoadShaderBytecodeFromFile(checksum, &szProfile[0]);
  }

  if (pShaderByteCode != NULL) {
    if (!compileOnly) {
      // restore ConstTable from cached bytecode
      HRESULT hr = D3DXGetShaderConstantTable(
          (DWORD*)pShaderByteCode->GetBufferPointer(),
          ppConstTable);
      if (SUCCEEDED(hr)) {
        bLoadedFromCache = true;
      } else {
        SafeRelease(pShaderByteCode);  // invalid cached bytecode, fall through to compile
      }
    } else if (skipIfCached) {
      bLoadedFromCache = true;
    } else {
      SafeRelease(pShaderByteCode);  // compile-only explicitly ignores stale cache
    }
  }

  if (!bLoadedFromCache) {
    HRESULT hresult = D3DXCompileShader(
        szShaderText,
        len,
        NULL,  // CONST D3DXMACRO* pDefines,
        NULL,  // LPD3DXINCLUDE pInclude,
        szFn,
        szProfile,
        m_dwShaderFlags,
        &pShaderByteCode,
        &m_pShaderCompileErrors,
        ppConstTable);

    if (D3D_OK != hresult) {
      failed = true;
    }
    // before we totally fail, let's try using ps_2_b instead of ps_2_a
    if (failed && !strcmp(szProfile, "ps_2_a")) {
      SafeRelease(m_pShaderCompileErrors);
      if (D3D_OK == D3DXCompileShader(szShaderText, len, NULL, NULL, szFn,
                                      "ps_2_b", m_dwShaderFlags, &pShaderByteCode, &m_pShaderCompileErrors, ppConstTable)) {
        failed = false;
      }
    }

    if (failed) {
      wchar_t wideErrorMsg[1024];

      if (m_pShaderCompileErrors) {
        const char* errorMsg = (const char*)m_pShaderCompileErrors->GetBufferPointer();
        // Convert to wide string
        MultiByteToWideChar(CP_ACP, 0, errorMsg, -1, wideErrorMsg, _countof(wideErrorMsg));
        dumpmsg(wideErrorMsg);

        SafeRelease(m_pShaderCompileErrors);
        AddNotification(wideErrorMsg);
      } else {
        if (MessageBoxA(GetPluginWindow(), "Milkwave Visualizer needs Microsoft DirectX End-User Runtime to work.\n\nInstalling this does not affect your current DirectX installation.\n\nOpen Microsoft Download Website now?", "Milkwave Visualizer", MB_YESNO | MB_SETFOREGROUND | MB_TOPMOST) == IDYES) {
          // open website in browser
          ShellExecuteA(NULL, "open", "https://www.microsoft.com/en-us/download/details.aspx?id=35", NULL, NULL, SW_SHOWNORMAL);
        }
      }
      return false;
    }

    if (m_ShaderCaching) {
      SaveShaderBytecodeToFile(pShaderByteCode, checksum, &szProfile[0]);
    }
  }

  // load ok, create the shader
  if (!compileOnly) {
    HRESULT hr = 1;
    if (szProfile[0] == 'v') {
      hr = GetDevice()->CreateVertexShader((const unsigned long*)(pShaderByteCode->GetBufferPointer()), (IDirect3DVertexShader9**)ppShader);
    } else if (szProfile[0] == 'p') {
      hr = GetDevice()->CreatePixelShader((const unsigned long*)(pShaderByteCode->GetBufferPointer()), (IDirect3DPixelShader9**)ppShader);
    }

    if (hr != D3D_OK && bLoadedFromCache) {
      // stale or incompatible cache file - recompile and retry once
      SafeRelease(pShaderByteCode);
      if (*ppConstTable) {
        (*ppConstTable)->Release();
        *ppConstTable = NULL;
      }
      *ppShader = nullptr;
      HRESULT compileResult = D3DXCompileShader(szShaderText, len, NULL, NULL, szFn, szProfile,
                                                m_dwShaderFlags, &pShaderByteCode, &m_pShaderCompileErrors, ppConstTable);
      if (D3D_OK == compileResult) {
        hr = 1;
        if (szProfile[0] == 'v')
          hr = GetDevice()->CreateVertexShader((const unsigned long*)pShaderByteCode->GetBufferPointer(), (IDirect3DVertexShader9**)ppShader);
        else if (szProfile[0] == 'p')
          hr = GetDevice()->CreatePixelShader((const unsigned long*)pShaderByteCode->GetBufferPointer(), (IDirect3DPixelShader9**)ppShader);
        if (D3D_OK == hr && m_ShaderCaching)
          SaveShaderBytecodeToFile(pShaderByteCode, checksum, &szProfile[0]);
      }
    }

    if (hr != D3D_OK) {
      wchar_t temp[512];
      wasabiApiLangString(IDS_ERROR_CREATING_SHADER, temp, sizeof(temp));
      // dumpmsg(temp);
      if (bHardErrors)
        MessageBoxW(GetPluginWindow(), temp, wasabiApiLangString(IDS_MILKDROP_ERROR, title, 64), MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
      else {
        AddError(temp, 6.0f, ERR_PRESET, true);
      }
      return false;
    }
  }

  pShaderByteCode->Release();
  pShaderByteCode = nullptr;

  return true;
}

//----------------------------------------------------------------------

void CPlugin::CompilePresetShadersToFile(wchar_t* sPresetFile, bool bSkipIfCached) {
  // The startup precompiler and the on-demand preset compiler can both run.
  // Serialize them because shader compilation uses shared error/notification state.
  static std::mutex compileMutex;
  std::lock_guard<std::mutex> lock(compileMutex);

  CState* pState = new CState();
  PShaderSet pShaders;
  RemoveAngleBrackets(sPresetFile);

  DWORD ApplyFlags = STATE_ALL;
  pState->Import(sPresetFile, GetTime(), NULL, ApplyFlags);
  LoadShaders(&pShaders, pState, false, true, bSkipIfCached);
  delete pState;
  pState = NULL;
}

void CPlugin::GenWarpPShaderText(char* szShaderText, float decay, bool bWrap) {
  // find the pixel shader body and replace it with custom code.

  lstrcpy(szShaderText, m_szDefaultWarpPShaderText);
  char LF = LINEFEED_CONTROL_CHAR;
  char* p = strrchr(szShaderText, '{');
  if (!p)
    return;
  p++;
  p += sprintf(p, "%c", 1);

  p += sprintf(p, "    // sample previous frame%c", LF);
  // SPOUT
  // Avoid freeze
  p += sprintf(p, "    ret = tex2D( sampler%ls_main, uv ).xyz;%c", bWrap ? L"" : L"_fc", LF);
  // p += sprintf(p, "    ret = tex2D( sampler%s_main, uv ).xyz;%c", bWrap ? L"" : L"_fc", LF);
  p += sprintf(p, "    %c", LF);
  p += sprintf(p, "    // darken (decay) over time%c", LF);
  p += sprintf(p, "    ret *= %.2f; //or try: ret -= 0.004;%c", decay, LF);
  // p += sprintf(p, "    %c", LF);
  // p += sprintf(p, "    ret.w = vDiffuse.w; // pass alpha along - req'd for preset blending%c", LF);
  p += sprintf(p, "}%c", LF);
}

void CPlugin::GenCompPShaderText(char* szShaderText, float brightness, float ve_alpha, float ve_zoom, int ve_orient, float hue_shader, bool bBrighten, bool bDarken, bool bSolarize, bool bInvert) {
  // find the pixel shader body and replace it with custom code.

  lstrcpy(szShaderText, m_szDefaultCompPShaderText);
  char LF = LINEFEED_CONTROL_CHAR;
  char* p = strrchr(szShaderText, '{');
  if (!p)
    return;
  p++;
  p += sprintf(p, "%c", 1);

  if (ve_alpha > 0.001f) {
    int orient_x = (ve_orient % 2) ? -1 : 1;
    int orient_y = (ve_orient >= 2) ? -1 : 1;
    p += sprintf(p, "    float2 uv_echo = (uv - 0.5)*%.3f*float2(%d,%d) + 0.5;%c", 1.0f / ve_zoom, orient_x, orient_y, LF);
    p += sprintf(p, "    ret = lerp( tex2D(sampler_main, uv).xyz, %c", LF);
    p += sprintf(p, "                tex2D(sampler_main, uv_echo).xyz, %c", LF);
    p += sprintf(p, "                %.2f %c", ve_alpha, LF);
    p += sprintf(p, "              ); //video echo%c", LF);
    p += sprintf(p, "    ret *= %.2f; //gamma%c", brightness, LF);
  } else {
    p += sprintf(p, "    ret = tex2D(sampler_main, uv).xyz;%c", LF);
    p += sprintf(p, "    ret *= %.2f; //gamma%c", brightness, LF);
  }
  if (hue_shader >= 1.0f)
    p += sprintf(p, "    ret *= hue_shader; //old hue shader effect%c", LF);
  else if (hue_shader > 0.001f)
    p += sprintf(p, "    ret *= %.2f + %.2f*hue_shader; //old hue shader effect%c", 1 - hue_shader, hue_shader, LF);

  if (bBrighten)
    p += sprintf(p, "    ret = sqrt(ret); //brighten%c", LF);
  if (bDarken)
    p += sprintf(p, "    ret *= ret; //darken%c", LF);
  if (bSolarize)
    p += sprintf(p, "    ret = ret*(1-ret)*4; //solarize%c", LF);
  if (bInvert)
    p += sprintf(p, "    ret = 1 - ret; //invert%c", LF);

  p += sprintf(p, "}%c", LF);
}

static std::string GetShaderCacheFileName(char* prefix, uint32_t shaderFlags, uint32_t checksum) {
  std::ostringstream fileName;
  fileName << prefix << "-" << std::hex << std::uppercase << shaderFlags << "-" << checksum << ".shader";
  return fileName.str();
}

// The shader cache always lives in the application directory, next to the executable and
// settings.ini - regardless of the working directory the app was launched from.
static std::filesystem::path GetShaderCacheFilePath(const wchar_t* baseDir, const std::string& fileName) {
  std::filesystem::path cacheDir = (baseDir != nullptr && baseDir[0] != L'\0') ? std::filesystem::path(baseDir) : std::filesystem::path(L".");
  return cacheDir / L"cache" / fileName;
}

void CPlugin::SaveShaderBytecodeToFile(ID3DXBuffer* pShaderByteCode, uint32_t checksum, char* prefix) {
  if (!pShaderByteCode || !checksum) return;

  std::filesystem::path filePath = GetShaderCacheFilePath(m_szBaseDir, GetShaderCacheFileName(prefix, m_dwShaderFlags, checksum));

  // Ensure the "cache" directory exists
  try {
    std::filesystem::create_directories(filePath.parent_path());
  } catch (const std::filesystem::filesystem_error& e) {
    std::cerr << "Failed to create or access cache directory: " << e.what() << std::endl;
    return;
  }

  std::ofstream outFile(filePath.wstring().c_str(), std::ios::binary);
  if (outFile.is_open()) {
    outFile.write(
        static_cast<const char*>(pShaderByteCode->GetBufferPointer()),
        pShaderByteCode->GetBufferSize());
    outFile.flush();
    outFile.close();
  }
}

ID3DXBuffer* CPlugin::LoadShaderBytecodeFromFile(uint32_t checksum, char* prefix) {
  ID3DXBuffer* pBuffer = nullptr;

  std::filesystem::path filePath = GetShaderCacheFilePath(m_szBaseDir, GetShaderCacheFileName(prefix, m_dwShaderFlags, checksum));

  std::ifstream inFile(filePath.wstring().c_str(), std::ios::binary | std::ios::ate);
  if (!inFile.is_open()) return nullptr;

  std::streamsize size = inFile.tellg();
  inFile.seekg(0, std::ios::beg);

  if (SUCCEEDED(D3DXCreateBuffer((UINT)size, &pBuffer))) {
    char* dest = static_cast<char*>(pBuffer->GetBufferPointer());
    if (!inFile.read(dest, size)) {
      pBuffer->Release();
      return nullptr;
    }
  }

  return pBuffer;
}

uint32_t CPlugin::crc32(const char* data, size_t length) {
  uint32_t crc = 0xFFFFFFFF;
  for (size_t i = 0; i < length; ++i) {
    crc ^= static_cast<uint8_t>(data[i]);
    for (int j = 0; j < 8; ++j) {
      if (crc & 1)
        crc = (crc >> 1) ^ 0xEDB88320;
      else
        crc >>= 1;
    }
  }
  return ~crc;
}
