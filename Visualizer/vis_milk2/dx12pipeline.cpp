#include "dx12pipeline.h"
#include <d3dcompiler.h>
#include <cstdio>

// Global VS bytecodes for preset PSO creation
ID3DBlob* g_pWarpVSBlob = nullptr;
ID3DBlob* g_pCompVSBlob = nullptr;

static bool CompileShaderFromString(const char* src, const char* entryPoint, const char* target,
                                     ID3DBlob** ppBlob, UINT flags = 0) {
    ID3DBlob* pErrors = nullptr;
    HRESULT hr = D3DCompile(src, strlen(src), nullptr, nullptr, nullptr,
                             entryPoint, target, flags, 0, ppBlob, &pErrors);
    if (FAILED(hr)) {
        if (pErrors) {
            OutputDebugStringA((const char*)pErrors->GetBufferPointer());
            pErrors->Release();
        }
        return false;
    }
    if (pErrors) pErrors->Release();
    return true;
}

bool DX12CreatePipelines(ID3D12Device* device, ID3D12RootSignature* rootSig,
                          DXGI_FORMAT rtvFormat,
                          ComPtr<ID3D12PipelineState> psoArray[PSO_COUNT]) {
    if (!device || !rootSig) return false;

    // Compile passthrough shaders
    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* psBlob = nullptr;
    if (!CompileShaderFromString(g_szPassthroughVS, "main", "vs_5_0", &vsBlob)) return false;
    if (!CompileShaderFromString(g_szPassthroughPS, "main", "ps_5_0", &psBlob)) {
        vsBlob->Release();
        return false;
    }

    // Common PSO descriptor
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = rootSig;
    psoDesc.VS = { vsBlob->GetBufferPointer(), vsBlob->GetBufferSize() };
    psoDesc.PS = { psBlob->GetBufferPointer(), psBlob->GetBufferSize() };

    // Input layout: WFVERTEX
    psoDesc.InputLayout = { g_WfVertexLayout, _countof(g_WfVertexLayout) };

    // Rasterizer: solid fill, no culling
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    psoDesc.RasterizerState.DepthClipEnable = TRUE;

    // No depth/stencil
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;

    // Sample desc
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.SampleDesc.Count = 1;

    // Render target
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = rtvFormat;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    // PSO 0: Solid, no blending
    psoDesc.BlendState.RenderTarget[0].BlendEnable = FALSE;
    psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    HRESULT hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&psoArray[PSO_SOLID_WFVERTEX]));
    if (FAILED(hr)) {
        vsBlob->Release(); psBlob->Release();
        return false;
    }

    // PSO 1: Alpha blend (SrcAlpha / InvSrcAlpha)
    psoDesc.BlendState.RenderTarget[0].BlendEnable           = TRUE;
    psoDesc.BlendState.RenderTarget[0].SrcBlend              = D3D12_BLEND_SRC_ALPHA;
    psoDesc.BlendState.RenderTarget[0].DestBlend             = D3D12_BLEND_INV_SRC_ALPHA;
    psoDesc.BlendState.RenderTarget[0].BlendOp               = D3D12_BLEND_OP_ADD;
    psoDesc.BlendState.RenderTarget[0].SrcBlendAlpha         = D3D12_BLEND_ONE;
    psoDesc.BlendState.RenderTarget[0].DestBlendAlpha        = D3D12_BLEND_INV_SRC_ALPHA;
    psoDesc.BlendState.RenderTarget[0].BlendOpAlpha          = D3D12_BLEND_OP_ADD;
    psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&psoArray[PSO_ALPHABLEND_WFVERTEX]));
    if (FAILED(hr)) {
        vsBlob->Release(); psBlob->Release();
        return false;
    }

    // ── Line topology PSOs (waveforms) ──
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;

    // PSO: Line + alpha blend (WFVERTEX)
    // (blend state already set to SrcAlpha/InvSrcAlpha from PSO_ALPHABLEND_WFVERTEX above)
    hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&psoArray[PSO_LINE_ALPHABLEND_WFVERTEX]));
    if (FAILED(hr)) { vsBlob->Release(); psBlob->Release(); return false; }

    // PSO: Line + additive blend (WFVERTEX)
    psoDesc.BlendState.RenderTarget[0].DestBlend      = D3D12_BLEND_ONE;
    psoDesc.BlendState.RenderTarget[0].DestBlendAlpha  = D3D12_BLEND_ONE;
    hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&psoArray[PSO_LINE_ADDITIVE_WFVERTEX]));
    if (FAILED(hr)) { vsBlob->Release(); psBlob->Release(); return false; }

    // ── Point topology PSOs (dot waveforms) ──
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;

    // PSO: Point + alpha blend (WFVERTEX) — reset dest blend back to InvSrcAlpha
    psoDesc.BlendState.RenderTarget[0].DestBlend      = D3D12_BLEND_INV_SRC_ALPHA;
    psoDesc.BlendState.RenderTarget[0].DestBlendAlpha  = D3D12_BLEND_INV_SRC_ALPHA;
    hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&psoArray[PSO_POINT_ALPHABLEND_WFVERTEX]));
    if (FAILED(hr)) { vsBlob->Release(); psBlob->Release(); return false; }

    // PSO: Point + additive blend (WFVERTEX)
    psoDesc.BlendState.RenderTarget[0].DestBlend      = D3D12_BLEND_ONE;
    psoDesc.BlendState.RenderTarget[0].DestBlendAlpha  = D3D12_BLEND_ONE;
    hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&psoArray[PSO_POINT_ADDITIVE_WFVERTEX]));
    if (FAILED(hr)) { vsBlob->Release(); psBlob->Release(); return false; }

    // ── Triangle + additive blend (untextured shapes) ──
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    // DestBlend is already ONE from above — keep it for additive
    hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&psoArray[PSO_ADDITIVE_WFVERTEX]));
    if (FAILED(hr)) { vsBlob->Release(); psBlob->Release(); return false; }

    vsBlob->Release();
    psBlob->Release();

    // ── Textured PSOs (for warp + composite passes) ──
    ID3DBlob* texVsBlob = nullptr;
    ID3DBlob* texPsBlob = nullptr;
    if (!CompileShaderFromString(g_szTexturedVS, "main", "vs_5_0", &texVsBlob)) return false;
    if (!CompileShaderFromString(g_szTexturedPS, "main", "ps_5_0", &texPsBlob)) {
        texVsBlob->Release();
        return false;
    }

    // Reset common state for textured PSOs
    psoDesc.VS = { texVsBlob->GetBufferPointer(), texVsBlob->GetBufferSize() };
    psoDesc.PS = { texPsBlob->GetBufferPointer(), texPsBlob->GetBufferSize() };
    psoDesc.BlendState.RenderTarget[0].BlendEnable = FALSE;
    psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    // PSO: Textured MYVERTEX (warp mesh)
    psoDesc.InputLayout = { g_MyVertexLayout, _countof(g_MyVertexLayout) };
    hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&psoArray[PSO_TEXTURED_MYVERTEX]));
    if (FAILED(hr)) {
        texVsBlob->Release(); texPsBlob->Release();
        return false;
    }

    // PSO: Textured SPRITEVERTEX (composite quad)
    psoDesc.InputLayout = { g_SpriteVertexLayout, _countof(g_SpriteVertexLayout) };
    hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&psoArray[PSO_TEXTURED_SPRITEVERTEX]));
    if (FAILED(hr)) { texVsBlob->Release(); texPsBlob->Release(); return false; }

    // PSO: Textured SPRITEVERTEX + additive blend (textured custom shapes)
    psoDesc.BlendState.RenderTarget[0].BlendEnable           = TRUE;
    psoDesc.BlendState.RenderTarget[0].SrcBlend              = D3D12_BLEND_SRC_ALPHA;
    psoDesc.BlendState.RenderTarget[0].DestBlend             = D3D12_BLEND_ONE;
    psoDesc.BlendState.RenderTarget[0].BlendOp               = D3D12_BLEND_OP_ADD;
    psoDesc.BlendState.RenderTarget[0].SrcBlendAlpha         = D3D12_BLEND_ONE;
    psoDesc.BlendState.RenderTarget[0].DestBlendAlpha        = D3D12_BLEND_ONE;
    psoDesc.BlendState.RenderTarget[0].BlendOpAlpha          = D3D12_BLEND_OP_ADD;
    psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&psoArray[PSO_ADDITIVE_SPRITEVERTEX]));
    if (FAILED(hr)) { texVsBlob->Release(); texPsBlob->Release(); return false; }

    // PSO: Textured SPRITEVERTEX + premultiplied alpha blend (GDI text overlay)
    psoDesc.BlendState.RenderTarget[0].SrcBlend      = D3D12_BLEND_ONE;
    psoDesc.BlendState.RenderTarget[0].DestBlend      = D3D12_BLEND_INV_SRC_ALPHA;
    psoDesc.BlendState.RenderTarget[0].SrcBlendAlpha  = D3D12_BLEND_ONE;
    psoDesc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
    hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&psoArray[PSO_PREMULALPHA_SPRITEVERTEX]));

    texVsBlob->Release();
    texPsBlob->Release();

    // Compile warp + comp vertex shaders for preset PSO creation
    // Must use ENABLE_BACKWARDS_COMPATIBILITY to match the PS compiled by D3DXCompileShader shim,
    // otherwise VS/PS get different hardware register assignments for the same semantics.
    if (!CompileShaderFromString(g_szWarpVS, "main", "vs_5_0", &g_pWarpVSBlob, D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY)) {
        OutputDebugStringA("DX12: Failed to compile warp VS\n");
        return false;
    }
    if (!CompileShaderFromString(g_szCompVS, "main", "vs_5_0", &g_pCompVSBlob, D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY)) {
        OutputDebugStringA("DX12: Failed to compile comp VS\n");
        return false;
    }

    return SUCCEEDED(hr);
}

ComPtr<ID3D12PipelineState> DX12CreatePresetPSO(
    ID3D12Device* device, ID3D12RootSignature* rootSig,
    DXGI_FORMAT rtvFormat,
    ID3DBlob* vsBlob,
    const void* psBytecode, UINT psSize,
    const D3D12_INPUT_ELEMENT_DESC* layout, UINT layoutCount,
    bool alphaBlend,
    UINT* pMainTexSlotOut)
{
    if (pMainTexSlotOut) *pMainTexSlotOut = UINT_MAX;
    if (!device || !rootSig || !vsBlob || !psBytecode || psSize == 0)
        return nullptr;

    // Reflect PS to find sampler_main texture slot
    if (pMainTexSlotOut) {
        ID3D12ShaderReflection* psRefl = nullptr;
        if (SUCCEEDED(D3DReflect(psBytecode, psSize, IID_PPV_ARGS(&psRefl)))) {
            D3D12_SHADER_DESC psd;
            psRefl->GetDesc(&psd);
            for (UINT i = 0; i < psd.BoundResources; i++) {
                D3D12_SHADER_INPUT_BIND_DESC rbd;
                psRefl->GetResourceBindingDesc(i, &rbd);
                if (*pMainTexSlotOut == UINT_MAX && rbd.Type == D3D_SIT_TEXTURE) {
                    const char* n = rbd.Name;
                    if (!strcmp(n, "sampler_main") || !strcmp(n, "sampler_fc_main") ||
                        !strcmp(n, "sampler_pc_main") || !strcmp(n, "sampler_fw_main") ||
                        !strcmp(n, "sampler_pw_main")) {
                        *pMainTexSlotOut = rbd.BindPoint;
                    }
                }
            }
            psRefl->Release();
        }
    }

    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
    desc.pRootSignature = rootSig;
    desc.VS = { vsBlob->GetBufferPointer(), vsBlob->GetBufferSize() };
    desc.PS = { psBytecode, psSize };
    desc.InputLayout = { layout, layoutCount };

    desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    desc.RasterizerState.DepthClipEnable = TRUE;

    desc.DepthStencilState.DepthEnable = FALSE;
    desc.DepthStencilState.StencilEnable = FALSE;

    desc.SampleMask = UINT_MAX;
    desc.SampleDesc.Count = 1;
    desc.NumRenderTargets = 1;
    desc.RTVFormats[0] = rtvFormat;
    desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    if (alphaBlend) {
        desc.BlendState.RenderTarget[0].BlendEnable           = TRUE;
        desc.BlendState.RenderTarget[0].SrcBlend              = D3D12_BLEND_SRC_ALPHA;
        desc.BlendState.RenderTarget[0].DestBlend             = D3D12_BLEND_INV_SRC_ALPHA;
        desc.BlendState.RenderTarget[0].BlendOp               = D3D12_BLEND_OP_ADD;
        desc.BlendState.RenderTarget[0].SrcBlendAlpha         = D3D12_BLEND_ONE;
        desc.BlendState.RenderTarget[0].DestBlendAlpha        = D3D12_BLEND_INV_SRC_ALPHA;
        desc.BlendState.RenderTarget[0].BlendOpAlpha          = D3D12_BLEND_OP_ADD;
    } else {
        desc.BlendState.RenderTarget[0].BlendEnable = FALSE;
    }
    desc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    ComPtr<ID3D12PipelineState> pso;
    HRESULT hr = device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pso));
    if (FAILED(hr)) {
        return nullptr;
    }
    return pso;
}
