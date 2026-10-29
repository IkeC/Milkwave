::@echo off
rem Create a release zip named Milkwave-Portable-<version>.zip using7-Zip.
rem This script stages files into a temp folder (excluding unwanted dirs) and archives that folder.

rem Resolve script and release directories
set "VERSION=4.2"

set "SCRIPT_DIR=%~dp0"
set "RELEASE_DIR=%SCRIPT_DIR%..\Release"
set "SEVENZ=C:\Program Files\7-Zip\7z.exe"

set "OUTPUT=%SCRIPT_DIR%\Milkwave-%VERSION%-Portable.zip"

for %%F in (MDropDX12.exe settings.ini messages.ini sprites.ini) do (
  if not exist "%RELEASE_DIR%\MDropDX12\%%F" (
    echo ERROR: Missing Release\MDropDX12\%%F
    echo Run Build\release.cmd before creating the zip.
    pause
    exit /b 1
  )
)

if exist "%OUTPUT%" del /f /q "%OUTPUT%" 2>nul

pushd "%RELEASE_DIR%"

"%SEVENZ%" a -tzip -mx=9 -mmt=on "%OUTPUT%" * ^
 -x!*.log ^
 -x!*error.txt ^
 -xr!log ^
 -xr!backup ^
 -xr!cache ^
 -xr!capture ^
 -x!resources\*.json ^
 -xr!resources\presets\CreamOfTheCrop ^
 -xr!resources\presets\Milkwave\Shader\Conv\* ^
 -xr!resources\presets\IkeC ^
 -xr!resources\presets\Quicksave ^
 -xr!resources\presets\Quicksave2

echo Created: "%OUTPUT%"
"%SEVENZ%" l "%OUTPUT%"

::pause
::cls
"%SEVENZ%" d "%OUTPUT%" "resources\shader"
"%SEVENZ%" d "%OUTPUT%" "resources\lyrics"
"%SEVENZ%" d "%OUTPUT%" "resources\presets\Milkwave\Shader\Conv"

popd
if /i not "%MILKWAVE_NO_PAUSE%"=="1" pause