# Plan: Add FFT Shader Functions (get_fft / get_fft_hz)

Bring MilkDrop3's `get_fft(pos)` and `get_fft_hz(freq)` functions to Milkwave Visualizer so FFT-based presets (equalizer visualizations) can run. This requires: (1) creating a D3D9 dynamic texture containing FFT spectrum data updated each frame, (2) binding it as `sampler_fft` in the shader pipeline, (3) defining `get_fft()` and `get_fft_hz()` HLSL functions in include.fx, and (4) parsing `FFTAttack`/`FFTDecay` preset parameters for per-preset smoothing.

---

## Phase 1: FFT Texture Infrastructure

### Step 1: Add FFT texture member and smoothed buffer to CPlugin

**File**: `Visualizer/vis_milk2/plugin.h` (around line ~780 with other texture members)

- Add `LPDIRECT3DTEXTURE9 m_lpFFTTexture = nullptr;` to CPlugin class
- Add `float m_fFFTSmoothed[MY_FFT_SAMPLES];` — smoothed mono FFT buffer (512 floats)
- Initialize both to zero/null in constructor

### Step 2: Create FFT texture at device allocation

**File**: `Visualizer/vis_milk2/plugin.cpp` — in `AllocateMyDX9Stuff()` (around line ~2220, near `m_lpVS` and `m_lpBlur` texture creation)

- Create: `GetDevice()->CreateTexture(MY_FFT_SAMPLES, 1, 1, D3DUSAGE_DYNAMIC, D3DFMT_R32F, D3DPOOL_DEFAULT, &m_lpFFTTexture, NULL);`
- `D3DFMT_R32F` = single 32-bit float channel per texel, ideal for FFT magnitudes
- Fallback: if `D3DFMT_R32F` not supported, use `D3DFMT_L16` (16-bit luminance) with value scaling — but DX9Ex should support R32F
- Log success/failure via `milkwave->LogInfo()`

### Step 3: Release FFT texture at cleanup

**File**: `Visualizer/vis_milk2/plugin.cpp` — in `CleanUpMyDX9Stuff()` (near other texture release calls)

- `SafeRelease(m_lpFFTTexture);`
- Zero out `m_fFFTSmoothed` array

### Step 4: Upload FFT data to texture each frame

**File**: `Visualizer/vis_milk2/plugin.cpp` — in `AnalyzeNewSound()` (around line ~11872, after `myfft.time_to_frequency_domain()` calls)

Steps:
1. Compute mono FFT: `mono[i] = (mysound.fSpecLeft[i] + mysound.fSpecRight[i]) * 0.5f`
2. Apply FFTAttack/FFTDecay smoothing (see Phase 3) against `m_fFFTSmoothed[]`
3. Lock the texture with `D3DLOCK_DISCARD`, write 512 floats, unlock

Reference pattern: noise texture creation at `plugin.cpp:2796` and video capture texture at `VideoCapture.cpp:717`

```cpp
if (m_lpFFTTexture) {
    D3DLOCKED_RECT r;
    if (D3D_OK == m_lpFFTTexture->LockRect(0, &r, NULL, D3DLOCK_DISCARD)) {
        float* dest = (float*)r.pBits;
        for (int i = 0; i < MY_FFT_SAMPLES; i++)
            dest[i] = m_fFFTSmoothed[i];
        m_lpFFTTexture->UnlockRect(0);
    }
}
```

---

## Phase 2: Shader Pipeline Integration

### Step 5: Add TEX_FFT to texture code enum

**File**: `Visualizer/vis_milk2/plugin.h` (line ~55)

- Change: `typedef enum { TEX_DISK, TEX_VS, TEX_FFT, TEX_BLUR0, TEX_BLUR1, ... TEX_BLUR_LAST } tex_code;`

### Step 6: Handle `sampler_fft` in CacheParams

**File**: `Visualizer/vis_milk2/plugin.cpp` — in `CShaderParams::CacheParams()` (around line ~3320, after the "main" check and before the blur checks)

Add an `else if` branch for the "fft" root name:
```cpp
else if (!wcscmp(L"fft", szRootName)) {
    m_texture_bindings[cd.RegisterIndex].texptr = g_plugin.m_lpFFTTexture;
    m_texcode[cd.RegisterIndex] = TEX_FFT;
    if (!bWrapFilterSpecified) {
        m_texture_bindings[cd.RegisterIndex].bWrap = false;  // clamp
        m_texture_bindings[cd.RegisterIndex].bBilinear = true; // linear interpolation between bins
    }
}
```

### Step 7: Bind FFT texture in ApplyShaderParams

**File**: `Visualizer/vis_milk2/milkdropfs.cpp` — in `ApplyShaderParams()` (around line ~4770, in the texture binding loop)

Existing code already handles `TEX_VS` specially. Add `TEX_FFT`:
```cpp
if (p->m_texcode[i] == TEX_VS)
    lpDevice->SetTexture(i, m_lpVS[0]);
else if (p->m_texcode[i] == TEX_FFT)
    lpDevice->SetTexture(i, m_lpFFTTexture);
else
    lpDevice->SetTexture(i, p->m_texture_bindings[i].texptr);
```

This ensures the FFT texture is always the live/current one (not a stale pointer from CacheParams time).

### Step 8: Add sampler and functions to include.fx

**File**: `Visualizer/resources/data/include.fx` (after the blur sampler declarations, around line ~200)

Add:
```hlsl
// FFT audio spectrum texture (512x1, R32F, updated each frame)
sampler2D sampler_fft;
#define texsize_fft float4(512.0, 1.0, 1.0/512.0, 1.0)

// Get FFT magnitude at normalized position [0..1] in the spectrum
// 0.0 = lowest frequency (DC), 1.0 = highest frequency (~22kHz)
float get_fft(float pos) {
    return tex2D(sampler_fft, float2(saturate(pos), 0.5)).x;
}

// Get FFT magnitude at a specific frequency in Hz
// Maps freq to normalized position: pos = freq / 22050.0
float get_fft_hz(float freq) {
    return get_fft(freq / 22050.0);
}
```

Note: Using `0.5` for the V coordinate centers the lookup on the single-row 512x1 texture.

### Step 9: Set texsize_fft constant

The `texsize_fft` is defined as a `#define` in include.fx since the texture size is fixed (512x1). No runtime constant binding is needed.

If a future preset declares `sampler sampler_fft;` explicitly with a `texsize_fft` uniform, the existing `texsize_params` mechanism in CacheParams should resolve it automatically, since it already processes `texsize_` prefixed params. Verify this works during testing.

---

## Phase 3: FFTAttack / FFTDecay Preset Parameters

### Step 10: Add FFTAttack/FFTDecay to CState

**File**: `Visualizer/vis_milk2/state.h`

- Add members: `float m_fFFTAttack = 0.5f; float m_fFFTDecay = 0.7f;` to CState class
- These are per-preset parameters, so they live in CState alongside `fDecay`, `fRating`, etc.

### Step 11: Parse FFTAttack/FFTDecay from preset files

**File**: `Visualizer/vis_milk2/state.cpp` — in the preset loading/parsing code (wherever `fDecay`, `fRating`, etc. are read from INI sections)

- Parse `FFTAttack` and `FFTDecay` from the `[preset00]` section
- Default: attack = 0.5, decay = 0.7 (matching MilkDrop3 defaults observed in presets)
- Clamp to [0.0, 1.0] range

### Step 12: Apply smoothing in AnalyzeNewSound

**File**: `Visualizer/vis_milk2/plugin.cpp` — in `AnalyzeNewSound()` (Step 4 location)

After computing mono FFT, apply per-preset smoothing using `m_pState->m_fFFTAttack` and `m_pState->m_fFFTDecay`:

```cpp
float attack = m_pState ? m_pState->m_fFFTAttack : 0.5f;
float decay  = m_pState ? m_pState->m_fFFTDecay  : 0.7f;

for (int i = 0; i < MY_FFT_SAMPLES; i++) {
    float mono = (mysound.fSpecLeft[i] + mysound.fSpecRight[i]) * 0.5f;
    if (mono > m_fFFTSmoothed[i])
        m_fFFTSmoothed[i] += (mono - m_fFFTSmoothed[i]) * attack;
    else
        m_fFFTSmoothed[i] += (mono - m_fFFTSmoothed[i]) * (1.0f - decay);
}
```

- Attack: higher value = faster response to rising FFT (0.5 = moderate responsiveness)
- Decay: higher value = slower falloff (0.7 = moderately slow decay)

---

## Phase 4: Testing & Verification

### Step 13: Copy FFT presets to Milkwave presets folder

Copy the 6 FFT presets from `c:\Portable\MilkDrop3\3.33\FFT\` to the Milkwave presets directory for testing.

### Step 14: Build and test

- Build Visualizer (Debug) using the existing build task
- Launch with debug logging enabled
- Load each FFT preset and verify equalizer visuals appear
- Compare against the reference screenshot (rainbow equalizer bars)

---

## Relevant Files

| Component | File | Location |
|-----------|------|----------|
| Texture member & smoothed buffer | `Visualizer/vis_milk2/plugin.h` | ~line 780 |
| TEX_FFT enum value | `Visualizer/vis_milk2/plugin.h` | line 55 |
| Texture create/release | `Visualizer/vis_milk2/plugin.cpp` | `AllocateMyDX9Stuff` / `CleanUpMyDX9Stuff` |
| Sampler binding (CacheParams) | `Visualizer/vis_milk2/plugin.cpp` | ~line 3320 |
| FFT upload (AnalyzeNewSound) | `Visualizer/vis_milk2/plugin.cpp` | ~line 11872 |
| ApplyShaderParams binding | `Visualizer/vis_milk2/milkdropfs.cpp` | ~line 4770 |
| HLSL sampler & functions | `Visualizer/resources/data/include.fx` | ~line 200 |
| FFTAttack/FFTDecay members | `Visualizer/vis_milk2/state.h` | CState class |
| FFTAttack/FFTDecay parsing | `Visualizer/vis_milk2/state.cpp` | preset loading |

---

## Decisions

- **Mono FFT** (L+R averaged) — matches MilkDrop3 preset usage where `get_fft()` has no channel parameter
- **HLSL shaders only** — `get_fft()`/`get_fft_hz()` only in pixel shaders (not expression evaluator per_frame/per_pixel)
- **D3DFMT_R32F** — full float precision for FFT magnitudes; DX9Ex supports this
- **texsize_fft as #define** — simpler than runtime constant since texture size is fixed (512x1)
- **FFTAttack/FFTDecay included** — per-preset smoothing implemented in this plan

---

## Further Considerations

1. **D3DFMT_R32F availability**: If R32F is not supported on some GPUs, fall back to `D3DFMT_A8R8G8B8` and encode float as 4 bytes, then decode in shader. Recommendation: start with R32F and add fallback only if needed.
2. **Preset blending during transitions**: During preset transitions, both `m_pOldState` and `m_pState` are active. The FFTAttack/FFTDecay should use the current (new) state's values. Only one FFT texture exists, so both old and new shaders will see the same FFT data — this is acceptable behavior.
3. **get_fft in comp shaders**: The presets use `get_fft()` in warp shaders. It should also work in comp shaders automatically since both use the same include.fx and texture binding pipeline. Verify during testing.
