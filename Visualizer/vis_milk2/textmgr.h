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

// Phase 5 TODO: Replace void* font handles with DirectXTK12 SpriteFont*.
// For now all font operations are no-ops until text rendering is ported.
#include <d3d12.h>
#include <wrl/client.h>
#include "md_defines.h"
#include "AutoWide.h"

#define MAX_MSGS 4096

typedef struct {
  wchar_t* msg;       // points to some character in g_szMsgPool[2][].
  void*    pfont;     // Phase 5: will be DirectXTK12 SpriteFont*; NULL for dark boxes
  RECT rect;
  DWORD flags;
  DWORD color;
  DWORD bgColor;
  int added, deleted;        // temporary; used during DrawNow()
  void* prev_dark_box_ptr;   // temporary; used during DrawNow()
}
td_string;

class CTextManager {
public:
  CTextManager();
  ~CTextManager();

  // Phase 5 TODO: fully implement with DirectXTK12 SpriteFont.
  // lpDevice stored for future use; lpTextSurface unused until Phase 5.
  void Init(ID3D12Device* lpDevice, void* lpTextSurface, int bAdditive);
  void Finish();

  // Phase 5 TODO: font rendering — currently queues entries but DrawNow is a no-op.
  // pFont is a void* placeholder (will be SpriteFont* in Phase 5).
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

protected:
  ID3D12Device*  m_lpDevice;       // Phase 5: used for DirectXTK12 rendering
  void*          m_lpTextSurface;  // Phase 5: render target for text
  int            m_blit_additively;

  int       m_nMsg[2];
  td_string m_msg[2][MAX_MSGS];
  wchar_t* m_next_msg_start_ptr;
  int       m_b;
};

#endif
