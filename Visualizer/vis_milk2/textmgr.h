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

#ifndef GEISS_TEXT_DRAWING_MANAGER
#define GEISS_TEXT_DRAWING_MANAGER 1

// Phase 6: CTextManager renders text via GDI to a DIB section, then uploads
// to a DX12 texture and composites with premultiplied alpha blending.
// The pFont parameter encodes a font index: (void*)(intptr_t)(fontIndex + 1).

#include <d3d12.h>
#include <wrl/client.h>
#include "md_defines.h"
#include "AutoWide.h"
#include "dx12helpers.h"

using Microsoft::WRL::ComPtr;

#define MAX_MSGS 4096

typedef struct {
  wchar_t* msg;       // points to some character in g_szMsgPool[2][].
  void*    pfont;     // encoded font index: (void*)(intptr_t)(fontIndex + 1)
  RECT rect;
  DWORD flags;
  DWORD color;
  DWORD bgColor;
  int added, deleted;        // temporary; used during DrawNow()
  void* prev_dark_box_ptr;   // temporary; used during DrawNow()
}
td_string;

class DXContext;  // forward declaration

class CTextManager {
public:
  CTextManager();
  ~CTextManager();

  // Legacy init — stores device pointer and initializes message queue.
  void Init(ID3D12Device* lpDevice, void* lpTextSurface, int bAdditive);
  void Finish();

  // DX12 GDI text rendering — call after GDI fonts are created.
  // pFonts: array of HFONT handles (from CPluginShell::m_font[])
  // nFonts: number of fonts in the array
  void InitDX12(DXContext* lpDX, HFONT* pFonts, int nFonts);
  void CleanupDX12();
  void OnResize(int newW, int newH);

  // Text drawing — queues entries; actual rendering happens in DrawNow().
  // DT_CALCRECT calls are handled immediately via GDI measurement.
  int  DrawText(void* pFont, char* szText, RECT* pRect, DWORD flags, DWORD color, bool bBlackBox, DWORD boxColor = 0xFF000000);
  int  DrawText(void* pFont, char* szText, int len, RECT* pRect, DWORD flags, DWORD color, bool bBox, DWORD boxColor = 0xFF000000) {
    return DrawTextW(pFont, AutoWide(szText), pRect, flags, color, bBox, boxColor);
  };
  int  DrawTextW(void* pFont, wchar_t* szText, RECT* pRect, DWORD flags, DWORD color, bool bBlackBox, DWORD boxColor = 0xFF000000);
  int  DrawTextW(void* pFont, wchar_t* szText, int len, RECT* pRect, DWORD flags, DWORD color, bool bBox, DWORD boxColor = 0xFF000000) {
    return DrawTextW(pFont, szText, pRect, flags, color, bBox, boxColor);
  };
  void DrawBox(LPRECT pRect, DWORD boxColor);
  void DrawDarkBox(LPRECT pRect) { DrawBox(pRect, 0xFF000000); }
  void DrawNow();
  void ClearAll(); // automatically called @ end of DrawNow()

  // Decode font index from the pFont sentinel value.
  // Returns -1 if pFont is null or invalid.
  static int DecodeFontIndex(void* pFont);

protected:
  ID3D12Device*  m_lpDevice;
  void*          m_lpTextSurface;
  int            m_blit_additively;

  int       m_nMsg[2];
  td_string m_msg[2][MAX_MSGS];
  wchar_t* m_next_msg_start_ptr;
  int       m_b;

  // DX12 GDI text rendering resources
  DXContext*  m_lpDX;
  HFONT*      m_pFonts;       // borrowed pointer to CPluginShell::m_font[]
  int         m_nFonts;

  // GDI DIB for CPU-side text rendering
  HDC         m_memDC;
  HBITMAP     m_hDIB;
  BYTE*       m_dibBits;
  UINT        m_texW;
  UINT        m_texH;

  // DX12 texture + upload buffer
  DX12Texture m_dx12Tex;
  ComPtr<ID3D12Resource> m_uploadBuf;
  bool        m_dirty;
  bool        m_dx12Ready;

  bool CreateDX12Resources(UINT w, UINT h);
  void DestroyDX12Resources();
  void RenderQueuedMessages();
  void UploadAndDraw();
};

#endif
