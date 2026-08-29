@echo off
setlocal

REM Paths are resolved relative to this script's own location (%~dp0), so the
REM task works no matter what working directory VS Code runs it in.
REM   <scriptdir>            = C:\Source\Milkwave\Build\
REM   <scriptdir>..\Release  = C:\Source\Milkwave\Release          (deployment target)
REM   <scriptdir>..\..\MDropDX12\src\mDropDX12\Release_x64        (MDropDX12 build output)

REM If releasePath is not defined, use the default location
if not defined releasePath (
	set "releasePath=%~dp0..\Release"
)

if not defined mDropDX12Path (
	set "mDropDX12Path=%~dp0..\..\MDropDX12\src\mDropDX12\Release_x64"
)

if not exist "%mDropDX12Path%\MDropDX12.exe" (
	echo ERROR: %mDropDX12Path%\MDropDX12.exe not found. Build MDropDX12 Release first.
	exit /b 1
)

rmdir /s /q "%releasePath%\MDropDX12"
mkdir "%releasePath%\MDropDX12"
copy /y "%mDropDX12Path%\MDropDX12.exe" "%releasePath%\MDropDX12\"
endlocal
