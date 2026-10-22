# Milkwave preset format and engine facts

## Minimal file layout

```ini
MILKDROP_DESKTOP=0
MILKDROP_PRESET_VERSION=201
PSVERSION=4
PSVERSION_WARP=4
PSVERSION_COMP=4
// <author> - <name> / https://www.shadertoy.com/view/<ID>
// License: <license>
// Transpiled to HLSL using Milkwave
comp_1=<hlsl line 1>
comp_2=<hlsl line 2>
```

- Comments are stripped by the engine wrapper before compiling, so header comments are safe.
- The file must be **UTF-8 without BOM**. A BOM stops `MILKDROP_PRESET_VERSION` from being
  parsed: the preset is treated as legacy (`PSVERSION=0`), the comp shader is disabled and the
  screen stays black with no error anywhere.
- `PSVERSION=4` means `MD2_PS_3_0`, compiled as `ps_3_0`. `PSVERSION=5` would be `ps_4_0`,
  which the DirectX 9 Visualizer cannot run.
- Code is read back by number, sequentially: `comp_1`, `comp_2`, … up to the first missing
  number. Adding or removing a line therefore requires renumbering everything after it — use
  `compcode.ps1 -Mode get -Milk <preset> -Txt <file>` / `-Mode set`.
- A minimal working preset is just the version headers plus the `comp_N=` lines; see
  `Visualizer\resources\presets\Milkwave\Shader\` for shipped examples.

## Uniforms available inside `comp_N` (from `include.fx`)

`time`, `fps`, `frame`, `progress`, `bass`/`mid`/`treb`/`vol`, `bass_att…`, `bass_smooth…`,
`aspect` (float4; `.xy` aspect correction, `.zw` inverse), `texsize`, `mouse`
(`.x/.y` in 0..1, `.z` down, `.w` last y), `ret` (float3 declared by the wrapper),
`sampler_main` + `GetPixel(uv)`, blur samplers, `sampler_noise_*` (`iChannel0` maps to
`sampler_noise_lq`), FFT helpers (`get_fft`), `q1..q32`, `rot_s1..`.

The wrapper also defines macros that shadow common identifiers: `mid`, `treb`, `bass`, `vol`,
`fps`, `frame`, `progress`, `aspect`, `texsize`, `mouse*`, `q1..q32`, and the comp defines `uv`,
`uv_orig`, `ang`, `rad`, `hue_shader`. Never use those names for locals or parameters.

Audio band vectors (see [audio-reactivity.md](./audio-reactivity.md) for the full contract):

| Uniform group | Contents |
| --- | --- |
| `bass`/`mid`/`treb`/`vol` (`_c3`) | immediate level, relative to the track average |
| `bass_att`/`mid_att`/`treb_att`/`vol_att` (`_c4`) | fast attack, relative |
| `bass_smooth`/`mid_smooth`/`treb_smooth`/`vol_smooth` (`_c15`) | about a 1-second average, relative |
| `vis_intensity`/`vis_shift`/`vis_version` (`_c16`) | Remote Settings tab, IPC `VAR_INTENSITY=`, `VAR_SHIFT=`, `VAR_VERSION=` |

All band levels are relative to the long-term average, so they hover around `1.0`. The `vol_*`
component is currently filled with `0.3333 * treb` by the engine — do not treat it as volume.

Verified engine behaviour:

- `aspect` is roughly `(1.0, …)` — **not** the window aspect. Shaders that need the real ratio
  must compute `texsize.x / texsize.y`.
- `texsize` holds the current render-target size in pixels.
- `sampler_main` is **not** a readable previous frame inside a comp shader; temporal blends
  (`mix(GetPixel(uv), x, a)`) read black.
- The wrapper strips comments, appends the output line, and compiles with flags `0x10000`.

## Verification gates

Deliver only when all of these pass:

1. `shadercheck` prints `COMPILE OK`.
2. `stats.ps1` reports a luminance **standard deviation of roughly 15 or more**. The 64×24 ASCII
   map alone cannot detect a flat image.
3. Two captures a few seconds apart have **different MD5s** (the frame animates). Compare against
   a known-good preset in the same Visualizer session if unsure.
4. `(Get-Process MilkwaveVisualizer).Responding` is `True` after loading the preset.
5. First 4 bytes of the file are `4D 49 4C 4B` (`MILK`).
6. Header contains author, Shadertoy URL and license.

Notes on capture analysis: mean RGB in the tens and/or a recognizable ASCII pattern means a
plausible frame; nearly uniform `(2,2,2)` or all-dark is degenerate. The top ASCII rows can
contain leftover UI text (preset name) — ignore them.

## Iterating against the running app

- Named pipe: `\\.\pipe\Milkwave_<pid>`, UTF-16LE plus terminating NUL, one `Write` per message.
- Commands: `PRESET=<absolute path>`, `CAPTURE`, `QUICKSAVE`, `CLEARPRESET`, `STATE`,
  `PRECOMPILE_CACHE`.
- `CAPTURE` writes `Release\capture\<yyyyMMdd-HHmmss>-<presetname>.png`.
- Compile errors are **not** written to `Release\log\*.visualizer.log` — only on-screen
  notifications. Always use `shadercheck`.
- Do not run `PRECOMPILE_CACHE` unless you accept that it truncates `Release\cache\compiled.txt`
  and that `precompile.txt` must be backed up and restored.
