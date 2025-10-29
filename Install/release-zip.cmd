::@echo off
rem Create a release zip named Milkwave-Portable-<version>.zip using7-Zip.
rem This script stages files into a temp folder (excluding unwanted dirs) and archives that folder.

rem Resolve script and release directories
set "VERSION=XXX"

set "SCRIPT_DIR=%~dp0"
set "RELEASE_DIR=%SCRIPT_DIR%..\Release"
set "SEVENZ=C:\Program Files\7-Zip\7z.exe"

set "OUTPUT=%SCRIPT_DIR%\Milkwave-Portable-%VERSION%.zip"
if exist "%OUTPUT%" del /f /q "%OUTPUT%" 2>nul

pushd "%RELEASE_DIR%"

"%SEVENZ%" a -tzip -mx=9 -mmt=on "%OUTPUT%" * ^
 -xr!log ^
 -xr!backup ^
 -xr!cache ^
 -xr!resources\presets\Quicksave ^
 -xr!resources\presets\CreamOfTheCrop ^
 -xr!resources\presets\Milkwave\Shader\Conv\* ^
 -xr!resources\presets\IkeC 

echo Created: "%OUTPUT%"
"%SEVENZ%" l "%OUTPUT%"

::pause
::cls
"%SEVENZ%" d "%OUTPUT%" "resources\shader"
"%SEVENZ%" d "%OUTPUT%" "resources\presets\Milkwave\Shader\Conv"

popd
pause