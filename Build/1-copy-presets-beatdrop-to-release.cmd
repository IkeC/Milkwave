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
robocopy "%SRCDIR%\Milkdrop2077 Presets" "%DESTDIR%\Milkdrop2077"

::exclusions, often because preset is using big textures that would bloat the release
cd "%DESTDIR%"
SET CMD=del /s /q
%CMD% "*Gillman*" "*Violetta*" "*Watery*" "*Grizelda*" "*Octobella*" "*Nerissa*" "*Spooky*" "*Abbie*" "*Chelsea*" "*Lucious*" "*Orticia*"
%CMD% "*Dancing Saber*" "*BPM Code Test*" "*Bing AI*Try*"
%CMD% "Aderrasi - Bitterfeld (Crystal Border Mix) - [Jian Simanjuntak with forest tree edit].milk"
%CMD% "Anandamide - Capitalist_Cathedral_Esper (Se7enSlasher's Composition Fix).milk"
%CMD% "*Altars Of Madness 2 (Cold Snap Edit)*"
%CMD% "*Jian Simanjuntak - Particle Nights in Jakarta*"
%CMD% "*Jetplane Military*"
%CMD% "*Tesla Coil*"

::exclusions, because the preset samples its own texture, which we don't ship
::(kept explicit so the result does not depend on the texture scan below)
%CMD% "adrian-sherman - JoltSource.milk" "Anandamide - Capitalist_Cathedral_Esper.milk" "Anandamide, ForceField - DiMethylTryptamine.milk"
%CMD% "Cope - Keanu Reeves is a bad actor (sharky mix).milk" "froth.milk" "Nivush - Bouncing Sounds On Water.milk" "ORB - Flying High.milk"
%CMD% "Geiss - Thumb Drum - [Jian Simanjuntak composite shader mod].milk" "Zylot - Blue Jewel.milk" "Zylot - We have surely lost our way.milk"
%CMD% "Flexi - 100*shader fractal*Jian Simanjuntak mod remix*"
%CMD% "Jian Simanjuntak - Alien Building Skyscraper.milk" "Jian Simanjuntak - Disco Ball Party.milk" "Jian Simanjuntak - Discotheque Architecture.milk"
%CMD% "Jian Simanjuntak - Landscapes Valley Invert Effects.milk" "Jian Simanjuntak - Nebula Space Environment 2.milk"
%CMD% "Jian Simanjuntak - Painting Fantastic - Astratta 562.milk" "Jian Simanjuntak - The Fractal In Hexagons (cope and flexi's sphere code remix).milk"
%CMD% "Jian Simanjuntak - The Observable Universe.milk" "Jian Simanjuntak + Flexi - Age Of Shading Chaos - [cell zoom edit].milk"
%CMD% "martin - beat det - exact BPM using DFT.milk" "martin - face down ass up.milk" "martin - funky illusions - *"
%CMD% "martin - harmony of colours MilkDrop2077 rmx3.milk" "martin + Se7enSlasher - rainbow spectrum in the mix3.milk"
%CMD% "*MilkDrop2077 - diamond lattice2*"
%CMD% "Se7enSlasher - DVD Logo Screensaver.milk" "Se7enSlasher - TCL Logo.milk" "Se7enSlasher - Trap Nation Visualizer.milk"
%CMD% "Se7enSlasher + Adrian Eddy + flight404 - Main Sequence Star.milk"
%CMD% "Se7enSlasher + Gemini - Laser Show - Chaotic and Epic Laser Beams (Digitally Distorted Remix).milk"
%CMD% "Se7enSlasher + Gemini - Laser Show - Chaotic and Epic Laser Beams (Digitally Distorted Remix) - Alternate Version.milk"
%CMD% "Se7enSlasher + Gemini - Laser Show - Chaotic and Epic Laser Beams (Random Texture Spin Trip Remix).milk"
%CMD% "Se7enSlasher + Jian Simanjuntak + flexi + Geiss + martin - Sweet Light Lines of Maari.milk"
%CMD% "Se7enSlasher + Jian Simanjuntak + Others - Sound Spectrum Floor.milk"
%CMD% "fishbrain+SFR -- S.O.C.S.v12 -- *" "SFR -- S.O.C.S.v12 -- 4Way Whisper_a.milk"

pause

::safety net: catches preset/texture combinations added upstream after this list was written
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0remove-presets-with-missing-textures.ps1" -PresetRoot "%~dp0..\Release\resources\presets" -TextureRoot "%~dp0..\Release\resources\textures" -Folder "BeatDrop,Butterchurn,Incubo_,Milkdrop2077"
pause

::textures
::we should only cherry-pick the required ones
::
::robocopy %SRCDIR%\textures %DESTDIR%\..\textures
