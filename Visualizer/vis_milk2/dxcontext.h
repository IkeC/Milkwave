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

#ifndef __NULLSOFT_DX12_PLUGIN_SHELL_DXCONTEXT_H__
#define __NULLSOFT_DX12_PLUGIN_SHELL_DXCONTEXT_H__ 1

#include <windows.h>
#include "shell_defines.h"

// DirectX 12 headers (part of Windows SDK — no legacy DXSDK_DIR required)
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <wrl/client.h>   // ComPtr

using Microsoft::WRL::ComPtr;

#include "dx12helpers.h"   // DX12Texture
#include "dx12pipeline.h"  // PSO IDs, DX12CreatePipelines

// Number of frames in flight (back-buffer count for the swap chain)
#define DXC_FRAME_COUNT 2

// Descriptor heap sizes
#define DXC_MAX_RTV  32   // 2 back buffers + 2 VS + 6 blur + 10 title + spare
#define DXC_MAX_SRV  256  // texture SRVs, CBVs, etc.
#define DXC_MAX_SAMPLERS 16

#define SNAP_WINDOWED_MODE_BLOCKSIZE  32

typedef struct {
  int  nbackbuf;
  int  allow_page_tearing;
  GUID adapter_guid;
  char adapter_devicename[256];

  UINT   adapter_index;    // DXGI adapter ordinal (replaces DX9 adapter GUID)
  HWND   parent_window;
  int m_dualhead_horz;
  int m_dualhead_vert;
  int m_skin;
} DXCONTEXT_PARAMS;

class DXContext {
public:
  // ----- Public interface -----

  // Create from an already-initialized DX12 device + command queue.
  // The swap chain is created internally from hwnd.
  DXContext(
    ID3D12Device*        device,
    ID3D12CommandQueue*  commandQueue,
    IDXGIFactory4*       factory,
    HWND                 hwnd,
    int                  width,
    int                  height,
    wchar_t*             szIniFile);

  ~DXContext();

  BOOL StartOrRestartDevice(DXCONTEXT_PARAMS* pParams);
  void OnTrulyExiting() { m_truly_exiting = 1; }
  void UpdateMonitorWorkRect();
  int  GetBitDepth() { return m_bpp; }

  void SaveWindow();
  HWND GetHwnd();
  bool OnUserResizeWindow(RECT* w, RECT* c, bool bSetBackBuffer);
  bool TempIgnoreDestroyMessages();

  // Resize the swap chain to match new client dimensions.
  bool ResizeSwapChain(int newWidth, int newHeight);

  // Frame lifecycle — called by the render loop.
  // BeginFrame: transitions back buffer to render-target state, returns true on success.
  bool BeginFrame();
  // EndFrame: transitions back buffer to present, executes the command list, presents.
  void EndFrame();

  // CPU/GPU synchronisation helpers.
  void WaitForGpu();      // flush all in-flight GPU work and idle the queue
  void MoveToNextFrame(); // advance frame index; waits if the next slot is still in use

  // ----- Public data (read-only from outside) -----
  int m_ready;
  HRESULT m_lastErr;
  int m_window_width;
  int m_window_height;
  int m_backbuffer_width;
  int m_backbuffer_height;
  int m_client_width;
  int m_client_height;
  int m_REAL_client_width;
  int m_REAL_client_height;
  int m_fake_fs_covers_all;
  int m_frame_delay;
  RECT m_all_monitors_rect;
  RECT m_monitor_rect;
  RECT m_monitor_rect_orig;
  RECT m_monitor_work_rect;
  RECT m_monitor_work_rect_orig;
  DXCONTEXT_PARAMS m_current_mode;

  // ----- DX12 objects (public for access from plugin render code) -----
  ComPtr<ID3D12Device>               m_device;
  ComPtr<ID3D12CommandQueue>         m_commandQueue;
  ComPtr<IDXGISwapChain4>            m_swapChain;
  ComPtr<ID3D12GraphicsCommandList>  m_commandList;

  // Per-frame resources (indexed by m_frameIndex)
  ComPtr<ID3D12CommandAllocator>     m_commandAllocators[DXC_FRAME_COUNT];

  // Back-buffer render targets
  ComPtr<ID3D12Resource>             m_renderTargets[DXC_FRAME_COUNT];

  // Descriptor heaps
  ComPtr<ID3D12DescriptorHeap>       m_rtvHeap;          // RTV heap (DXC_MAX_RTV entries)
  ComPtr<ID3D12DescriptorHeap>       m_srvHeap;          // SRV/CBV/UAV heap (shader-visible)
  ComPtr<ID3D12DescriptorHeap>       m_samplerHeap;      // Sampler heap (shader-visible)
  UINT                               m_rtvDescriptorSize;
  UINT                               m_srvDescriptorSize;
  UINT                               m_samplerDescriptorSize;

  // Bump allocators for descriptor slots (slots 0..N-1 for back buffers already taken)
  UINT                               m_nextFreeRtvSlot;  // starts at DXC_FRAME_COUNT
  UINT                               m_nextFreeSrvSlot;  // starts at 0

  // Baseline values after one-time init (null texture + per-frame bindings).
  // ResetDynamicDescriptors() rewinds the bump allocators to these values so that
  // textures allocated in AllocateMyDX9Stuff can be safely re-created on resize/toggle.
  UINT                               m_rtvSlotBaseline = 0;
  UINT                               m_srvSlotBaseline = 0;

  // Helpers for allocating descriptor slots
  D3D12_CPU_DESCRIPTOR_HANDLE AllocateRtv();   // returns CPU handle, bumps m_nextFreeRtvSlot
  D3D12_CPU_DESCRIPTOR_HANDLE AllocateSrvCpu(); // returns CPU handle at m_nextFreeSrvSlot
  D3D12_GPU_DESCRIPTOR_HANDLE AllocateSrvGpu(); // returns GPU handle, bumps m_nextFreeSrvSlot

  // Reset bump allocators to post-init baseline (call before re-creating dynamic textures)
  void ResetDynamicDescriptors();

  // Texture creation helpers (Phase 2)
  DX12Texture CreateRenderTargetTexture(UINT width, UINT height, DXGI_FORMAT format);

  // Resource state transitions (Phase 4)
  void TransitionResource(DX12Texture& tex, D3D12_RESOURCE_STATES newState);
  D3D12_GPU_DESCRIPTOR_HANDLE GetSrvGpuHandle(const DX12Texture& tex);
  D3D12_CPU_DESCRIPTOR_HANDLE GetRtvCpuHandle(const DX12Texture& tex);

  // Root signature (Phase 3, updated Phase 4: static samplers + 1-SRV table)
  // Layout: [0] CBV (b0), [1] descriptor table 1 SRV (t0), + 4 static samplers s0-s3
  ComPtr<ID3D12RootSignature> m_rootSignature;
  bool CreateRootSignature();

  // Pipeline state objects (Phase 3)
  ComPtr<ID3D12PipelineState> m_PSOs[PSO_COUNT];
  bool CreatePipelines();

  // Upload heap ring buffers (Phase 3) — per-frame suballocation for DrawPrimitiveUP replacement
  static const UINT UPLOAD_BUFFER_SIZE = 8 * 1024 * 1024; // 8 MB vertex upload per frame
  ComPtr<ID3D12Resource> m_uploadBuffer[DXC_FRAME_COUNT];
  BYTE*                  m_uploadBufferPtr[DXC_FRAME_COUNT]; // persistently mapped
  UINT                   m_uploadBufferOffset[DXC_FRAME_COUNT]; // current write position
  bool CreateUploadBuffers();
  void ResetUploadBuffer(); // called at BeginFrame — resets offset to 0

  // Draw helper: suballocates from upload buffer, copies vertices, issues draw call
  void DrawVertices(D3D12_PRIMITIVE_TOPOLOGY topology, const void* vertexData,
                    UINT vertexCount, UINT vertexStride);

  // CBV upload helper: suballocates from upload buffer with 256-byte alignment, returns GPU VA
  D3D12_GPU_VIRTUAL_ADDRESS UploadConstantBuffer(const void* data, UINT sizeBytes);

  // Null texture (1x1 black) for filling unused SRV slots
  DX12Texture m_nullTexture;
  bool CreateNullTexture();

  // Create a 16-entry binding block for a texture (all slots = tex for simple passthrough)
  // Writes to tex.bindingBlockStart. Call after CreateNullTexture().
  static const UINT BINDING_BLOCK_SIZE = 16;
  void CreateBindingBlockForTexture(DX12Texture& tex);

  // Create a 16-entry binding block with tex at a specific slot, null elsewhere.
  // Returns the starting SRV heap index for the block. Used for preset shaders
  // where sampler_main maps to a t-register other than t0.
  UINT CreateBindingBlockAtSlot(const DX12Texture& tex, UINT mainSlot);

  // Get GPU handle for a texture's binding block (all 16 SRV slots)
  D3D12_GPU_DESCRIPTOR_HANDLE GetBindingBlockGpuHandle(const DX12Texture& tex);

  // Get GPU handle for a binding block by its starting SRV index
  D3D12_GPU_DESCRIPTOR_HANDLE GetBindingBlockGpuHandleByIndex(UINT blockStart);

  // Per-frame binding blocks: avoids GPU race by using separate descriptor ranges per frame.
  // 2 frames × 2 passes (warp + comp) × 16 SRV descriptors = 64 total.
  UINT m_perFrameBindingBase = UINT_MAX;
  bool AllocatePerFrameBindings(); // call once at init, after CreateNullTexture
  void UpdatePerFrameBindings(const DX12Texture& warpSrc, UINT warpMainSlot,
                              const DX12Texture& compSrc, UINT compMainSlot);
  D3D12_GPU_DESCRIPTOR_HANDLE GetWarpBindingGpuHandle();
  D3D12_GPU_DESCRIPTOR_HANDLE GetCompBindingGpuHandle();

  // Current frame index within [0, DXC_FRAME_COUNT)
  UINT  m_frameIndex;

  // Fence for CPU/GPU synchronisation
  ComPtr<ID3D12Fence> m_fence;
  UINT64              m_fenceValues[DXC_FRAME_COUNT];
  HANDLE              m_fenceEvent;

protected:
  HWND    m_hwnd;
  wchar_t m_szIniFile[MAX_PATH];
  int     m_truly_exiting;
  int     m_bpp;
  char    m_szWindowCaption[512];

  void WriteSafeWindowPos();
  bool Internal_Init(IDXGIFactory4* factory, HWND hwnd, int width, int height);
  void Internal_CleanUp();
  void SetViewport(int width, int height);
  void CreateRtvsForSwapChain();
  void ReleaseSwapChainRtvs();
};

// Error codes (kept for compatibility)
#define DXC_ERR_REGWIN    -2
#define DXC_ERR_CREATEWIN -3
#define DXC_ERR_CREATE3D  -4
#define DXC_ERR_GETFORMAT -5
#define DXC_ERR_FORMAT    -6
#define DXC_ERR_CREATEDEV_PROBABLY_OUTOFVIDEOMEMORY -7
#define DXC_ERR_RESIZEFAILED -8
#define DXC_ERR_CAPSFAIL  -9
#define DXC_ERR_BAD_FS_DISPLAYMODE -10
#define DXC_ERR_USER_CANCELED -11
#define DXC_ERR_CREATEDEV_NOT_AVAIL -12
#define DXC_ERR_CREATEDDRAW  -13

#endif // __NULLSOFT_DX12_PLUGIN_SHELL_DXCONTEXT_H__
