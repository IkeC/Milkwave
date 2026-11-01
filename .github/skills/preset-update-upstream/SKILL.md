---
name: preset-update-upstream
description: 'Sync the preset collections Milkwave ships from the upstream BeatDrop Music Visualizer (BMV) repository: copy BeatDrop/Butterchurn/Incubo_/Milkdrop2077 presets into Release and into the tracked Visualizer tree, apply the exclusion lists (big textures, presets that sample their own texture, name blacklist), prove that no shipped preset references a texture we do not bundle, compile-check the new presets and report what was added or removed. Use when asked to update/sync/refresh Milkwave presets from BeatDrop, when a preset reports a missing texture, or after presets were added upstream.'
argument-hint: '(optional) focus, e.g. "only check the exclusions" or "no compile check"'
---

# Milkwave preset update from upstream

Brings the BMV (BeatDrop Music Visualizer) preset collections into Milkwave. The upstream
repository is the source of truth for four collections; everything else in `resources\presets`
is Milkwave's own and must never be touched.

```
C:\Source\BeatDrop-Music-Visualizer\resources\Milkdrop2\presets
        |  robocopy (no /MIR)
        v
C:\Source\Milkwave\Release\resources\presets\{BeatDrop,Butterchurn,Incubo_,Milkdrop2077}
        |  exclusions: name blacklist + presets needing textures we do not ship
        |  texture scan (safety net)
        v
C:\Source\Milkwave\Visualizer\resources\presets\{...}   <- git-tracked deliverable
```

`Release\*` is gitignored, so only the tracked copy is a deliverable — but the running Visualizer
reads `Release\`, so both have to be updated or nothing is visible.

## Hard rules

- **Never touch** `Visualizer\resources\presets\Milkwave` or `...\Milkwave\Shader` — those are
  authored here, not synced. Same for `.vscode\tasks.json`, `settings.ini`, `precompile.txt`.
- **Never use `/MIR`** for the tracked copy: it intentionally keeps presets upstream later
  removed. The exception is the *excluded* presets below — those must not ship, so step 6 deletes
  them by hand.
- Do not run `Build\1-copy-presets-beatdrop-to-release.cmd` unattended: it starts with `git pull`
  and stops at `pause`. Run the equivalent robocopy/`del` commands yourself and keep the script in
  sync with what you did.
- A preset that needs a texture we do not ship is broken (missing-texture error, wrong look), so
  excluding it is not cosmetic. Never "fix" one by adding a texture unless the user asks.

## Procedure

### 1. Check what upstream has

```powershell
git -C C:\Source\BeatDrop-Music-Visualizer fetch --all
git -C C:\Source\BeatDrop-Music-Visualizer status -sb          # behind/ahead of origin
git -C C:\Source\BeatDrop-Music-Visualizer log --oneline -5
git -C C:\Source\BeatDrop-Music-Visualizer rev-parse --short HEAD   # goes into the report
```

If the working tree is stale, say so before pulling — the sync must describe the upstream state
that is actually on disk.

### 2. Copy upstream into `Release`

```powershell
$src = 'C:\Source\BeatDrop-Music-Visualizer\resources\Milkdrop2\presets'
$dst = 'C:\Source\Milkwave\Release\resources\presets'
robocopy $src "$dst\BeatDrop" /LEV:1
robocopy "$src\Butterchurn Presets" "$dst\Butterchurn"
robocopy "$src\Incubo_'s Presets"  "$dst\Incubo_"
robocopy "$src\Milkdrop2077 Presets" "$dst\Milkdrop2077"
```

`/LEV:1` keeps the BeatDrop collection flat. robocopy exit codes below 8 are success (1 = copied,
2 = extras, 3 = both); 8 and above is an error worth reporting.

### 3. Apply the exclusions of `Build\1-copy-presets-beatdrop-to-release.cmd`

That file holds two blocks, both `del /s /q` on the Release preset folder:

| Block | Reason |
| --- | --- |
| `::exclusions, often because preset is using big textures…` | name blacklist (character families, Violetta/Gillman/Octobella, Bing AI, Tesla Coil, …) |
| `::exclusions, because the preset samples its own texture…` | one entry per preset whose `sampler_*` texture is not bundled |

Reuse it instead of retyping: extract every `%CMD% …` line, replace `%CMD%` with `del /s /q`, run
them in the Release preset folder.

When a new preset has to be excluded, add it to the matching block **and** verify the pattern:

- cmd understands only `*` and `?` as wildcards; `[`, `]` and `%` are literal. PowerShell's
  `-like` treats `[…]` as a character class — never use `-like` to predict what `del` matches.
  Predict instead: `'^' + ([regex]::Escape($pattern).Replace('\*','.*').Replace('\?','.')) + '$'`
  and run it over `Get-ChildItem -Recurse -Filter *.milk`. Every name in the blacklist block
  normally means "delete it"; in the texture block every hit must really need a texture
  (`verify-preset-sync.ps1 -Rehearse` enforces that half).
- Filenames containing `%` are awkward in a `.cmd` (variable expansion); use a distinctive wildcard
  fragment such as `"Flexi - 100*shader fractal*Jian Simanjuntak mod remix*"`.
- Prefer the exact file name. Wide wildcards over-delete: `"Se7enSlasher + Gemini - Laser Show*"`
  matched 10 presets of which only 3 need a texture, so it became three exact names.
- Families are fine when every member is affected (`"*MilkDrop2077 - diamond lattice2*"`,
  `"martin - funky illusions - *"`).

### 4. Texture scan as the safety net

`Build\remove-presets-with-missing-textures.ps1` recomputes the texture dependency from the
presets themselves (it mirrors `CShaderParams::CacheParams`: `sampler_`/`pw_`/`fc_` prefixes are
stripped, `main`/`blur1-6`/`fft`/`noise*` are built in, `randNN[_prefix]` resolves against the
textures folder). Run it dry on the Release tree:

```powershell
& C:\Source\Milkwave\Build\remove-presets-with-missing-textures.ps1 `
    -PresetRoot 'C:\Source\Milkwave\Release\resources\presets' `
    -TextureRoot 'C:\Source\Milkwave\Release\resources\textures' `
    -Folder 'BeatDrop,Butterchurn,Incubo_,Milkdrop2077' -DryRun
```

Expected result after step 3: `0 of N presets would be removed`. Anything it reports means the
explicit list is incomplete — add those names to the matching block, re-run until it is clean.
Never leave the deletion to the scan alone: the shipped tree must be reproducible from the script.
Repeat the dry run on the tracked tree after step 6.

### 5. Rehearse before touching the tracked tree

```powershell
& C:\Source\Milkwave\.github\skills\preset-update-upstream\scripts\verify-preset-sync.ps1 -Rehearse
```

It copies the four upstream folders into a scratch tree, applies every `del` line of the script and
re-scans. Pass conditions:

- `0 of N presets would be removed, 0 distinct textures missing` in the scratch tree,
- nothing that still has all of its textures was deleted (the script lists the over-deleted ones).

### 6. Deploy `Release` → tracked `Visualizer`

```powershell
$rel = 'C:\Source\Milkwave\Release\resources\presets'
$vis = 'C:\Source\Milkwave\Visualizer\resources\presets'
foreach ($f in 'BeatDrop','Butterchurn','Incubo_','Milkdrop2077') { robocopy "$rel\$f" "$vis\$f" }
```

Then mirror the deletions by hand (script 2 does not delete):

```powershell
foreach ($f in 'BeatDrop','Butterchurn','Incubo_','Milkdrop2077') {
    $r = Get-ChildItem "$rel\$f" -Recurse -File -Filter *.milk |
         ForEach-Object { $_.FullName.Substring("$rel\$f".Length + 1) }
    Get-ChildItem "$vis\$f" -Recurse -File -Filter *.milk |
        Where-Object { $r -notcontains $_.FullName.Substring("$vis\$f".Length + 1) } |
        Remove-Item -Force
}
```

This removes exactly the presets the current exclusion lists reject, plus anything upstream
dropped — the counts of the four collections must match afterwards.

### 7. Verify and report

```powershell
& C:\Source\Milkwave\.github\skills\preset-update-upstream\scripts\verify-preset-sync.ps1 -Compare
```

- counts per collection, `Release` vs tracked, must be equal,
- every tracked `.milk` byte-identical to its `Release` twin (the script hashes them),
- scan the **tracked** tree once more with the `-Folder` list; it must report 0.

Compile-check the newly added presets with
`.local\shader-tools\check\bin\Debug\net8.0\shadercheck.exe <preset> <include.fx>` (60 s per file,
`Visualizer\resources\data\include.fx`; rebuild it with `dotnet build` in
`.local\shader-tools\check` if the exe is missing, and skip the check when the folder is gone).
Two results always show up here:

- BeatDrop writes a leading backtick on shader lines (`comp_1=`shader_body`); the Visualizer
  strips it, the harness must strip it too, otherwise every preset "fails" with
  ``X3000: unexpected token '`'``.
- `shader_body not found` means the preset has no `comp_*` at all — a classic MilkDrop preset
  (`per_frame_*`, `wave_*`). Report those as "no shader, nothing to compile", never as failures.

Finally refresh the `Changes.md` line under the current version:

```
  - Updated presets from BeatDrop, excluding those that need textures we don't ship
```

Report a table of `Release`/tracked counts per collection, the added presets (names), the removed
ones (name -> missing texture), the patterns added to the script, the upstream commit that was
synced, and anything upstream that was intentionally left out.

## Known traps

- **Tracked counts drift.** The tracked copy never deletes, so it can hold more files than
  `Release` (upstream removals, older exclusions). The counts only line up after step 6.
- **`del /s /q` on the Release root** reaches all four collections; a pattern meant for BeatDrop
  can hit `Incubo_` — usually desired, but check.
- **Duplicated names** (`Se7enSlasher - Trap Nation Visualizer.milk` exists in the BeatDrop root
  and in `Incubo_`) mean one pattern deletes two files; count files, not names.
- **`-Folder` defaults to every sub folder** — always pass the four collection names so `Milkwave`
  and `Milkwave\Shader` are not scanned; they may reference custom textures on purpose.
- Presets using a `randNN_` prefixed texture are only a problem when no file matches, so a texture
  folder cleanup can turn working presets into excluded ones — re-run the scan after touching
  `resources\textures`.
