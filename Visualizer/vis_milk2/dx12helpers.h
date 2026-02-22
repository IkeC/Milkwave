/*
  dx12helpers.h
  DX12 resource types and helper functions for the Milkwave DX9->DX12 migration.
  Phase 2: provides DX12Texture struct and texture creation utilities.
*/

#ifndef DX12HELPERS_H
#define DX12HELPERS_H

#include <d3d12.h>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

// ---------------------------------------------------------------------------
// DX12Texture — wraps an ID3D12Resource with its descriptor heap indices.
// Used as a drop-in companion to the old LPDIRECT3DTEXTURE9 members.
// ---------------------------------------------------------------------------
struct DX12Texture {
    ComPtr<ID3D12Resource> resource;
    UINT srvIndex  = UINT_MAX;   // index in DXContext::m_srvHeap (UINT_MAX = none)
    UINT rtvIndex  = UINT_MAX;   // index in DXContext::m_rtvHeap (UINT_MAX = none)
    UINT bindingBlockStart = UINT_MAX;  // 16 contiguous SRV slots: [0]=this tex, [1-15]=null
    UINT width     = 0;
    UINT height    = 0;
    DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
    D3D12_RESOURCE_STATES currentState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

    bool IsValid() const { return resource != nullptr; }
    void Reset() { resource.Reset(); srvIndex = rtvIndex = bindingBlockStart = UINT_MAX; width = height = 0; format = DXGI_FORMAT_UNKNOWN; currentState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE; }
};

#endif // DX12HELPERS_H
