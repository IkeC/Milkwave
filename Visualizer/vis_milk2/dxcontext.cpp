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

#include "DXContext.h"
#include "utility.h"
#include "shell_defines.h"
#include <strsafe.h>
#include <cassert>
#include <cstdio>

// Swap chain back-buffer pixel format
static const DXGI_FORMAT k_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

DXContext::DXContext(
    ID3D12Device*        device,
    ID3D12CommandQueue*  commandQueue,
    IDXGIFactory4*       factory,
    HWND                 hwnd,
    int                  width,
    int                  height,
    wchar_t*             szIniFile)
{
    m_szWindowCaption[0] = 0;
    m_hwnd               = hwnd;
    m_truly_exiting      = 0;
    m_bpp                = 32;
    m_frame_delay        = 0;
    m_ready              = FALSE;
    m_lastErr            = S_OK;
    m_frameIndex         = 0;
    m_fenceEvent         = nullptr;
    m_rtvDescriptorSize     = 0;
    m_srvDescriptorSize     = 0;
    m_samplerDescriptorSize = 0;
    m_nextFreeRtvSlot       = DXC_FRAME_COUNT;  // slots 0..1 reserved for back buffers
    m_nextFreeSrvSlot       = 0;

    for (int i = 0; i < DXC_FRAME_COUNT; i++)
        m_fenceValues[i] = 0;

    ZeroMemory(&m_current_mode, sizeof(m_current_mode));

    StringCbCopyW(m_szIniFile, sizeof(m_szIniFile), szIniFile);

    // Store device references passed from the initializer
    m_device       = device;
    m_commandQueue = commandQueue;

    // Create swap chain, RTVs, command allocators, command list, fence
    if (!Internal_Init(factory, hwnd, width, height)) {
        m_lastErr = DXC_ERR_CREATE3D;
        return;
    }

    m_client_width       = m_REAL_client_width  = width;
    m_client_height      = m_REAL_client_height = height;
    m_window_width       = width;
    m_window_height      = height;
    m_backbuffer_width   = width;
    m_backbuffer_height  = height;
    m_ready              = TRUE;

    {
        char buf[256];
        sprintf(buf, "DX12: DXContext init: swap chain %dx%d, client %dx%d\n",
                width, height, m_client_width, m_client_height);
        OutputDebugStringA(buf);
    }
}

DXContext::~DXContext()
{
    Internal_CleanUp();
}

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

bool DXContext::Internal_Init(IDXGIFactory4* factory, HWND hwnd, int width, int height)
{
    HRESULT hr;

    // 1. RTV descriptor heap (expanded: back buffers + render target textures)
    {
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = DXC_MAX_RTV;
        rtvHeapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        hr = m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap));
        if (FAILED(hr)) return false;
        m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }

    // 1b. SRV/CBV/UAV descriptor heap (shader-visible, for texture SRVs and CBVs)
    {
        D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
        srvHeapDesc.NumDescriptors = DXC_MAX_SRV;
        srvHeapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srvHeapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        hr = m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap));
        if (FAILED(hr)) return false;
        m_srvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }

    // 1c. Sampler descriptor heap (shader-visible)
    {
        D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
        samplerHeapDesc.NumDescriptors = DXC_MAX_SAMPLERS;
        samplerHeapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
        samplerHeapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        hr = m_device->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&m_samplerHeap));
        if (FAILED(hr)) return false;
        m_samplerDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
    }

    // 2. Swap chain
    {
        DXGI_SWAP_CHAIN_DESC1 scDesc = {};
        scDesc.Width              = (UINT)width;
        scDesc.Height             = (UINT)height;
        scDesc.Format             = k_BackBufferFormat;
        scDesc.Stereo             = FALSE;
        scDesc.SampleDesc.Count   = 1;
        scDesc.SampleDesc.Quality = 0;
        scDesc.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scDesc.BufferCount        = DXC_FRAME_COUNT;
        scDesc.Scaling            = DXGI_SCALING_STRETCH;
        scDesc.SwapEffect         = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        scDesc.AlphaMode          = DXGI_ALPHA_MODE_UNSPECIFIED;
        scDesc.Flags              = 0;

        ComPtr<IDXGISwapChain1> swapChain1;
        hr = factory->CreateSwapChainForHwnd(
            m_commandQueue.Get(),
            hwnd,
            &scDesc,
            nullptr,  // no fullscreen desc — windowed only for now
            nullptr,
            &swapChain1);
        if (FAILED(hr)) return false;

        // Disable Alt+Enter fullscreen toggle (handled by the app)
        factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);

        hr = swapChain1.As(&m_swapChain);
        if (FAILED(hr)) return false;
    }

    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    // 3. RTVs for each back buffer
    CreateRtvsForSwapChain();

    // 4. Per-frame command allocators
    for (UINT i = 0; i < DXC_FRAME_COUNT; i++) {
        hr = m_device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(&m_commandAllocators[i]));
        if (FAILED(hr)) return false;
    }

    // 5. Command list (initially closed after creation)
    hr = m_device->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        m_commandAllocators[m_frameIndex].Get(),
        nullptr,
        IID_PPV_ARGS(&m_commandList));
    if (FAILED(hr)) return false;
    m_commandList->Close();

    // 6. Fence + event for CPU/GPU synchronisation
    {
        hr = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
        if (FAILED(hr)) return false;
        m_fenceValues[m_frameIndex] = 1;

        m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (!m_fenceEvent) return false;
    }

    // 7. Root signature (Phase 3)
    if (!CreateRootSignature()) return false;

    // 8. Upload heap buffers (Phase 3)
    if (!CreateUploadBuffers()) return false;

    // 9. Pipeline state objects (Phase 3)
    if (!CreatePipelines()) return false;

    // 10. Null texture for filling unused SRV slots (Phase 5)
    if (!CreateNullTexture()) return false;

    // 11. Per-frame binding block ranges (Phase 5)
    if (!AllocatePerFrameBindings()) return false;

    // 12. Populate monitor geometry
    UpdateMonitorWorkRect();

    // 13. Snapshot allocator positions — everything above is permanent.
    //     Dynamic textures (VS, blur) allocated in AllocateMyDX9Stuff
    //     will be reclaimed on resize/toggle by resetting to these baselines.
    m_rtvSlotBaseline = m_nextFreeRtvSlot;
    m_srvSlotBaseline = m_nextFreeSrvSlot;

    return true;
}

void DXContext::CreateRtvsForSwapChain()
{
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
    for (UINT i = 0; i < DXC_FRAME_COUNT; i++) {
        HRESULT hr = m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i]));
        if (SUCCEEDED(hr)) {
            m_device->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);
        }
        rtvHandle.ptr += m_rtvDescriptorSize;
    }
}

void DXContext::ReleaseSwapChainRtvs()
{
    for (UINT i = 0; i < DXC_FRAME_COUNT; i++)
        m_renderTargets[i].Reset();
}

void DXContext::Internal_CleanUp()
{
    if (m_ready) {
        WaitForGpu();
        m_ready = FALSE;
    }

    if (m_fenceEvent) {
        CloseHandle(m_fenceEvent);
        m_fenceEvent = nullptr;
    }
}

// ---------------------------------------------------------------------------
// Frame lifecycle
// ---------------------------------------------------------------------------

bool DXContext::BeginFrame()
{
    if (!m_ready) return false;

    // One-time dimension sanity check: verify swap chain matches stored dimensions
    static bool s_checkedOnce = false;
    if (!s_checkedOnce && m_swapChain && m_hwnd) {
        s_checkedOnce = true;
        DXGI_SWAP_CHAIN_DESC1 scDesc = {};
        m_swapChain->GetDesc1(&scDesc);
        RECT rc;
        GetClientRect(m_hwnd, &rc);
        int clientW = rc.right - rc.left;
        int clientH = rc.bottom - rc.top;
        char buf[512];
        sprintf(buf, "DX12: BeginFrame check: swapchain=%ux%u, m_client=%dx%d, actual_client=%dx%d\n",
                scDesc.Width, scDesc.Height, m_client_width, m_client_height, clientW, clientH);
        OutputDebugStringA(buf);
    }

    // Reset command allocator and command list for this frame
    HRESULT hr = m_commandAllocators[m_frameIndex]->Reset();
    if (FAILED(hr)) return false;

    hr = m_commandList->Reset(m_commandAllocators[m_frameIndex].Get(), nullptr);
    if (FAILED(hr)) return false;

    // Transition back buffer: PRESENT → RENDER_TARGET
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource   = m_renderTargets[m_frameIndex].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    m_commandList->ResourceBarrier(1, &barrier);

    // Set back buffer as the render target
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
    rtvHandle.ptr += (SIZE_T)m_frameIndex * m_rtvDescriptorSize;
    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    // Reset upload buffer for this frame
    ResetUploadBuffer();

    return true;
}

void DXContext::EndFrame()
{
    if (!m_ready) return;

    // Transition back buffer: RENDER_TARGET → PRESENT
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource   = m_renderTargets[m_frameIndex].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    m_commandList->ResourceBarrier(1, &barrier);

    HRESULT hrClose = m_commandList->Close();
    if (FAILED(hrClose)) {
        char buf[256];
        sprintf(buf, "DX12: EndFrame: Close FAILED hr=0x%08X devRemoved=0x%08X",
                (unsigned)hrClose, (unsigned)m_device->GetDeviceRemovedReason());
        DebugLogA(buf);
    }

    // Execute
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(1, ppCommandLists);

    // Present (vsync = 1 sync interval)
    HRESULT hrPresent = m_swapChain->Present(1, 0);
    if (FAILED(hrPresent)) {
        char buf[256];
        sprintf(buf, "DX12: Present FAILED hr=0x%08X", (unsigned)hrPresent);
        DebugLogA(buf);
        if (hrPresent == DXGI_ERROR_DEVICE_REMOVED || hrPresent == DXGI_ERROR_DEVICE_RESET) {
            HRESULT reason = m_device->GetDeviceRemovedReason();
            sprintf(buf, "DX12: Device removed reason: 0x%08X", (unsigned)reason);
            DebugLogA(buf);
        }
    }

    MoveToNextFrame();
}

// ---------------------------------------------------------------------------
// Synchronisation
// ---------------------------------------------------------------------------

void DXContext::WaitForGpu()
{
    if (!m_commandQueue || !m_fence || !m_fenceEvent) return;

    // Signal a fence value from the GPU
    UINT64 fenceValue = m_fenceValues[m_frameIndex];
    m_commandQueue->Signal(m_fence.Get(), fenceValue);

    // Wait for the GPU to reach that value (5s timeout)
    if (m_fence->GetCompletedValue() < fenceValue) {
        m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent);
        DWORD result = WaitForSingleObjectEx(m_fenceEvent, 5000, FALSE);
        if (result == WAIT_TIMEOUT) {
            DebugLogA("DX12: WaitForGpu TIMEOUT (5s) — possible GPU hang");
            HRESULT reason = m_device->GetDeviceRemovedReason();
            if (reason != S_OK) {
                char buf[256];
                sprintf(buf, "DX12: Device removed reason: 0x%08X", (unsigned)reason);
                DebugLogA(buf);
            }
        }
    }

    // Advance the fence value for the current frame
    m_fenceValues[m_frameIndex]++;
}

void DXContext::MoveToNextFrame()
{
    // Signal the current fence value
    const UINT64 currentFenceValue = m_fenceValues[m_frameIndex];
    m_commandQueue->Signal(m_fence.Get(), currentFenceValue);

    // Advance frame index
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    // Wait if the next frame is still in use by the GPU (5s timeout to detect TDR)
    UINT64 waitTarget = m_fenceValues[m_frameIndex];
    if (m_fence->GetCompletedValue() < waitTarget) {
        m_fence->SetEventOnCompletion(waitTarget, m_fenceEvent);
        DWORD result = WaitForSingleObjectEx(m_fenceEvent, 5000, FALSE);
        if (result == WAIT_TIMEOUT) {
            HRESULT reason = m_device->GetDeviceRemovedReason();
            char buf[256];
            sprintf(buf, "DX12: MoveToNextFrame TIMEOUT (5s) target=%llu reason=0x%08X",
                    (unsigned long long)waitTarget, (unsigned)reason);
            DebugLogA(buf);
        }
    }

    // Set fence value for the upcoming frame
    m_fenceValues[m_frameIndex] = currentFenceValue + 1;
}

// ---------------------------------------------------------------------------
// Swap chain resize
// ---------------------------------------------------------------------------

bool DXContext::ResizeSwapChain(int newWidth, int newHeight)
{
    if (!m_ready) return false;

    // Compare against actual swap chain buffer dimensions, not tracked m_client_width/height.
    // OnUserResizeWindow(bSetBackBuffer=false) may have already updated m_client_width/height
    // before this call, which would cause a false match and skip the actual resize.
    DXGI_SWAP_CHAIN_DESC1 scDesc = {};
    if (m_swapChain) m_swapChain->GetDesc1(&scDesc);
    if ((int)scDesc.Width == newWidth && (int)scDesc.Height == newHeight) {
        m_client_width  = m_REAL_client_width  = newWidth;
        m_client_height = m_REAL_client_height = newHeight;
        return true;
    }

    {
        char buf[256];
        sprintf(buf, "DX12: ResizeSwapChain: swap=%ux%u -> %dx%d\n",
                scDesc.Width, scDesc.Height, newWidth, newHeight);
        OutputDebugStringA(buf);
    }

    // Flush GPU before resizing
    WaitForGpu();

    // Release references to back-buffer resources
    ReleaseSwapChainRtvs();
    m_commandList.Reset();
    for (UINT i = 0; i < DXC_FRAME_COUNT; i++) {
        m_commandAllocators[i].Reset();
        m_fenceValues[i] = m_fenceValues[m_frameIndex];
    }

    HRESULT hr = m_swapChain->ResizeBuffers(
        DXC_FRAME_COUNT,
        (UINT)newWidth,
        (UINT)newHeight,
        k_BackBufferFormat,
        0);
    if (FAILED(hr)) {
        m_lastErr = DXC_ERR_RESIZEFAILED;
        return false;
    }

    m_frameIndex    = m_swapChain->GetCurrentBackBufferIndex();
    m_client_width  = m_REAL_client_width  = newWidth;
    m_client_height = m_REAL_client_height = newHeight;
    m_window_width  = newWidth;
    m_window_height = newHeight;

    // Recreate RTVs, command allocators, and command list
    CreateRtvsForSwapChain();

    for (UINT i = 0; i < DXC_FRAME_COUNT; i++) {
        hr = m_device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(&m_commandAllocators[i]));
        if (FAILED(hr)) return false;
    }

    hr = m_device->CreateCommandList(
        0, D3D12_COMMAND_LIST_TYPE_DIRECT,
        m_commandAllocators[m_frameIndex].Get(),
        nullptr, IID_PPV_ARGS(&m_commandList));
    if (FAILED(hr)) return false;
    m_commandList->Close();

    SetViewport(newWidth, newHeight);
    UpdateMonitorWorkRect();
    return true;
}

// ---------------------------------------------------------------------------
// Miscellaneous
// ---------------------------------------------------------------------------

BOOL DXContext::StartOrRestartDevice(DXCONTEXT_PARAMS* pParams)
{
    memcpy(&m_current_mode, pParams, sizeof(DXCONTEXT_PARAMS));
    m_ready = TRUE;
    return TRUE;
}

void DXContext::SetViewport(int width, int height)
{
    // Viewport is set per-draw-call on the command list in DX12, not on the device.
    // This is a no-op here; callers set it on m_commandList directly.
    (void)width; (void)height;
}

HWND DXContext::GetHwnd()              { return m_hwnd; }
bool DXContext::TempIgnoreDestroyMessages() { return false; }
void DXContext::SaveWindow()           {}
void DXContext::UpdateMonitorWorkRect()
{
    if (!m_hwnd) return;

    // Current monitor geometry
    HMONITOR hMon = MonitorFromWindow(m_hwnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi = { sizeof(mi) };
    if (GetMonitorInfo(hMon, &mi)) {
        m_monitor_rect      = mi.rcMonitor;
        m_monitor_rect_orig = mi.rcMonitor;
        m_monitor_work_rect      = mi.rcWork;
        m_monitor_work_rect_orig = mi.rcWork;
    }

    // Virtual screen (all monitors combined)
    m_all_monitors_rect.left   = GetSystemMetrics(SM_XVIRTUALSCREEN);
    m_all_monitors_rect.top    = GetSystemMetrics(SM_YVIRTUALSCREEN);
    m_all_monitors_rect.right  = m_all_monitors_rect.left + GetSystemMetrics(SM_CXVIRTUALSCREEN);
    m_all_monitors_rect.bottom = m_all_monitors_rect.top  + GetSystemMetrics(SM_CYVIRTUALSCREEN);
}

// ---------------------------------------------------------------------------
// Descriptor heap allocators (simple bump allocators)
// ---------------------------------------------------------------------------

D3D12_CPU_DESCRIPTOR_HANDLE DXContext::AllocateRtv()
{
    assert(m_nextFreeRtvSlot < DXC_MAX_RTV);
    D3D12_CPU_DESCRIPTOR_HANDLE handle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += (SIZE_T)m_nextFreeRtvSlot * m_rtvDescriptorSize;
    m_nextFreeRtvSlot++;
    return handle;
}

D3D12_CPU_DESCRIPTOR_HANDLE DXContext::AllocateSrvCpu()
{
    assert(m_nextFreeSrvSlot < DXC_MAX_SRV);
    D3D12_CPU_DESCRIPTOR_HANDLE handle = m_srvHeap->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += (SIZE_T)m_nextFreeSrvSlot * m_srvDescriptorSize;
    return handle;  // does NOT bump — caller must call AllocateSrvGpu to bump
}

D3D12_GPU_DESCRIPTOR_HANDLE DXContext::AllocateSrvGpu()
{
    assert(m_nextFreeSrvSlot < DXC_MAX_SRV);
    D3D12_GPU_DESCRIPTOR_HANDLE handle = m_srvHeap->GetGPUDescriptorHandleForHeapStart();
    handle.ptr += (SIZE_T)m_nextFreeSrvSlot * m_srvDescriptorSize;
    m_nextFreeSrvSlot++;
    return handle;
}

void DXContext::ResetDynamicDescriptors()
{
    // Rewind bump allocators to the post-init baseline so that dynamic textures
    // (VS, blur, etc.) can be re-created without leaking descriptor heap slots.
    m_nextFreeRtvSlot = m_rtvSlotBaseline;
    m_nextFreeSrvSlot = m_srvSlotBaseline;
}

// ---------------------------------------------------------------------------
// Texture creation helpers
// ---------------------------------------------------------------------------

DX12Texture DXContext::CreateRenderTargetTexture(UINT width, UINT height, DXGI_FORMAT format)
{
    DX12Texture tex;
    if (!m_device || width == 0 || height == 0) return tex;

    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Width              = width;
    desc.Height             = height;
    desc.DepthOrArraySize   = 1;
    desc.MipLevels          = 1;
    desc.Format             = format;
    desc.SampleDesc.Count   = 1;
    desc.SampleDesc.Quality = 0;
    desc.Flags              = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format   = format;
    clearValue.Color[0] = 0.f;
    clearValue.Color[1] = 0.f;
    clearValue.Color[2] = 0.f;
    clearValue.Color[3] = 1.f;

    HRESULT hr = m_device->CreateCommittedResource(
        &heapProps, D3D12_HEAP_FLAG_NONE, &desc,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        &clearValue, IID_PPV_ARGS(&tex.resource));
    if (FAILED(hr)) return tex;

    tex.width  = width;
    tex.height = height;
    tex.format = format;

    // Allocate RTV descriptor
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = AllocateRtv();
    tex.rtvIndex = m_nextFreeRtvSlot - 1;
    m_device->CreateRenderTargetView(tex.resource.Get(), nullptr, rtvHandle);

    // Allocate SRV descriptor
    D3D12_CPU_DESCRIPTOR_HANDLE srvCpuHandle = AllocateSrvCpu();
    tex.srvIndex = m_nextFreeSrvSlot;

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format                  = format;
    srvDesc.ViewDimension           = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels     = 1;
    m_device->CreateShaderResourceView(tex.resource.Get(), &srvDesc, srvCpuHandle);

    AllocateSrvGpu();  // bump the slot counter

    return tex;
}

// ---------------------------------------------------------------------------
// Resource state transitions + descriptor handle helpers (Phase 4)
// ---------------------------------------------------------------------------

void DXContext::TransitionResource(DX12Texture& tex, D3D12_RESOURCE_STATES newState)
{
    if (!tex.IsValid() || tex.currentState == newState)
        return;
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource   = tex.resource.Get();
    barrier.Transition.StateBefore = tex.currentState;
    barrier.Transition.StateAfter  = newState;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    m_commandList->ResourceBarrier(1, &barrier);
    tex.currentState = newState;
}

D3D12_GPU_DESCRIPTOR_HANDLE DXContext::GetSrvGpuHandle(const DX12Texture& tex)
{
    D3D12_GPU_DESCRIPTOR_HANDLE handle = m_srvHeap->GetGPUDescriptorHandleForHeapStart();
    handle.ptr += (SIZE_T)tex.srvIndex * m_srvDescriptorSize;
    return handle;
}

D3D12_CPU_DESCRIPTOR_HANDLE DXContext::GetRtvCpuHandle(const DX12Texture& tex)
{
    D3D12_CPU_DESCRIPTOR_HANDLE handle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += (SIZE_T)tex.rtvIndex * m_rtvDescriptorSize;
    return handle;
}

// ---------------------------------------------------------------------------
// Root signature creation (Phase 3, updated Phase 4)
// ---------------------------------------------------------------------------

bool DXContext::CreateRootSignature()
{
    // Root parameter 0: CBV at b0 ($Globals constant buffer)
    D3D12_ROOT_PARAMETER rootParams[2] = {};

    rootParams[0].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParams[0].Descriptor.ShaderRegister = 0;
    rootParams[0].Descriptor.RegisterSpace  = 0;
    rootParams[0].ShaderVisibility          = D3D12_SHADER_VISIBILITY_ALL;

    // Root parameter 1: Descriptor table — 16 SRVs (t0-t15)
    // Preset shaders may reference multiple textures (main, blur1-3, noise, custom)
    D3D12_DESCRIPTOR_RANGE srvRange = {};
    srvRange.RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    srvRange.NumDescriptors                    = 16;
    srvRange.BaseShaderRegister                = 0;
    srvRange.RegisterSpace                     = 0;
    srvRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    rootParams[1].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParams[1].DescriptorTable.NumDescriptorRanges = 1;
    rootParams[1].DescriptorTable.pDescriptorRanges   = &srvRange;
    rootParams[1].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_PIXEL;

    // Static samplers — 16 slots (s0-s15) to cover all sampler registers preset shaders may use.
    // The backwards-compat compiler auto-assigns sampler registers based on declaration order
    // in include.fx. We use Wrap+Linear as the default mode for all slots since most MilkDrop
    // textures expect it, and the sampler mode prefixes (FW_, FC_, PW_, PC_) are handled
    // by binding the right texture to the right register, not by changing the sampler.
    D3D12_STATIC_SAMPLER_DESC staticSamplers[16] = {};
    for (int i = 0; i < 16; i++) {
        staticSamplers[i].Filter           = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        staticSamplers[i].AddressU         = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        staticSamplers[i].AddressV         = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        staticSamplers[i].AddressW         = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        staticSamplers[i].MaxLOD           = D3D12_FLOAT32_MAX;
        staticSamplers[i].ShaderRegister   = i;
        staticSamplers[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    }

    D3D12_ROOT_SIGNATURE_DESC rootSigDesc = {};
    rootSigDesc.NumParameters     = 2;
    rootSigDesc.pParameters       = rootParams;
    rootSigDesc.NumStaticSamplers = 16;
    rootSigDesc.pStaticSamplers   = staticSamplers;
    rootSigDesc.Flags             = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
                                              &signature, &error);
    if (FAILED(hr)) {
        if (error) OutputDebugStringA((const char*)error->GetBufferPointer());
        return false;
    }

    hr = m_device->CreateRootSignature(0, signature->GetBufferPointer(),
                                        signature->GetBufferSize(),
                                        IID_PPV_ARGS(&m_rootSignature));
    return SUCCEEDED(hr);
}

// ---------------------------------------------------------------------------
// Upload heap ring buffers (Phase 3)
// ---------------------------------------------------------------------------

bool DXContext::CreateUploadBuffers()
{
    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

    D3D12_RESOURCE_DESC bufDesc = {};
    bufDesc.Dimension        = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufDesc.Width            = UPLOAD_BUFFER_SIZE;
    bufDesc.Height           = 1;
    bufDesc.DepthOrArraySize = 1;
    bufDesc.MipLevels        = 1;
    bufDesc.SampleDesc.Count = 1;
    bufDesc.Layout           = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    for (UINT i = 0; i < DXC_FRAME_COUNT; i++) {
        HRESULT hr = m_device->CreateCommittedResource(
            &heapProps, D3D12_HEAP_FLAG_NONE, &bufDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            IID_PPV_ARGS(&m_uploadBuffer[i]));
        if (FAILED(hr)) return false;

        // Persistently map the upload buffer
        void* pData = nullptr;
        D3D12_RANGE readRange = { 0, 0 }; // CPU won't read from it
        hr = m_uploadBuffer[i]->Map(0, &readRange, &pData);
        if (FAILED(hr)) return false;
        m_uploadBufferPtr[i] = (BYTE*)pData;
        m_uploadBufferOffset[i] = 0;
    }
    return true;
}

void DXContext::ResetUploadBuffer()
{
    m_uploadBufferOffset[m_frameIndex] = 0;
}

void DXContext::DrawVertices(D3D12_PRIMITIVE_TOPOLOGY topology, const void* vertexData,
                              UINT vertexCount, UINT vertexStride)
{
    if (vertexCount == 0) return;

    UINT totalBytes = vertexCount * vertexStride;
    UINT fi = m_frameIndex;

    // Check if there's space in the upload buffer
    if (m_uploadBufferOffset[fi] + totalBytes > UPLOAD_BUFFER_SIZE) {
        char buf[128];
        sprintf_s(buf, "DX12: Upload buffer exhausted! offset=%u + needed=%u > %u",
                  m_uploadBufferOffset[fi], totalBytes, UPLOAD_BUFFER_SIZE);
        DebugLogA(buf);
        return;
    }

    // Copy vertex data to upload buffer
    BYTE* dest = m_uploadBufferPtr[fi] + m_uploadBufferOffset[fi];
    memcpy(dest, vertexData, totalBytes);

    // Set up vertex buffer view
    D3D12_VERTEX_BUFFER_VIEW vbv = {};
    vbv.BufferLocation = m_uploadBuffer[fi]->GetGPUVirtualAddress() + m_uploadBufferOffset[fi];
    vbv.SizeInBytes    = totalBytes;
    vbv.StrideInBytes  = vertexStride;

    m_uploadBufferOffset[fi] += totalBytes;

    // Issue draw
    m_commandList->IASetPrimitiveTopology(topology);
    m_commandList->IASetVertexBuffers(0, 1, &vbv);
    m_commandList->DrawInstanced(vertexCount, 1, 0, 0);
}

D3D12_GPU_VIRTUAL_ADDRESS DXContext::UploadConstantBuffer(const void* data, UINT sizeBytes) {
    UINT fi = m_frameIndex;

    // DX12 CBVs require 256-byte alignment
    UINT alignedOffset = (m_uploadBufferOffset[fi] + 255) & ~255;
    UINT alignedSize   = (sizeBytes + 255) & ~255;

    if (alignedOffset + alignedSize > UPLOAD_BUFFER_SIZE)
        return 0; // out of space

    BYTE* dest = m_uploadBufferPtr[fi] + alignedOffset;
    memcpy(dest, data, sizeBytes);
    if (alignedSize > sizeBytes)
        memset(dest + sizeBytes, 0, alignedSize - sizeBytes);

    D3D12_GPU_VIRTUAL_ADDRESS gpuAddr =
        m_uploadBuffer[fi]->GetGPUVirtualAddress() + alignedOffset;
    m_uploadBufferOffset[fi] = alignedOffset + alignedSize;

    return gpuAddr;
}

// ---------------------------------------------------------------------------
// Null texture + binding block (Phase 5)
// ---------------------------------------------------------------------------

bool DXContext::CreateNullTexture()
{
    // Create a 1x1 black RGBA texture for filling unused SRV slots
    D3D12_RESOURCE_DESC texDesc = {};
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDesc.Width = 1;
    texDesc.Height = 1;
    texDesc.DepthOrArraySize = 1;
    texDesc.MipLevels = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

    HRESULT hr = m_device->CreateCommittedResource(
        &heapProps, D3D12_HEAP_FLAG_NONE, &texDesc,
        D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
        IID_PPV_ARGS(&m_nullTexture.resource));
    if (FAILED(hr)) return false;

    // Upload 1 black pixel (0,0,0,255)
    UINT8 blackPixel[4] = { 0, 0, 0, 255 };

    D3D12_RESOURCE_DESC uploadDesc = {};
    uploadDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    uploadDesc.Width = 256; // minimum upload alignment
    uploadDesc.Height = 1;
    uploadDesc.DepthOrArraySize = 1;
    uploadDesc.MipLevels = 1;
    uploadDesc.Format = DXGI_FORMAT_UNKNOWN;
    uploadDesc.SampleDesc.Count = 1;
    uploadDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    D3D12_HEAP_PROPERTIES uploadHeapProps = {};
    uploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

    ComPtr<ID3D12Resource> uploadBuf;
    hr = m_device->CreateCommittedResource(
        &uploadHeapProps, D3D12_HEAP_FLAG_NONE, &uploadDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(&uploadBuf));
    if (FAILED(hr)) return false;

    UINT8* mapped = nullptr;
    uploadBuf->Map(0, nullptr, (void**)&mapped);
    memcpy(mapped, blackPixel, 4);
    uploadBuf->Unmap(0, nullptr);

    // Copy to the texture via a temporary command list
    m_commandAllocators[0]->Reset();
    m_commandList->Reset(m_commandAllocators[0].Get(), nullptr);

    D3D12_TEXTURE_COPY_LOCATION src = {};
    src.pResource = uploadBuf.Get();
    src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    src.PlacedFootprint.Footprint.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    src.PlacedFootprint.Footprint.Width = 1;
    src.PlacedFootprint.Footprint.Height = 1;
    src.PlacedFootprint.Footprint.Depth = 1;
    src.PlacedFootprint.Footprint.RowPitch = 256; // minimum row pitch

    D3D12_TEXTURE_COPY_LOCATION dst = {};
    dst.pResource = m_nullTexture.resource.Get();
    dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dst.SubresourceIndex = 0;

    m_commandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

    // Transition to PIXEL_SHADER_RESOURCE
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = m_nullTexture.resource.Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    m_commandList->ResourceBarrier(1, &barrier);

    m_commandList->Close();
    ID3D12CommandList* lists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(1, lists);

    // Wait for copy to complete
    m_fenceValues[0]++;
    m_commandQueue->Signal(m_fence.Get(), m_fenceValues[0]);
    m_fence->SetEventOnCompletion(m_fenceValues[0], m_fenceEvent);
    WaitForSingleObject(m_fenceEvent, INFINITE);

    // Create SRV for the null texture
    D3D12_CPU_DESCRIPTOR_HANDLE srvCpuHandle = AllocateSrvCpu();
    m_nullTexture.srvIndex = m_nextFreeSrvSlot;

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = 1;
    m_device->CreateShaderResourceView(m_nullTexture.resource.Get(), &srvDesc, srvCpuHandle);

    AllocateSrvGpu(); // bump counter

    m_nullTexture.currentState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    m_nullTexture.width = 1;
    m_nullTexture.height = 1;
    m_nullTexture.format = DXGI_FORMAT_R8G8B8A8_UNORM;

    return true;
}

void DXContext::CreateBindingBlockForTexture(DX12Texture& tex)
{
    if (m_nullTexture.srvIndex == UINT_MAX || tex.srvIndex == UINT_MAX) return;

    // Reserve 16 contiguous SRV slots for this texture's binding block
    tex.bindingBlockStart = m_nextFreeSrvSlot;

    D3D12_CPU_DESCRIPTOR_HANDLE nullSrvCpu;
    nullSrvCpu.ptr = m_srvHeap->GetCPUDescriptorHandleForHeapStart().ptr +
                     (SIZE_T)m_nullTexture.srvIndex * m_srvDescriptorSize;

    D3D12_CPU_DESCRIPTOR_HANDLE texSrvCpu;
    texSrvCpu.ptr = m_srvHeap->GetCPUDescriptorHandleForHeapStart().ptr +
                    (SIZE_T)tex.srvIndex * m_srvDescriptorSize;

    for (UINT i = 0; i < BINDING_BLOCK_SIZE; i++) {
        D3D12_CPU_DESCRIPTOR_HANDLE dst;
        dst.ptr = m_srvHeap->GetCPUDescriptorHandleForHeapStart().ptr +
                  (SIZE_T)m_nextFreeSrvSlot * m_srvDescriptorSize;
        // Slot 0 = the actual texture, slots 1-15 = null (safe placeholder)
        m_device->CopyDescriptorsSimple(1, dst, (i == 0) ? texSrvCpu : nullSrvCpu,
                                         D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        m_nextFreeSrvSlot++;
    }
}

D3D12_GPU_DESCRIPTOR_HANDLE DXContext::GetBindingBlockGpuHandle(const DX12Texture& tex)
{
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
    gpuHandle.ptr = m_srvHeap->GetGPUDescriptorHandleForHeapStart().ptr +
                    (SIZE_T)tex.bindingBlockStart * m_srvDescriptorSize;
    return gpuHandle;
}

UINT DXContext::CreateBindingBlockAtSlot(const DX12Texture& tex, UINT mainSlot)
{
    if (m_nullTexture.srvIndex == UINT_MAX || tex.srvIndex == UINT_MAX)
        return UINT_MAX;
    if (mainSlot >= BINDING_BLOCK_SIZE) mainSlot = 0;

    UINT blockStart = m_nextFreeSrvSlot;

    D3D12_CPU_DESCRIPTOR_HANDLE nullSrvCpu;
    nullSrvCpu.ptr = m_srvHeap->GetCPUDescriptorHandleForHeapStart().ptr +
                     (SIZE_T)m_nullTexture.srvIndex * m_srvDescriptorSize;

    D3D12_CPU_DESCRIPTOR_HANDLE texSrvCpu;
    texSrvCpu.ptr = m_srvHeap->GetCPUDescriptorHandleForHeapStart().ptr +
                    (SIZE_T)tex.srvIndex * m_srvDescriptorSize;

    for (UINT i = 0; i < BINDING_BLOCK_SIZE; i++) {
        D3D12_CPU_DESCRIPTOR_HANDLE dst;
        dst.ptr = m_srvHeap->GetCPUDescriptorHandleForHeapStart().ptr +
                  (SIZE_T)m_nextFreeSrvSlot * m_srvDescriptorSize;
        m_device->CopyDescriptorsSimple(1, dst, (i == mainSlot) ? texSrvCpu : nullSrvCpu,
                                         D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        m_nextFreeSrvSlot++;
    }

    char buf[128];
    sprintf(buf, "DX12: CreateBindingBlockAtSlot: tex srv=%u at slot %u, blockStart=%u",
            tex.srvIndex, mainSlot, blockStart);
    DebugLogA(buf);
    return blockStart;
}

D3D12_GPU_DESCRIPTOR_HANDLE DXContext::GetBindingBlockGpuHandleByIndex(UINT blockStart)
{
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
    gpuHandle.ptr = m_srvHeap->GetGPUDescriptorHandleForHeapStart().ptr +
                    (SIZE_T)blockStart * m_srvDescriptorSize;
    return gpuHandle;
}

bool DXContext::AllocatePerFrameBindings()
{
    // Reserve 64 contiguous SRV slots: 2 frames × 2 passes × 16 descriptors
    m_perFrameBindingBase = m_nextFreeSrvSlot;
    m_nextFreeSrvSlot += DXC_FRAME_COUNT * 2 * BINDING_BLOCK_SIZE;
    char buf[128];
    sprintf(buf, "DX12: AllocatePerFrameBindings: base=%u (reserved %u slots)",
            m_perFrameBindingBase, DXC_FRAME_COUNT * 2 * BINDING_BLOCK_SIZE);
    DebugLogA(buf);
    return true;
}

void DXContext::UpdatePerFrameBindings(const DX12Texture& warpSrc, UINT warpMainSlot,
                                       const DX12Texture& compSrc, UINT compMainSlot)
{
    if (m_perFrameBindingBase == UINT_MAX) return;
    if (warpMainSlot >= BINDING_BLOCK_SIZE) warpMainSlot = 0;
    if (compMainSlot >= BINDING_BLOCK_SIZE) compMainSlot = 0;

    // Each frame gets 32 slots: [0..15] = warp, [16..31] = comp
    UINT frameBase = m_perFrameBindingBase + m_frameIndex * 2 * BINDING_BLOCK_SIZE;

    D3D12_CPU_DESCRIPTOR_HANDLE nullSrc;
    nullSrc.ptr = m_srvHeap->GetCPUDescriptorHandleForHeapStart().ptr +
                  (SIZE_T)m_nullTexture.srvIndex * m_srvDescriptorSize;

    D3D12_CPU_DESCRIPTOR_HANDLE warpTexSrc;
    warpTexSrc.ptr = m_srvHeap->GetCPUDescriptorHandleForHeapStart().ptr +
                     (SIZE_T)warpSrc.srvIndex * m_srvDescriptorSize;

    D3D12_CPU_DESCRIPTOR_HANDLE compTexSrc;
    compTexSrc.ptr = m_srvHeap->GetCPUDescriptorHandleForHeapStart().ptr +
                     (SIZE_T)compSrc.srvIndex * m_srvDescriptorSize;

    // Warp binding block: warpSrc at warpMainSlot, null elsewhere
    for (UINT i = 0; i < BINDING_BLOCK_SIZE; i++) {
        D3D12_CPU_DESCRIPTOR_HANDLE dst;
        dst.ptr = m_srvHeap->GetCPUDescriptorHandleForHeapStart().ptr +
                  (SIZE_T)(frameBase + i) * m_srvDescriptorSize;
        m_device->CopyDescriptorsSimple(1, dst, (i == warpMainSlot) ? warpTexSrc : nullSrc,
                                         D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }

    // Comp binding block: compSrc at compMainSlot, null elsewhere
    UINT compBase = frameBase + BINDING_BLOCK_SIZE;
    for (UINT i = 0; i < BINDING_BLOCK_SIZE; i++) {
        D3D12_CPU_DESCRIPTOR_HANDLE dst;
        dst.ptr = m_srvHeap->GetCPUDescriptorHandleForHeapStart().ptr +
                  (SIZE_T)(compBase + i) * m_srvDescriptorSize;
        m_device->CopyDescriptorsSimple(1, dst, (i == compMainSlot) ? compTexSrc : nullSrc,
                                         D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }
}

D3D12_GPU_DESCRIPTOR_HANDLE DXContext::GetWarpBindingGpuHandle()
{
    UINT slot = m_perFrameBindingBase + m_frameIndex * 2 * BINDING_BLOCK_SIZE;
    D3D12_GPU_DESCRIPTOR_HANDLE h;
    h.ptr = m_srvHeap->GetGPUDescriptorHandleForHeapStart().ptr +
            (SIZE_T)slot * m_srvDescriptorSize;
    return h;
}

D3D12_GPU_DESCRIPTOR_HANDLE DXContext::GetCompBindingGpuHandle()
{
    UINT slot = m_perFrameBindingBase + m_frameIndex * 2 * BINDING_BLOCK_SIZE + BINDING_BLOCK_SIZE;
    D3D12_GPU_DESCRIPTOR_HANDLE h;
    h.ptr = m_srvHeap->GetGPUDescriptorHandleForHeapStart().ptr +
            (SIZE_T)slot * m_srvDescriptorSize;
    return h;
}

// ---------------------------------------------------------------------------
// Pipeline state objects (Phase 3)
// ---------------------------------------------------------------------------

bool DXContext::CreatePipelines()
{
    return DX12CreatePipelines(m_device.Get(), m_rootSignature.Get(),
                                k_BackBufferFormat, m_PSOs);
}

// ---------------------------------------------------------------------------

void DXContext::WriteSafeWindowPos()
{
    WritePrivateProfileIntW(64, L"nMainWndTop",    m_szIniFile, L"Settings");
    WritePrivateProfileIntW(64, L"nMainWndLeft",   m_szIniFile, L"Settings");
    WritePrivateProfileIntW(64 + 256, L"nMainWndRight",  m_szIniFile, L"Settings");
    WritePrivateProfileIntW(64 + 256, L"nMainWndBottom", m_szIniFile, L"Settings");
}

bool DXContext::OnUserResizeWindow(RECT* new_window_rect, RECT* new_client_rect, bool bSetBackBuffer)
{
    if (!m_ready) return false;

    int newW = new_client_rect->right  - new_client_rect->left;
    int newH = new_client_rect->bottom - new_client_rect->top;

    if (newW == m_client_width && newH == m_client_height &&
        (new_window_rect->right - new_window_rect->left) == m_window_width &&
        (new_window_rect->bottom - new_window_rect->top) == m_window_height)
    {
        return true;
    }

    m_window_width  = new_window_rect->right  - new_window_rect->left;
    m_window_height = new_window_rect->bottom - new_window_rect->top;

    if (bSetBackBuffer) {
        return ResizeSwapChain(newW, newH);
    }

    m_client_width  = m_REAL_client_width  = newW;
    m_client_height = m_REAL_client_height = newH;
    return true;
}
