@echo off
setlocal

REM Builds both release archives without stopping at the scripts' interactive pause.
set "MILKWAVE_NO_PAUSE=1"

call "%~dp0release-zip.cmd"
if errorlevel 1 exit /b %errorlevel%

call "%~dp0release-zip-minimal.cmd"
exit /b %errorlevel%
