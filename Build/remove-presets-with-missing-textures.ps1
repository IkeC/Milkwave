<#
.SYNOPSIS
    Deletes the presets in a Milkwave release folder that reference textures we don't ship.

.DESCRIPTION
    Presets that sample a texture which is neither bundled in resources\textures nor one
    of MilkDrop's generated textures render with a broken texture and report a "could not
    load texture" error at runtime, so they are removed here. Sampler declarations that
    the shader compiler optimizes away are ignored, because those never get loaded.

    The texture-name resolution mirrors CShaderParams::CacheParams() in
    Visualizer\milkwave\plugin_shaders.cpp:
      * the "sampler_" prefix is stripped,
      * a two letter filtering prefix such as "pw_"/"fc_" is stripped,
      * main/blurN/fft and the generated noise* textures are built in,
      * "randNN", "randNN_prefix" and "[randNN] prefix*" pick files from the
        textures folder at runtime and are only a problem when no file matches,
      * everything else must exist as jpg/jpeg/jfif/dds/png/tga/bmp/dib in
        resources\textures (or next to the presets).

.PARAMETER PresetRoot
    The preset folder of the release, for example Release\resources\presets.

.PARAMETER TextureRoot
    The texture folder of the release, for example Release\resources\textures.

.PARAMETER Folder
    Names of the preset sub folders to scan, "." for the preset root itself.
    Defaults to the preset root and every sub folder.

.PARAMETER DryRun
    Only report what would be deleted.

.EXAMPLE
    .\remove-presets-with-missing-textures.ps1 -PresetRoot ..\Milkwave\Release\resources\presets -TextureRoot ..\Milkwave\Release\resources\textures -DryRun
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$PresetRoot,
    [Parameter(Mandatory = $true)][string]$TextureRoot,
    [string[]]$Folder = @(),
    [switch]$DryRun
)

$ErrorActionPreference = 'Stop'

# "powershell -File" hands over comma separated values as a single string, so split them
if ($Folder.Count -eq 1 -and $Folder[0].Contains(',')) {
    $Folder = $Folder[0].Split(',')
}

$textureExtensions = @('jpg', 'jpeg', 'jfif', 'dds', 'png', 'tga', 'bmp', 'dib')

# textures MilkDrop generates (or binds) itself; see plugin_shaders.cpp / plugin_dx9.cpp
$builtinTextures = @(
    'main', 'fft',
    'blur1', 'blur2', 'blur3', 'blur4', 'blur5', 'blur6',
    'noise_lq', 'noise_lq_lite', 'noise_mq', 'noise_hq',
    'noisevol_lq', 'noisevol_hq'
)

$samplerRegex = [regex]'sampler_([A-Za-z0-9_]+)'
$texsizeRegex = [regex]'texsize_([A-Za-z0-9_]+)'
$defineRegex = [regex]'#\s*define\s+((?:sampler|texsize)_[A-Za-z0-9_]+)'
$randRegex = [regex]'^rand(\d\d)(?:_(.*))?$'
# "warp_1=`sampler sampler_x;" or "sampler sampler_x = sampler_state {" - declaration only
$declarationRegex = [regex]'^\s*(?:[A-Za-z0-9_]+\s*=\s*`)?\s*(?:sampler|sampler1D|sampler2D|sampler3D|samplerCUBE)\s+(?:[A-Za-z0-9_]+\s*,\s*)*$'
$texsizeDeclarationRegex = [regex]'^\s*(?:[A-Za-z0-9_]+\s*=\s*`)?\s*(?:float|float2|float3|float4|half|half2|half3|half4|vector)\s+(?:texsize_[A-Za-z0-9_]+\s*,\s*)*$'
# "texsize_x = ..." is a value the preset brings along, not an engine lookup
$texsizeDefinitionRegex = [regex]'texsize_([A-Za-z0-9_]+)\s*='

# the two kinds of texture references CShaderParams::CacheParams() resolves
$textureReferences = @(
    # a sampled texture; the filtering prefix (pw_/fc_/...) is peeled off
    @{ Name = 'sampler'; Regex = $samplerRegex; Declaration = $declarationRegex; StripPrefix = $true }
    # a texsize_* parameter, which is bound to an already loaded texture; no prefix peeling
    @{ Name = 'texsize'; Regex = $texsizeRegex; Declaration = $texsizeDeclarationRegex; StripPrefix = $false }
)

$textureNames = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
$textureFileNames = [System.Collections.Generic.List[string]]::new()
foreach ($dir in @($TextureRoot, $PresetRoot)) {
    if (-not (Test-Path -LiteralPath $dir)) { continue }
    foreach ($file in Get-ChildItem -LiteralPath $dir -File) {
        if ($textureExtensions -notcontains $file.Extension.TrimStart('.').ToLowerInvariant()) { continue }
        [void]$textureNames.Add($file.BaseName)
        $textureFileNames.Add($file.Name)
    }
}

function Test-RandomTexturePrefix([string]$prefix) {
    # PickRandomTexture() matches the prefix against the file names, extension included.
    foreach ($name in $textureFileNames) {
        if ($name.Length -ge $prefix.Length -and $name.Substring(0, $prefix.Length).Equals($prefix, [System.StringComparison]::OrdinalIgnoreCase)) {
            return $true
        }
    }
    return $false
}

if ($Folder.Count -gt 0) {
    $presetDirs = foreach ($name in $Folder) {
        if ($name -eq '.') { $PresetRoot } else { Join-Path $PresetRoot $name }
    }
    $presetDirs = $presetDirs | Where-Object { Test-Path -LiteralPath $_ }
} else {
    $presetDirs = @($PresetRoot) + (Get-ChildItem -LiteralPath $PresetRoot -Directory).FullName
}

$presets = foreach ($dir in $presetDirs) {
    if ($dir -eq $PresetRoot) {
        # the copy step mirrors the top level of the preset folder into its own sub folder
        Get-ChildItem -LiteralPath $dir -File -Filter *.milk
    } else {
        Get-ChildItem -LiteralPath $dir -Recurse -File -Filter *.milk
    }
}

$removed = [System.Collections.Generic.List[object]]::new()
$missingTextures = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)

foreach ($preset in $presets) {
    $lines = [System.IO.File]::ReadAllLines($preset.FullName)

    $defines = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
    # presets may provide their own "static const float4 texsize_x = ..."; those never
    # reach the engine, so they are not a texture size lookup
    $ownTexsize = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
    $missing = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)

    # first pass: collect macro aliases and preset supplied texsize constants
    foreach ($line in $lines) {
        if ($line.IndexOf('#define', [System.StringComparison]::Ordinal) -ge 0) {
            foreach ($match in $defineRegex.Matches($line)) { [void]$defines.Add($match.Groups[1].Value) }
        }
        if ($line.IndexOf('=', [System.StringComparison]::Ordinal) -ge 0 -and
            $line.IndexOf('texsize_', [System.StringComparison]::Ordinal) -ge 0) {
            foreach ($match in $texsizeDefinitionRegex.Matches($line)) { [void]$ownTexsize.Add($match.Groups[1].Value) }
        }
    }

    foreach ($line in $lines) {
        # drop line comments and "/* ... */" fragments, they are not compiled
        $code = $line
        $comment = $code.IndexOf('//')
        if ($comment -ge 0) { $code = $code.Substring(0, $comment) }
        $code = [regex]::Replace($code, '/\*.*?\*/', '')
        if ($code.Length -eq 0) { continue }

        # "sampler_state { ... }" is an HLSL keyword, not a texture
        $code = $code.Replace('sampler_state', '')

        foreach ($reference in $textureReferences) {
            if ($code.IndexOf($reference.Name, [System.StringComparison]::OrdinalIgnoreCase) -lt 0) { continue }

            foreach ($match in $reference.Regex.Matches($code)) {
                if ($defines.Contains($match.Value)) { continue }  # alias, e.g. "#define sampler_pic sampler_cells"
                if ($reference.Name -eq 'texsize' -and $ownTexsize.Contains($match.Groups[1].Value)) { continue }
                # unused declarations are optimized away by the shader compiler and never
                # trigger a texture load, so only real references matter here
                if ($reference.Declaration.IsMatch($code.Substring(0, $match.Index))) { continue }

                $root = $match.Groups[1].Value
                if ($reference.StripPrefix -and $root.Length -gt 3 -and $root[2] -eq '_') { $root = $root.Substring(3) }
                if ($builtinTextures -contains $root) { continue }

                $rand = $randRegex.Match($root)
                if ($rand.Success -and [int]$rand.Groups[1].Value -le 15) {
                    $prefix = $rand.Groups[2].Value
                    if ($prefix.Length -eq 0 -or (Test-RandomTexturePrefix $prefix)) { continue }
                } elseif ($textureNames.Contains($root)) {
                    continue
                }

                [void]$missing.Add($root)
            }
        }
    }

    if ($missing.Count -eq 0) { continue }

    foreach ($name in $missing) { [void]$missingTextures.Add($name) }
    $removed.Add([pscustomobject]@{
        Preset   = $preset.FullName.Substring($PresetRoot.Length).TrimStart('\')
        Textures = ($missing | Sort-Object) -join ', '
    })
    if (-not $DryRun) { Remove-Item -LiteralPath $preset.FullName -Force }
}

$removed = $removed | Sort-Object Preset
foreach ($entry in $removed) {
    '{0}  ==>  {1}' -f $entry.Preset, $entry.Textures
}

'{0} of {1} presets {2}, {3} distinct textures missing.' -f `
    $removed.Count, $presets.Count, $(if ($DryRun) { 'would be removed' } else { 'removed' }), $missingTextures.Count
