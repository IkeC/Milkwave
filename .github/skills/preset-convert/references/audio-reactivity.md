# Audio reactivity for converted presets

How the shipped presets in `Visualizer\resources\presets\Milkwave\Shader\` (all 40) implement
audio reaction. Follow this contract; it is what the Remote's controls and the engine expect.

## Contract: version / intensity / shift

The engine fills one uniform vector from the Remote's Settings tab
(`milkdropfs.cpp`: `_c16 = (m_VisIntensity, m_VisShift, m_VisVersion)`), so every preset sees
three live values (`include.fx`):

| Variable | Meaning | Default | IPC message |
| --- | --- | --- | --- |
| `vis_intensity` | user's strength multiplier (0..~3) | `1.0` | `VAR_INTENSITY=` |
| `vis_shift` | user's bias/offset, negative or positive | `0.0` | `VAR_SHIFT=` |
| `vis_version` | selects the reaction style | `1` | `VAR_VERSION=` |

The version contract used by every shipped preset:

| `vis_version` | Band variable | Character |
| --- | --- | --- |
| `1` | `bass_smooth`, `mid_smooth`, `treb_smooth` (about a 1-second average) | calm, smooth breathing |
| `2` | `bass_att`, `mid_att`, `treb_att` (fast attack) | snappy, beat-accurate |
| `>= 3` (`(int)vis_version > 2`) | none | **completely static**, no audio term at all |

Version 3+ still honours `vis_intensity` and `vis_shift`, so the user keeps those two knobs.
Every branch therefore ends in `… * vis_intensity + vis_shift`, including the static one, which
uses a neutral reference value (typically `1*vis_intensity+vis_shift` for multiplicative
parameters, `0*vis_intensity+vis_shift` for additive ones).

## Band value semantics

`bass_att`/`mid_att`/`treb_att` and the `*_smooth` variants are levels **relative to the
track's long-term average** (`plugin.cpp`), so they hover around `1.0` and swing roughly
`0.5 .. 2`. Normalise with `smoothstep`:

- `smoothstep(0,2,x)` — the default; maps the useful swing to `0..1`.
- `smoothstep(0,3,x)` — softer/rarer peaks (`Undulating Urchin`).
- `smoothstep(0.5,3,x)` — ignores quiet passages (`a basic thing`).
- `clamp(x, 0.3, 1.7)` — linear, keeps the sign of the movement (`Shadow Party`).
- `clamp(x, 0.97, 1.03)` — very subtle shimmer (`Shadow Party` fallback branch).

Do **not** trust `vol_smooth` / `vol_att`: the engine currently fills that component with
`0.3333f * treb` (a comma-operator slip in the three `SetVector` calls), so it is a third of
the treble, not a volume level. Use bass/mid/treble. `Neon Chill` gets away with it only
because it feeds a minor hue channel.

## Canonical code shapes

Inline ternary, one expression per parameter (most common, 30+ presets):

```hlsl
#ifdef bass_smooth
  float fac = (int)vis_version > 2
    ? 1*vis_intensity+vis_shift
    : (0.8 + smoothstep(0,2,(int)vis_version == 1 ? bass_smooth : bass_att)*0.5)*vis_intensity+vis_shift;
#else
  float fac = 0.95 + 0.1*smoothstep(0,2,bass_att);
#endif
...
finalColor.r *= fac;
```

Explicit branch chain, when the versions need different expressions or several parameters
(`Neon Chill`, `Apollonian Structure`, `glooper`, `Fractal Land`, `Vibe Ride`):

```hlsl
float shift1 = 0;
#ifdef bass_smooth
  if (vis_version == 1)      shift1 = (-0.5+smoothstep(0,2,bass_smooth))*vis_intensity+vis_shift;
  else if (vis_version == 2) shift1 = 0.4*smoothstep(0,2,bass_att)*vis_intensity+vis_shift;
  else                       shift1 = (-1+vis_intensity)+vis_shift;
#else
  shift1 = 0.4*smoothstep(0,2,bass_att);
#endif
```

Rules for both shapes:

- Declare the target variable **before** the `#ifdef` block; only assign inside it.
- The `#else` branch is the portability path for other MilkDrop-based visualizers where
  `bass_smooth`/`vis_*` do not exist. It may only use `bass_att`/`mid_att`/`treb_att` and must
  stay subtle (amplitudes of `0.05 .. 0.2`).
- If you use `vis_*` anywhere outside a guarded block, wrap that use in
  `#ifndef vis_intensity` / `#define vis_intensity 1` / `#endif` (see `Manual.md`, and
  `glooper` for a `#ifdef vis_intensity` guard).
- Cast before comparing: `(int)vis_version == 1`.

## What to modulate

Prefer parameters that already exist in the shader, so the preset keeps its identity:

| Target kind | Good choice | Shipped example |
| --- | --- | --- |
| Shape / size | SDF blend offset, object radius, `sunsize`, `edge`, scale of a primitive | `CineShader Lava` (`d1 -= …`), `Fractal Land` (`sunsize -= …`), `Raymarched Hexagonal Truchet` (`edge *= …`) |
| Position | depth offset, `pos.z`, noise offset | `Tunnel Cylinders` (`pos.z += …`), `Aurora Radar` (`n += …`) |
| Colour | channel gain, saturation/brightness, per-band hue slice | `Heartfelt I/II` (`col + 0.05*(…-1)`), `Breeze` (saturation/brightness), `Neon Chill` (four band-weighted colour lanes) |
| Zoom | `uv *= 1 ± k` — last resort, feels cheap if overdone | `fractal pyramid` (`uv *= 1-0.25*…`), `cube go spinny` |

Amplitudes actually used for the reactive (version 2) branch: `0.1 .. 0.5` for a normalised
`0..1` parameter, `1.4 .. 1.5` only where it operates in world/UV units (`Fractal Land`,
`Acidic`). Start around `0.2` of the parameter's natural range for a shape/colour term and
`0.05 .. 0.1` for a zoom factor. Version 1 must not be stronger than version 2.

**Version 2 is the usual overshoot.** Presets built with `aRng = 0.45` read as a jump instead
of an accent — a full hue rotation on every beat turns the whole scene neon. Keep the reactive
branch at most about `1.3 .. 1.4x` the calm range (`aRng` `0.26` : `0.34`), cap colour/hue
swings at about `1.5 rad` (~85°), and prefer `1.2 .. 1.3x` for camera/zoom terms. Milder v2 is
safer: the user can raise `vis_intensity`, but an over-driven v2 cannot be dialled back.

Do not add or remove geometry, toggle objects, or change the composition; and never make the
*base* look depend on the audio level (silence must still show the full preset).

Optional: offer the calm and the punchy variant as two preset files if the shader supports a
striking and a subtle reaction, so the user can pick. Keep the earlier file.

## Shared-helper style (the `Milkwave\42` house style)

The 42 presets factor the reaction into one deviation value and derive everything from it. Use
this when a preset needs **two** reaction points (one geometric, one colour) so they stay in
sync:

```hlsl
// AUDIO: 1 = calm (bass_smooth), 2 = more reactive (bass_att), 3+ = static - see the remote's Version tooltip
#ifdef bass_smooth
  float aRng = (int)vis_version == 1 ? 0.26 : 0.34;
  float aFac = (int)vis_version > 2 ? 1*vis_intensity+vis_shift
    : (1.0 - aRng*0.5 + smoothstep(0,2,(int)vis_version == 1 ? bass_smooth : bass_att)*aRng) * vis_intensity + vis_shift;
#else
  float aFac = (1.0 - 0.15) + smoothstep(0,2,bass_att)*0.30;
#endif
float aDev = aFac - 1.0;   // 0 at the track average, +loud, -quiet
float aBe  = aDev * 4.0, aCol = (int)vis_version == 1 ? 0.7 : 1.1;   // beat scale, colour reaction scale

fragCoord.x += aDev * 0.15;                                   // AUDIO: geometry, e.g. a pan
ret = lerp(ret, ret.zyx, saturate(aBe) * 0.18 * aCol);        // AUDIO: colour, e.g. a hue swing
```

- `aDev` is signed and zero at the average, so the preset's base look never depends on the
  audio level, and `vis_intensity`/`vis_shift` reach both reaction points through it.
- `aRng` is the version role: `0.26` = calm for version 1, `0.34` = reactive for version 2
  (the older `0.45` reads too strong). Never give version 1 the bigger range.
- `aCol` scales only the colour term (`0.7` calm, `1.1` reactive); `aBe` saturates at peaks, so
  `saturate(aBe)` keeps the tint bounded.
- `aDev`/`aBe`/`aCol` are declared **outside** the `#ifdef`; only `aFac` lives inside, so the
  rest of the shader also compiles for other MilkDrop variants.

## Pitfalls seen in the shipped presets

- **A second audio block without the version guard** (`Raymarched Hexagonal Truchet`, second
  `#ifdef`) still reacts at version 3+. Every block needs the `> 2` static branch.
- **`clamp(0,2,bass_smooth)`** — wrong argument order (clamps the literal 0). Correct form is
  `clamp(bass_smooth,0,2)`. Present in `Astral Jelly Journey` (x2), `Vibe Ride`, `Fractal Land`.
- **`smoothstep(bass_att,0,2)`** — swapped edge arguments, degenerate ramp. Correct form is
  `smoothstep(0,2,bass_att)`. Present in `Phantom Star for CineShader` (x2).
- **Never let an audio term flip a sign.** Adding `aDev` to a *blend weight* extrapolates the
  mix outside `0..1` (`lerp(a, b, 0.5-.5*cos(t) + aDev*0.25)`). If that value ends up as a
  divisor or a density — `Unimagined` accumulates `exp(...)/v` — a negative `v` makes the whole
  accumulator negative and the tone map turns the *brightest* cores black (bright rims around
  black blobs). `saturate()` the weight, or clamp the derived value with `max(v, eps)`.
- Do not stack the same band on several sensitive parameters; one or two reaction points read
  better than five.
- Do not use `_smooth` bands in the version 2 branch or `_att` in the version 1 branch — that
  inverts the intended character.

## Verification

1. Structural check (deterministic, ignore `//` comment text): every *code* line containing
   `bass`, `mid_`, `treb` or `vol_` must sit inside an `#ifdef bass_smooth … #endif` block, and
   every such block must contain a `(int)vis_version > 2` or `else` static branch.
2. Behavioural check with the running Visualizer, **without live music** (see the next section):
   ```powershell
   powershell -File C:\Source\Milkwave\.local\shader-tools\audio-verify.ps1 `
     -Preset "<abs path>" -Version 1 -Bands '0,1,1;2,1,1'
   ```
   Repeat with `-Version 2` and `-Version 3`. Expected: versions 1 and 2 give different MD5s
   per band setting (and v2's spread is larger than v1's), version 3 gives one MD5 for every
   setting. Identical arguments must reproduce the same MD5 in a later session.
3. Keep the numbers you measured in the delivery notes, so the next conversion is compared
   against the same base.

## Reproducible verification (no live audio)

Live music is not a test signal: the band values depend on the track's long-term average, on
what else is playing and on the playback position, so consecutive runs are not comparable and
"it reacted" cannot be reproduced later. Use the injected-band harness instead.

### The pieces

| Mechanism | What it does |
| --- | --- |
| `AUDIO_TEST=bass,mid,treb[,notify][,wave]` (named pipe) | Engine test hook: replaces the analysed bands with fixed **relative** levels (`1.0` = track average) before they reach the shader. Silent — nothing is played. `AUDIO_TEST=off` releases it. One value broadcasts to all three bands. The optional `notify` flag shows the active levels as a notification; `wave` also synthesises waveform samples and a band-shaped `get_fft()` spectrum for wave/FFT presets. Omit both for byte-identical shader capture comparisons. |
| `Build\audio-test.txt` | Human-readable `hold-seconds bass mid treble [label]` sequence played by the Remote's Settings → Audio → Test button. The button sends `AUDIO_TEST=…,1,1` (notification + waveform/FFT) and `AUDIO_TEST=off` on Stop. |
| `VAR_TIME=0` | Multiplies the engine clock by 0, so the shader's `time` is frozen. |
| `VAR_FRAME=0` | Stops the frame counter (only helps shaders that use `frame`; the counter cannot be reset to 0). |
| `HUE_AUTO=0`, `COL_HUE=0` | Stops the automatic hue drift and any colour shift, which otherwise repaints every frame. |
| `audio-verify.ps1` | Wraps all of the above: load → settle → inject → capture → report means/spread/MD5. |

### Interactive check with `audio-test.txt`

Load the candidate preset, then press **Settings → Audio → Test** in the Remote. The Visualizer
shows the active values in a notification; press **Stop** to restore live audio. This is the
fast way to see whether a preset responds at all and which band drives it. It also synthesises
a waveform and a band-shaped `get_fft()` spectrum, so wave- and FFT-based presets respond even
though they have no band expressions. It is not the acceptance gate: the button sends
`AUDIO_TEST=…,1,1`, so the changing notification, waveform and FFT would break byte-identical
captures. Use plain `AUDIO_TEST=` through `audio-verify.ps1` for the recorded v1/v2/v3 result.

### Why the clock is zeroed *before* the preset is loaded

The shader's `time` is `GetTime() - presetStartTime`, and `GetTime()` is the engine clock times
`m_timeFactor`. Loading the preset while the clock is zeroed records `presetStartTime = 0`, so
freezing the clock afterwards pins `time` to exactly `0` in every run — without that, the frozen
value is `-presetStartTime`, which differs per run and makes captures incomparable.

The preset cross-fade reads the same clock, so freezing it mid-fade leaves a permanent mix of the
old and the new preset. `m_bBlending` is latched once `progress > 1`, so the harness runs the
clock for ~5 s after loading (fade completes) and only then freezes. Symptom of getting this
wrong: the capture shows the *previous* preset, or a blend of both.

### Reading the results

- Version 3 must produce **one distinct MD5** across all band settings — that is the hard proof
  that no audio term escaped the guard.
- Versions 1 and 2 must produce different frames per setting. Compare the per-channel spreads:
  v1 spread must not exceed v2's.
- Measured base (Visualizer 4.2, 1020×530, clock frozen at `time = 0`):

| Preset | v1 spread (R/G/B) | v2 spread (R/G/B) | v3 |
| --- | --- | --- | --- |
| `diatribes + IkeC - Blue Red Shiny 2` | reacts, MD5 changes | — (single guarded block) | 1 MD5 for all settings |
| `z0rg + IkeC - Psychedelic tube` | 3.8 / 1.9 / 1.8 | 5.8 / 3.4 / 2.4 | 1 MD5 for all settings |

### Limits of the injection

- `get_fft()` / `sampler_fft` reads the FFT **texture**, which plain `AUDIO_TEST` (the
  `audio-verify.ps1` path) does not synthesise: a preset that samples the spectrum directly is
  still silent-static there. The interactive Test button's `wave` flag does synthesise a
  band-shaped spectrum for manual checks, but a recorded gate still needs either a port of those
  terms to `bass`/`mid`/`treb` bands (preferred, they are version-guardable) or the audible
  fallback below. Keep `get_fft` out of the *base* look regardless — version 3 must stay static.
- The injected values skip the engine's smoothing, so v1 (`*_smooth` inputs) and v2 (`*_att`
  inputs) receive the same step. That is what makes the comparison exactly reproducible; the
  real smoothing is exercised by the fallback.
- Keep the injected sum low (each value is clamped to `0..4`): with hard cuts enabled
  (`bHardCutsDisabled=0`) the engine changes the preset when
  `bass+mid+treb > fHardCutLoudnessThresh * 6` (≈ 19.5 with the 3.25 default). Values around
  `0..2` are far below that.
- Shaders that use `frame` (noise variation) stay reproducible only as long as the counter does
  not move; `VAR_FRAME=0` freezes it but at an arbitrary value, so cross-run MD5 equality holds
  only when the counter is already stopped.

### Audible fallback (last resort)

`.local\shader-tools\audio-sim.ps1` generates a fixed 4 s pattern (`-Action make`) that sweeps one
band per second — bass, mids, treble, near-silence — and plays it on the default render device
(`-Action start` / `-Action stop`), which the engine captures through WASAPI loopback. Use it only
for presets that read `get_fft()`, and run it with `VAR_TIME=0` so only the audio can change the
frame. It is audible by design: loopback captures the rendered mix, so a muted or very quiet
endpoint also mutes the analysis.

### Engine testing gotchas

- Both `shadercheck` and the Visualizer read **only `comp_N=` lines** from a `.milk`. A probe
  file without them parses as an empty comp shader and renders black — no error anywhere.
- Loading a preset from a folder outside the preset tree rewrites `szPresetDir` in
  `Release\settings.ini` to that folder, which changes the user's preset browsing. Load test
  presets from inside the presets folders, or restore the value afterwards (the app also
  rewrites the file on exit, so stop it first).
- With `ChangePresetWithSong=1` the engine advances presets on its own after roughly 10–60 s.
  Capture within ~3 s of a `PRESET=` load, otherwise you capture the *next* preset (or a black
  blend frame) and misdiagnose it as a broken preset.
- Notifications (`AddError` banners) are drawn into the capture and use the engine clock, so a
  frozen clock makes them permanent. That is why plain `AUDIO_TEST=bass,mid,treb` prints
  nothing; any handler that shows a banner will break byte-identical comparisons. The Remote's
  Test button deliberately sends the optional `,1`/`notify` flag, so use plain `AUDIO_TEST=`
  for captures and `audio-test.txt` only for the interactive check.
- Wait for the compile before capturing: heavy presets need 30 s or more to appear.
