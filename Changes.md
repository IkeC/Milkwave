## v2.3 (unreleased)

- Settings Tab: Change internal time, FPS and frame counters (#13)
- Replaced expression evaluation library ns-eel2 with projectM-eval (by @kblaschke)
- Option to disable saving last used preset as startup preset (bEnablePresetStartupSavingOnClose=0)
- Many new and updated presets (by @OfficialIncubo)

## v2.2 (2025-06-21)

- Support for input devices (eg. microphones)
- Show current audio device (Ctrl+D)
- Remote: Allow including subdirs when loading directories
- Set window to fixed dimensions from config (Ctrl+Shift+F2) (#10)
- startx/starty not always working correctly (#11)
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