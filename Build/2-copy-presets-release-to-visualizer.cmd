:: copy presets and textures from the Milkwave Release folder to the Visualizer codebase

set SRCDIR=..\Release\resources\presets
set DESTDIR=..\Visualizer\resources\presets

::presets
robocopy %SRCDIR%\BeatDrop %DESTDIR%\BeatDrop
robocopy %SRCDIR%\Butterchurn %DESTDIR%\Butterchurn
robocopy %SRCDIR%\Incubo_ %DESTDIR%\Incubo_
robocopy %SRCDIR%\Incubo_ Picks %DESTDIR%\Incubo_ Picks
robocopy %SRCDIR%\Milkdrop2077 %DESTDIR%\Milkdrop2077
robocopy %SRCDIR%\Milkwave %DESTDIR%\Milkwave
robocopy %SRCDIR%\Shader %DESTDIR%\Shader

:: textures
robocopy %SRCDIR%\..\textures %DESTDIR%\..\textures

pause