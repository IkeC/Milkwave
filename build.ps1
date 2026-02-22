# build.ps1 — Invoke from any shell (Git Bash, cmd, Claude's Bash tool, etc.)
# Usage:  powershell -ExecutionPolicy Bypass -File build.ps1 [Debug|Release] [Clean]
#
# Examples:
#   powershell -ExecutionPolicy Bypass -File build.ps1            # Debug build
#   powershell -ExecutionPolicy Bypass -File build.ps1 Release    # Release build
#   powershell -ExecutionPolicy Bypass -File build.ps1 Debug Clean # Clean then build

param(
    [string]$Configuration = "Debug",
    [string]$Target        = "Build"   # "Build" or "Clean"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

# ── 1. Locate vswhere ──────────────────────────────────────────────────────────
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (-not (Test-Path $vswhere)) {
    $vswhere = "${env:ProgramFiles}\Microsoft Visual Studio\Installer\vswhere.exe"
}
if (-not (Test-Path $vswhere)) {
    Write-Error "vswhere.exe not found. Install Visual Studio 2022 or later."
    exit 1
}

# ── 2. Find MSBuild ────────────────────────────────────────────────────────────
$msbuild = & $vswhere -latest -requires Microsoft.Component.MSBuild `
           -find "MSBuild\**\Bin\MSBuild.exe" | Select-Object -First 1

if (-not $msbuild -or -not (Test-Path $msbuild)) {
    Write-Error "MSBuild not found. Ensure Visual Studio 2022 with C++ workload is installed."
    exit 1
}

Write-Host "MSBuild : $msbuild"
Write-Host "Config  : $Configuration"
Write-Host "Target  : $Target"
Write-Host ""

# ── 3. Run the build ───────────────────────────────────────────────────────────
$project = Join-Path $PSScriptRoot "Visualizer\vis_milk2\plugin.vcxproj"

& $msbuild $project `
    /t:$Target `
    /p:Configuration=$Configuration `
    /p:Platform=Win32 `
    /p:PlatformToolset=v143 `
    /m `
    /nologo `
    /clp:Summary

exit $LASTEXITCODE
