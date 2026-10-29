<#
.SYNOPSIS
    Builds the latest projectM-eval release and updates the Milkwave Visualizer libraries.

.DESCRIPTION
    Exports a clean copy of the newest version tag from the projectM-eval repository,
    builds the Win32 Release static libraries with the same configuration Milkwave uses
    (x86, /MD, double precision, fast math, ns-eel2 shim), verifies the artifacts and
    copies them into Visualizer\lib. The ns-eel2 shim header is updated when its
    contents changed; BOM and line-ending differences alone do not count as a change.

.PARAMETER ProjectMEvalPath
    Path to the projectM-eval Git checkout. Defaults to C:\Source\projectm-eval.

.PARAMETER Ref
    Git tag, branch or commit to build. Defaults to the newest vX.Y.Z tag.

.PARAMETER MilkwaveRoot
    Milkwave repository root. Defaults to the parent directory of this script.

.PARAMETER SkipFetch
    Do not fetch tags from the projectM-eval remotes before selecting the latest tag.

.PARAMETER KeepTemp
    Keep the temporary export/build directory for inspection.

.EXAMPLE
    .\Build\update-projectm-libs.ps1

.EXAMPLE
    .\Build\update-projectm-libs.ps1 -Ref v1.0.8
#>
[CmdletBinding()]
param(
    [string]$ProjectMEvalPath = '',
    [string]$Ref = '',
    [string]$MilkwaveRoot = '',
    [switch]$SkipFetch,
    [switch]$KeepTemp
)

$ErrorActionPreference = 'Stop'

if ([string]::IsNullOrWhiteSpace($MilkwaveRoot)) {
    $MilkwaveRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
}

if ([string]::IsNullOrWhiteSpace($ProjectMEvalPath)) {
    $ProjectMEvalPath = [IO.Path]::GetFullPath((Join-Path $MilkwaveRoot '..\projectm-eval'))
}

$ProjectMEvalPath = [IO.Path]::GetFullPath($ProjectMEvalPath)
$libTargetDir = Join-Path $MilkwaveRoot 'Visualizer\lib'
$shimHeaderTarget = Join-Path $MilkwaveRoot 'Visualizer\ns-eel2-shim\ns-eel.h'

foreach ($tool in @('git', 'cmake')) {
    if (-not (Get-Command $tool -ErrorAction SilentlyContinue)) {
        throw "Required tool '$tool' was not found in PATH."
    }
}

if (-not (Test-Path (Join-Path $ProjectMEvalPath '.git'))) {
    throw "projectM-eval Git repository not found at '$ProjectMEvalPath'."
}

if (-not (Test-Path $libTargetDir)) {
    throw "Milkwave library directory not found at '$libTargetDir'."
}

if (-not $SkipFetch) {
    Write-Host 'Fetching projectM-eval tags...'
    & git -C $ProjectMEvalPath fetch --all --tags --prune --quiet
    if ($LASTEXITCODE -ne 0) {
        Write-Warning 'git fetch failed; continuing with the locally available tags.'
    }
}

if ([string]::IsNullOrWhiteSpace($Ref)) {
    $tags = @(& git -C $ProjectMEvalPath tag --list 'v*')
    $Ref = $tags |
        Where-Object { $_ -match '^v\d+\.\d+\.\d+$' } |
        Sort-Object { [version]($_ -replace '^v', '') } -Descending |
        Select-Object -First 1

    if ([string]::IsNullOrWhiteSpace($Ref)) {
        $Ref = 'HEAD'
        Write-Warning 'No version tag found; building HEAD instead.'
    }
}
$Ref = "$Ref".Trim()

$resolvedRef = & git -C $ProjectMEvalPath rev-parse --verify "$Ref^{commit}" 2>$null
$revParseExitCode = $LASTEXITCODE
if ($revParseExitCode -ne 0 -or [string]::IsNullOrWhiteSpace($resolvedRef)) {
    throw "projectM-eval ref '$Ref' could not be resolved."
}
$resolvedRef = @($resolvedRef)[0].Trim()

# Keep the build outside %TEMP%: MSBuild warns when intermediate/output folders live there.
$workRoot = Join-Path ([Environment]::GetFolderPath('LocalApplicationData')) ('Milkwave\projectm-eval-' + [Guid]::NewGuid().ToString('N').Substring(0, 8))
$sourceDir = Join-Path $workRoot 'src'
$buildDir = Join-Path $workRoot 'build'
$archivePath = Join-Path $workRoot 'source.zip'

try {
    New-Item -ItemType Directory -Path $workRoot -Force | Out-Null

    Write-Host "Exporting projectM-eval $Ref ($($resolvedRef.Substring(0, 12)))..."
    & git -C $ProjectMEvalPath archive --format=zip -o $archivePath $resolvedRef
    if ($LASTEXITCODE -ne 0) {
        throw 'git archive failed.'
    }
    Expand-Archive -LiteralPath $archivePath -DestinationPath $sourceDir -Force

    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (-not (Test-Path $vswhere)) {
        throw 'vswhere.exe was not found; Visual Studio with MSBuild is required.'
    }

    $vsPath = (& $vswhere -latest -products * -requires Microsoft.Component.MSBuild -property installationPath | Select-Object -First 1)
    $vsVersion = (& $vswhere -latest -products * -requires Microsoft.Component.MSBuild -property installationVersion | Select-Object -First 1)
    if ([string]::IsNullOrWhiteSpace($vsPath) -or [string]::IsNullOrWhiteSpace($vsVersion)) {
        throw 'No Visual Studio installation with MSBuild was found.'
    }

    $generator = switch -Regex ($vsVersion) {
        '^18\.' { 'Visual Studio 18 2026'; break }
        '^17\.' { 'Visual Studio 17 2022'; break }
        '^16\.' { 'Visual Studio 16 2019'; break }
        default { throw "Unsupported Visual Studio version '$vsVersion'." }
    }

    Write-Host "Configuring projectM-eval with $generator (Win32, Release)..."
    & cmake -S $sourceDir -B $buildDir `
        -G $generator -A Win32 `
        -DBUILD_TESTING=OFF `
        -DBUILD_NS_EEL_SHIM=ON `
        -DPROJECTM_EVAL_FLOAT_SIZE=8 `
        -DENABLE_FAST_MATH=ON `
        -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL
    if ($LASTEXITCODE -ne 0) {
        throw 'CMake configuration failed.'
    }

    Write-Host 'Building projectM_eval and projectM_ns-eel2...'
    & cmake --build $buildDir --config Release --target projectM_eval projectM_ns-eel2 --parallel
    if ($LASTEXITCODE -ne 0) {
        throw 'projectM-eval build failed.'
    }

    $evalLib = Join-Path $buildDir 'projectm-eval\Release\projectM_eval.lib'
    $shimLib = Join-Path $buildDir 'ns-eel2-shim\Release\projectM_ns-eel2.lib'
    if (-not (Test-Path $evalLib) -or -not (Test-Path $shimLib)) {
        throw 'The expected projectM-eval libraries were not produced.'
    }

    $dumpbin = Get-ChildItem (Join-Path $vsPath 'VC\Tools\MSVC\*\bin\Hostx86\x86\dumpbin.exe') -ErrorAction SilentlyContinue |
        Sort-Object FullName -Descending |
        Select-Object -First 1
    if (-not $dumpbin) {
        throw 'dumpbin.exe was not found in the selected Visual Studio installation.'
    }

    foreach ($library in @($evalLib, $shimLib)) {
        $headers = (& $dumpbin.FullName /HEADERS $library | Out-String)
        if ($headers -notmatch 'machine \(x86\)') {
            throw "Library is not x86: $library"
        }
        $directives = (& $dumpbin.FullName /DIRECTIVES $library | Out-String)
        if ($directives -notmatch 'DEFAULTLIB:MSVCRT') {
            throw "Library does not use the release /MD runtime: $library"
        }
        if ($directives -match 'DEFAULTLIB:MSVCRTD') {
            throw "Library uses the debug runtime: $library"
        }
    }

    $shimHeaderSource = Join-Path $sourceDir 'ns-eel2-shim\ns-eel.h'
    $headerNeedsUpdate = $true
    if (Test-Path $shimHeaderTarget) {
        $sourceText = [IO.File]::ReadAllText($shimHeaderSource).TrimStart([char]0xFEFF).Replace("`r`n", "`n")
        $targetText = [IO.File]::ReadAllText($shimHeaderTarget).TrimStart([char]0xFEFF).Replace("`r`n", "`n")
        $headerNeedsUpdate = ($sourceText -cne $targetText)
    }
    if ($headerNeedsUpdate) {
        Copy-Item -LiteralPath $shimHeaderSource -Destination $shimHeaderTarget -Force
        Write-Host "Updated $shimHeaderTarget"
    }

    foreach ($pair in @(
        @{ Source = $evalLib; Target = (Join-Path $libTargetDir 'projectM_eval.lib') },
        @{ Source = $shimLib; Target = (Join-Path $libTargetDir 'projectM_ns-eel2.lib') }
    )) {
        $oldSize = if (Test-Path $pair.Target) { (Get-Item -LiteralPath $pair.Target).Length } else { 0 }
        $newSize = (Get-Item -LiteralPath $pair.Source).Length
        Copy-Item -LiteralPath $pair.Source -Destination $pair.Target -Force
        Write-Host ("Updated {0} ({1} -> {2} bytes)" -f (Split-Path $pair.Target -Leaf), $oldSize, $newSize)
    }

    Write-Host ''
    Write-Host "projectM-eval $Ref libraries installed successfully."
    Write-Host "Rebuild the Visualizer to link against them."
}
finally {
    if ($KeepTemp) {
        Write-Host "Temporary files kept at: $workRoot"
    }
    elseif (Test-Path $workRoot) {
        Remove-Item -LiteralPath $workRoot -Recurse -Force -ErrorAction SilentlyContinue
    }
}
