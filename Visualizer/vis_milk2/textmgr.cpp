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

// Phase 5 TODO: Re-implement CTextManager using DirectXTK12 SpriteFont/SpriteBatch.
// All drawing is currently a no-op; the message queue is maintained so call sites
// don't need to change.  Only DrawNow() and the per-message Draw* methods are stubbed.

#include "textmgr.h"

#define MAX_MSG_CHARS (65536*2)
wchar_t g_szMsgPool[2][MAX_MSG_CHARS];

CTextManager::CTextManager() {}

CTextManager::~CTextManager() {}

void CTextManager::Init(ID3D12Device* lpDevice, void* lpTextSurface, int bAdditive) {
  m_lpDevice        = lpDevice;
  m_lpTextSurface   = lpTextSurface;
  m_blit_additively = bAdditive;

  m_b = 0;
  m_nMsg[0] = 0;
  m_nMsg[1] = 0;
  m_next_msg_start_ptr = g_szMsgPool[m_b];
}

void CTextManager::Finish() {}

void CTextManager::ClearAll() {
  m_nMsg[m_b] = 0;
  m_next_msg_start_ptr = g_szMsgPool[m_b];
}

void CTextManager::DrawBox(LPRECT pRect, DWORD boxColor) {
  if (!pRect)
    return;

  if ((m_nMsg[m_b] < MAX_MSGS) &&
    (DWORD)((DWORD_PTR)m_next_msg_start_ptr - (DWORD_PTR)g_szMsgPool[m_b]) + 0 + 1 < MAX_MSG_CHARS) {
    *m_next_msg_start_ptr = 0;

    m_msg[m_b][m_nMsg[m_b]].msg   = m_next_msg_start_ptr;
    m_msg[m_b][m_nMsg[m_b]].pfont = nullptr;
    m_msg[m_b][m_nMsg[m_b]].rect  = *pRect;
    m_msg[m_b][m_nMsg[m_b]].flags = 0;
    m_msg[m_b][m_nMsg[m_b]].color = 0xFFFFFFFF;
    m_msg[m_b][m_nMsg[m_b]].bgColor = boxColor;
    m_nMsg[m_b]++;
    m_next_msg_start_ptr += 1;
  }
}

// Phase 5 TODO: implement with SpriteFont when DirectXTK12 is integrated.
int CTextManager::DrawText(void* pFont, char* szText, RECT* pRect, DWORD flags, DWORD color, bool bBox, DWORD boxColor) {
  return 0;
}

// Phase 5 TODO: implement with SpriteFont when DirectXTK12 is integrated.
int CTextManager::DrawTextW(void* pFont, wchar_t* szText, RECT* pRect, DWORD flags, DWORD color, bool bBox, DWORD boxColor) {
  return 0;
}

// Phase 5 TODO: replace with DirectXTK12 SpriteBatch rendering.
// For now just advance the double-buffer and clear — no text is drawn.
void CTextManager::DrawNow() {
  // flip:
  m_b = 1 - m_b;
  ClearAll();
}
