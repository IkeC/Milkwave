# Code Issues

Remaining build warnings as of the last clean Release rebuild.
**Build status: 0 errors, 63 warnings.**

All warnings are in external SDK headers or in `audio/audiobuf.cpp`.
No warnings exist in the main visualizer source files.

---

## Build Warnings

### External SDK headers — `dxgitype.h` (27 × C4005)

| Warning | Description | Action |
|---------|-------------|--------|
| **C4005** | DXGI/D3D10/D3D11 error and status macros redefined — the DirectX SDK (June 2010) `dxgitype.h` defines these constants first, then the Windows SDK redefines them | External header conflict; not fixable in project code. Could be suppressed with `#pragma warning(disable: 4005)` around the DX SDK includes, but harmless in practice |

Affected macros (all in `DirectX SDK (June 2010)/Include/dxgitype.h` lines 12–35):
`DXGI_STATUS_*` (7), `DXGI_ERROR_*` (14), `D3D10_ERROR_*` (2), `D3D11_ERROR_*` (4).

---

### `audio/audiobuf.cpp`

| Warning | Line(s) | Description | Action |
|---------|---------|-------------|--------|
| **C4101** | 102, 104, 105 | `BlockOffset`, `LeftSample8`, `RightSample8` declared but never used — leftover from a previous implementation, replaced by `blockOffset`, `sampleLeft`, `sampleRight` | Remove the three dead declarations |
| **C4018** | 143 | `inputIndex >= nNumFramesToRead`: `inputIndex` is `int`, `nNumFramesToRead` is `UINT32` — signed/unsigned comparison | Change `int inputIndex` to `UINT32 inputIndex`, or cast: `(UINT32)inputIndex >= nNumFramesToRead` |
| **C4244** | 175, 176 | `pcmLeftLpb[...] = sumLeft / downsampleRatio * milkwave_amp_left` — result is `float` (due to `milkwave_amp_*` multiplier) assigned to `uint8_t` array, possible loss of data | Add explicit cast: `(uint8_t)(sumLeft / downsampleRatio * milkwave_amp_left)` |

---

### Linker — LNK4075

| Warning | Description | Action |
|---------|-------------|--------|
| **LNK4075** | `audiobuf.obj` was compiled with `/ZI` (Edit-and-Continue debug info) but the linker is using `/OPT:ICF` (identical COMDAT folding), which are incompatible | In Release config, ensure `audiobuf.cpp` compiles with `/Zi` not `/ZI`, or verify the Release compile flags for the audio source group in `plugin.vcxproj` |
