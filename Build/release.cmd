set releasePath=..\Release
set remoteBuildPath=..\Remote\bin\Release\net8.0-windows10.0.17763.0
set visualizerBuildPath=..\Visualizer\vis_milk2\Release

copy *.ini %releasePath%
copy *.txt %releasePath%
copy settings-remote.json %releasePath%
copy %remoteBuildPath%\MilkwaveRemote.exe %releasePath%
copy %remoteBuildPath%\MilkwaveRemote.dll %releasePath%
copy %remoteBuildPath%\MilkwaveRemote.runtimeconfig.json %releasePath%
copy %remoteBuildPath%\NAudio.Wasapi.dll %releasePath%
copy %visualizerBuildPath%\MilkwaveVisualizer.exe %releasePath%

pause