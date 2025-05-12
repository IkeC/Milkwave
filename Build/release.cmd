set releasePath=..\Release
copy *.ini %releasePath%
copy *.txt %releasePath%
copy *.json %releasePath%
copy ..\Remote\bin\Release\net8.0-windows10.0.17763.0\MilkwaveRemote.exe %releasePath%
copy ..\Remote\bin\Release\net8.0-windows10.0.17763.0\MilkwaveRemote.dll %releasePath%
copy ..\Remote\bin\Release\net8.0-windows10.0.17763.0\MilkwaveRemote.runtimeconfig.json %releasePath%
copy ..\Remote\bin\Release\net8.0-windows10.0.17763.0\NAudio.Wasapi.dll %releasePath%
copy ..\Visualizer\vis_milk2\Release\MilkwaveVisualizer.exe %releasePath%
pause