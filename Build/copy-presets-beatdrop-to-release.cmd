:: copy updated presets to the Milkwave Release folder

set SRCDIR=resources\Milkdrop2\presets
set DESTDIR=..\Milkwave\Release\resources\presets

cd ..\..\BeatDrop-Music-Visualizer
git pull
pause

::presets
robocopy %SRCDIR% %DESTDIR%\BeatDrop /LEV:1
robocopy "%SRCDIR%\Butterchurn Presets" "%DESTDIR%\Butterchurn"
robocopy "%SRCDIR%\Incubo_'s Presets" "%DESTDIR%\Incubo_"
robocopy "%SRCDIR%\Incubo_ Picks" "%DESTDIR%\Incubo_ Picks"
robocopy "%SRCDIR%\Milkdrop2077 Presets" "%DESTDIR%\Milkdrop2077"

:: textures
:: we should cherry-pick the required ones
::robocopy %SRCDIR%\textures %DESTDIR%\..\textures

pause 