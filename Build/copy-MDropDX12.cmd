@echo off
setlocal enabledelayedexpansion

REM If releasePath is not defined, use the default location
if not defined releasePath (
	set releasePath=.\Release
)

if not defined mDropDX12Path (
	set mDropDX12Path=..\MDropDX12\src\mDropDX12\Release_x64
)

rmdir /s /q %releasePath%\MDropDX12
mkdir %releasePath%\MDropDX12
copy %mDropDX12Path%\MDropDX12.exe %releasePath%\MDropDX12\
copy %mDropDX12Path%\settings.MDropDX12.ini %releasePath%\MDropDX12\settings.ini
endlocal
