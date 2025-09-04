## v3.1 (2025-09-03)

- 10 new shader-based presets in Milkwave/Shader directory
- All 30 Milkwave shader presets now react to audio input
- New "vis_intensity", "vis_shift" and "vis_version" preset variables
- Adjust "Intensity", "Shift" and "Version" live from Remote for supported presets (eg. Milkwaves shader presets)
- Set default audio device using Ctrl+D (eg. after disconnecting Bluetooth headphones)
- New message parameter "fadeout" (also for custom messages)
- Draw a background box for text messages with custom transparency and color using box_* message parameters
- New shortcuts: Ctrl+B for toggling button panel, Ctrl+O for opening Visualizer window
- Force soft preset transition type using "Mixtype" in settings.ini
- Miscellaneous stability improvements

## v3.0.2 (2025-08-26)

- Pretty code formatting for HLSL shader code in Shader tab
- Shader-based "Heartfelt" preset added to Milkwave/Shader
- Fix: Visualizer crashing when using "Link" button in Milkwave Remote "Preset" tab
- Improved shader precompile notification and error messages

## v3.0.1 (2025-08-18)

- settings.ini: LogLevel=2 for verbose application logging
- Shader precompiler can now handle multibyte-encoded filenames

## v3.0 (2025-08-17)

- Shader Tab: Convert GLSL shader code to HLSL and send it to the Visualizer instantly
- 20 new presets in Milkwave/Shader directory
- Shader precompiling and caching (configurable)
- New preset variables: bass_smooth, mid_smooth, treb_smooth, vol_smooth
- AMD GPU detection and support for PSVersion=4 (by @OfficialIncubo)
- New waveform, transitions and updated presets (by @OfficialIncubo)
- Age filter: Only load presets modified within the last X days
- Ctrl+Click on labels "Preset" or "Running" to open preset file in editor
- Improved scaling of tab heights with high DPI displays

## v2.3 (2025-07-13)

- Settings Tab: Change internal time, FPS and frame counters (#13)
- Replaced expression evaluation library ns-eel2 with projectM-eval (by @kblaschke)
- Fix: Visualizer crashing when resizing or going fullscreen with more than one sprite displayed (by @kblaschke)
- Option to disable saving last used preset as startup preset (bEnablePresetStartupSavingOnClose=0)
- Option to only load presets containing a specific text
- Many new and updated presets (by @OfficialIncubo)

## v2.2 (2025-06-21)

- Support for input devices (eg. microphones)
- Show current audio device (Ctrl+D)
- Remote: Allow including subdirs when loading directories
- Set window to fixed dimensions from config (Ctrl+Shift+F2) (#10)
- Fix: startx/starty not always working correctly (#11)
- Screen-dependent render mode feature (by @OfficialIncubo)

## v2.1 (2025-06-13)

- Display multiple messages at once (#9)
  - Move text around using start and end coordinates (startx, starty, movetime)
  - Adjust the "burn in"-time of messages
- Improved font proportional display and handling of font message coordinates
- Save and restore "Always on top" window state
- New script file commands (see script-default.txt for details)

## v2.0 (2025-05-26)

- Tabbed interface
- Preset tagging with dynamic buttons
- Tag-based playlists
- More live wave manipulation options
- Live font manipulation
- Pixel shader: MinPSVersion, MaxPSVersion with Up Arrow indicator

## v1.6 (2025-04-29)

- Improved performance: 30% -> 5% CPU usage on test system
- Black mode (Ctrl+F12): Hides all preset rendering
- Script file lines as drop down list in Remote
  - Select lines from default or custom script file
  - Send timed, randomly or manually
- New Remote buttons: Song info, transparency, watermark mode, 88, 99 (sprites)
- New Visualizer mouse controls: 
  - Middle Mouse Button: Song Info
  - Right+Left Mouse Button: Close Visualizer
  - Right+Middle Mouse Button: Open Remote
- Additional quicksave folder: Ctrl+Shift+S saves to presets\Quicksave2
- New configuration options:
  - Show cover when requesting song info
  - Choose corner for song info
  - Close Visualizer when closing Remote
  - Hide notifications in Visualizer when Remote is active
- Help screen now two pages

## v1.5 (2025-04-26)

- Windowed clickthrough fullscreen ("watermark mode") (Ctrl+Shift+F9)
- Display current track information and artwork (eg. from Spotify or YouTube)
- Font and color customization options
- Improved device and window handling

## v1.4 (2025-04-19)

- Basic wave manipulation
- Show notifications (only) in Remote

## v1.3 (2025-04-17)

- On-the-fly audio device selection

## v1.2 (2025-04-14)

- Double-line text with auto-wrap option
- Remote: Signal amplification

## v1.1 (2025-04-11)

- Remote: Select preset files and send them to the visualizer
- Show the currently playing Visualizer preset in the Remote window
 
## v1.0 (2025-04-07)

- Customize message text, color, font size, position and other parameters
- Save named style presets for quick access
- Automatically send messages and set parameters from a file at configurable intervals
- Send common key combinations using buttons
- Save and restore Remote and Visualizer window positions