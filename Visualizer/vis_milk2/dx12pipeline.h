#ifndef DX12PIPELINE_H
#define DX12PIPELINE_H

#include <d3d12.h>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

// PSO identifiers
enum DX12PsoId {
    PSO_SOLID_WFVERTEX = 0,     // Solid untextured (WFVERTEX, tri-strip, no blend)
    PSO_ALPHABLEND_WFVERTEX,    // Alpha blend (WFVERTEX, tri-strip, src/invsrc alpha)
    PSO_TEXTURED_MYVERTEX,      // Textured solid (MYVERTEX layout, for warp pass)
    PSO_TEXTURED_SPRITEVERTEX,  // Textured solid (SPRITEVERTEX layout, for comp pass)
    PSO_LINE_ALPHABLEND_WFVERTEX,   // Line topology + SrcAlpha/InvSrcAlpha (waveforms)
    PSO_LINE_ADDITIVE_WFVERTEX,     // Line topology + SrcAlpha/One (additive waveforms)
    PSO_POINT_ALPHABLEND_WFVERTEX,  // Point topology + SrcAlpha/InvSrcAlpha (dot waveforms)
    PSO_POINT_ADDITIVE_WFVERTEX,    // Point topology + SrcAlpha/One (additive dot waveforms)
    PSO_ADDITIVE_WFVERTEX,          // Triangle + SrcAlpha/One (additive shapes, untextured)
    PSO_ADDITIVE_SPRITEVERTEX,      // Triangle + SrcAlpha/One (additive shapes, textured)
    PSO_COUNT
};

// Input layout descriptions for each vertex format
// WFVERTEX: float3 POSITION + DWORD COLOR (16 bytes)
static const D3D12_INPUT_ELEMENT_DESC g_WfVertexLayout[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM,     0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
};

// SPRITEVERTEX: float3 POSITION + DWORD COLOR + float2 TEXCOORD0 (24 bytes)
static const D3D12_INPUT_ELEMENT_DESC g_SpriteVertexLayout[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM,     0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
};

// MYVERTEX: float3 POSITION + DWORD COLOR + float2 TEX0 + float2 TEX1 + float2 TEX2 (40 bytes)
static const D3D12_INPUT_ELEMENT_DESC g_MyVertexLayout[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM,     0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT,       0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 2, DXGI_FORMAT_R32G32_FLOAT,       0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
};

// Passthrough shaders (embedded HLSL compiled at init time)
// VS: position passthrough + .bgra color swizzle (DX9 D3DCOLOR is BGRA in memory)
static const char g_szPassthroughVS[] =
    "struct VSInput {\n"
    "    float3 pos : POSITION;\n"
    "    float4 col : COLOR;\n"
    "};\n"
    "struct VSOutput {\n"
    "    float4 pos : SV_POSITION;\n"
    "    float4 col : COLOR;\n"
    "};\n"
    "VSOutput main(VSInput input) {\n"
    "    VSOutput output;\n"
    "    output.pos = float4(input.pos, 1.0);\n"
    "    output.col = input.col.bgra;\n"  // swap R and B for DX9 D3DCOLOR compatibility
    "    return output;\n"
    "}\n";

// PS: output vertex color
static const char g_szPassthroughPS[] =
    "struct PSInput {\n"
    "    float4 pos : SV_POSITION;\n"
    "    float4 col : COLOR;\n"
    "};\n"
    "float4 main(PSInput input) : SV_TARGET {\n"
    "    return input.col;\n"
    "}\n";

// Textured passthrough shaders (for warp + composite passes)
// VS: position passthrough + .bgra color swizzle + texcoord0 passthrough
static const char g_szTexturedVS[] =
    "struct VSInput {\n"
    "    float3 pos : POSITION;\n"
    "    float4 col : COLOR;\n"
    "    float2 uv  : TEXCOORD0;\n"
    "};\n"
    "struct VSOutput {\n"
    "    float4 pos : SV_POSITION;\n"
    "    float4 col : COLOR;\n"
    "    float2 uv  : TEXCOORD0;\n"
    "};\n"
    "VSOutput main(VSInput input) {\n"
    "    VSOutput output;\n"
    "    output.pos = float4(input.pos, 1.0);\n"
    "    output.col = input.col.bgra;\n"
    "    output.uv  = input.uv;\n"
    "    return output;\n"
    "}\n";

// PS: sample texture at texcoord0, modulate by vertex color
static const char g_szTexturedPS[] =
    "Texture2D    tex  : register(t0);\n"
    "SamplerState samp : register(s0);\n"
    "struct PSInput {\n"
    "    float4 pos : SV_POSITION;\n"
    "    float4 col : COLOR;\n"
    "    float2 uv  : TEXCOORD0;\n"
    "};\n"
    "float4 main(PSInput input) : SV_TARGET {\n"
    "    return tex.Sample(samp, input.uv) * input.col;\n"
    "}\n";

// Warp vertex shader: passes all MYVERTEX fields to match MilkDrop PS signatures.
// SV_POSITION is declared LAST so that the compiler assigns user semantics
// (COLOR, TEXCOORD0, TEXCOORD1) to registers 0,1,2 — matching the PS input.
// Output: float4 TEXCOORD0 = (tu, tv, tu_orig, tv_orig), float2 TEXCOORD1 = (rad, ang)
static const char g_szWarpVS[] =
    "void main(\n"
    "    float3 pos     : POSITION,\n"
    "    float4 col     : COLOR,\n"
    "    float2 uv      : TEXCOORD0,\n"
    "    float2 uv_orig : TEXCOORD1,\n"
    "    float2 rad_ang : TEXCOORD2,\n"
    "    out float4 oCol     : COLOR,\n"
    "    out float4 oUv      : TEXCOORD0,\n"
    "    out float2 oRadAng  : TEXCOORD1,\n"
    "    out float4 oPos     : SV_POSITION\n"
    ") {\n"
    "    oPos     = float4(pos, 1.0);\n"
    "    oCol     = col.bgra;\n"
    "    oUv      = float4(uv, uv_orig);\n"
    "    oRadAng  = rad_ang;\n"
    "}\n";

// Composite vertex shader: float2 UV + float2 rad_ang
static const char g_szCompVS[] =
    "void main(\n"
    "    float3 pos     : POSITION,\n"
    "    float4 col     : COLOR,\n"
    "    float2 uv      : TEXCOORD0,\n"
    "    float2 uv_orig : TEXCOORD1,\n"
    "    float2 rad_ang : TEXCOORD2,\n"
    "    out float4 oCol     : COLOR,\n"
    "    out float2 oUv      : TEXCOORD0,\n"
    "    out float2 oRadAng  : TEXCOORD1,\n"
    "    out float4 oPos     : SV_POSITION\n"
    ") {\n"
    "    oPos     = float4(pos, 1.0);\n"
    "    oCol     = col.bgra;\n"
    "    oUv      = uv;\n"
    "    oRadAng  = rad_ang;\n"
    "}\n";

// Pre-compiled warp/comp VS bytecodes (populated by DX12CreatePipelines)
extern ID3DBlob* g_pWarpVSBlob;
extern ID3DBlob* g_pCompVSBlob;

// Create all initial PSOs needed for first draw
bool DX12CreatePipelines(ID3D12Device* device, ID3D12RootSignature* rootSig,
                          DXGI_FORMAT rtvFormat,
                          ComPtr<ID3D12PipelineState> psoArray[PSO_COUNT]);

// Create a PSO from preset-compiled pixel shader bytecode.
// If pMainTexSlotOut is non-null, reflects the PS to find which t-register
// "sampler_main" (or "sampler_fc_main" etc.) maps to and writes it there.
// Returns UINT_MAX if not found.
ComPtr<ID3D12PipelineState> DX12CreatePresetPSO(
    ID3D12Device* device, ID3D12RootSignature* rootSig,
    DXGI_FORMAT rtvFormat,
    ID3DBlob* vsBlob,
    const void* psBytecode, UINT psSize,
    const D3D12_INPUT_ELEMENT_DESC* layout, UINT layoutCount,
    bool alphaBlend,
    UINT* pMainTexSlotOut = nullptr);

#endif // DX12PIPELINE_H
