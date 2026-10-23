# HLSL (ps_3_0 legacy compiler) pitfalls

The engine compiles with the DirectX 9 `ps_3_0` compiler. Most failures are mechanical and
repeat across shaders; work this list top to bottom before debugging logic.

## Mechanical fix pass

1. **BOM.** Rewrite the file with `new UTF8Encoding($false)`.
2. **Global `const` → `static const`**, and `static` on every other top-level declaration
   (`fixup.ps1` does this). Unknown globals become unset uniforms with the value 0.
3. **Local `const` → plain local** (`const float R = 2.8;` → `float R = 2.8;`).
4. **Renamed parameters.** Functions whose parameters were renamed declare `uv_conv`, but their
   bodies may still use the `uv` macro. Replace those uses with `uv_conv`. Grep for stray
   `_conv` identifiers produced by the converter's suffix renamer (`g_time_conv`,
   `rotxang_conv`) and re-unify declaration and uses.
5. **Matrix products.** GLSL `M * v` → `mul(transpose(M), v)`; `v * M` and `v *= M` (including
   swizzles such as `q.xy *= rot(a)`) → `v = mul(v, transpose(M))`. Inline `matN(...)` literals
   are auto-fixed; named variables and function results are not.
6. **Scalar splats.** `float2(x)` → `float2(x, x)`, `float3(x)` → `float3(x, x, x)`. A vector
   argument (`float2(someFloat2)`) is valid and needs no change.
7. **Corrupted aspect / `iResolution`.** `uv.x *= uv.x/uv.y;` came from
   `uv.x *= iResolution.x/iResolution.y;` → use `uv.x *= aspect.x/aspect.y;` or delete it when
   the injected CONV block already corrects. `fragCoord.xy / uv.xy` → `fragCoord.xy /
   texsize.xy`. Keep the unit space intact: expressions that mix `fragCoord` (pixels) with
   `iResolution` need `fragCoord.xy * texsize.xy`. Getting this wrong yields a structureless
   image that still compiles.
8. **`length`/`distance`** → `lenv()` helpers (float2, float3 **and float4** overloads):
   ```hlsl
   float lenv(float2 v) { return sqrt(max(dot(v, v), 0.000001)); }
   ```
9. **`continue`** → wrap the remainder of the loop body in `if (...)`.
10. **`break`** → `if (cond) i = <limit>;` **plus `else { … }`** around the rest of the body.
    The counter rewrite does not stop the remaining body from running and would corrupt
    distance accumulators.

## Error code → fix

| Error | Cause | Fix |
| --- | --- | --- |
| `X3011: initial value must be a literal expression` | non-literal global or local `const` initializer | literal default on the global, real assignment at the top of `shader_body`; drop `const` locally |
| `X3014: incorrect number of arguments to numeric-type constructor` | scalar splat | `float2(x, x)` |
| `X3013: 'mul': intrinsic function does not take 3 parameters` | 3-factor product | nest: `mul(mul(a, b), c)` |
| `X3020: type mismatch` | swizzle `*=` with a matrix | `q.xy = mul(q.xy, transpose(m));` |
| `X3000: unexpected token 'continue'` | `continue` unsupported | `if (...)` wrapper |
| `X3000: unexpected token '.'` / `redefinition of '_c2'` | name collides with a wrapper macro | rename the local/parameter (`midpt`, `tm`, …) |
| `X3000: unexpected token 'line'` / `'point'` | HLSL reserved word | rename (`lineSeg`) |
| `X3500` / `X4001: asymetric returns from if statements` | early `return` inside `if` | single `return` at the end via a local |
| `X4001: division by zero` | `length()`/`distance()`/`normalize()` rewritten to rsqrt form | `lenv()` (error is reported at the line defining the *input*) |
| `X3526: can't use gradient instructions in loops with break` | texture sampling inside a loop | replace with a hash (`frac(sin(dot(...)) * 43758.5453)`) or small value noise |
| `X3017: cannot convert from 'const int' to 'float3'` | runtime-filled array, or `static` struct global | per-element scalars or an `if`/`else` lookup helper; push structs into function parameters |
| `X3037: constructors only defined for numeric base types` / `X1500` | struct constructor, or empty-argument macro | explicit field assignment; rewrite the macro with arguments |
| `X3019: float expected` | vector used as an `if` condition | compare components (`if (c.x != 0. \|\| …)`) |
| `X4000: variable used without having been completely initialized` | `const` local | drop `const` |
| `X3511: loop does not appear to terminate in a timely manner` | uniform-derived loop start applied to a small loop | keep `int i = 0` for small loops |
| `X3067: 'lenv': ambiguous function call` | missing overload | add the `float4` overload |
| `X4004` (register limit) | too much per-pixel work | lower march steps / taps, tighten tolerance |

## Converter blind spots to review every time

- Review every `// CONV:` comment the converter emitted — each one marks a guess.
- Component-wise compound assignments survive the conversion: `v.xy *= v.z;` → `v.xy = v.xy * v.z;`.
- The same matrix line can be emitted twice; a duplicated rotation moves the camera away from the
  subject. Delete the duplicate.
- **`matN(name, …) * vec` is left as `*`** and fails with `X3020: type mismatch` — the rewrite only
  covers `v * M` and inline literals. Complex products are cheapest written out by hand:
  `z = mat2(z, -z.y, z.x)*z + cc;` → `z = float2(z.x*z.x - z.y*z.y, 2.*z.x*z.y) + cc;`
  (`Mandelbrot Pattern Decoration`; the author's own comment carried the expanded form).
- Loop counters are emitted as `for (int k=min(frame,0); …)` (a trick to keep the loop un-unrolled);
  restore `int k = 0` for readability — behaviour is the same at `frame >= 0`.
- **`iResolution.y` outside the coordinate setup is rewritten to `uv.y`**, which is the 0..1 texture
  coordinate, not the height in pixels: `float k = 5./iResolution.y;` became `5./uv.y`
  (`Golden Electric Spiral`) and made the edge width depend on the pixel position. Restore
  `texsize.y`.
- `cosh` / `sinh` / `tanh` are built into the ps_3_0 compiler — no hand-written `(exp(x)+exp(-x))/2`
  needed.
- Corner-case GLSL that needs manual rewriting: struct constructors, array literals, multiple
  returns, ternary side effects, and overloads that still reference `gl_FragCoord`.

## Tone map: bright cores turning black

An accumulator (raymarch colour, volumetric density) that picks up a negative or NaN value renders
as **black cores with bright rims** — the brightest part of the image turns black. Wrap the final
accumulation so this cannot reach the output:

```hlsl
O = tanh(max(O, 0.) / 2e2);   // tanh squashes huge positives, max() kills negative/NaN input
```

- `tanh` alone is not enough: `tanh(-1e6)` is `-1`, which the engine draws as black. Hence `max(..., 0.)`.
- Typical sources: `exp(...)/v` with `v` negative or ~0 (a *blend weight* that went out of
  `0..1`), overflow to `inf`, `inf - inf` → NaN, and `1/length(x)` at the centre of the image.
- Shipped precedents: `diatribes + IkeC - Night Cloud Dance`
  (`o = tanh(float4(1,fac*2,6,0)*o/1e1/length(u-.65));`) and `OldEclipse + IkeC - Unimagined`
  (`v` turned negative through the audio-modulated `mix` weight; fixed with `saturate()` plus
  `tanh(max(O, 0.)/2e2)`).
- Verify with a capture, not the compile: force the failing state (e.g. `VAR_SHIFT=-0.2`, or the
  quiet end of the audio range) and compare the near-black pixel share before/after — Unimagined
  went from 5.3 % to 0.1 %.
- Keep blend/mix weights inside `0..1` (`saturate`) whenever a modulation offset is added to them;
  extrapolating a mix is how a density acquires the wrong sign.

## Multi-pass / feedback shaders (`Psychedelic tube`, `sdcGRX`)

The `Image` pass is usually a blit of `Buffer A`; convert only the buffer's code. Detect real
feedback in the browser with

```js
const p = gShaderToy.mEffect.mPasses[1];
p.mInputs.map(x => x && x.mInfo && [x.mInfo.mType, String(x.mInfo.mID)])
```

`type == "buffer"` and an `mID` equal to the pass's own `mOutputs[0]` (e.g. `4dXGR8`) means the
pass samples its previous frame; `type == "musicstream"` is the FFT texture; `type == "texture"`
is an image whose id you cannot resolve without the API — substitute `sampler_noise_lq`.

Port rules used for `sdcGRX`:

- Drop the self-feedback `mix(col, prevFrame, …)` line and keep the rest of the shader intact;
  keeping the trail is impossible, so say so in the header comment.
- `FFT(a)` (Shadertoy audio texture) becomes a **constant** in the base look, e.g. `0.5`, so the
  geometry never depends on the audio; wire real bands in the version-guarded reaction block.
- The image overlay `col += 0.2*tex2D(iChannel0, …)` must drop to about `0.05`: noise textures are
  much brighter than the dark photo/texture the original used, and 0.2 turns the whole frame into
  coloured grain instead of dark background with sparkle.

## Singular SDF at the camera start (`Blue Red Shiny 2`)

GLSL often leaves the march accumulators uninitialized (`float i,s,d;`, then `d += s = map(p)`),
and the first sample lands exactly on the camera position `ro = P(T)`. When `map()` divides by a
position term (`sin(p*cos(p.yzx/3e1)*4.)/(p*4.)`), that sample is `0/0`: the original survives only
because `d` happens to be non-zero garbage, while a port with `d = 0.0` produces NaN in the first
iteration, `d` runs away negative and every pixel renders flat (black, or one constant colour).

```hlsl
float s = 0., d = 0.05;                       // start a hair along the ray, off the singular point
...
float tex = abs(lenv(sin(p*cos(p.yzx/3e1)*4.)/(p*4. + 1e-4)));   // and guard the divisor
```

## Control flow the legacy compiler rejects

These all compile with `flags=0x0` and fail with the engine's `flags=0x10000`:

| Code | Error | Rewrite |
| --- | --- | --- |
| `break;` inside a `for` | `X3000: syntax error: unexpected token 'break'` | `if (hit) { outv = …; i = steps; } else { …accumulate… }` |
| early `return` inside a loop body | `X3500: asymetric returns from if statements` | single `return outv;` after the loop, result kept in a local |
| `if (a.x < b.x) return a; return b;` in a helper | same `X3500` risk | `return a.x < b.x ? a : b;` |
| `float rad = …;` (any local named like a wrapper macro) | `X3000: syntax error: unexpected token '.'` | rename (`trad`, `radv`, …) |
| early `return` from the GLSL `mainImage` body (e.g. cinema-bar clip) | shader_body cannot return — the wrapper appends the output line | put the work in `if (inside) { … }` and leave the colour black outside (`Dark Transit`) |

Note the two warnings that mean the same thing in disguise: `X3083: Truncating N-vector to size 1`
on a helper like `float mod_conv(float2 x, float2 y)` — declare the vector return type
(`float2 mod_conv(...)`).

## Bisecting a wrong frame

Compiles-but-wrong is much faster to solve by rendering intermediate accumulators than by reading
code. Output the suspects as channels, load, capture, and read `stats.ps1` plus the image:

```hlsl
o = float4(saturate(d*.02), saturate(s), saturate(o.r*.001), 1.);   // distance / step / glow
```

`(128,0,0)`-style flat colours identify the culprit immediately (e.g. red 0 = the marched distance
never became positive while blue 255 = the glow accumulator was already saturated). Note `stats.ps1`
averages all three channels, so decode the image before trusting a "mean 85" report.

## Performance



- **Hard rule: 60 seconds per compile.** Run the checker under a timeout and cancel it when the
  limit is hit; do not wait for a slow compile and do not background it to "see if it finishes".
  Over 60 s means the preset is not shippable (the engine would freeze for the same time on
  first load), so record the reason and skip.
  ```powershell
  $exe = 'C:\Source\Milkwave\.local\shader-tools\check\bin\Debug\net8.0\shadercheck.exe'
  $p = Start-Process $exe -ArgumentList @("`"<preset>`"") -PassThru -NoNewWindow `
         -RedirectStandardOutput "$env:TEMP\sc.out"
  if (-not $p.WaitForExit(60000)) { Stop-Process -Id $p.Id -Force; 'TIMEOUT (>60 s) -> skip' }
  else { Get-Content "$env:TEMP\sc.out" }
  ```
- The engine compiles with `D3DXSHADER_SKIPOPTIMIZATION` (`m_dwShaderFlags = (1 << 16)`,
  `plugin_config.cpp`). This path is far slower on big shaders than an optimized compile: a
  double-raymarch shader that reached `COMPILE OK` in ~1 s with flags 0 needed **463 s** with
  `0x10000` for the same 47636-byte result. The checker prints both attempts — trust the
  `flags=0x10000` line, never the `flags=0x0` fallback, and time the run.
- Unoptimized compile time is **superlinear in the number of inlined `map()` copies**. Measured
  on one shader (`XcVyWD` Cave Diving): one `map()` call 1 s / 2.9 KB, two marches 1 s / 5.3 KB,
  march + `getNormal` 4 s / 11.5 KB, march + two `getNormal` 18 s / 19.3 KB, the full shader
  (2 marches, 2 normals, AO, bump map, 2 `doColor`) **463 s / 47.6 KB**. Every part is cheap on
  its own. Neither a single-return `map()`, a hoisted refraction march, a branchless select, nor
  a 40 % cut in map call sites changed the order of magnitude, so treat "each part compiles in
  seconds" as no evidence that the whole preset will, and time the full file early.
- An early `return` inside `map()` (`if (G) { ID = 1.; return tunel; }` plus code after it) is
  the `X3500 asymetric returns` pattern and is what makes the legacy compiler grind. Restructure
  to one return before touching anything else.
- Heavy loops are superlinear in compile time; a big SDF inlined into the normal and the AO can
  take minutes. Prefer derivative normals and a dynamic (audio- or frame-driven) march bound.
- **Anti-unroll trick:** start big loops with a uniform, e.g. `for (int i = min(frame,0); i < 200; i++)`.
  This took one preset from a 5-minute hang to 3 s. Multiplying by a literal `0` does not work
  (the compiler folds it), and the trick backfires with `X3511` on small loops — apply it only
  when `iterations × body size` is large.
- `[loop]` attributes are rejected by the legacy compiler.
- Texture sampling inside loops is both a compile risk and a runtime cost; replace with procedural
  noise.

## Tooling pitfalls

- Do not build ad-hoc PowerShell line patchers that index nested arrays: PowerShell flattens
  `@(@("a","b"))` into `("a","b")`, so `$s[0]` becomes the first *character* and a helper silently
  rewrites single characters across the file. Use exact-string replacements in an editor instead.
- Do not edit `.milk` `comp_N=` lines by hand; extract/apply with `compcode.ps1` so numbering
  stays sequential.
- Compile errors are not in the Visualizer log; only `shadercheck` shows them.
- `PRESET=` loads asynchronously: a `CAPTURE` sent right after is handled *before* the load
  finishes (the engine compiles the comp shader on the render thread, 10..20 s for a heavy one),
  and the capture then shows the previous preset. Wait, then confirm the capture filename matches.
  The Visualizer also auto-advances presets on its own, so a later capture may be a different shader.
- `stats.ps1` averages R, G and B into one `mean`; a flat blue frame reports `mean 85`. Read the
  image (or the per-channel spread) before concluding anything from it.
