## v4.2 (unreleased)

* Visualizer: Desktop Mode (Ctrl+F9) renders the visualizer behind the desktop icons, replacing the wallpaper
* Visualizer: Desktop Mode keeps the visualizer in the taskbar; the taskbar right-click menu offers "Toggle Desktop Mode"
* Visualizer: Taskbar thumbnail toolbar with buttons for the previous preset, the opacity (5% steps), the preset lock and the next preset
  - The lock button shows the current state and toggles the preset lock (same as the `~` key)
* Visualizer: Taskbar right-click menu also offers "Open Milkwave Remote" (starts it, or brings the running Remote to the front)
* Visualizer: Desktop Mode has its own window opacity (settings.ini: WindowDesktopModeOpacity, adjustable with the mouse wheel or the taskbar buttons)
* Visualizer: Desktop Mode is restored on the next start when the Visualizer was closed in Desktop Mode (settings.ini: WindowDesktopMode)
* Visualizer: Leaving Desktop Mode restores the normal window opacity reliably, also for opacities below 100%. Windows bakes the Desktop Mode alpha into the window surface while the window is layered, and that alpha survived hiding/showing, the device reset, style changes and further opacity changes - the visualizer stayed translucent (e.g. 0.5 * 0.98) instead of returning to the opacity it had before. The layered style is now dropped before the window is hidden and re-applied afterwards, so the surface is built again without the Desktop Mode alpha. The text margin of the top and bottom corner texts is restored as well, so the preset name and notifications stay the same distance from the window edge as before
* Visualizer: F4 no longer closes the visualizer when Windows reports it as a system key without Alt being pressed; F4 toggles the preset info display as usual, Alt+F4 still closes the window
* Visualizer: The shader cache (_cache_ folder) is always created next to the executable, not in the working directory the Visualizer was started from. Starting it with a different working directory (e.g. the repository root) created a stray _cache_ folder there and ignored the shaders precompiled into the application folder
* Visualizer: _precompile.txt_ is looked up in the application directory as well, and relative preset paths in it are resolved from there, so shader precompilation works no matter which working directory the Visualizer was started from
* Remote: Messages read from the pipe are reassembled before they are evaluated. A message that arrived split across two reads was passed on as a fragment, which handed empty text to numeric settings fields and raised "'The input string '' was not in a correct format'"
* Visualizer: Clickthrough keeps the opacity of the active mode instead of re-applying a stale value, Transparency Mode (colour key) is only restored when the window really used it before Desktop Mode, and a window that needs no transparency (100% opacity, no Clickthrough, no Transparency Mode) is not kept as a layered window
* Visualizer: In Desktop Mode the bottom screen texts (song info, FPS and similar) stay above the taskbar
* Visualizer: 'B' toggles a permanently visible song info display (settings.ini: ShowSongInfo); the automatic display on track changes is unaffected. Switching the song info polling off (Ctrl+B) hides the pinned display as well, and the notification then reports that instead of "on track change only"
* Visualizer: Borderless Fullscreen is no longer bound to a hotkey (still available as right-click function of Remote button 14)
* Remote: Button 15 now defaults to Desktop Mode (Ctrl+F9) instead of Transparency
* Remote: Button 13 (Clear Sprite/Msg) now defaults to "Clear all Sprites" (Ctrl+K) on right-click
* Remote: Preset change and info buttons with improved locking logic dependency
* Remote: Command button panel configurable via the [Buttons] section of _settings.ini_ (captions, commands and right-click commands with tooltips)
* Remote: Command button functions assignable from a context menu (middle-click or Ctrl+left-click; right-click functions via Ctrl+right-click, including a "Clear Function" entry) listing ready-made Visualizer functions
* Lyrics: default font size is 40 now (settings.ini: LyricsFontSize)
* Settings: Invert preset output

## v4.1 (2026-09-10)

* Lyrics support
  - Automatic lookup, download and caching of synced lyrics (LRC) for the current track (Ctrl+L)
  - Dedicated Remote "Lyrics" tab: on/off, manual file loading, font, color, position, zoom, fade, width, time offset, auto-scale and burn-in effects
* Visualizer IPC multi-client support (by @shanevbg)
* Configurable Remote tab visibility
* Updated presets

## v4.0 (2026-04-19)

* [MDropDX12](https://github.com/shanevbg/MDropDX12) Visualizer support (experimental)
  - Milkwave DirectX 12 Visualizer fork by @shanevbg (included)
  - Start Milkwave Visualizer or MDropDX12 instances from the Remote
  - Communication based on Milkwave IPC Protocol (MIP)
* Network Mode
  - Control Milkwave Visualizer or MDropDX12 on remote PCs (MIP over TCP)
  - Press Ctrl-N in Visualizer, see Manual for details
* Multi Mode
  - Send commands to multiple Visualizers simultaneously
  - Supports local and network connections
* Support for equalizer shader presets (FFT)
  - Adjust EQ Attack, EQ Decay and EQ Boost on Settings tab
* New preset: IkeC - Equalizer
  - 5 different versions (switch on Settings tab)
  - Band peak marking and mouse support (Right-click)
  - Supports Intensity and Shift parameters
* 100 new shader-based "Basic" presets
  - Simple and calm presets eg. for mixing
* Basic .milk2 file support
* Visualizer menu font can be configured from Remote
* Visualizer help (F1) adjusts to window size
* Restructured Remote tabs and controls
* Improved transitions quality
* Global stability improvements

## v3.5 (2026-03-01)

* "Input" tab
  - Mix Visualizer output with Webcam or Spout senders
  - Choose layer order and blending mode (luma key, opacity)
* Controller support
  - Trigger Visualizer functions with a game controller
  - Fully configurable: Assign any scriptable action to any button
* "Preset mode" for button panel
  - Assign up to 45 presets to buttons for quick access
  - Right-click button to assign current preset (with screenshot capture)
  - Left-click to send preset to Visualizer
* "Settings" tab
  - Set preset auto change time, lock and random
  - Automatically change Hue over time (color cycle)
  - Open bundled Milkwave Message Editor (by @shanevbg)
  - Run shader precompilation and clear cache
* Toggle color/monochrome display for button images
* Visualizer updates
  - Save screenshot (Ctrl+X)
  - Key bindings to alter Quality (Ctrl+Q) and Hue (Ctrl+H)
  - Stability improvements and bug fixes

## v3.4 (2025-11-22)

* Remote: Display current sprite preview image on button
* Select new sprite by right-clicking sprite button
* Fix: Audio capture not working on some systems (#16)
* Fix: Inconsistent display of switched audio device
* Fix: Setting Spout output to Fixed causing high GPU load
* New presets
  - Del + IkeC - glooper
  - Shane + IkeC - Apollonian Structure
  - SnoopethDuckDuck + IkeC - Breeze

## v3.3 (2025-10-31)

* Settings Tab: Shift Hue/Saturation/Brightness of the entire output
* Presets Tab: Filter preset list (Ctrl+F)
* Mouse interaction mode (Ctrl+M) (base by @OfficialIncubo)
* "Auto" quality setting for constant perceived quality on different window sizes
* Option to disable audio capture (bEnableAudioCapture=0)
* Fix: Restored amp functionality
* New or modified presets
  - New: diatribes + IkeC - Vibe Ride
  - New: lara - MilkDrop2077 - Flexi Romanesco Uncompressed (mouse)
  - Mouse support: Kali + IkeC - Fractal Land
  - Mouse support: chronos + IkeC - Astral Jelly Journey

## v3.2 (2025-10-03)

* MIDI Tab: MIDI automation for up to 50 controls
* Window independent Spout output (#14)
* Adjust render quality from Remote
* CPU/GPU monitoring
* HiRes audio support (by @OfficialIncubo)
* 5 new shader-based presets
  - chronos + IkeC - Astral Jelly Journey
  - diatribes + IkeC - Deep Cloud Mix
  - diatribes + IkeC - Neon Chill
  - Kali + IkeC - Fractal Land
  - kasari39 + IkeC - Phantom Star for CineShader

## v3.1 (2025-09-04)

* 10 new shader-based presets in Milkwave/Shader directory
* All 30 Milkwave shader presets now react to audio input
* New "vis_intensity", "vis_shift" and "vis_version" preset variables
* Adjust "Intensity", "Shift" and "Version" live from Remote for supported presets (eg. *Milkwaves shader presets)
* Set default audio device using Ctrl+D (eg. after disconnecting Bluetooth headphones)
* Draw a background box for text messages with custom transparency and color using box_* *message parameters
* New message parameter "fadeout" (also for custom messages)
* New shortcuts: Ctrl+B for toggling button panel, Ctrl+O for opening Visualizer window
* Press Windows Media "Play/Pause" and "Stop" keys from script
* Force soft preset transition type using "Mixtype" in settings.ini
* Miscellaneous stability improvements

## v3.0.2 (2025-08-26)

* Pretty code formatting for HLSL shader code in Shader tab
* Shader-based "Heartfelt" preset added to Milkwave/Shader
* Fix: Visualizer crashing when using "Link" button in Milkwave Remote "Preset" tab
* Improved shader precompile notification and error messages

## v3.0.1 (2025-08-18)

* settings.ini: LogLevel=2 for verbose application logging
* Shader precompiler can now handle multibyte-encoded filenames

## v3.0 (2025-08-17)

* Shader Tab: Convert GLSL shader code to HLSL and send it to the Visualizer instantly
* 20 new presets in Milkwave/Shader directory
* Shader precompiling and caching (configurable)
* New preset variables: bass_smooth, mid_smooth, treb_smooth, vol_smooth
* AMD GPU detection and support for PSVersion=4 (by @OfficialIncubo)
* New waveform, transitions and updated presets (by @OfficialIncubo)
* Age filter: Only load presets modified within the last X days
* Ctrl+Click on labels "Preset" or "Running" to open preset file in editor
* Improved scaling of tab heights with high DPI displays

## v2.3 (2025-07-13)

* Settings Tab: Change internal time, FPS and frame counters (#13)
* Replaced expression evaluation library ns-eel2 with projectM-eval (by @kblaschke)
* Fix: Visualizer crashing when resizing or going fullscreen with more than one sprite displayed (by @kblaschke)
* Option to disable saving last used preset as startup preset (bEnablePresetStartupSavingOnClose=0)
* Option to only load presets containing a specific text
* Many new and updated presets (by @OfficialIncubo)

## v2.2 (2025-06-21)

* Support for input devices (eg. microphones)
* Show current audio device (Ctrl+D)
* Remote: Allow including subdirs when loading directories
* Set window to fixed dimensions from config (Ctrl+Shift+F2) (#10)
* Fix: startx/starty not always working correctly (#11)
* Screen-dependent render mode feature (by @OfficialIncubo)

## v2.1 (2025-06-13)

* Display multiple messages at once (#9)
  - Move text around using start and end coordinates (startx, starty, movetime)
  - Adjust the "burn in"-time of messages
* Improved font proportional display and handling of font message coordinates
* Save and restore "Always on top" window state
* New script file commands (see script-default.txt for details)

## v2.0 (2025-05-26)

* Tabbed interface
* Preset tagging with dynamic buttons
* Tag-based playlists
* More live wave manipulation options
* Live font manipulation
* Pixel shader: MinPSVersion, MaxPSVersion with Up Arrow indicator

## v1.6 (2025-04-29)

* Improved performance: 30% -> 5% CPU usage on test system
* Black mode (Ctrl+F12): Hides all preset rendering
* Script file lines as drop down list in Remote
  - Select lines from default or custom script file
  - Send timed, randomly or manually
* New Remote buttons: Song info, transparency, watermark mode, 88, 99 (sprites)
* New Visualizer mouse controls: 
  - Middle Mouse Button: Song Info
  - Right+Left Mouse Button: Close Visualizer
  - Right+Middle Mouse Button: Open Remote
* Additional quicksave folder: Ctrl+Shift+S saves to presets\Quicksave2
* New configuration options:
  - Show cover when requesting song info
  - Choose corner for song info
  - Close Visualizer when closing Remote
  - Hide notifications in Visualizer when Remote is active
* Help screen now two pages

## v1.5 (2025-04-26)

- Windowed clickthrough fullscreen ("watermark mode") (Ctrl+Shift+F9)
* Display current track information and artwork (eg. from Spotify or YouTube)
* Font and color customization options
* Improved device and window handling

## v1.4 (2025-04-19)

* Basic wave manipulation
* Show notifications (only) in Remote

## v1.3 (2025-04-17)

* On-the-fly audio device selection

## v1.2 (2025-04-14)

* Double-line text with auto-wrap option
* Remote: Signal amplification

## v1.1 (2025-04-11)

* Remote: Select preset files and send them to the visualizer
* Show the currently playing Visualizer preset in the Remote window
 
## v1.0 (2025-04-07)

* Customize message text, color, font size, position and other parameters
* Save named style presets for quick access
* Automatically send messages and set parameters from a file at configurable intervals
* Send common key combinations using buttons
* Save and restore Remote and Visualizer window positions