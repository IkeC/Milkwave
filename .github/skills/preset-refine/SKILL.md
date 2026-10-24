---
name: preset-refine
description: 'Refine an existing Milkwave .milk preset instead of converting a new shader: retune the audio reaction (calmer/stronger, smooth vs attack, hue/colour amplitude), add or adjust a mouse tilt/camera reaction, fix a preset that is too extreme or too static, or verify and redeploy an edited preset. Applies the preset skill''s version contract, toolchain and verification gates. stop after 3 passes/attempts to avoid endless cycles and get user feedback. running this skill shouldnt take longer than 3 minutes.'
argument-hint: 'Path of the .milk preset + what should change (e.g. "v2 too strong", "hue towards pink", "add mouse tilt")'
---

# Milkwave preset refinement

Changes an existing preset in place — its audio reaction, colour behaviour, mouse reaction or a
specific visual parameter — without re-converting the shader.

The conversion contract, code shapes, pitfalls and harness details live in the companion
**preset** skill. Read the relevant parts before editing; do not invent a second convention:

- `../preset/SKILL.md` — procedure, hard rules, symptom table (step 6 audio, step 7 mouse)
- `../preset-convert/references/audio-reactivity.md` — version/intensity/shift contract, amplitudes
- `../preset-convert/references/hlsl-pitfalls.md` — compiler errors and the mechanical fix pass
- `../preset-convert/references/preset-format.md` — file format and verification gates

## Inputs

- The `.milk` path (usually given as the `Release\resources\presets\...` file).
- The requested change in the user's words ("v2 is too extreme", "hue towards pink", "add mouse
  tilt", "reaction is invisible").
- If the request is ambiguous, state the interpretation you will implement, then do it — the
  gates below prove whether it works.

## Both copies, always

| Purpose | Path |
| --- | --- |
| Git-tracked deliverable | `Visualizer\resources\presets\<collection>\<file>.milk` |
| Live copy for the running Visualizer (`Release\*` is gitignored) | `Release\resources\presets\<collection>\<file>.milk` |

Edit the tracked copy, then copy it over the live one; both must end up byte-identical, BOM-free
(`4D 49 4C 4B`), `comp_1..N=` sequential, no trailing newline.

## Procedure

1. **Read the preset end to end** and list every reaction point it already has (audio `#ifdef`
   blocks, colour terms, mouse use, zoom/pan). Use the code, not the comments — comments drift.
   ```powershell
   powershell -NoProfile -File C:\Source\Milkwave\.local\shader-tools\compcode.ps1 `
     -Mode get -Milk "<tracked .milk>" -Txt C:\Source\Milkwave\.local\shadertoy\work\refine.txt
   ```
2. **Plan the smallest change** that satisfies the request. Prefer retuning an existing
   parameter over adding a new effect; never add or remove geometry, and never let the base look
   (quiet, no button, version 3) depend on audio or mouse.
3. **Edit** the extracted text, then write it back (this renumbers `comp_N=`), strip the trailing
   newline and keep the file BOM-free:
   ```powershell
   powershell -NoProfile -File C:\Source\Milkwave\.local\shader-tools\compcode.ps1 `
     -Mode set -Milk "<tracked .milk>" -Txt C:\Source\Milkwave\.local\shadertoy\work\refine.txt
   ```
   Never renumber by hand, never `Set-Content -Encoding UTF8`.
4. **Keep the contract** (details in the preset skill):
   - v1 = `*_smooth` (calm), v2 = `*_att` (accent), v3+ = static; every audio term inside
     `#ifdef bass_smooth … #else … #endif`, every branch ending in `* vis_intensity + vis_shift`.
   - Mouse terms never inside the audio guard, always scaled by `saturate(mouse.z)`.
   - Prefer tilting the scene / turning the camera over global zooms.
5. **Compile** before rendering anything (60 s limit, kill on `TIMEOUT`):
   ```powershell
   $exe = 'C:\Source\Milkwave\.local\shader-tools\check\bin\Debug\net8.0\shadercheck.exe'
   $p = Start-Process $exe -ArgumentList @("`"<tracked .milk>`"") -PassThru -NoNewWindow `
          -RedirectStandardOutput "$env:TEMP\sc.out"
   if (-not $p.WaitForExit(60000)) { Stop-Process -Id $p.Id -Force; 'TIMEOUT' } else { Get-Content "$env:TEMP\sc.out" }
   ```
   Add `'--dump','wrapped.fx'` to inspect which preprocessor branch was compiled.
6. **Verify the audio gates** on the live copy:
   ```powershell
   powershell -File C:\Source\Milkwave\.local\shader-tools\audio-verify.ps1 `
     -Preset "<abs Release .milk>" -Version 1 -Bands '0,1,1;2,1,1'
   ```
   Repeat with `-Version 2` and `-Version 3`. Expected: v1/v2 differ per band setting, v2's
   spread is only *slightly* larger than v1's, v3 gives one MD5 for every setting. Record the
   numbers — they are the before/after evidence.
7. **Verify the mouse gates** if the reaction uses `mouse`: freeze (`VAR_TIME=0`,
   `AUDIO_TEST=1,1,1`, `HUE_AUTO=0`, `COL_HUE=0`), make the window topmost, then send
   `WM_RBUTTONDOWN`/`WM_RBUTTONUP` (0x0204/0x0205) with `SendMessage`, **not** `mouse_event`.
   Expected: `released` = `baseline` (same MD5) and two different pointer positions give
   different frames. Restore topmost, the clock and the pointer afterwards.
8. **Deploy and report**: copy the file to the live folder, confirm equal hashes, then report the
   measured before/after table plus anything you deliberately left alone.

## Calibration defaults (the fixes that keep coming up)

- **"v2 is too extreme"** — the usual overshoot. `aRng` goes from `0.45` to `0.34` (keep v1 at
  `0.26`), colour scales go down by roughly 20 %, and hue/colour swings are capped at about
  `1.5 rad` (~85°). v2 should read as an accent, not a jump.
- **"reaction is invisible"** — wrong `smoothstep` argument order (`smoothstep(0,2,x)`), an `_att`
  band in the v1 branch, the block outside `shader_body`, or `vis_intensity` applied twice.
- **"make it hue towards <colour>"** — rotate around the grey axis instead of scaling channels
  (channel gains cannot create a hue that is not there; a blue scene has no red to boost):
  ```hlsl
  float3 hueRotate(float3 c, float a)
  {
    float3 k = float3(0.57735027, 0.57735027, 0.57735027);
    float ca = cos(a), sa = sin(a);
    return c * ca + cross(k, c) * sa + k * dot(k, c) * (1.0 - ca);
  }
  ```
  Drive it with `saturate(aBe) * k`, where the resulting angle `saturate(aBe) * k` stays at or
  below about `1.5 rad` (~85°); `hueRotate(c, 0)` is exactly `c`.
- **"add mouse support"** — camera yaw/pitch or a `uv` tilt, gated by `saturate(mouse.z)`; no
  re-normalize after the rotation, otherwise the un-pressed frame is no longer bit-identical.
- **"still reacts at version 3"** — a second audio block without the `> 2` branch, or an audio
  term used outside the guard.

## Guardrails

- Never edit only one of the two copies, and never change the attribution/license header.
- Do not touch `.vscode\tasks.json`, `settings.ini`, `precompile.txt`, or other preset folders.
- Keep the previous look recoverable: git holds the tracked version, and for a large rewrite keep
  a numbered copy (`… 2.milk`) until the user has seen the result.
- A big reaction change is not a licence to change the composition; compare a capture before and
  after with `render-test.ps1` + `stats.ps1`.
- After 5 failed attempts on the same symptom, stop and report what was tried.
