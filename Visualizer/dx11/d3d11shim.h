s/*
 * d3d11shim.h - Direct3D 9 to Direct3D 11 conversion functions header.
 *
 * Copyright (c) 2021-2024 Jimmy Cassis
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include <memory>
#include <string>
#include <d3d11_1.h>
#include <DirectXMath.h>
#include <CommonStates.h>
#include "constanttable.h"

#define MAX_NUM_SHADERS (4)
#define MAX_VERTICES_COUNT (3072U)
#define MAX_INDICES_COUNT (MAX_VERTICES_COUNT * 3)
#define D3D_PRIMITIVE_TOPOLOGY_TRIANGLEFAN D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP + 1 // D3DPT_TRIANGLEFAN (0x6): Triangle fans are not supported in Direct3D 10 or later.

class D3D11Shim
{
  public:
    D3D11Shim(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    ~D3D11Shim();

    void Initialize();

    bool CreateTexture(unsigned int uWidth, unsigned int uHeight, unsigned int mipLevels, UINT bindFlags, DXGI_FORMAT format, ID3D11Texture2D** ppTexture, unsigned int miscFlags = 0, D3D11_USAGE usage = D3D11_USAGE_DEFAULT);
    bool CreateVolumeTexture(unsigned int uWidth, unsigned int uHeight, unsigned int uDepth, unsigned int mipLevels, UINT bindFlags, DXGI_FORMAT format, ID3D11Texture3D** ppTexture, unsigned int miscFlags = 0, D3D11_USAGE usage = D3D11_USAGE_DEFAULT);

    void DrawPrimitive(unsigned int primType, unsigned int iPrimCount, const void* pVData, unsigned int vertexStride);
    void DrawIndexedPrimitive(unsigned int primType, unsigned int uStartIndex, unsigned int iNumVertices, unsigned int iPrimCount, const void* pIData, const void* pVData, unsigned int vertexStride);

    void GetRenderTarget(ID3D11Texture2D** ppTexture);
    void GetDepthView(ID3D11DepthStencilView** ppView);
    void GetViewport(D3D11_VIEWPORT* vp);
    void SetBlendState(bool bEnable, D3D11_BLEND srcBlend = D3D11_BLEND_ONE, D3D11_BLEND destBlend = D3D11_BLEND_ZERO);
    void SetDepth(bool bEnabled);
    void SetRasterizerState(D3D11_CULL_MODE cullMode, D3D11_FILL_MODE fillMode);
    void SetRenderTarget(ID3D11Texture2D* pTexture, ID3D11DepthStencilView** ppView = nullptr);
    void SetSamplerState(UINT uSlot, D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressMode);
    void SetShader(unsigned int iIndex);
    void SetTexture(unsigned int iSlot, ID3D11Resource* pResource);
    void SetTransform(unsigned int transType, DirectX::XMMATRIX* pMatrix);
    void SetVertexColor(bool bUseColor);

    D3D_FEATURE_LEVEL GetFeatureLevel() { return m_pDevice->GetFeatureLevel(); }

    HRESULT CreateVertexShader(const void* pByteCode, SIZE_T codeLength, ID3D11VertexShader** ppShader, CConstantTable* p_constant_table);
    HRESULT CreatePixelShader(const void* pByteCode, SIZE_T codeLength, ID3D11PixelShader** ppShader, CConstantTable* p_constant_table);
    void SetVertexShader(ID3D11VertexShader* pVShader, CConstantTable* pTable);
    void SetPixelShader(ID3D11PixelShader* pPShader, CConstantTable* pTable);
    void ClearRenderTarget(ID3D11Texture2D* pRTTexture, const float color[4]);

    void CopyResource(ID3D11Resource* pDstResource, ID3D11Resource* pSrcResource);
    bool LockRect(ID3D11Resource* pResource, UINT uSubRes, D3D11_MAP mapType, D3D11_MAPPED_SUBRESOURCE* res);
    void UnlockRect(ID3D11Resource* pResource, UINT uSubRes);

    HRESULT CreateTextureFromFile(LPCWSTR szFileName, ID3D11Resource** texture);
    HRESULT CreateTextureFromMemory(const uint8_t* data, size_t dataSize, ID3D11Resource** texture, UINT type = 0);

    UINT GetMaxPrimitiveCount() { return MAX_VERTICES_COUNT; };

  private:
    struct cbTransforms
    {
        DirectX::XMFLOAT4X4 world;
        DirectX::XMFLOAT4X4 view;
        DirectX::XMFLOAT4X4 proj;
    };

    int NumVertsFromType(unsigned int primType, int iPrimCount);
    void UpdateVBuffer(unsigned int iNumVerts, const void* pVData, unsigned int vertexStride);
    void UpdateIBuffer(unsigned int iNumIndices, const void* pIData);

    cbTransforms m_transforms;
    ID3D11Device* m_pDevice;
    ID3D11DeviceContext* m_pContext;
    ID3D11DeviceContext* m_pImmContext;
    ID3D11VertexShader* m_pVShader;
    ID3D11PixelShader* m_pPShader[MAX_NUM_SHADERS];
    ID3D11InputLayout* m_pInputLayout;
    ID3D11Buffer* m_pVBuffer;
    ID3D11Buffer* m_pIBuffer;
    ID3D11Buffer* m_pIFanBuffer;
    ID3D11Buffer* m_pCBuffer;
    bool m_bCBufferIsDirty;
    unsigned int m_uCurrShader;
    ID3D11BlendState* m_pState;

    std::unique_ptr<DirectX::CommonStates> m_states;
};