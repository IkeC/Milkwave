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
robocopy "%SRCDIR%\Incubo_'s Picks" "%DESTDIR%\Incubo_ Picks"
robocopy "%SRCDIR%\Milkdrop2077 Presets" "%DESTDIR%\Milkdrop2077"

::exclusions, often because preset is using big textures that would bloat the release
cd "%DESTDIR%"
SET CMD=del /s /q
%CMD% "*Gillman*" "*Violetta*" "*Watery*" "*Grizelda*" "*Octobella*" "*Nerissa*" "*Spooky*" "*Abbie*" "*Chelsea*" "*Lucious*" "*Orticia*"
%CMD% "*Dancing Saber*" "*BPM Code Test*" "*Bing AI*Try*"
%CMD% "Aderrasi - Bitterfeld (Crystal Border Mix) - [Jian Simanjuntak with forest tree edit].milk"
%CMD% "*Altars Of Madness 2 (Cold Snap Edit)*"
pause

::textures
::we should only cherry-pick the required ones
::
::robocopy %SRCDIR%\textures %DESTDIR%\..\textures
