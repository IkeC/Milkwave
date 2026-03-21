set releasePath=..\Release
set backupPath=..\Release\backup
set remoteBuildPath=..\Remote\bin\Release\net8.0-windows10.0.17763.0
set visualizerSourcePath=..\Visualizer
set visualizerBuildPath=..\Visualizer\vis_milk2\Release

copy %releasePath%\*.ini %backupPath%
copy %releasePath%\*.json %backupPath%
copy %releasePath%\*.txt %backupPath%

del /q %releasePath%\capture\*.*
del /q %releasePath%\presetdeck-remote.json
del /q %releasePath%\midi-remote.json
del /q %releasePath%\resources\buttons\btn-0*.png
del /q %releasePath%\resources\buttons\btn-1*.png
del /q %releasePath%\resources\buttons\btn-2*.png
del /q %releasePath%\resources\buttons\btn-3*.png
del /q %releasePath%\resources\buttons\btn-4*.png

set mDropDX12Path=..\..\MDropDX12\src\mDropDX12\Release_x64
call copy-MDropDX12.cmd

copy settings.ini %releasePath%
copy MDropDX12\*.ini %releasePath%\MDropDX12

copy *.txt %releasePath%
copy settings-remote.json %releasePath%
copy controller-remote.json %releasePath%
copy messages-editor.html %releasePath%
copy %remoteBuildPath%\MilkwaveRemote.exe %releasePath%
copy %remoteBuildPath%\MilkwaveRemote.dll %releasePath%
copy %remoteBuildPath%\MilkwaveRemote.runtimeconfig.json %releasePath%
copy %remoteBuildPath%\NAudio.Core.dll %releasePath%
copy %remoteBuildPath%\NAudio.Wasapi.dll %releasePath%
copy %remoteBuildPath%\NAudio.Midi.dll %releasePath%
copy %remoteBuildPath%\runtimes\win\lib\net8.0\System.Management.dll %releasePath%
copy %visualizerBuildPath%\MilkwaveVisualizer.exe %releasePath%

copy %visualizerSourcePath%\resources\sprites\cover.png %releasePath%\resources\sprites\cover.png

pause