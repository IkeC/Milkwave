<#
.SYNOPSIS
    Verifies the Milkwave preset sync from the upstream BeatDrop Music Visualizer repository.

.DESCRIPTION
    -Rehearse copies the four upstream preset collections into a scratch tree, applies every
    "del" line of Build\1-copy-presets-beatdrop-to-release.cmd, and proves that
      * no preset still references a texture we do not ship,
      * the texture exclusion block does not delete presets whose textures all exist.
    -Compare proves that the git-tracked Visualizer tree matches the Release tree (counts and
    file hashes) and that the tracked presets reference no missing texture.
    Both modes run when no switch is given.

.PARAMETER Rehearse
    Rehearse the copy + exclusion pipeline in %TEMP%\milkwave-preset-rehearsal.

.PARAMETER Compare
    Compare Release\resources\presets with Visualizer\resources\presets.

.PARAMETER SkipScan
    Skip the texture scan of the tracked tree in -Compare mode.

.PARAMETER RepoRoot
    Milkwave repository root. Defaults to the repository this script lives in.

.PARAMETER Upstream
    Upstream preset folder. Defaults to the sibling BeatDrop-Music-Visualizer checkout.

.EXAMPLE
    .\verify-preset-sync.ps1 -Rehearse

.EXAMPLE
    .\verify-preset-sync.ps1 -Compare -SkipScan
#>
[CmdletBinding()]
param(
    [switch]$Rehearse,
    [switch]$Compare,
    [switch]$SkipScan,
    [string]$RepoRoot = '',
    [string]$Upstream = ''
)

$ErrorActionPreference = 'Stop'

$collections = @('BeatDrop', 'Butterchurn', 'Incubo_', 'Milkdrop2077')
$sourceFolders = @{
    'BeatDrop'      = ''
    'Butterchurn'   = 'Butterchurn Presets'
    'Incubo_'       = "Incubo_'s Presets"
    'Milkdrop2077'  = 'Milkdrop2077 Presets'
}

if (-not $RepoRoot) { $RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..\..')).Path }
if (-not $Upstream) { $Upstream = Join-Path (Split-Path $RepoRoot -Parent) 'BeatDrop-Music-Visualizer' }
$Upstream = Join-Path $Upstream 'resources\Milkdrop2\presets'

$releasePresets = Join-Path $RepoRoot 'Release\resources\presets'
$trackedPresets = Join-Path $RepoRoot 'Visualizer\resources\presets'
$releaseTextures = Join-Path $RepoRoot 'Release\resources\textures'
$trackedTextures = Join-Path $RepoRoot 'Visualizer\resources\textures'
$copyScript = Join-Path $RepoRoot 'Build\1-copy-presets-beatdrop-to-release.cmd'
$scanScript = Join-Path $RepoRoot 'Build\remove-presets-with-missing-textures.ps1'

if (-not $Rehearse -and -not $Compare) { $Rehearse = $true; $Compare = $true }

$script:failed = $false
function Pass([string]$message) { "  PASS  $message" }
function Fail([string]$message) { $script:failed = $true; "  FAIL  $message" }
function Info([string]$message) { "        $message" }

# cmd wildcards: only * and ? are special, everything else (including [ ] %) is literal
function ConvertTo-CmdRegex([string]$pattern) {
    '^' + ([regex]::Escape($pattern).Replace('\*', '.*').Replace('\?', '.')) + '$'
}

function Copy-Upstream([string]$destination) {
    foreach ($collection in $collections) {
        $source = $sourceFolders[$collection]
        $from = if ($source) { Join-Path $Upstream $source } else { $Upstream }
        $to = Join-Path $destination $collection
        $arguments = @($from, $to, '/NFL', '/NDL', '/NJH', '/NJS', '/NP')
        if (-not $source) { $arguments += '/LEV:1' }
        robocopy @arguments | Out-Null
        if ($LASTEXITCODE -ge 8) { throw "robocopy failed ($LASTEXITCODE): $from -> $to" }
    }
}

function Get-PresetList([string]$root) {
    Get-ChildItem -LiteralPath $root -Recurse -File -Filter *.milk |
        ForEach-Object { $_.FullName.Substring($root.Length).TrimStart('\') }
}

function Get-ExclusionLines([string]$path) {
    Get-Content -LiteralPath $path | Where-Object { $_ -match '^\s*%CMD%' }
}

# patterns of the "samples its own texture" block only
function Get-TextureBlockPatterns([string]$path) {
    $patterns = @()
    $inBlock = $false
    foreach ($line in Get-Content -LiteralPath $path) {
        if ($line -like '*samples its own texture*') { $inBlock = $true; continue }
        if (-not $inBlock) { continue }
        if ($line -match '^\s*::' -or $line.Trim() -eq '') { continue }
        if ($line -notmatch '^\s*%CMD%') { break }
        foreach ($match in [regex]::Matches($line, '"([^"]+)"')) { $patterns += $match.Groups[1].Value }
    }
    $patterns
}

function Invoke-Scan([string]$presetRoot, [string]$textureRoot, [string[]]$folders, [switch]$DryRun) {
    $arguments = @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', $scanScript,
        '-PresetRoot', $presetRoot, '-TextureRoot', $textureRoot,
        '-Folder', ($folders -join ','))
    if ($DryRun) { $arguments += '-DryRun' }
    $output = & powershell @arguments 2>&1
    $entries = @()
    $summary = ''
    foreach ($line in $output) {
        $text = [string]$line
        if ($text -match '^\s*(.+?)\s+==>\s+(.+)$') {
            $entries += [pscustomobject]@{ Preset = $matches[1]; Textures = $matches[2] }
        } elseif ($text -match 'of \d+ presets') {
            $summary = $text.Trim()
        }
    }
    [pscustomobject]@{ Entries = $entries; Summary = $summary }
}

function Invoke-Rehearsal {
    ''
    '== Rehearsal (upstream -> exclusions -> texture scan) =='
    if (-not (Test-Path $Upstream)) { Fail "upstream presets not found: $Upstream"; return }
    if (-not (Test-Path $copyScript)) { Fail "copy script not found: $copyScript"; return }
    if (-not (Test-Path $releaseTextures)) { Fail "textures not found: $releaseTextures"; return }

    $scratch = Join-Path $env:TEMP 'milkwave-preset-rehearsal'
    if (Test-Path $scratch) { Remove-Item -LiteralPath $scratch -Recurse -Force }
    New-Item -ItemType Directory -Path $scratch | Out-Null
    try {
        Copy-Upstream $scratch
        $before = @(Get-PresetList $scratch)
        Info "upstream presets: $($before.Count)"

        $dependency = Invoke-Scan $scratch $releaseTextures $collections -DryRun
        $textureDependent = @($dependency.Entries | ForEach-Object { $_.Preset })
        Info "texture-dependent presets (before exclusions): $($textureDependent.Count)"

        $lines = Get-ExclusionLines $copyScript
        Info "exclusion lines: $($lines.Count)"
        $batch = Join-Path $scratch '_exclusions.bat'
        Set-Content -LiteralPath $batch -Value (($lines -replace '%CMD%', 'del /s /q') -join "`r`n") -Encoding ASCII
        Push-Location $scratch
        try { & $env:ComSpec /c '_exclusions.bat' *> $null } finally { Pop-Location }
        Remove-Item -LiteralPath $batch -Force

        $after = @(Get-PresetList $scratch)
        $deleted = @($before | Where-Object { $after -notcontains $_ })
        Info "deleted by the exclusion lists: $($deleted.Count)"

        # over-deletion: the texture block may only remove presets that really need a texture
        $patterns = @(Get-TextureBlockPatterns $copyScript)
        $matched = @()
        foreach ($pattern in $patterns) {
            $regex = ConvertTo-CmdRegex $pattern
            $hits = @($before | Where-Object { (Split-Path $_ -Leaf) -match $regex })
            if ($hits.Count -eq 0) { Fail "texture exclusion pattern matches no preset: $pattern" }
            $matched += $hits
        }
        $overDeleted = @($matched | Sort-Object -Unique | Where-Object { $textureDependent -notcontains $_ })
        if ($overDeleted.Count -eq 0) {
            Pass "texture exclusions only remove presets with a missing texture ($($matched.Count) files)"
        } else {
            Fail "texture exclusions also remove presets whose textures exist:"
            $overDeleted | ForEach-Object { Info "  $_" }
        }

        $leftover = @($textureDependent | Where-Object { $after -contains $_ })
        if ($leftover.Count -eq 0) {
            Pass "no texture-dependent preset survives the exclusion lists"
        } else {
            Fail "texture-dependent presets still shipped after the exclusion lists:"
            $leftover | ForEach-Object { Info "  $_" }
        }

        $rescan = Invoke-Scan $scratch $releaseTextures $collections -DryRun
        if ($rescan.Summary -match '^0 of') {
            Pass "rescan of the rehearsal tree: $($rescan.Summary)"
        } else {
            Fail "rescan of the rehearsal tree: $($rescan.Summary)"
        }
    } finally {
        Remove-Item -LiteralPath $scratch -Recurse -Force -ErrorAction SilentlyContinue
    }
}

function Invoke-Comparison {
    ''
    '== Release vs tracked Visualizer =='
    if (-not (Test-Path $releasePresets)) { Fail "release presets not found: $releasePresets"; return }
    if (-not (Test-Path $trackedPresets)) { Fail "tracked presets not found: $trackedPresets"; return }

    $hashMismatch = 0
    $countMismatch = 0
    foreach ($collection in $collections) {
        $releaseDir = Join-Path $releasePresets $collection
        $trackedDir = Join-Path $trackedPresets $collection
        $releaseFiles = @(Get-ChildItem -LiteralPath $releaseDir -Recurse -File -Filter *.milk)
        $trackedFiles = @(Get-ChildItem -LiteralPath $trackedDir -Recurse -File -Filter *.milk)
        Info ("{0,-14} Release={1,-6} Visualizer={2,-6}" -f $collection, $releaseFiles.Count, $trackedFiles.Count)
        if ($releaseFiles.Count -ne $trackedFiles.Count) { $countMismatch++ }

        $releaseHashes = @{}
        foreach ($file in $releaseFiles) {
            $relative = $file.FullName.Substring($releaseDir.Length).TrimStart('\')
            $releaseHashes[$relative] = (Get-FileHash -LiteralPath $file.FullName).Hash
        }
        foreach ($file in $trackedFiles) {
            $relative = $file.FullName.Substring($trackedDir.Length).TrimStart('\')
            if (-not $releaseHashes.ContainsKey($relative)) { $hashMismatch++; Info "  not in Release: $collection\$relative" }
            elseif ($releaseHashes[$relative] -ne (Get-FileHash -LiteralPath $file.FullName).Hash) { $hashMismatch++; Info "  differs: $collection\$relative" }
        }
    }
    if ($countMismatch -eq 0) { Pass "preset counts match for all four collections" } else { Fail "$countMismatch collection(s) with different preset counts" }
    if ($hashMismatch -eq 0) { Pass "every tracked preset is byte-identical to its Release twin" } else { Fail "$hashMismatch tracked preset(s) missing or different in Release" }

    if ($SkipScan) { Info 'texture scan skipped (-SkipScan)'; return }
    $scan = Invoke-Scan $trackedPresets $trackedTextures $collections -DryRun
    if ($scan.Summary -match '^0 of') { Pass "texture scan of the tracked tree: $($scan.Summary)" } else { Fail "texture scan of the tracked tree: $($scan.Summary)" }
}

"Milkwave preset sync verification"
"  repository: $RepoRoot"
"  upstream:   $Upstream"
"  copy script:$copyScript"

if ($Rehearse) { Invoke-Rehearsal }
if ($Compare) { Invoke-Comparison }

''
if ($script:failed) { 'RESULT: FAILED'; exit 1 } else { 'RESULT: OK'; exit 0 }
