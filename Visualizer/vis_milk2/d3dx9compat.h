/*
  d3dx9compat.h
  Minimal compatibility shim replacing the legacy DirectX SDK d3dx9.h / d3dx9math.h
  headers that are not present in the modern Windows 10/11 SDK.

  Include this file wherever d3dx9.h or d3dx9math.h was previously included.
  It provides:
    - D3DX math structs (D3DXVECTOR2/3/4, D3DXMATRIX, D3DXCOLOR, D3DXHANDLE)
    - Inline implementations of the matrix math functions used by this codebase
    - Forward COM interface stubs for ID3DXBuffer, ID3DXConstantTable, ID3DXFont
      (full DX12 replacements are Phase 2-5 TODO)
    - A compile-time no-op stub for D3DXCompileShader (Phase 3 TODO: D3DCompile)
    - Texture loading stubs (D3DXCreateTextureFromFileExW returns error; Phase 2 TODO)

  The memory layout of D3DXVECTOR* and D3DXMATRIX is intentionally identical to
  the original D3DX types so that any persistent data or binary blobs remain valid.
*/

#ifndef D3DX9COMPAT_H
#define D3DX9COMPAT_H

#include <d3d9.h>    // IDirect3DDevice9, D3DFORMAT, D3DFVF_*, D3DCOLOR, D3DMATRIX, etc.
#include <d3dcompiler.h> // D3DCompile, D3DReflect
#include <math.h>    // sinf, cosf, sqrtf
#include <string.h>  // memset, memcpy
#include <vector>
#include <string>

// ── Vector types ──────────────────────────────────────────────────────────────

typedef struct _D3DXVECTOR2 {
    float x, y;
    _D3DXVECTOR2() {}
    _D3DXVECTOR2(float _x, float _y) : x(_x), y(_y) {}
} D3DXVECTOR2, *LPD3DXVECTOR2;

typedef struct _D3DXVECTOR3 {
    float x, y, z;
    _D3DXVECTOR3() {}
    _D3DXVECTOR3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
} D3DXVECTOR3, *LPD3DXVECTOR3;

typedef struct _D3DXVECTOR4 {
    float x, y, z, w;
    _D3DXVECTOR4() {}
    _D3DXVECTOR4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
    // Bracket access used by some callers
    float& operator[](int i) { return (&x)[i]; }
    float  operator[](int i) const { return (&x)[i]; }
} D3DXVECTOR4, *LPD3DXVECTOR4;

// D3DXCOLOR — same layout as D3DXVECTOR4 (r,g,b,a)
typedef struct _D3DXCOLOR {
    float r, g, b, a;
    _D3DXCOLOR() {}
    _D3DXCOLOR(float _r, float _g, float _b, float _a) : r(_r), g(_g), b(_b), a(_a) {}
    _D3DXCOLOR(DWORD argb) {
        a = ((argb >> 24) & 0xFF) / 255.0f;
        r = ((argb >> 16) & 0xFF) / 255.0f;
        g = ((argb >>  8) & 0xFF) / 255.0f;
        b = ((argb      ) & 0xFF) / 255.0f;
    }
    operator DWORD() const {
        return (((DWORD)(a * 255)) << 24) | (((DWORD)(r * 255)) << 16)
             | (((DWORD)(g * 255)) <<  8) |  ((DWORD)(b * 255));
    }
} D3DXCOLOR, *LPD3DXCOLOR;

// ── Matrix type ───────────────────────────────────────────────────────────────

// Row-major 4×4 matrix — inherits from D3DMATRIX (same memory layout as original D3DXMATRIX).
// Inheriting from D3DMATRIX allows implicit conversion to const D3DMATRIX* required by
// IDirect3DDevice9::SetTransform() and similar DX9 calls that take a D3DMATRIX*.
struct _D3DXMATRIX : public D3DMATRIX {
    _D3DXMATRIX() {}
    // Row accessor used by some callers
    float* operator[](int row) { return m[row]; }
    const float* operator[](int row) const { return m[row]; }
};
typedef _D3DXMATRIX D3DXMATRIX, *LPD3DXMATRIX;

// ── Handle type ───────────────────────────────────────────────────────────────

#ifndef D3DXHANDLE_DEFINED
typedef const char* D3DXHANDLE;
#define D3DXHANDLE_DEFINED
#endif

// ── COM interface stubs ───────────────────────────────────────────────────────

// ID3DXBuffer — used for compiled shader blobs and error messages.
#ifndef __ID3DXBuffer_FWD_DEFINED__
#define __ID3DXBuffer_FWD_DEFINED__
struct ID3DXBuffer : public IUnknown {
    virtual void* STDMETHODCALLTYPE GetBufferPointer() = 0;
    virtual DWORD STDMETHODCALLTYPE GetBufferSize()    = 0;
};
typedef ID3DXBuffer* LPD3DXBUFFER;

// DX12ShaderBlob — wraps an ID3DBlob into the ID3DXBuffer interface.
class DX12ShaderBlob : public ID3DXBuffer {
    ID3DBlob* m_blob;
    LONG m_refCount;
public:
    DX12ShaderBlob(ID3DBlob* blob) : m_blob(blob), m_refCount(1) {
        if (m_blob) m_blob->AddRef();
    }
    ~DX12ShaderBlob() { if (m_blob) m_blob->Release(); }
    void* STDMETHODCALLTYPE GetBufferPointer() override { return m_blob ? m_blob->GetBufferPointer() : nullptr; }
    DWORD STDMETHODCALLTYPE GetBufferSize() override { return m_blob ? (DWORD)m_blob->GetBufferSize() : 0; }
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) override {
        if (riid == __uuidof(IUnknown)) { *ppv = this; AddRef(); return S_OK; }
        *ppv = nullptr; return E_NOINTERFACE;
    }
    ULONG STDMETHODCALLTYPE AddRef() override { return InterlockedIncrement(&m_refCount); }
    ULONG STDMETHODCALLTYPE Release() override {
        LONG r = InterlockedDecrement(&m_refCount);
        if (r == 0) delete this;
        return r;
    }
};
#endif

// ID3DXConstantTable — shader constant binding interface.
// Forward-declare types used in method signatures (full definitions are below).
struct _D3DXCONSTANT_DESC;
typedef _D3DXCONSTANT_DESC D3DXCONSTANT_DESC;
struct _D3DXCONSTANTTABLE_DESC;
typedef _D3DXCONSTANTTABLE_DESC D3DXCONSTANTTABLE_DESC;

#ifndef __ID3DXConstantTable_FWD_DEFINED__
#define __ID3DXConstantTable_FWD_DEFINED__
struct ID3DXConstantTable : public IUnknown {
    // Query methods (needed by CShaderParams::CacheParams in plugin.cpp)
    virtual HRESULT    STDMETHODCALLTYPE GetDesc(D3DXCONSTANTTABLE_DESC* pDesc) = 0;
    virtual HRESULT    STDMETHODCALLTYPE GetConstantDesc(D3DXHANDLE hConstant, D3DXCONSTANT_DESC* pDesc, UINT* pCount) = 0;
    virtual D3DXHANDLE STDMETHODCALLTYPE GetConstant(D3DXHANDLE hConstant, UINT Index) = 0;
    virtual D3DXHANDLE STDMETHODCALLTYPE GetConstantByName(D3DXHANDLE hConstant, LPCSTR pName) = 0;
    // Set methods
    virtual HRESULT    STDMETHODCALLTYPE SetDefaults(LPDIRECT3DDEVICE9 pDevice) = 0;
    virtual HRESULT    STDMETHODCALLTYPE SetFloat(LPDIRECT3DDEVICE9 pDevice, D3DXHANDLE hConstant, FLOAT f) = 0;
    virtual HRESULT    STDMETHODCALLTYPE SetFloatArray(LPDIRECT3DDEVICE9 pDevice, D3DXHANDLE hConstant, const FLOAT* pf, UINT Count) = 0;
    virtual HRESULT    STDMETHODCALLTYPE SetVector(LPDIRECT3DDEVICE9 pDevice, D3DXHANDLE hConstant, const D3DXVECTOR4* pVector) = 0;
    virtual HRESULT    STDMETHODCALLTYPE SetVectorArray(LPDIRECT3DDEVICE9 pDevice, D3DXHANDLE hConstant, const D3DXVECTOR4* pVector, UINT Count) = 0;
    virtual HRESULT    STDMETHODCALLTYPE SetMatrix(LPDIRECT3DDEVICE9 pDevice, D3DXHANDLE hConstant, const D3DXMATRIX* pMatrix) = 0;
    virtual HRESULT    STDMETHODCALLTYPE SetMatrixArray(LPDIRECT3DDEVICE9 pDevice, D3DXHANDLE hConstant, const D3DXMATRIX* pMatrix, UINT Count) = 0;
    virtual HRESULT    STDMETHODCALLTYPE SetBool(LPDIRECT3DDEVICE9 pDevice, D3DXHANDLE hConstant, BOOL b) = 0;
    virtual HRESULT    STDMETHODCALLTYPE SetInt(LPDIRECT3DDEVICE9 pDevice, D3DXHANDLE hConstant, INT n) = 0;
};
typedef ID3DXConstantTable* LPD3DXCONSTANTTABLE;
#endif

// ID3DXFont — GDI-based font renderer.
// Phase 5 TODO: replaced by DirectXTK12 SpriteFont.
// DrawText/DrawTextW are the D3DX method names; the Win32 macros in winuser.h
// (#define DrawText DrawTextA / DrawTextW) resolve these to the correct overload.
#ifndef __ID3DXFont_FWD_DEFINED__
#define __ID3DXFont_FWD_DEFINED__
struct ID3DXFont : public IUnknown {
    // Minimal vtable — matches original D3DX layout for compiled code compatibility.
    virtual HRESULT STDMETHODCALLTYPE GetDevice(LPDIRECT3DDEVICE9* ppDevice) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetDescA(void* pDesc) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetDescW(void* pDesc) = 0;
    virtual BOOL    STDMETHODCALLTYPE GetTextMetricsA(TEXTMETRICA* pTextMetrics) = 0;
    virtual BOOL    STDMETHODCALLTYPE GetTextMetricsW(TEXTMETRICW* pTextMetrics) = 0;
    virtual HDC     STDMETHODCALLTYPE GetDC() = 0;
    virtual HRESULT STDMETHODCALLTYPE GetGlyphData(UINT Glyph, LPDIRECT3DTEXTURE9* ppTexture, RECT* pBlackBox, POINT* pCellInc) = 0;
    virtual HRESULT STDMETHODCALLTYPE PreloadCharacters(UINT First, UINT Last) = 0;
    virtual HRESULT STDMETHODCALLTYPE PreloadGlyphs(UINT First, UINT Last) = 0;
    virtual HRESULT STDMETHODCALLTYPE PreloadTextA(LPCSTR pString, INT Count) = 0;
    virtual HRESULT STDMETHODCALLTYPE PreloadTextW(LPCWSTR pString, INT Count) = 0;
    virtual INT     STDMETHODCALLTYPE DrawTextA(void* pSprite, LPCSTR  pString, INT Count, LPRECT pRect, DWORD Format, D3DCOLOR Color) = 0;
    virtual INT     STDMETHODCALLTYPE DrawTextW(void* pSprite, LPCWSTR pString, INT Count, LPRECT pRect, DWORD Format, D3DCOLOR Color) = 0;
    virtual HRESULT STDMETHODCALLTYPE OnLostDevice()  = 0;
    virtual HRESULT STDMETHODCALLTYPE OnResetDevice() = 0;
};
typedef ID3DXFont* LPD3DXFONT;
#endif

// ── Matrix math functions (inline — same semantics as D3DX originals) ─────────

inline D3DXMATRIX* D3DXMatrixIdentity(D3DXMATRIX* pOut) {
    memset(pOut, 0, sizeof(D3DXMATRIX));
    pOut->_11 = pOut->_22 = pOut->_33 = pOut->_44 = 1.0f;
    return pOut;
}

inline D3DXMATRIX* D3DXMatrixMultiply(D3DXMATRIX* pOut,
                                       const D3DXMATRIX* pM1,
                                       const D3DXMATRIX* pM2) {
    D3DXMATRIX tmp;
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++) {
            tmp.m[row][col] = 0.0f;
            for (int k = 0; k < 4; k++)
                tmp.m[row][col] += pM1->m[row][k] * pM2->m[k][col];
        }
    *pOut = tmp;
    return pOut;
}

inline D3DXMATRIX* D3DXMatrixOrthoLH(D3DXMATRIX* pOut,
                                      float w, float h, float zn, float zf) {
    D3DXMatrixIdentity(pOut);
    pOut->_11 = 2.0f / w;
    pOut->_22 = 2.0f / h;
    pOut->_33 = 1.0f / (zf - zn);
    pOut->_43 = -zn  / (zf - zn);
    return pOut;
}

inline D3DXMATRIX* D3DXMatrixTranslation(D3DXMATRIX* pOut, float x, float y, float z) {
    D3DXMatrixIdentity(pOut);
    pOut->_41 = x; pOut->_42 = y; pOut->_43 = z;
    return pOut;
}

inline D3DXMATRIX* D3DXMatrixScaling(D3DXMATRIX* pOut, float sx, float sy, float sz) {
    D3DXMatrixIdentity(pOut);
    pOut->_11 = sx; pOut->_22 = sy; pOut->_33 = sz;
    return pOut;
}

inline D3DXMATRIX* D3DXMatrixRotationX(D3DXMATRIX* pOut, float angle) {
    D3DXMatrixIdentity(pOut);
    float s = sinf(angle), c = cosf(angle);
    pOut->_22 =  c; pOut->_23 = s;
    pOut->_32 = -s; pOut->_33 = c;
    return pOut;
}

inline D3DXMATRIX* D3DXMatrixRotationY(D3DXMATRIX* pOut, float angle) {
    D3DXMatrixIdentity(pOut);
    float s = sinf(angle), c = cosf(angle);
    pOut->_11 = c; pOut->_13 = -s;
    pOut->_31 = s; pOut->_33 =  c;
    return pOut;
}

inline D3DXMATRIX* D3DXMatrixRotationZ(D3DXMATRIX* pOut, float angle) {
    D3DXMatrixIdentity(pOut);
    float s = sinf(angle), c = cosf(angle);
    pOut->_11 =  c; pOut->_12 = s;
    pOut->_21 = -s; pOut->_22 = c;
    return pOut;
}

inline D3DXMATRIX* D3DXMatrixTranspose(D3DXMATRIX* pOut, const D3DXMATRIX* pM) {
    D3DXMATRIX tmp;
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++)
            tmp.m[r][c] = pM->m[c][r];
    *pOut = tmp;
    return pOut;
}

// Left-handed view matrix — same semantics as D3DXMatrixLookAtLH.
// D3D row-vector convention: output rows are the camera's right/up/forward axes,
// and the translation row encodes the negated eye position in camera space.
inline D3DXMATRIX* D3DXMatrixLookAtLH(D3DXMATRIX* pOut,
                                        const D3DXVECTOR3* pEye,
                                        const D3DXVECTOR3* pAt,
                                        const D3DXVECTOR3* pUp) {
    // zaxis = normalize(pAt - pEye)
    D3DXVECTOR3 zaxis = { pAt->x - pEye->x, pAt->y - pEye->y, pAt->z - pEye->z };
    float zlen = sqrtf(zaxis.x*zaxis.x + zaxis.y*zaxis.y + zaxis.z*zaxis.z);
    if (zlen > 0.0f) { zaxis.x /= zlen; zaxis.y /= zlen; zaxis.z /= zlen; }

    // xaxis = normalize(cross(pUp, zaxis))
    D3DXVECTOR3 xaxis = {
        pUp->y*zaxis.z - pUp->z*zaxis.y,
        pUp->z*zaxis.x - pUp->x*zaxis.z,
        pUp->x*zaxis.y - pUp->y*zaxis.x
    };
    float xlen = sqrtf(xaxis.x*xaxis.x + xaxis.y*xaxis.y + xaxis.z*xaxis.z);
    if (xlen > 0.0f) { xaxis.x /= xlen; xaxis.y /= xlen; xaxis.z /= xlen; }

    // yaxis = cross(zaxis, xaxis)
    D3DXVECTOR3 yaxis = {
        zaxis.y*xaxis.z - zaxis.z*xaxis.y,
        zaxis.z*xaxis.x - zaxis.x*xaxis.z,
        zaxis.x*xaxis.y - zaxis.y*xaxis.x
    };

    memset(pOut, 0, sizeof(D3DXMATRIX));
    pOut->_11 = xaxis.x;  pOut->_12 = yaxis.x;  pOut->_13 = zaxis.x;  pOut->_14 = 0.0f;
    pOut->_21 = xaxis.y;  pOut->_22 = yaxis.y;  pOut->_23 = zaxis.y;  pOut->_24 = 0.0f;
    pOut->_31 = xaxis.z;  pOut->_32 = yaxis.z;  pOut->_33 = zaxis.z;  pOut->_34 = 0.0f;
    pOut->_41 = -(xaxis.x*pEye->x + xaxis.y*pEye->y + xaxis.z*pEye->z);
    pOut->_42 = -(yaxis.x*pEye->x + yaxis.y*pEye->y + yaxis.z*pEye->z);
    pOut->_43 = -(zaxis.x*pEye->x + zaxis.y*pEye->y + zaxis.z*pEye->z);
    pOut->_44 = 1.0f;
    return pOut;
}

// ── Texture loading stubs ─────────────────────────────────────────────────────
// Phase 2 TODO: replace with DirectXTex (LoadFromWICFile + CreateTexture).
// These stubs allow Phase 1 code to compile; textures simply won't load until Phase 2.

#define D3DX_DEFAULT ((UINT)0xFFFFFFFF)

typedef struct _D3DXIMAGE_INFO {
    UINT      Width;
    UINT      Height;
    UINT      Depth;
    UINT      MipLevels;
    D3DFORMAT Format;
    DWORD     ResourceType;   // D3DRESOURCETYPE equivalent
    DWORD     ImageFileFormat; // D3DXIMAGE_FILEFORMAT equivalent
} D3DXIMAGE_INFO;

inline HRESULT D3DXCreateTextureFromFileExW(
    LPDIRECT3DDEVICE9  pDevice,
    LPCWSTR            pSrcFile,
    UINT               Width,
    UINT               Height,
    UINT               MipLevels,
    DWORD              Usage,
    D3DFORMAT          Format,
    D3DPOOL            Pool,
    DWORD              Filter,
    DWORD              MipFilter,
    D3DCOLOR           ColorKey,
    D3DXIMAGE_INFO*    pSrcInfo,
    PALETTEENTRY*      pPalette,
    LPDIRECT3DTEXTURE9* ppTexture)
{
    if (ppTexture) *ppTexture = nullptr;
    if (pSrcInfo)  memset(pSrcInfo, 0, sizeof(*pSrcInfo));
    return D3DERR_INVALIDCALL;  // Phase 2 TODO: replace with DirectXTex
}

// ── Shader constant table types ───────────────────────────────────────────────
// These types are from d3dx9shader.h / d3dx9effect.h in the legacy D3DX library.
// Stubs provided here so that Phase 3 shader-constant code in plugin.cpp compiles.
// Phase 3 TODO: replace with D3D12 root-signature + constant-buffer equivalent.

#define D3DXSHADER_DEBUG        (1 << 0)  // include debug info in shader bytecode
#define D3DXSHADER_SKIPVALIDATION (1 << 1)

#define D3DX_DEFAULT_NONPOW2    ((UINT)0xFFFFFFFE)  // use nearest non-power-of-2 size

typedef enum _D3DXREGISTER_SET {
    D3DXRS_BOOL    = 0,
    D3DXRS_INT4    = 1,
    D3DXRS_FLOAT4  = 2,
    D3DXRS_SAMPLER = 3,
} D3DXREGISTER_SET;

typedef enum _D3DXPARAMETER_CLASS {
    D3DXPC_SCALAR         = 0,
    D3DXPC_VECTOR         = 1,
    D3DXPC_MATRIX_ROWS    = 2,
    D3DXPC_MATRIX_COLUMNS = 3,
    D3DXPC_OBJECT         = 4,
    D3DXPC_STRUCT         = 5,
} D3DXPARAMETER_CLASS;

typedef enum _D3DXPARAMETER_TYPE {
    D3DXPT_VOID       = 0,
    D3DXPT_BOOL       = 1,
    D3DXPT_INT        = 2,
    D3DXPT_FLOAT      = 3,
    D3DXPT_STRING     = 4,
    D3DXPT_TEXTURE    = 5,
    D3DXPT_TEXTURE1D  = 6,
    D3DXPT_TEXTURE2D  = 7,
    D3DXPT_TEXTURE3D  = 8,
    D3DXPT_TEXTURECUBE = 9,
    D3DXPT_SAMPLER    = 10,
    D3DXPT_SAMPLER1D  = 11,
    D3DXPT_SAMPLER2D  = 12,
    D3DXPT_SAMPLER3D  = 13,
    D3DXPT_SAMPLERCUBE = 14,
    D3DXPT_PIXELSHADER = 15,
    D3DXPT_VERTEXSHADER = 16,
    D3DXPT_PIXELFRAGMENT = 17,
    D3DXPT_VERTEXFRAGMENT = 18,
    D3DXPT_UNSUPPORTED = 19,
} D3DXPARAMETER_TYPE;

typedef struct _D3DXCONSTANT_DESC {
    LPCSTR              Name;
    D3DXREGISTER_SET    RegisterSet;
    UINT                RegisterIndex;
    UINT                RegisterCount;
    D3DXPARAMETER_CLASS Class;
    D3DXPARAMETER_TYPE  Type;
    UINT                Rows;
    UINT                Columns;
    UINT                Elements;
    UINT                StructMembers;
    UINT                Bytes;
    LPCVOID             DefaultValue;
} D3DXCONSTANT_DESC;

typedef struct _D3DXCONSTANTTABLE_DESC {
    LPCSTR  Creator;
    DWORD   Version;
    UINT    Constants;
} D3DXCONSTANTTABLE_DESC;

// DX12ConstantTable — implements ID3DXConstantTable using D3DReflect on SM5.0 bytecode.
// Introspects the $Globals cbuffer and bound resources, populating D3DXCONSTANT_DESC
// structs so that CShaderParams::CacheParams works as-is.
class DX12ConstantTable : public ID3DXConstantTable {
    struct ConstEntry {
        std::string name;
        D3DXCONSTANT_DESC desc;
    };
    std::vector<ConstEntry> m_constants;
    std::vector<BYTE> m_shadowCB; // shadow constant buffer data
    LONG m_refCount;

public:
    DX12ConstantTable() : m_refCount(1) {}

    static DX12ConstantTable* CreateFromBytecode(const void* bytecode, SIZE_T bytecodeSize) {
        ID3D11ShaderReflection* pReflect = nullptr;
        HRESULT hr = D3DReflect(bytecode, bytecodeSize, IID_ID3D11ShaderReflection, (void**)&pReflect);
        if (FAILED(hr) || !pReflect) return nullptr;

        DX12ConstantTable* ct = new DX12ConstantTable();
        D3D11_SHADER_DESC shaderDesc;
        pReflect->GetDesc(&shaderDesc);

        // Enumerate bound resources for samplers
        for (UINT i = 0; i < shaderDesc.BoundResources; i++) {
            D3D11_SHADER_INPUT_BIND_DESC bindDesc;
            pReflect->GetResourceBindingDesc(i, &bindDesc);
            if (bindDesc.Type == D3D_SIT_SAMPLER) {
                ConstEntry e;
                e.name = bindDesc.Name;
                memset(&e.desc, 0, sizeof(e.desc));
                e.desc.Name = nullptr; // set after push_back (pointer to e.name.c_str())
                e.desc.RegisterSet = D3DXRS_SAMPLER;
                e.desc.RegisterIndex = bindDesc.BindPoint;
                e.desc.RegisterCount = bindDesc.BindCount;
                e.desc.Class = D3DXPC_OBJECT;
                e.desc.Type = D3DXPT_SAMPLER2D;
                e.desc.Rows = 0;
                e.desc.Columns = 0;
                e.desc.Elements = 0;
                e.desc.Bytes = 0;
                ct->m_constants.push_back(std::move(e));
            }
        }

        // Enumerate $Globals cbuffer variables for float4 constants
        ID3D11ShaderReflectionConstantBuffer* pCB = pReflect->GetConstantBufferByName("$Globals");
        if (pCB) {
            D3D11_SHADER_BUFFER_DESC cbDesc;
            if (SUCCEEDED(pCB->GetDesc(&cbDesc))) {
                ct->m_shadowCB.resize(cbDesc.Size, 0);
                for (UINT i = 0; i < cbDesc.Variables; i++) {
                    ID3D11ShaderReflectionVariable* pVar = pCB->GetVariableByIndex(i);
                    D3D11_SHADER_VARIABLE_DESC varDesc;
                    pVar->GetDesc(&varDesc);
                    ID3D11ShaderReflectionType* pType = pVar->GetType();
                    D3D11_SHADER_TYPE_DESC typeDesc;
                    pType->GetDesc(&typeDesc);

                    ConstEntry e;
                    e.name = varDesc.Name;
                    memset(&e.desc, 0, sizeof(e.desc));
                    e.desc.RegisterSet = D3DXRS_FLOAT4;
                    e.desc.RegisterIndex = varDesc.StartOffset / 16;
                    e.desc.RegisterCount = (varDesc.Size + 15) / 16;
                    e.desc.Rows = typeDesc.Rows;
                    e.desc.Columns = typeDesc.Columns;
                    e.desc.Elements = typeDesc.Elements;
                    e.desc.Bytes = varDesc.Size;

                    if (typeDesc.Class == D3D_SVC_SCALAR) e.desc.Class = D3DXPC_SCALAR;
                    else if (typeDesc.Class == D3D_SVC_VECTOR) e.desc.Class = D3DXPC_VECTOR;
                    else if (typeDesc.Class == D3D_SVC_MATRIX_ROWS) e.desc.Class = D3DXPC_MATRIX_ROWS;
                    else if (typeDesc.Class == D3D_SVC_MATRIX_COLUMNS) e.desc.Class = D3DXPC_MATRIX_COLUMNS;
                    else e.desc.Class = D3DXPC_SCALAR;

                    if (typeDesc.Type == D3D_SVT_FLOAT) e.desc.Type = D3DXPT_FLOAT;
                    else if (typeDesc.Type == D3D_SVT_INT) e.desc.Type = D3DXPT_INT;
                    else if (typeDesc.Type == D3D_SVT_BOOL) e.desc.Type = D3DXPT_BOOL;
                    else e.desc.Type = D3DXPT_FLOAT;

                    if (varDesc.DefaultValue && varDesc.Size <= ct->m_shadowCB.size()) {
                        memcpy(&ct->m_shadowCB[varDesc.StartOffset], varDesc.DefaultValue, varDesc.Size);
                        e.desc.DefaultValue = &ct->m_shadowCB[varDesc.StartOffset];
                    }

                    ct->m_constants.push_back(std::move(e));
                }
            }
        }

        // Fix up Name pointers to point at stable std::string storage
        for (auto& e : ct->m_constants)
            e.desc.Name = e.name.c_str();

        pReflect->Release();
        return ct;
    }

    // IUnknown
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) override {
        if (riid == __uuidof(IUnknown)) { *ppv = this; AddRef(); return S_OK; }
        *ppv = nullptr; return E_NOINTERFACE;
    }
    ULONG STDMETHODCALLTYPE AddRef() override { return InterlockedIncrement(&m_refCount); }
    ULONG STDMETHODCALLTYPE Release() override {
        LONG r = InterlockedDecrement(&m_refCount);
        if (r == 0) delete this;
        return r;
    }

    // ID3DXConstantTable
    HRESULT STDMETHODCALLTYPE GetDesc(D3DXCONSTANTTABLE_DESC* pDesc) override {
        if (!pDesc) return E_INVALIDARG;
        pDesc->Creator = "DX12ConstantTable";
        pDesc->Version = 0x0500; // SM5
        pDesc->Constants = (UINT)m_constants.size();
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE GetConstantDesc(D3DXHANDLE hConstant, D3DXCONSTANT_DESC* pDesc, UINT* pCount) override {
        if (!hConstant || !pDesc) return E_INVALIDARG;
        // hConstant is a pointer into our ConstEntry array
        const ConstEntry* entry = reinterpret_cast<const ConstEntry*>(hConstant);
        *pDesc = entry->desc;
        if (pCount) *pCount = 1;
        return S_OK;
    }
    D3DXHANDLE STDMETHODCALLTYPE GetConstant(D3DXHANDLE /*hConstant*/, UINT Index) override {
        if (Index >= m_constants.size()) return nullptr;
        return reinterpret_cast<D3DXHANDLE>(&m_constants[Index]);
    }
    D3DXHANDLE STDMETHODCALLTYPE GetConstantByName(D3DXHANDLE /*hConstant*/, LPCSTR pName) override {
        if (!pName) return nullptr;
        for (auto& e : m_constants)
            if (e.name == pName) return reinterpret_cast<D3DXHANDLE>(&e);
        return nullptr;
    }

    // Set methods — write into shadow cbuffer (no DX9 device needed)
    HRESULT STDMETHODCALLTYPE SetDefaults(LPDIRECT3DDEVICE9) override { return S_OK; }

    int FindConstIndex(D3DXHANDLE h) const {
        if (!h) return -1;
        const ConstEntry* entry = reinterpret_cast<const ConstEntry*>(h);
        ptrdiff_t idx = entry - m_constants.data();
        if (idx >= 0 && idx < (ptrdiff_t)m_constants.size()) return (int)idx;
        return -1;
    }
    void WriteShadow(int idx, const void* data, UINT bytes) {
        if (idx < 0) return;
        UINT offset = m_constants[idx].desc.RegisterIndex * 16;
        UINT maxBytes = m_constants[idx].desc.RegisterCount * 16;
        if (bytes > maxBytes) bytes = maxBytes;
        if (offset + bytes <= m_shadowCB.size())
            memcpy(&m_shadowCB[offset], data, bytes);
    }

    HRESULT STDMETHODCALLTYPE SetFloat(LPDIRECT3DDEVICE9, D3DXHANDLE h, FLOAT f) override {
        float v[4] = { f, 0, 0, 0 };
        WriteShadow(FindConstIndex(h), v, 16);
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE SetFloatArray(LPDIRECT3DDEVICE9, D3DXHANDLE h, const FLOAT* pf, UINT Count) override {
        WriteShadow(FindConstIndex(h), pf, Count * sizeof(float));
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE SetVector(LPDIRECT3DDEVICE9, D3DXHANDLE h, const D3DXVECTOR4* pV) override {
        WriteShadow(FindConstIndex(h), pV, 16);
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE SetVectorArray(LPDIRECT3DDEVICE9, D3DXHANDLE h, const D3DXVECTOR4* pV, UINT Count) override {
        WriteShadow(FindConstIndex(h), pV, Count * 16);
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE SetMatrix(LPDIRECT3DDEVICE9, D3DXHANDLE h, const D3DXMATRIX* pM) override {
        WriteShadow(FindConstIndex(h), pM, 64);
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE SetMatrixArray(LPDIRECT3DDEVICE9, D3DXHANDLE h, const D3DXMATRIX* pM, UINT Count) override {
        WriteShadow(FindConstIndex(h), pM, Count * 64);
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE SetBool(LPDIRECT3DDEVICE9, D3DXHANDLE h, BOOL b) override {
        float v[4] = { b ? 1.0f : 0.0f, 0, 0, 0 };
        WriteShadow(FindConstIndex(h), v, 16);
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE SetInt(LPDIRECT3DDEVICE9, D3DXHANDLE h, INT n) override {
        float v[4] = { (float)n, 0, 0, 0 };
        WriteShadow(FindConstIndex(h), v, 16);
        return S_OK;
    }

    // Accessors for DX12 rendering
    const BYTE* GetShadowData() const { return m_shadowCB.data(); }
    UINT GetShadowSize() const { return (UINT)m_shadowCB.size(); }
};

// D3DXCreateFontW stub — Phase 5 TODO: replaced by DirectXTK12 SpriteFont.
// Returning E_NOTIMPL means font creation always fails gracefully.
inline HRESULT D3DXCreateFontW(
    LPDIRECT3DDEVICE9 pDevice,
    INT               Height,
    UINT              Width,
    UINT              Weight,
    UINT              MipLevels,
    BOOL              Italic,
    DWORD             CharSet,
    DWORD             OutputPrecision,
    DWORD             Quality,
    DWORD             PitchAndFamily,
    LPCWSTR           pFaceName,
    LPD3DXFONT*       ppFont)
{
    if (ppFont) *ppFont = nullptr;
    return E_NOTIMPL;  // Phase 5 TODO: DirectXTK12 SpriteFont
}

// D3DXCreateTexture stub — creates a plain texture on the device.
// Phase 2 TODO: replaced by ID3D12Resource creation via D3D12 upload heap.
inline HRESULT D3DXCreateTexture(
    LPDIRECT3DDEVICE9   pDevice,
    UINT                Width,
    UINT                Height,
    UINT                MipLevels,
    DWORD               Usage,
    D3DFORMAT           Format,
    D3DPOOL             Pool,
    LPDIRECT3DTEXTURE9* ppTexture)
{
    if (!pDevice || !ppTexture) return D3DERR_INVALIDCALL;
    return pDevice->CreateTexture(Width, Height, MipLevels, Usage, Format, Pool, ppTexture, NULL);
}

// D3DXGetShaderConstantTable — extracts constant table from compiled SM5.0 bytecode via reflection.
inline HRESULT D3DXGetShaderConstantTable(
    const DWORD*          pFunction,
    LPD3DXCONSTANTTABLE*  ppConstantTable)
{
    if (ppConstantTable) *ppConstantTable = nullptr;
    if (!pFunction || !ppConstantTable) return E_INVALIDARG;

    // We need to know the bytecode size. ID3DBlob from D3DCompile stores size,
    // but raw DWORD* from a cached file doesn't carry length metadata.
    // Use D3DGetBlobPart to probe validity; if it fails, try D3DReflect directly.
    // For cached blobs loaded via our LoadShaderBytecodeFromFile, the buffer
    // is a DX12ShaderBlob whose GetBufferSize() gives the real size.
    // Since we can't get size from a raw DWORD*, use D3DReflect with a generous size.
    // The caller should use the blob's size when available.
    return E_NOTIMPL; // Use D3DXCompileShader's built-in reflection instead
}

// D3DXCreateBuffer — creates an ID3DXBuffer of the given size via D3DCreateBlob.
inline HRESULT D3DXCreateBuffer(DWORD NumBytes, LPD3DXBUFFER* ppBuffer) {
    if (ppBuffer) *ppBuffer = nullptr;
    if (!ppBuffer || NumBytes == 0) return E_INVALIDARG;
    ID3DBlob* blob = nullptr;
    HRESULT hr = D3DCreateBlob(NumBytes, &blob);
    if (FAILED(hr)) return hr;
    *ppBuffer = new DX12ShaderBlob(blob);
    blob->Release();
    return S_OK;
}

// D3DXIMAGE_FILEFORMAT — image file format enum used by D3DXSaveSurfaceToFileW etc.
// Phase 2 TODO: replaced by DirectXTex / WIC image saving.
typedef enum _D3DXIMAGE_FILEFORMAT {
    D3DXIFF_BMP  = 0,
    D3DXIFF_JPG  = 1,
    D3DXIFF_TGA  = 2,
    D3DXIFF_PNG  = 3,
    D3DXIFF_DDS  = 4,
    D3DXIFF_PPM  = 5,
    D3DXIFF_DIB  = 6,
    D3DXIFF_HDR  = 7,
    D3DXIFF_PFM  = 8,
} D3DXIMAGE_FILEFORMAT;

// D3DXSaveSurfaceToFileW stub — saves a surface to a file.
// Phase 2 TODO: replaced by DirectXTex SaveToWICFile / SaveToDDSFile.
inline HRESULT D3DXSaveSurfaceToFileW(
    LPCWSTR                 pDestFile,
    D3DXIMAGE_FILEFORMAT    DestFormat,
    LPDIRECT3DSURFACE9      pSrcSurface,
    const PALETTEENTRY*     pSrcPalette,
    const RECT*             pSrcRect)
{
    return E_NOTIMPL;  // Phase 2 TODO: DirectXTex
}

// ── Shader compilation — D3DCompile wrapper ───────────────────────────────────

// Maps legacy DX9 shader profiles to SM5.0 equivalents.
inline const char* MapToSM5Profile(const char* profile) {
    if (!profile) return "ps_5_0";
    if (profile[0] == 'v' && profile[1] == 's') return "vs_5_0";
    if (profile[0] == 'p' && profile[1] == 's') return "ps_5_0";
    return profile;
}

inline HRESULT D3DXCompileShader(
    LPCSTR              pSrcData,
    UINT                SrcDataLen,
    const void*         pDefines,
    void*               pInclude,
    LPCSTR              pFunctionName,
    LPCSTR              pProfile,
    DWORD               Flags,
    LPD3DXBUFFER*       ppShader,
    LPD3DXBUFFER*       ppErrorMsgs,
    LPD3DXCONSTANTTABLE* ppConstantTable)
{
    if (ppShader)        *ppShader        = nullptr;
    if (ppErrorMsgs)     *ppErrorMsgs     = nullptr;
    if (ppConstantTable) *ppConstantTable = nullptr;

    // Map legacy flags to D3DCOMPILE flags
    UINT compileFlags = D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY;
    if (Flags & D3DXSHADER_DEBUG)
        compileFlags |= D3DCOMPILE_DEBUG;
    if (Flags & D3DXSHADER_SKIPVALIDATION)
        compileFlags |= D3DCOMPILE_SKIP_VALIDATION;

    const char* sm5Profile = MapToSM5Profile(pProfile);

    ID3DBlob* pCode = nullptr;
    ID3DBlob* pErrors = nullptr;
    HRESULT hr = D3DCompile(
        pSrcData, SrcDataLen,
        nullptr,                         // pSourceName
        (const D3D_SHADER_MACRO*)pDefines,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        pFunctionName,
        sm5Profile,
        compileFlags,
        0,                               // effect flags
        &pCode,
        &pErrors);

    if (pErrors && ppErrorMsgs) {
        *ppErrorMsgs = new DX12ShaderBlob(pErrors);
    }
    if (pErrors) pErrors->Release();

    if (FAILED(hr)) {
        if (pCode) pCode->Release();
        return hr;
    }

    if (ppShader && pCode) {
        *ppShader = new DX12ShaderBlob(pCode);
    }

    // Create constant table via reflection
    if (ppConstantTable && pCode) {
        DX12ConstantTable* ct = DX12ConstantTable::CreateFromBytecode(
            pCode->GetBufferPointer(), pCode->GetBufferSize());
        *ppConstantTable = ct; // may be nullptr if reflection fails, that's ok
    }

    if (pCode) pCode->Release();
    return S_OK;
}

#endif // D3DX9COMPAT_H
