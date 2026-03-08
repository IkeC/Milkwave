::@echo off
rem Create a release zip named Milkwave-Portable-<version>.zip using 7-Zip.
rem This script excludes unwanted directories and includes only textures used by Milkwave presets.

rem Resolve script and release directories
set "VERSION=3.6.XXX"

set "SCRIPT_DIR=%~dp0"
set "RELEASE_DIR=%SCRIPT_DIR%..\Release"
set "SEVENZ=C:\Program Files\7-Zip\7z.exe"

set "OUTPUT=%SCRIPT_DIR%\Milkwave-%VERSION%-Minimal.zip"
if exist "%OUTPUT%" del /f /q "%OUTPUT%" 2>nul

pushd "%RELEASE_DIR%"

echo Creating base archive (excluding all textures)...
"%SEVENZ%" a -tzip -mx=9 -mmt=on "%OUTPUT%" * ^
 -x!*.log ^
 -x!*error.txt ^
 -xr!log ^
 -xr!backup ^
 -xr!cache ^
 -xr!capture ^
 -xr!resources\textures\* ^
 -xr!resources\presets\BeatDrop ^
 -xr!resources\presets\Butterchurn ^
 -xr!resources\presets\CreamOfTheCrop ^
 -xr!resources\presets\IkeC ^
 -xr!resources\presets\Incubo_ ^
 "-xr!resources\presets\Incubo_ Picks" ^
 -xr!resources\presets\Milkdrop2077 ^
 -xr!resources\presets\Milkwave\Shader\Conv\* ^
 -xr!resources\presets\Quicksave ^
 -xr!resources\presets\Quicksave2 ^
 -xr!resources\shader

echo.
echo Adding textures used by Milkwave presets...
"%SEVENZ%" a "%OUTPUT%" "resources\textures\grace.jpg"
"%SEVENZ%" a "%OUTPUT%" "resources\textures\heartfelt2.jpg"
"%SEVENZ%" a "%OUTPUT%" "resources\textures\stoy_cubemap_uffizi_blurred.png"
"%SEVENZ%" a "%OUTPUT%" "resources\textures\stoy_london.jpg"
"%SEVENZ%" a "%OUTPUT%" "resources\textures\stoy_noise.jpg"
"%SEVENZ%" a "%OUTPUT%" "resources\textures\stoy_nyancat.png"
"%SEVENZ%" a "%OUTPUT%" "resources\textures\stoy_pebbles.png"

echo.
echo Created: "%OUTPUT%"
echo.
echo Archive contents:
"%SEVENZ%" l "%OUTPUT%"

popd
pause