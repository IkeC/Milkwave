set releasePath=..\Release
set backupPath=..\Release\backup
set remoteBuildPath=..\Remote\bin\Release\net8.0-windows10.0.17763.0
set visualizerBuildPath=..\Visualizer\vis_milk2\Release

copy %releasePath%\settings.ini %backupPath%\settings.ini.bak
copy %releasePath%\settings-remote.json %backupPath%\settings-remote.json.bak
copy %releasePath%\script-default.json %backupPath%\script-default.json.bak

copy *.ini %releasePath%
copy *.txt %releasePath%
copy settings-remote.json %releasePath%
copy %remoteBuildPath%\MilkwaveRemote.exe %releasePath%
copy %remoteBuildPath%\MilkwaveRemote.dll %releasePath%
copy %remoteBuildPath%\MilkwaveRemote.runtimeconfig.json %releasePath%
copy %remoteBuildPath%\NAudio.Wasapi.dll %releasePath%
copy %visualizerBuildPath%\MilkwaveVisualizer.exe %releasePath%

pause