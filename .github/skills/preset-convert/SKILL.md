---
name: preset-convert
description: 'Convert a Shadertoy GLSL shader into a working Milkwave .milk preset for the DX9 Visualizer, including its vis_version audio reactivity. Use when: converting or creating a preset, adding or tuning audio reactivity (bass/mid/treb, vis_version, vis_intensity, vis_shift) or a mouse reaction (scene tilt, camera angle), a preset renders black or flat, the legacy shader compiler reports X3xxx/X4xxx errors, or a preset must be verified and deployed to presets/Milkwave/Conv.'
argument-hint: 'Shadertoy URL or shader ID (e.g. https://www.shadertoy.com/view/MdVXzw) [+ optional preset title]'
---

# Milkwave preset conversion

Turns one Shadertoy shader into a shipped Milkwave preset. A preset is a `.milk` text file whose
`comp_1..N=` lines hold HLSL; the engine wraps that code with
`Visualizer\resources\data\include.fx` and compiles it as `ps_3_0`.

## Destinations

| Purpose | Path |
| --- | --- |
| Git-tracked deliverable | `Visualizer\resources\presets\Milkwave\Conv\` |
| Live copy for the running Visualizer (`Release\*` is gitignored) | `Release\resources\presets\Milkwave\Conv\` |

File name: `<author> + IkeC - <shader name>.milk`. Create the `Conv` folder on first use.

## Prerequisites

- `Release\MilkwaveVisualizer.exe` for render tests. Reuse a running instance; never kill it.
- Local toolchain in `.local\shader-tools\` (untracked, usually present on this machine):
  `conv\` (GLSL→HLSL), `check\` (compile checker), `convert.ps1`, `compcode.ps1`, `fixup.ps1`,
  `render-test.ps1`, `stats.ps1`, `analyze-capture.ps1`, `ipc-send.ps1`, `iterate.ps1`
  (set code → 60 s check → load → wait → capture → stats, the fast edit/render loop),
  `audio-verify.ps1` (silent, reproducible audio-reactivity check via the `AUDIO_TEST` pipe hook)
  and `audio-sim.ps1` (audible deterministic WAV, only for presets that read `get_fft`).
- `Build\audio-test.txt`: the commented band sequence played by the Remote's
  Settings → Audio → Test button. It is the human-readable counterpart to
  `audio-verify.ps1 -Bands`: use it for a quick manual reaction check, then confirm with the
  reproducible harness. The Visualizer shows the active values and synthesises a matching
  waveform and FFT spectrum, so shader, wave-based and FFT-based presets react.
- If the toolchain is missing, convert in-app instead (Remote → tab "Shader",
  see `Manual.md` → "Tab Shader"), then save the result with `QUICKSAVE`. The fix rules below
  still apply.
- Richest background notes (untracked): `.local\shader-conversion-shadertoy.md`.

## Procedure

1. **Screen the candidate first.** Skip when any of these is true; record the reason:
   multi-pass (`mEffect.mPasses.length > 1`), license forbids reuse, more than roughly 700 lines
   of GLSL, compile over 60 s, or heavy nested loops. Prefer single-pass shaders of roughly
   400 lines or less. Time the checker and **cancel it at 60 s** — a multi-minute compile is a
   skip, not a wait (wrapper in step 4). Only the `flags=0x10000` line counts (the engine uses
   `SKIPOPTIMIZATION`); the `flags=0x0` fallback compile is not representative.

   Before skipping a multi-pass shader, look at what the passes actually are: the `Image` pass is
   very often a plain `texture(iChannel0, uv)` blit, so the real work is one pass and the port is a
   single comp shader. The genuine blocker is **feedback** — a pass whose input is its own buffer
   (`mInputs[k].mInfo.type == "buffer"` with the pass's own output id, typically `iChannel1..3`).
   A comp shader cannot read the previous frame (`sampler_main` is not readable), so the feedback
   `mix()` has to be dropped or approximated; that changes the look, so ask the user before
   spending the effort (see `sdcGRX` below).
2. **Fetch the source.** `web_fetch`/`curl` are blocked by Cloudflare; use the browser:
   - `openBrowserPage("https://www.shadertoy.com/view/<ID>")`, wait for the app to load.
   - `page.evaluate` → `window.gShaderToy.mEffect.mPasses[0].mSource`, plus the metadata from
     `gShaderToy.mInfo` (id, name, username, description).
   - Copy through a focused `<textarea>` + `document.execCommand('copy')` (the clipboard API
     fails with "Document is not focused"), then write it BOM-free:
     ```powershell
     $c = (Get-Clipboard -Raw) -replace "`r`n","`n"
     [IO.File]::WriteAllText('C:\Source\Milkwave\.local\shadertoy\sources\<ID>.glsl', $c,
       (New-Object System.Text.UTF8Encoding($false)))
     ```
3. **Convert.**
   ```powershell
   powershell -File C:\Source\Milkwave\.local\shader-tools\convert.ps1 `
     -Id <ID> -Author <author> -Title '<title>' `
     -License 'License: Creative Commons Attribution-NonCommercial-ShareAlike 3.0 (Shadertoy default)'
   ```
   This runs converter → `compcode get` → `fixup.ps1` → `compcode set` → `shadercheck`, and the
   built-in check is already under the 60 s limit (it prints `TIMEOUT: compile exceeded 60 s`).
   Output lands in `.local\shadertoy\work\<author> + IkeC - <title>.milk`. It **overwrites**
   `work\<ID>_code.txt` and any work preset with the same title, so re-test with a distinct
   `-Title` instead of clobbering a finished candidate.
4. **Fix until `COMPILE OK`.** Apply the mechanical pass in
   [hlsl-pitfalls.md](./references/hlsl-pitfalls.md#mechanical-fix-pass) first, then work the
   per-error table. Re-run the checker after each batch of edits — **always under the 60-second
   timeout**:
   ```powershell
   dotnet build -c Debug C:\Source\Milkwave\.local\shader-tools\check   # once
   $exe = 'C:\Source\Milkwave\.local\shader-tools\check\bin\Debug\net8.0\shadercheck.exe'
   $p = Start-Process $exe -ArgumentList @("`"<preset>`"") -PassThru -NoNewWindow `
          -RedirectStandardOutput "$env:TEMP\sc.out"
   if (-not $p.WaitForExit(60000)) { Stop-Process -Id $p.Id -Force; 'TIMEOUT (>60 s) -> skip' }
   else { Get-Content "$env:TEMP\sc.out" }
   ```
   Add `'--dump','wrapped.fx'` to the argument list to inspect the exact compiled source.
   **On `TIMEOUT`: kill it immediately, record "compile > 60 s", and treat the shader as over
   budget** — try at most one cheaper restructure (fewer inlined `map()` calls, shorter loops),
   then skip. Never re-run a compile hoping it finishes, and never leave one running in the
   background.
5. **Verify by rendering**, not just compiling:
   ```powershell
   powershell -File C:\Source\Milkwave\.local\shader-tools\render-test.ps1 -Preset "<abs path>"
   powershell -File C:\Source\Milkwave\.local\shader-tools\stats.ps1 -Path "<newest capture.png>"
   ```
   The pipe reader handles `PRESET` on another thread, so a `CAPTURE` sent immediately after is
   executed *before* the preset finishes loading (the engine compiles the comp shader on the render
   thread — 10..20 s for a heavy shader). Capturing too early yields the **previous** preset's
   frame, and since the Visualizer also auto-advances presets, always check that the newest capture
   filename matches the preset you meant to test, otherwise wait (`iterate.ps1` waits, then
   re-checks the filename).
   Gates are in [preset-format.md](./references/preset-format.md#verification-gates).
6. **Add audio reactivity** (`vis_version` style, as in all 40 shipped
   `Milkwave\Shader` presets). Details, code shapes and pitfalls:
   [audio-reactivity.md](./references/audio-reactivity.md).
   - Version 1 stays **calm**: `bass_smooth`/`mid_smooth`/`treb_smooth` (about a 1-second
     average). Version 2 is **snappy**: the `*_att` bands. Version 3 and above
     (`(int)vis_version > 2`) must be **completely static** — no audio term anywhere.
   - Wrap the reaction in `#ifdef bass_smooth … #else … #endif`; the `#else` branch is a subtle
     `*_att`-only fallback for other MilkDrop variants. End every branch, including the static
     one, with `… * vis_intensity + vis_shift` so the Remote's Intensity/Shift knobs keep
     working.
   - React by changing an **existing** parameter: the shape/size of an object (SDF blend,
     radius, edge, sun size), a position/depth offset, or a colour slice (channel gain,
     saturation/brightness, per-band hue lane). Zoom (`uv *=`) only as a last resort, and never
     add or remove geometry or let the base look depend on the audio level.
   - Put the block next to the value it feeds (inside the function that owns the parameter, as
     `CineShader Lava`, `kishimisu`, `Breeze` do), not floating at the end of the shader.
   - Amplitude: about `0.1 .. 0.5` of a normalised `0..1` parameter, `0.05 .. 0.1` for a zoom
     factor; version 1 must not be stronger than version 2. **Version 2 is the usual
     overshoot**: keep it an accent, at most ~`1.3 .. 1.4x` the calm range (house style `aRng`
     `0.26` for v1 and `0.34` for v2 — not `0.45`), and cap colour/hue swings at about
     `1.5 rad` (~85°). When in doubt ship it milder; the user can still raise vis_intensity.
   - If the shader has no parameter that can carry a reaction, ship it without one instead of
     inventing a global zoom pulse.
   - For a quick manual check, press Settings → Audio → Test in the Remote. It plays
     `Build\audio-test.txt` (average, one band at a time, quiet, loud, silence); the
     Visualizer shows the active levels and also synthesises a waveform and FFT spectrum,
     so wave- and FFT-based presets respond even without band expressions. Click **Stop**
     to restore live audio.
     The file's columns are the same `bass,mid,treble` values accepted by
     `audio-verify.ps1 -Bands`.
   - Confirm with the **silent, reproducible harness** — never live music:
     ```powershell
     powershell -File C:\Source\Milkwave\.local\shader-tools\audio-verify.ps1 `
       -Preset "<abs path>" -Version 1 -Bands '0,1,1;2,1,1'
     ```
     It zeroes the clock, loads the preset (so the shader's `time` is exactly 0), lets the
     cross-fade finish, then injects fixed band levels with the `AUDIO_TEST=bass,mid,treb`
     pipe command. Nothing is played through the speakers and identical arguments reproduce
     byte-identical captures. Expect v1/v2 to differ between settings (v2 spread > v1) and v3 to
     give the *same MD5* for every setting. Details, limits and the base results:
     [audio-reactivity.md](./references/audio-reactivity.md#reproducible-verification-no-live-audio).
   - Optional: offer a calm and a punchy variant as two preset files so the user can choose.
7. **Add a mouse reaction** (optional, but usually wanted). Mouse input is independent of the
   audio contract: it must not touch `vis_version`, nothing goes into the `#ifdef bass_smooth`
   blocks, and version 3+ may still react to the mouse. `include.fx` supplies `mouse`
   (a `float4`; `mouse_x`, `mouse_y`, `mouse_pos` are aliases):

   | Component | Meaning |
   | --- | --- |
   | `mouse.x` | pointer X, `0` = left .. `1` = right (same direction as `uv.x`) |
   | `mouse.y` | pointer Y, `0` = top .. `1` = bottom (matches `uv.y`; the warp-script `mouse_y` variable is the other way round) |
   | `mouse.z` | `1` while a mouse button is held, `0` otherwise (Milkwave: the **right** button; the left one is reserved for the preset UI) |
   | `mouse_clicked` | `1` for only 2 frames after a click — too short for a visible effect, do not use it for the reaction |
   | `mouse.w` | last Y while clicked (iMouse emulation), rarely needed |

   - **Prefer tilting the scene and/or changing the camera angle** — that is what the user
     usually wants. Shipped examples: `ADOB + IkeC - Waves Remix` (whole-scene tilt),
     `chronos + IkeC - Astral Jelly Journey` (viewport angle from `mouse.xy`),
     `hughsk + IkeC - 2015-12-12` (camera rotation), `Apollonian Structure` (view offset).
   - Multiply the reaction with `saturate(mouse.z)` so the frame without a pressed button is
     *exactly* unchanged, and keep the amount small: about `±0.1 .. ±0.2 rad` for a tilt,
     `±0.1 .. ±0.5` of the existing angle for a camera reaction.
   - 2D tilt: rotate `uv` around the centre, branch-free. `sin(0) = 0` and `cos(0) = 1`, so the
     untouched frame stays byte-identical:
     ```hlsl
     // CONV: holding the mouse button tilts the scene slightly, pointer X sets the angle
     float aTilt = (mouse.x - 0.5) * 0.3 * saturate(mouse.z);
     float aTiltS = sin(aTilt), aTiltC = cos(aTilt);
     uv = float2(uv.x * aTiltC - uv.y * aTiltS, uv.x * aTiltS + uv.y * aTiltC);
     ```
   - Raymarcher: rotate the ray direction (or the eye position around the target) instead of the
     screen — `mouse.x` → yaw, `mouse.y` → pitch, both scaled by `saturate(mouse.z)`.
   - Never let the base look depend on the pointer position: with no button held the preset must
     render exactly like the unreacted version (the `vis_version` 3 gate compares such captures).
   - Milkwave only feeds the mouse while *interactive mouse control* is enabled (`Ctrl+M`,
     default on; `settings.ini` → `bEnableMouseInteraction=1`). The pointer position is polled
     every frame, so it needs no mouse messages, but the button state does.
   - Verify with window messages, **not** `mouse_event` (injected clicks are unreliable and may
     not reach the window). Freeze the frame first (`VAR_TIME=0`, `AUDIO_TEST=1,1,1`,
     `HUE_AUTO=0`, `COL_HUE=0`), make the window topmost, place the pointer with `SetCursorPos`,
     then use a small P/Invoke class (`GetWindowRect`, `SetWindowPos`, `SetCursorPos`,
     `SendMessage`) with `$h` = `MainWindowHandle`, `$r` = window rect, `$w` = its width and
     `$cx`/`$cy` = its centre:
     ```powershell
     # baseline -> WM_RBUTTONDOWN at ~15% width -> move pointer to ~85% -> WM_RBUTTONUP
     $lp = [IntPtr](($cy - $r.Top) -shl 16 -bor ([int]($cx - 0.35*$w) - $r.Left))
     [MwMsg]::SendMessage($h, 0x0204, [IntPtr]2, $lp)   # WM_RBUTTONDOWN
     [MwMsg]::SendMessage($h, 0x0205, [IntPtr]0, $lp)   # WM_RBUTTONUP
     ```
     Expected: `press-left` ≠ `press-right` (the pointer steers the effect) and `released` =
     `baseline` (same MD5). Always restore the window's topmost state and the pointer position
     afterwards.
8. **Deliver.** Copy the preset (BOM-free) to both folders above, keep the attribution header.

## Hard rules

- **No UTF-8 BOM, ever.** With a BOM the version header is not parsed, the comp shader is
  silently disabled and the frame stays black with no error. Use
  `[IO.File]::WriteAllText($p, $t, (New-Object System.Text.UTF8Encoding($false)))`; never
  `Set-Content -Encoding UTF8`. Check that the first 4 bytes are `4D 49 4C 4B`.
- `PSVERSION=4` (`ps_3_0`). Never `PSVERSION=5` (`ps_4_0` is unsupported).
- `comp_N=` numbering must stay sequential from 1; edit code through
  `compcode.ps1 -Mode get` / `-Mode set` so it renumbers.
- Never early-`return` from `shader_body`; the wrapper appends the output line after it.
- Attribute every preset: `// <author> - <name> / <shadertoy url>` plus a `// License: …` line.
- Audio terms (`bass*`, `mid*`, `treb*`, `vol*`) may only live inside a version-guarded block,
  and **every** such block needs the `(int)vis_version > 2` static branch. `vis_version` 3 and
  above must be indistinguishable from an unreacted preset.
- Audio reactivity is verified with injected bands (`AUDIO_TEST=`, `audio-verify.ps1`), never by
  relying on live music. The audible WAV (`audio-sim.ps1`) is only for presets reading `get_fft()`.
- Mouse reactions are independent of the audio guards: keep `mouse`/`mouse_clicked` out of the
  `#ifdef bass_smooth` blocks, and always scale the reaction with `saturate(mouse.z)` so the
  frame without a pressed button is identical to the unreacted preset. Prefer tilting the scene
  or turning the camera; `mouse_clicked` (2 frames) is not a usable trigger.
- **Budget: after 5 failed attempts, stop, note the reason and pick another shader.**
- **60-second compile limit.** No compile run may exceed 60 s. Use the timeout wrapper in step 4;
  on `TIMEOUT` kill the process by PID, record "compile > 60 s", and skip the shader (one cheaper
  restructure at most). The engine needs the same time on first load, so a slow compile is a
  broken preset for the user. Never leave a long compile running in the background as a way to
  get the answer.
- Do not touch `.vscode\tasks.json`, `settings.ini`, `precompile.txt` (back it up if used), or
  preset folders other than the two destinations above.

## Symptom to cause

| Symptom | Likely cause |
| --- | --- |
| Compiles fine, constant black frame | UTF-8 BOM, global `const` read as 0, NaN collapsing the output |
| Compiles fine, frame is one flat colour, no structure | an SDF term that is singular exactly at the camera start (`/p`, `/(p*4.)`): GLSL hides it behind the *uninitialized* loop accumulator, `d = 0.` does not — start the march a hair along the ray (`d = 0.05`) and add a tiny epsilon to the divisor |
| Compiles fine, uniform/flat image | corrupted aspect/`iResolution` rewrite, `break` rewrite falling through |
| `X3000: syntax error: unexpected token 'break'` | the `flags=0x10000` path rejects `break` (the `flags=0x0` compile accepts it) — use the counter rewrite `i = steps;` inside an `if/else`, never `break` |
| `X3500: asymetric returns from if statements` | an early `return` inside a loop body: collect the result in a variable, `if/else` the accumulation, and exit via the loop counter |
| Adapted shader washed out / rainbow grain everywhere | the texture-overlay term (`col += 0.2*tex2D(iChannel0, …)`) sits on top of an almost-black result; `sampler_noise_lq` is bright and colourful, so 0.2 swamps it — use about 0.05 |
| `X3000 unexpected token '.'` | local or parameter name collides with an `include.fx` macro (`mid`, `time`, `frame`, `rad`, `ang`, …) |
| Compile hangs for minutes | large loop body, or texture sampling inside a loop |
| Visualizer freezes on load | shader too heavy for DX9; restart the engine and drop it |
| Bright cores turn black (bright rims around black blobs) | negative or NaN accumulator reaching the output — e.g. `exp(...)/v` with `v` made negative by an audio-modulated blend weight, or `1/length()` at the centre. Use `tanh(max(O, 0.)/2e2)` and keep blend weights `saturate()`d |
| Still reacts to audio at version 3 | an audio term outside the version guard, or a second `#ifdef bass_smooth` block without the `> 2` branch |
| Mouse does nothing | `mouse_clicked` used as the trigger (2-frame pulse), `mouse.z` missing from the expression, or interactive mouse control is off (`Ctrl+M`) |
| Tilt/angle sticks after releasing the button | the reaction is not multiplied by `saturate(mouse.z)` |
| Mouse test makes the whole frame move | the clock was still running (`VAR_TIME=0` missing) or live audio/waveform was active — freeze with `VAR_TIME=0` + `AUDIO_TEST=1,1,1` before comparing captures |
| Reaction is dead or inverted | swapped `smoothstep`/`clamp` arguments (`smoothstep(bass_att,0,2)`, `clamp(0,2,bass_att)`), or `vol_*` used as a volume level (the engine fills it with `treb/3`) |

## Configuration

Edit this file to retune the skill: destination folders (above), the try budget, the license
policy in step 3, and the gates in `references/preset-format.md`. Keep `name:` identical to the
folder name (`.github/skills/preset/`) and the description under 1024 characters; both are
enforced silently by the discovery layer.

## IMPORTANT

* Always keep all .milk versions (increment the filename with a number) while iterating/working on a shader - the user might like an earlier version better than the final one. If you are not sure, keep it. The user can always delete it later.
* Write findings/problem solutions back to this file to speed up the process for future conversions.
* If you open browser tabs, close them as soon as all required data is loaded to save resources.