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

// Phase 6: CTextManager renders text via GDI to a DIB section, then uploads
// to a DX12 texture and composites with premultiplied alpha blending.

#include "textmgr.h"
#include "dxcontext.h"
#include "dx12pipeline.h"
#include "support.h"

#define MAX_MSG_CHARS (65536*2)
wchar_t g_szMsgPool[2][MAX_MSG_CHARS];

CTextManager::CTextManager()
  : m_lpDevice(nullptr)
  , m_lpTextSurface(nullptr)
  , m_blit_additively(0)
  , m_lpDX(nullptr)
  , m_pFonts(nullptr)
  , m_nFonts(0)
  , m_memDC(nullptr)
  , m_hDIB(nullptr)
  , m_dibBits(nullptr)
  , m_texW(0)
  , m_texH(0)
  , m_dirty(false)
  , m_dx12Ready(false)
{
  m_b = 0;
  m_nMsg[0] = 0;
  m_nMsg[1] = 0;
  m_next_msg_start_ptr = g_szMsgPool[m_b];
}

CTextManager::~CTextManager() {
  CleanupDX12();
}

void CTextManager::Init(ID3D12Device* lpDevice, void* lpTextSurface, int bAdditive) {
  m_lpDevice        = lpDevice;
  m_lpTextSurface   = lpTextSurface;
  m_blit_additively = bAdditive;

  m_b = 0;
  m_nMsg[0] = 0;
  m_nMsg[1] = 0;
  m_next_msg_start_ptr = g_szMsgPool[m_b];
}

void CTextManager::Finish() {
  CleanupDX12();
}

int CTextManager::DecodeFontIndex(void* pFont) {
  if (!pFont) return -1;
  int idx = (int)(intptr_t)pFont - 1;
  return idx;
}

void CTextManager::InitDX12(DXContext* lpDX, HFONT* pFonts, int nFonts) {
  m_lpDX   = lpDX;
  m_pFonts = pFonts;
  m_nFonts = nFonts;

  if (!lpDX) return;
  UINT w = (UINT)lpDX->m_client_width;
  UINT h = (UINT)lpDX->m_client_height;
  if (w > 0 && h > 0)
    CreateDX12Resources(w, h);
}

void CTextManager::CleanupDX12() {
  DestroyDX12Resources();
  m_lpDX = nullptr;
  m_pFonts = nullptr;
  m_nFonts = 0;
}

void CTextManager::OnResize(int newW, int newH) {
  if (newW <= 0 || newH <= 0) return;
  if ((UINT)newW == m_texW && (UINT)newH == m_texH) return;
  DestroyDX12Resources();
  if (m_lpDX)
    CreateDX12Resources((UINT)newW, (UINT)newH);
}

bool CTextManager::CreateDX12Resources(UINT w, UINT h) {
  if (!m_lpDX || !m_lpDX->m_device) return false;

  auto* device = m_lpDX->m_device.Get();
  m_texW = w;
  m_texH = h;

  // Create GDI DIB section (32-bit BGRA, top-down)
  BITMAPINFO bmi = {};
  bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth       = w;
  bmi.bmiHeader.biHeight      = -(int)h;  // top-down
  bmi.bmiHeader.biPlanes      = 1;
  bmi.bmiHeader.biBitCount    = 32;
  bmi.bmiHeader.biCompression = BI_RGB;

  m_memDC = CreateCompatibleDC(nullptr);
  if (!m_memDC) return false;

  m_hDIB = CreateDIBSection(m_memDC, &bmi, DIB_RGB_COLORS, (void**)&m_dibBits, nullptr, 0);
  if (!m_hDIB || !m_dibBits) {
    DeleteDC(m_memDC);
    m_memDC = nullptr;
    return false;
  }
  SelectObject(m_memDC, m_hDIB);
  SetBkMode(m_memDC, TRANSPARENT);

  // Clear to transparent
  memset(m_dibBits, 0, (size_t)w * h * 4);

  // Create DX12 default-heap texture
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
      nullptr, IID_PPV_ARGS(&m_dx12Tex.resource));
  if (FAILED(hr)) {
    DeleteObject(m_hDIB); m_hDIB = nullptr;
    DeleteDC(m_memDC); m_memDC = nullptr;
    return false;
  }

  m_dx12Tex.width  = w;
  m_dx12Tex.height = h;
  m_dx12Tex.format = DXGI_FORMAT_B8G8R8A8_UNORM;
  m_dx12Tex.currentState = D3D12_RESOURCE_STATE_COPY_DEST;

  // Allocate SRV descriptor
  D3D12_CPU_DESCRIPTOR_HANDLE srvCpu = m_lpDX->AllocateSrvCpu();
  m_dx12Tex.srvIndex = m_lpDX->m_nextFreeSrvSlot;

  D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
  srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  srvDesc.Format                  = DXGI_FORMAT_B8G8R8A8_UNORM;
  srvDesc.ViewDimension           = D3D12_SRV_DIMENSION_TEXTURE2D;
  srvDesc.Texture2D.MipLevels     = 1;
  device->CreateShaderResourceView(m_dx12Tex.resource.Get(), &srvDesc, srvCpu);
  m_lpDX->AllocateSrvGpu();

  // Create 16-entry binding block for texture binding
  m_lpDX->CreateBindingBlockForTexture(m_dx12Tex);

  // Create upload buffer (row-pitch aligned)
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
      nullptr, IID_PPV_ARGS(&m_uploadBuf));
  if (FAILED(hr)) {
    m_dx12Tex.Reset();
    DeleteObject(m_hDIB); m_hDIB = nullptr;
    DeleteDC(m_memDC); m_memDC = nullptr;
    return false;
  }

  m_dx12Ready = true;
  m_dirty = false;
  return true;
}

void CTextManager::DestroyDX12Resources() {
  m_dx12Ready = false;
  m_uploadBuf.Reset();
  m_dx12Tex.Reset();
  if (m_hDIB) { DeleteObject(m_hDIB); m_hDIB = nullptr; }
  if (m_memDC) { DeleteDC(m_memDC); m_memDC = nullptr; }
  m_dibBits = nullptr;
  m_texW = 0;
  m_texH = 0;
}

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

int CTextManager::DrawText(void* pFont, char* szText, RECT* pRect, DWORD flags, DWORD color, bool bBox, DWORD boxColor) {
  return DrawTextW(pFont, AutoWide(szText), pRect, flags, color, bBox, boxColor);
}

int CTextManager::DrawTextW(void* pFont, wchar_t* szText, RECT* pRect, DWORD flags, DWORD color, bool bBox, DWORD boxColor) {
  if (!pRect) return 0;

  // For DT_CALCRECT: measure text immediately using GDI and return the height.
  // This is needed because callers use the result for layout calculations.
  if (flags & DT_CALCRECT) {
    if (m_memDC) {
      int fontIdx = DecodeFontIndex(pFont);
      HGDIOBJ oldFont = nullptr;
      if (fontIdx >= 0 && fontIdx < m_nFonts && m_pFonts && m_pFonts[fontIdx])
        oldFont = SelectObject(m_memDC, m_pFonts[fontIdx]);

      RECT rc = *pRect;
      int h = ::DrawTextW(m_memDC, szText ? szText : L"", -1, &rc, flags);
      *pRect = rc;

      if (oldFont) SelectObject(m_memDC, oldFont);
      return h;
    }
    return 0;
  }

  // Queue the text entry for rendering in DrawNow()
  if (!szText || !szText[0]) return 0;
  int len = (int)wcslen(szText);

  if ((m_nMsg[m_b] < MAX_MSGS) &&
      (DWORD)((DWORD_PTR)m_next_msg_start_ptr - (DWORD_PTR)g_szMsgPool[m_b]) + len + 1 < MAX_MSG_CHARS) {
    wcscpy(m_next_msg_start_ptr, szText);

    td_string& entry = m_msg[m_b][m_nMsg[m_b]];
    entry.msg     = m_next_msg_start_ptr;
    entry.pfont   = pFont;
    entry.rect    = *pRect;
    entry.flags   = flags;
    entry.color   = color;
    entry.bgColor = boxColor;
    m_nMsg[m_b]++;
    m_next_msg_start_ptr += len + 1;

    // If a dark box was requested, queue a box entry before this text
    if (bBox) {
      DrawBox(pRect, boxColor);
    }
  }

  return pRect->bottom - pRect->top;
}

void CTextManager::RenderQueuedMessages() {
  if (!m_memDC || !m_dibBits) return;

  // Clear DIB to fully transparent
  memset(m_dibBits, 0, (size_t)m_texW * m_texH * 4);

  int readBuf = 1 - m_b;  // read from the buffer that was filled last frame
  int nMsgs = m_nMsg[readBuf];
  if (nMsgs == 0) return;

  m_dirty = true;

  for (int i = 0; i < nMsgs; i++) {
    td_string& entry = m_msg[readBuf][i];

    if (!entry.pfont) {
      // Box entry (no font = dark box)
      BYTE a = (BYTE)((entry.bgColor >> 24) & 0xFF);
      BYTE r = (BYTE)((entry.bgColor >> 16) & 0xFF);
      BYTE g = (BYTE)((entry.bgColor >>  8) & 0xFF);
      BYTE b = (BYTE)((entry.bgColor >>  0) & 0xFF);

      // Clamp rect to texture bounds
      LONG x0 = max(0L, entry.rect.left);
      LONG y0 = max(0L, entry.rect.top);
      LONG x1 = min((LONG)m_texW, entry.rect.right);
      LONG y1 = min((LONG)m_texH, entry.rect.bottom);

      for (LONG y = y0; y < y1; y++) {
        for (LONG x = x0; x < x1; x++) {
          UINT idx = (y * m_texW + x) * 4;
          // Premultiplied alpha: store color * alpha
          m_dibBits[idx + 0] = (BYTE)(b * a / 255);  // B
          m_dibBits[idx + 1] = (BYTE)(g * a / 255);  // G
          m_dibBits[idx + 2] = (BYTE)(r * a / 255);  // R
          m_dibBits[idx + 3] = a;
        }
      }
      continue;
    }

    // Text entry
    int fontIdx = DecodeFontIndex(entry.pfont);
    HGDIOBJ oldFont = nullptr;
    if (fontIdx >= 0 && fontIdx < m_nFonts && m_pFonts && m_pFonts[fontIdx])
      oldFont = SelectObject(m_memDC, m_pFonts[fontIdx]);

    // Extract RGB from ARGB color (D3DCOLOR format)
    BYTE cr = (BYTE)((entry.color >> 16) & 0xFF);
    BYTE cg = (BYTE)((entry.color >>  8) & 0xFF);
    BYTE cb = (BYTE)((entry.color >>  0) & 0xFF);
    SetTextColor(m_memDC, RGB(cr, cg, cb));

    RECT rc = entry.rect;
    // Remove DT_CALCRECT if somehow set (shouldn't be, but safety)
    DWORD drawFlags = entry.flags & ~DT_CALCRECT;
    ::DrawTextW(m_memDC, entry.msg, -1, &rc, drawFlags | DT_NOPREFIX);

    if (oldFont) SelectObject(m_memDC, oldFont);
  }

  // Post-process: GDI renders colored text with A=0.
  // Convert to premultiplied alpha: A = max(R,G,B), keep R,G,B as-is.
  // Pixels already set by box rendering (A > 0) are left untouched.
  BYTE* pixels = m_dibBits;
  for (UINT y = 0; y < m_texH; y++) {
    for (UINT x = 0; x < m_texW; x++) {
      UINT idx = (y * m_texW + x) * 4;
      BYTE b = pixels[idx + 0];
      BYTE g = pixels[idx + 1];
      BYTE r = pixels[idx + 2];
      BYTE a = pixels[idx + 3];
      if (a == 0 && (r | g | b) != 0) {
        // This pixel was drawn by GDI text (A=0 but has color).
        // Set alpha to coverage (max channel value).
        BYTE intensity = (r > g) ? ((r > b) ? r : b) : ((g > b) ? g : b);
        pixels[idx + 3] = intensity;
      }
    }
  }
}

void CTextManager::UploadAndDraw() {
  if (!m_dx12Ready || !m_lpDX || !m_dirty) return;
  if (!m_dx12Tex.IsValid() || !m_uploadBuf) return;

  UINT rowPitch = (m_texW * 4 + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1)
                  & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1);

  // Copy DIB to upload buffer with row-pitch alignment
  BYTE* uploadPtr = nullptr;
  m_uploadBuf->Map(0, nullptr, (void**)&uploadPtr);
  if (uploadPtr) {
    for (UINT y = 0; y < m_texH; y++) {
      memcpy(uploadPtr + y * rowPitch, m_dibBits + y * m_texW * 4, m_texW * 4);
    }
    m_uploadBuf->Unmap(0, nullptr);
  }

  auto* cmdList = m_lpDX->m_commandList.Get();

  // Transition to COPY_DEST, upload, then to PIXEL_SHADER_RESOURCE
  m_lpDX->TransitionResource(m_dx12Tex, D3D12_RESOURCE_STATE_COPY_DEST);

  D3D12_TEXTURE_COPY_LOCATION src = {};
  src.pResource = m_uploadBuf.Get();
  src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
  src.PlacedFootprint.Offset = 0;
  src.PlacedFootprint.Footprint.Format   = DXGI_FORMAT_B8G8R8A8_UNORM;
  src.PlacedFootprint.Footprint.Width    = m_texW;
  src.PlacedFootprint.Footprint.Height   = m_texH;
  src.PlacedFootprint.Footprint.Depth    = 1;
  src.PlacedFootprint.Footprint.RowPitch = rowPitch;

  D3D12_TEXTURE_COPY_LOCATION dst = {};
  dst.pResource = m_dx12Tex.resource.Get();
  dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
  dst.SubresourceIndex = 0;

  cmdList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

  m_lpDX->TransitionResource(m_dx12Tex, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

  // Draw fullscreen quad with premultiplied alpha blend
  ID3D12DescriptorHeap* heaps[] = { m_lpDX->m_srvHeap.Get() };
  cmdList->SetDescriptorHeaps(1, heaps);
  cmdList->SetGraphicsRootSignature(m_lpDX->m_rootSignature.Get());

  D3D12_GPU_DESCRIPTOR_HANDLE srvHandle = m_lpDX->GetBindingBlockGpuHandle(m_dx12Tex);
  cmdList->SetGraphicsRootDescriptorTable(1, srvHandle);

  cmdList->SetPipelineState(m_lpDX->m_PSOs[PSO_PREMULALPHA_SPRITEVERTEX].Get());

  SPRITEVERTEX verts[6];
  DWORD white = 0xFFFFFFFF;
  verts[0] = { -1.f,  1.f, 0, white, 0, 0 };
  verts[1] = {  1.f,  1.f, 0, white, 1, 0 };
  verts[2] = { -1.f, -1.f, 0, white, 0, 1 };
  verts[3] = { -1.f, -1.f, 0, white, 0, 1 };
  verts[4] = {  1.f,  1.f, 0, white, 1, 0 };
  verts[5] = {  1.f, -1.f, 0, white, 1, 1 };

  m_lpDX->DrawVertices(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, verts, 6, sizeof(SPRITEVERTEX));
}

void CTextManager::DrawNow() {
  // Render all queued text from the previous buffer to the DIB
  RenderQueuedMessages();

  // Upload to GPU and draw the textured quad
  UploadAndDraw();

  // Flip double buffer and clear for next frame
  m_b = 1 - m_b;
  ClearAll();
  m_dirty = false;
}
