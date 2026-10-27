# Milkwave Manual

Welcome to the Milkwave manual! This document will help you get started with the Milkwave Remote, a tool to control the Milkwave Visualizer.

In general, make sure to hover the mouse over all labels and buttons to see some inline help tooltips describing most of the features and displaying keyboard shortcuts.

If you need help with the Visualizer itself, press F1 there to cycle through the three on-screen help pages. The font is fitted automatically so every page stays fully visible in both windowed and fullscreen mode. Also take a look at the documentation in the _resources/docs_ folder.

# Interface

The Tabs panel (top half) holds most features available in the Remote, described in detail below. You can show or hide any of the tabs by setting the corresponding _ShowTab_ key in the _Milkwave_ section of _settings.ini_ (_ShowTabPreset_, _ShowTabMessage_, _ShowTabInOut_, _ShowTabSettings_, _ShowTabLyrics_, _ShowTabFonts_, _ShowTabMidi_, _ShowTabWave_ and _ShowTabShader_; all are on by default except the Shader tab). The same can be done from the "Show/Hide Tabs" submenu of the "Milkwave" menu in the bottom right corner, which toggles each tab individually and writes the setting to _settings.ini_.

The Buttons panel (bottom half) has two modes: "Command" mode and "Preset" mode. You can switch between modes using the blue arrows button in the lower right corner.

**Command mode** allows to trigger some commonly used functions in the Visualizer without having to focus the Visualizer window. At the bottom, the buttons will show preview images and trigger the sprites from _sprites.ini_ when Visualizer is in sprite mode (default). You can change the images for all buttons (except the first one) by right-clicking the button and selecting an image.

Press K in Visualizer or the button "Sprite/Msg Mode" to change modes. When Visualizer is in message mode, the Remote panel buttons will be labeled 00-99. The button tooltip will show what message will be sent (if a matching definition is defined in _messages.ini_).

The 15 command buttons can be fully customized. The easiest way is to assign functions directly in the Remote: middle-click or Ctrl+left-click a command button to open a menu of ready-made Visualizer functions (Window, Display, Presets, Track, Lyrics, Sprites & Messages, Wave & Warp, Shader) covering the shortcuts from the Visualizer's F1 help. Picking one sets that button's caption and command and saves both to _settings.ini_ right away. Ctrl+right-click opens the same menu for the button's right-click function; its last entry, "Clear Function", removes that right-click function again.

You can also edit the **[Buttons]** section of _settings.ini_ by hand (restart the Remote afterwards). Each button has one line `Button<number>=<caption>|<command>`, and the optional `Button<number>RightClick=<caption>|<command>` defines the right-click command, with its caption shown in the tooltip as "Right-Click: <caption>" (no right-click command means no tooltip). Set either key to `none` to disable it, and use `\n` in a caption for a line break. `<number>` is the button position (1-15) in reading order - Button1 is "Next Preset", Button15 is "Desktop Mode". The `<command>` is the raw message sent to the Visualizer through the named pipe: `SEND=0x20` sends a virtual key by hex key code, any other `SEND=<text>` (e.g. `SEND=~`) sends the text as keystrokes, and `SIGNAL|<name>` triggers a signal such as `SIGNAL|FULLSCREEN`, `SIGNAL|DESKTOP_MODE` or `SIGNAL|BORDERLESS_FS`. For example: `Button1=Next Preset\n(Space)|SEND=0x20` or `Button11RightClick=Borderless Fullscreen|SIGNAL|BORDERLESS_FS`. Only Button11 (Fullscreen, right-click "Borderless Fullscreen") and Button13 (Clear Sprite/Msg, right-click "Clear all Sprites") come with a right-click command, and missing keys are filled in with their defaults.

**Preset mode** allows you to assign the currently running preset to a button by right-clicking it. After assignmwent, the button will display a preview image of the preset. Left-clicking the button will start the preset in Visualizer, a middle-click will clear it. You can use blue numbered button in the lower right to switch between 3 banks of preset buttons, giving you a total of 45 assignable preset buttons. You can also press each of these buttons from script or MIDI using the command "BTN=<number>" (see _script-default.txt_ for details).

Note that you can choose to hide either the Tabs or the Button panel with the popup menu that opens by clicking "Milkwave" in the bottom right corner ("Tabs Panel" and "Button Panel"). The "Show/Hide Tabs" submenu of the same menu turns single tabs on or off. You may also use the menu to access help resources, open the Visualizer window, switch between light and dark mode, and toggle color or monochrome button image display.

At the bottom, you can left-click the status bar to copy its content to the clipboard, middle-click to set a "compact" mode, or right-click to quickly switch between tabs and button panel. You can set the "compact" mode size in _settings-remote.json_ using the key _RemoteWindowCompactSize_.

CPU and GPU usage are displayed at the bottom right if enabled. Click the "Milkwave" button at the bottom right to expand a menu providing quick access to additional application settings and information resources.

## Tab "Preset"

Send a preset from the drop-down list to display in the Visualizer using "Send". The list will show the presets from _presets/Milkwave_ by default. You can fill the list using the buttons "Tags", "File" and "Dir".

The currently playing Visualizer preset is displayed after the "Running" label. After "Tags" you can supply your own set of tags describing the current preset, separated by comma. If "Running" is checked, the tags are loaded (and saved to) the currently running preset, if it is unchecked the preset from the drop-down list in the first line is used. Click "Save" to save the tag information to tags-remote.json.

Your most used tags are displayed as buttons automatically after the "Most used" label. Note that you may delete the file _tags-remote.json_ anytime if you want to start from scratch.

At the bottom, the "Preset" row controls when and how presets change. "Random" toggles between random and sequential preset order (same as pressing "R" in the Visualizer). "Locked" freezes the current preset (same as pressing "~" in the Visualizer) so that nothing changes it automatically; the preset starts unlocked (settings.ini:bPresetLockOnAtStartup), and while it is locked both "Song" and "After" are disabled. "Song" changes the preset whenever the played track changes (same as pressing Ctrl+A in the Visualizer, settings.ini:ChangePresetWithSong); it replaces the "After" interval, so while "Song" is enabled presets follow the song only and the "After" number box is disabled. "After" is the number of seconds until the next preset change (settings.ini:fTimeBetweenPresets); note that the blending time and a random extra are added to determine the actual duration (settings.ini:fBlendTimeAuto and fTimeBetweenPresetsRand). 

The "Display" toggles control what is shown when the track changes: "Track" displays the song info (same as pressing Ctrl+B in the Visualizer, settings.ini:SongInfoPollingEnabled), and "Cover" displays the cover art whenever the song changes (same as pressing Ctrl+C in the Visualizer, settings.ini:DisplayCover). Long artist, title or album lines are truncated with an ellipsis so they stay on screen. "Preset" shows the preset info for 6.5 seconds by default on every preset change while the preset info is not shown permanently (same as pressing Ctrl+F4 in the Visualizer, settings.ini:PresetInfoOnChange and PresetInfoOnChangeSeconds). This toggle has no effect when preset info is shown permanently with F4.

The Remote keeps these toggles in sync with the Visualizer, so changing them on either side updates the other. Controls that have no effect in the current state are disabled - hover them to see a tooltip explaining why.

### Opening preset files from Windows

If you associate _.milk_ (or _.milk2_) files with Milkwave Visualizer in Windows, opening a preset file displays it: the installer offers this on its last page ("Associate .milk files", enabled by default), and you can also set it yourself via Explorer's "Open with". When the Visualizer is not running it starts with that preset, and when it is already running the preset is loaded in the running instance instead of opening a second window. That window is restored from the taskbar and brought to the front (if Windows lets the new process pass the activation on, otherwise the taskbar button flashes). The file is loaded as-is, so your preset folder (_settings.ini:szPresetDir_) stays unchanged and the next automatic preset change continues in your own library. In Desktop Mode there is no window to raise, so only the preset changes.

If Windows still opens presets in another program, or keeps asking "How do you want to open this file?", pick Milkwave Visualizer once via "Open with" - Windows keeps a per-file-type choice that overrides the installer's association until it is refreshed.

## Tab "Message"

Use "Send" to send the text after "Message" to display in the Visualizer window. Toggle "Preview" to see a "what you see is what you get" version of your text. If "Wrap" is checked, text will be wrapped to two lines if the text is longer than the number in the drop-down box. You can manually define a line break within your text by writing "//". Note that this feature is rather experimental and may clip your text sometimes depending on your used font face and size.

Right-Click the "Parameters" label to display a list of all available parameters for the text message display. You can save a parameter line to the named slot after "Style" by pressing "Save".

Changing "Font", "Size" and "Color" parameters will change the text display. Note that these definitions can be "Set" to the "Parameters" line. If any of these values are defined in "Parameters", they will override your UI settings. To remove a set definition from "Parameters", you may double-click the "Font", "Size" and "Color" labels.

You can load a list of parameter lines from a file and send them to the Visualizer individually by pressing the "Send" button in the bottom right. By default, the file _script-default.txt_ from the Milkwave main directory is loaded, but you can load any file by right-clicking the "From File" label. Please read the documentation in _script-default.txt_ for a detailed description on possible commands.

Instead of selecting and sending lines from the fine manually, you may check the "Autoplay" to play them sequentially. Use the "BPM" and "Beats" parameters to control the interval between sending each line. Check the "Rand" button to send a random line on each interval (excluding the last sent line).

"Config" allows you to quickly open the most important configuration files in a text editor. You may also open Shanes [Message Editor](https://github.com/shanevbg/MDx12Messages) from here to edit the _messages.ini_ file in a more user-friendly way.

### Fade-in, Fade-Out and Burntime

You can define default values for fade-in ("fade", default 0.2), fade-out ("fadeout", default 0.0) and burntime ("burntime", default 0.1) in settings.ini, and override them per message as a parameter.

If a burntime > 0 is defined, the message will be "baked" into the background and slowly fade away for the defined burntime duration. Note that this won't work with all presets, and if you define a fadeout > 0, the burntime will be irrelevant because the message will fade out before there's something to "burn in".

## Tab "In/Out"

The "Window" label shows the window title of the Visualizer that the Remote will send its commands to. You may start multiple Visualizer windows and control them all by changing the target window. Change the opacity of the Visualizer window using the "%" box.

Check "DX12" to use the MDropDX12 Visualizer instead of the standard Milkwave Visualizer. In this mode, commands are sent via named pipe rather than window messages. The configurable executable names are stored in _settings-remote.json_ as _VisualizerExe_ and _VisualizerExeDX12_.

Your audio input and output devices are displayed after the "Device" label. If you don't wish to see input devices (such as microphones), set _settings.ini:IncludeInputDevices=0_. Use "Set" to set the Visualizer input source to the selected device. Use Ctrl+D in the Visualizer window to display the currently active device.

Use "Amp" to amplify the virtual audio level for the Visualizer, eg. if you feel the preset should react more to your music. If "Link" is checked, left and right audio channels are amplified equally.

The elements here also allow you to mix video sources such as webcams or Spout feeds into the Visualizer output. You can either mix a webcam input or a Spout feed, using the "Mix" buttons. If you want to use [OBS](https://obsproject.com/) as a source, activate Spout output in OBS rather than using the OBS virtual camera, as the latter is not supported by Milkwave.

Set Luma Key to "Active" to make a certain color in the input source transparent. Adjust threshold to set the tolerance for the keying, and adjust "Softness" to make the edges of the keyed area softer.

"Top Layer" allows you to set either the input source or the Visualizer's preset rendering as the top layer of the Visualizer output. "Opacity" sets the opacity of the top layer.

"Controller" allows you to select a game controller. If "Active", it will send the commands defined in _controller-remote.json_ to the Visualizer when you press buttons. You can edit the command list in _controller-remote.json_ or use the "Config" button to open the file in a text editor.

### Network Remote

You can control Milwave Visualuzer or MDropDX12 instances running on remote machines over TCP. 

On the client side, network targets are configured in _network-remote.json_ and appear in the Visualizer combobox alongside local instances. Each target must set `active=true` to show up, and the entry label is the value of `name`. When a network target is selected, commands are sent to the Visualizer's TCP server instead of a local named pipe.

On the machine running Milkwave Remote, edit _network-remote.json_ and configure an entry with the remote machine's IP address, port, and (optional) PIN:

```json
{
  "targets": [
    {
      "active": true,
      "name": "MDropDX12 (Remote)",
      "host": "192.168.1.100",
      "port": 9270,
      "pin": "",
      "deviceId": "milkwave-remote-1",
      "deviceName": "Milkwave Remote"
    }
  ]
}
```

Only targets with `active=true` are shown in the combobox. If "Multi" is checked, commands are broadcast to all local instances and all active network targets.

To configure **MDropDX12** on the server side, start MDropDX12 and enable the TCP server in **Settings (F8) → "Tools" tab → "Open Remote"**. In the **"Authorization"** tab, enable the TCP Server. Alternatively, set `TcpEnabled=1` in the `[Network]` section of _settings.ini_. The default port is **9270**. If the remote MDropDX12 has not authorized this device yet, the status bar will show "Waiting for approval" — approve the pending device in "Clients".

To configure **Milkwave Visualizer** on the server side and see the server status, press Ctrl+N in the Visualizer. Alternatively, set `TcpEnabled=1` in the `[Network]` section of _settings.ini_. You may also specify a different port with `TcpPort` and a PIN (plain text) using `TcpPIN`.

The wire protocol itself - framing, the `AUTH` handshake, every command and the state notifications - is documented in the [appendix](#appendix-milkwave-ipc-protocol-mip), together with example clients in PowerShell.

## Tab "Settings"

Change the internal "Time", "FPS" and "Frame" values that the Visualizer sends to the preset. This may speed up, slow down or otherwise change the behaviour of the preset, depending on how the preset is built and how (or if) it uses any of these variables.

The "Intensity", "Shift" and "Version" values can be read by presets that support the Milkwave specific vis_intensity, vis_shift and vis_version variables (see below). As above, you can change these values live while a preset is running.

With "Hue", "Saturation" and "Brightness", you can change the overall color tone of the Visualizer output. This may be useful if you want to adapt the colors of a preset to your room lighting, or just want to experiment with different looks. You may also set "Auto" to automatically shift the colors over time.

With the "Quality" setting, you can reduce the size of the backbuffer used for rendering, eg. a quality factor of 0.5 will render to an internal buffer with half the width and height of your Visualizer window. This will improve performance on slower systems, but will also reduce visual quality. A low quality may also yield in a pixellated look, giving a nice retro effect. Note that the quality setting will be ignored if "Fixed" Spout resolution is used.

At reduced quality, messages, notifications, menus, song titles and lyrics are still drawn at the full window resolution in a separate screen overlay, so they stay sharp while the preset itself keeps the lower internal resolution. Spout output and screenshots contain the lower-resolution copy with the same text. Text and lyrics burn-in effects remain part of the preset and are not affected by the overlay.

If you select "Auto", Milkwave tries to make the "perceived" Visualizer quality similar on different window sizes by adjusting the backbuffer size accordingly.

"Invert" inverts the colors of the preset output while leaving the text overlays (song titles, messages, lyrics) unaffected — useful for light-on-dark environments. This is a global setting stored as _InvertPresetOutput_ in the _Milkwave_ section of _settings.ini_, independent of the per-preset "Invert" filter on the Wave tab.

"Textborder" sets the outline thickness used for messages and notifications in the Visualizer (default 2, 0 turns the outline off). The outline color is configured directly in _settings.ini_ with the _TextBorderColorR/G/B_ keys in the _Milkwave_ section (black by default); the thickness itself is stored as _TextBorder_.

Keep in mind that many settings can be automated using script commands in the _script-default.txt_ file or your own script files. See the comments in _script-default.txt_ for details. They can also be MIDI-controlled (see below).

For [Spout](https://spout.zeal.co/), you can set the output to a "Fixed" resolution instead of the Visualizer window size. This may be useful if you want to use Milkwave as a source for other applications that expect a certain resolution. The Visualizer window will then use the fixed backbuffer size and aspect ratio for display.

"Compile" allows you to precompile the shaders defined in _precompile.txt_. This will usually take 3-5 minutes and happen in the background. You may also "Clear" the cache, which will simply remove all files from the _cache_ folder. This may be useful if you want to reset all shaders to be recompiled, eg. after a Milkwave update. If you switch to a shader preset that is not cached yet, the current preset keeps playing while it compiles in the background. If compilation takes longer than one second, a "Compiling <name>" notification shows the preset being prepared.

With FFT you may control equalizer attack and decay values for presets supporting FFT functions *get_fft(pos)* and *get_fft_hz(freq)*, such as _IkeC - Equalizer.milk_ from the _Milkwave_ presets folder. *get_fft_peak_hz(freq)* is exclusive to Milkwave.

The "Test" button next to "Audio" plays the simulated band sequence from _audio-test.txt_ instead of the captured audio. Each line of the file contains a hold time and relative bass, mid and treble levels (1.0 = track average); lines starting with # are comments. The Visualizer shows the active values in a notification and synthesises a matching waveform and FFT spectrum, so shader, wave-based and FFT-based presets react. Click "Stop" to return to live audio. This is useful for checking whether a preset reacts as intended without playing music, and it is the interactive counterpart to the preset-conversion audio checks.

## Tab "Lyrics"

The Lyrics tab controls the synchronized lyrics overlay that the Visualizer draws on top of the visual output, and that is also included in the Spout sender. When "Auto" is active, Milkwave looks up the lyrics for the currently playing track automatically (eg. from [LRCLIB](https://lrclib.net)) and caches them locally; otherwise you can load a lyrics file manually.

The "Outline/Background" selector styles the lyrics text: "Text Outline" (the default, thickness 3) draws an outline around the text; "Background (25%)", "Background (50%)" and "Background (75%)" draw a box behind it at 25%, 50% or 75% opacity; "Background (solid)" draws a fully opaque box; and "No Outline/Background" keeps the plain look. The color panel sets the outline/box color, and the amount field controls the outline thickness or box padding. A box uses at least 8 pixels on the left and right (the amount itself once it is larger), plus the amount above and below the text, so the border stays balanced without becoming excessive (0 means the box ends exactly at the text). The values are stored in _settings.ini_ as _LyricsOutline_, _LyricsOutlineAmount_ and _LyricsOutlineColorR/G/B_.

The "Status" field shows the current lyrics state ("Lyrics off", "Lyrics loading", "Lyrics loaded", "Lyrics missing timestamps" or "Lyrics unavailable"). "Active" turns the lyrics overlay on or off (settings.ini:LyricsEnabled), and "Auto" toggles the automatic lookup of lyrics for the current track (settings.ini:LyricsAutoLoad).

The "File" field shows the lyrics file in use — only the filename is displayed, the full path appears when you hover the mouse over it. "Load" lets you pick a custom _.lrc_ or _.txt_ file, "Edit" opens the current lyrics file in the associated editor, and the current line is shown in the "Line" field. "Restart" resets the internal timeline to the start of the track, which is handy for players that don't report a position; the lyrics then begin again from the first line.

The "Font" row sets the font face, size, bold/italic/antialiasing options and the text color used for the lyrics.

The two bottom rows control the layout and timing of the overlay. "Pos X"/"Pos Y" place the lyrics block on the canvas, while "Start X"/"Start Y" define where the lyrics move from while they fade in and out. Moving Pos X or Pos Y shifts the corresponding start position by the same amount, preserving the fade-in/out travel distance. Double-clicking the Pos label resets Pos X and Pos Y to 0.50 and both start positions to their defaults (0.50 / 0.52); double-clicking the Start label resets both start positions. "Zoom" scales the font at the fade extremes (1.0 means no scaling), and "Fade" sets the fade-in/out duration in seconds (settings.ini:LyricsFade). "Width" limits the maximum width of the lyrics block as a ratio of the canvas (settings.ini:LyricsMaxWidth); longer lines wrap at word boundaries.

"Burntime" bakes the previous line into the background and lets it fade out over the given time in seconds (0 turns this off), similar to the burntime effect for messages. "Burnmode" selects when the burn-in is applied: 0 turns it off, 1 burns the leaving line when its fade-out starts, 2 bakes the current line as it fades in (the original behavior), and 3 only draws the burn-in without the crisp overlay (settings.ini:LyricsBurnTime / LyricsBurnType). "Offset" shifts the lyrics timestamps by the given number of seconds (settings.ini:LyricsOffsetSeconds), eg. to sync the lyrics with a delayed audio feed. The drop shadow is configured directly in _settings.ini_ with the _LyricsShadow_ key.

"Autoscale" (on by default) auto-sizes the font so about (60 - font size) characters fit per line within the "Width" area; a larger font-size value therefore renders the lyrics bigger (settings.ini:LyricsAutoScale).

You can double-click any of the labels in the two bottom rows to reset the accompanying input to its default value. All lyrics settings are stored in the _Lyrics_ section of _settings.ini_.

## Tab "Fonts"

Modify most of the fonts used to display information in the Visualizer window. Use "Save" and "Test" to see your changes. You can save and preview changes instantly if you hold the ALT key while changing fonts or sizes.

Changes are saved to the _Fonts_ section in _settings.ini_. Of course you may edit them there manually as well.

## Tab "MIDI"

You can control many of the Remote and Visualizer features using MIDI controllers. First, select your MIDI input device from the drop-down list. If your device is not listed, make sure it is connected and recognized by Windows, then click "Scan".

You can assign up to 50 actions for your controls. Switch to higher rows using the "Bank" control. Press "Learn" and push or turn your MIDI control for Milkwave to recognize it. You'll see the "Channel", "Value" and "Controller" boxes change their values if this works. When the control is learned, turn off "Learn" and select an "Action" for your control.

When not in "learning" mode, all rows marked as "Active" will be taken into account when a MIDI event is received. Double-click a row number to clear the row.

Milkwave differentiates between two types of MIDI controls: "Button/Note" and "Knob/Fader". A button or note is just a trigger for an action, while a knob or fader sends a MIDI value between 0 and 127.

"Knob/Fader" actions are fixed, you can select the different options from the the drop-down list. The default value of a control is usually set to the middle setting of your knob or fader (MIDI value 64). In the "Inc" box, you can define how much the value of the target control is changed for every MIDI value change.

Example: You assign a knob on your MIDI keyboard to "Settings: Intensity", with an "Inc" value of 0.05. The default value for "Intensity" is 1, which is assigned to the middle setting of your knob (MIDI value 64). Now if you turn your keyboard knob all the way up to 127, the "Intensity" value in the Remote will be set to 1+(63\*0.05)=3.15. If you turn it down, it will go down to 0 in the Remote. Values are capped between the possible values a control supports, so eg. "Intensity" will never go below 0.

"Button/Note" events can trigger a lot of actions. You can select some default actions from the drop-down list, but these are just some common examples - you can type freely into the box. You may also edit the default action list by editing _midi-default.txt_ in the Milkwave folder.

An action can be any command or string of commands that can also be triggered by script. You can also trigger lines (current, next, previous or specific) from the currently loaded script file ("From File" in the "Message" tab). Actions include selecting and changing presets, sending messages, triggering sprites, starting external programs and lots more. Please read the documentation in _script-default.txt_ for a detailed description on possible commands and command chains.

Your MIDI assignments are kept in _midi-remote.json_ and automatically loaded and saved when you open/close the Remote. You may also load and save settings manually using the "L" and "S" buttons in the upper right corner.

If you want to hide the MIDI tab and prevent initialisation completely (eg. because it interferes with your MIDI setup in other programs), set MidiEnabled=false in _settings-remote.json_.

## Tab "Wave"

Manipulate factors of the default wave of the current preset and display the changes instantly in the Visualizer. If the "Link" button is checked, values are received from and sent to the currently running preset instantly, if not click "Send" to send your values. Use "Clear" to display a default wave. 

Use the "Quicksave" button or press Ctrl+S in the Visualizer to save the current preset instantly to the _presets/Quicksave_ folder as a new file. Press Shift+Ctrl+S to save to _presets/Quicksave2_ instead.

## Tab "Shader"

This tab can convert GLSL shader code (eg. from Shadertoy.com) to HLSL shader code, save it to a preset file and display it in the Visualizer instantly.

Click "Dir" to select a folder containing Shadertoy .json files. A possible source is [github.com/GabeRundlett/shadertoy-api-shaders](https://github.com/GabeRundlett/shadertoy-api-shaders). Select the file you want to load by changing the index number and clicking "Load". "Next" will automatically load the next file. Both "Load" and "Next" will convert and send the shader code to the Visualizer immediately. Your last selected directory and index number are saved to _settings-remote.json_ and restored upon restart.

You can show/hide the left pane with the "<" button. Click "Convert" to convert the GLSL code from the left pane to HLSL code in the right pane. Use "Send" to save a preset from the code and send it to the Visualizer.

The "File" checkbox determines whether the preset filename is built from the shaderinfo line or not. If you test many different shader code snippets, you may not want to produce a (possibly not working) preset file everytime, so if "File" is unchecked, the default filename "Shader.milk" will be used (and overwritten each time). You can set the pixel shader version written to the preset file using the number box before the "Send" button. If unsure, leave it at 4.

Note that the conversion process will most likely not produce a working preset right away, as some logical statements, terms and expressions cannot be converted easily. Some common problems are listed below, and I highly recommend at least watching the great [How to create Presets from OpenGL Shaders](https://www.youtube.com/watch?v=Ur2gPa996Aw) video by Patrick Pomerleau (also listed below) to understand the process.

If you get conversion errors, they will be displayed in the Remote status bar. Milkwave will try to find the correct error line in the right pane and jump there, but it's not always exact due to the way the Visualizer compiles the preset file. You can use the number box ("offset") in the upper right corner to highlight the acutal error line. A common pratice is to add a deliberate error (like "xxx") to the code, set the offset to the correct line, then remove it. Subsequent errors will then be marked more precisely.

The upper right corner also features a search box to find text within the right pane. Use Ctrl+Shift+F to jump to the search box, and Ctrl+F to actually find the next occurence of the search text. The "L" button allows you to load shader code from a file, or extract it from an existing preset (comp shader lines only). The "?" button opens this manual in your browser.

Remember that it is your responsibility to check and respect the individual license of the code you use and convert. In any case, always make sure you give proper credit to the original author. If you converted a preset successfully, consider sharing it with the community so it may be included with future Milkwave releases.

### Shadertoy examples

Some of the Shadertoy shaders that can be converted to a Milkwave preset using the Shader tab, along with the required manual corrections after conversion.

| Title | ID  | Corrections |
| ----- | --- | ------------------------------------- |
| Shader Art Coding Introduction | [mtyGWy](https://www.shadertoy.com/view/mtyGWy) | - |
| Cyber Fuji 2020 | [Wt33Wf](https://www.shadertoy.com/view/Wt33Wf) | - |
| Tunnel of Lights | [w3KGRK](https://www.shadertoy.com/view/w3KGRK) | - |
| String Theory 2 | [33sSzf](https://www.shadertoy.com/view/33sSzf) | - |
| Fractal Pyramid | [tsXBzS](https://www.shadertoy.com/view/tsXBzS) | replace `break;` with `i=64.;` |
| CineShader Lava | [3sySRK](https://www.shadertoy.com/view/3sySRK) | replace `break;` with `i=64;` / replace aspect correction with `uv.x *= aspect.x;` / remove flipping |

### Resources

- [An introduction to Shader Art Coding](https://www.youtube.com/watch?v=f4s1h2YETNY) (Video)
- [How to create Presets from OpenGL Shaders](https://www.youtube.com/watch?v=Ur2gPa996Aw) (Video)
- [GLSL-to-HLSL reference](https://learn.microsoft.com/en-us/windows/uwp/gaming/glsl-to-hlsl-reference)
- [MilkDrop Preset Authoring Guide](https://www.geisswerks.com/milkdrop/milkdrop_preset_authoring.html#3f)
- [Shadertoy How-To](https://www.shadertoy.com/howto)

### Known limitations

Here are some common terms that cannot be converted automatically and need to be edited manually after conversion.

| Term | Fix  |
| ---- | ---- |
| `break` | Replace with a statement setting a condition to end the loop |
| `myFloat3 *= myMatrix` | `myFloat3 = mul(myFloat3, transpose(myMatrix))` |
| `float3(1)` | Explicity set all components: `float3(1,1,1)` |
| `atan(a,b)` | `atan2(a,b)` |
| `float[3] arr` | `float arr[3]` |
| `radians(a)` | Multiply by π/180 directly: `a * (M_PI/180)` |
| `int[3] arr = int[](1,2,3)` | `int arr[3] = {1,2,3}` |
| `int ix = i & 1` (bitwise) | `int ix = i % 2` |
| `int yx = y >> 1` (bitwise) | `int yx = y / 2` |
| `if (i==0) return` (asymetric returns) | Put subsequent code in else branch: `if (i==0) {...} else {...} return` |
| `mul(a, b, c)` (three arguments) | Nest the calls: `mul(mul(a, b), c)` |
| `line(...)` (function name) | `line` is a reserved HLSL word - rename it (e.g. `lineSeg`) |
| Heavy loops with many function calls | The legacy compiler unrolls them and can take minutes or hang. Start the loop with `for (int i = min(frame,0); ...)` to keep it rolled, or reduce the iteration count |
| `texture(iChannel0, p)` inside a loop | Very slow to compile (gradient instruction in a loop). Replace it with procedural noise or a hash |
| `float3 arr[N];` filled at runtime | The legacy compiler rejects arrays assigned in a function (`X3017`). Use a helper with `if`/`else if` chains returning the values instead |
| `float3 col = vec3(0.7,...)*someGlobal;` (global) | Global initializers must be literals (`X3011`). Initialize with a literal or assign the value at the start of `shader_body` |
| `Foo f = Foo(a, b);` (struct) | Struct constructors are rejected (`X3037`) - use field assignment; `static` struct globals are also rejected (`X3017`), so store the fields in separate globals |
| `if (myFloat4 != float4(0,0,0,0))` | The legacy compiler wants a scalar condition (`X3019`). Compare the components individually |
| `const float x = otherVar + 1.;` (local) | The initializer must be a literal (`X3011`) - drop `const` (or inline the value) |
| `for`-loop with a dynamic start (`min(frame,0)`) in a *small* loop | Can make the compiler give up (`X3511: loop does not appear to terminate`). Only use the trick for large/heavy loops |

# Milkwave Visualizer specifics

Milkwave Visualizer is based on MilkDrop2, supporting all its options and settings. In addition, some new variables were introduced to give preset authors more possibilities while keeping presets compatible to other MilkDrop2 based visualizers (eg. NestDrop, BeatDrop Music Visualizer or MilkDrop 3).

## "_smooth" preset variables

Milkwave 3 introduced these addtional variables for preset authors to use:
-  _bass_smooth, mid_smooth, treb_smooth, vol_smooth_

These provide much more softed versions than the standard _\*\_att_ variables, meaning the value change is much more subtle between each frame. If you want to use them in your preset but also stay compatible with other MilkDrop based visualizers, you can use the following code snippet:
```
#ifndef bass_smooth
#define bass_smooth bass_att
#endif
```
Or wrap your code in a conditional block:
```
#ifdef bass_smooth
float3 myColor = float3(sin(bass_smooth)+1, 0, 0);
#endif
```
## "vis_intensity", "vis_shift" and "vis_version" preset variables

Presets can use these variables to modify the intensity or other appearance aspects of a preset. The default values are 1.0 (float) for vis_intensity, 0.0 for vis_shift (float) and 1 for version (integer). Users can adjust these values live from the "Settings" tab in the Remote while the preset is running. They can also be assigned to MIDI controls. Press Ctrl+V in the Visualizer to cycle the version between 1, 2 and 3; the active version is shown as a notification, and Remote changes show the same notification.

See the Shader presets included with Milkwave for examples that make use of these parameters.

As with the _smooth parameters above, you may want to use defines like this in your preset to stay compatible with other MilkDrop based visualizers:
```
#ifndef vis_intensity
#define vis_intensity 1
#endif
```

## "colshift_hue", "colshift_saturation" and "colshift_brightness" preset variables

These all default to 0 and can be changed in the Remote settings tab. The values are used in the _shiftHSV()_ function which is included in Milkwaves _include.fx_ file.

If you've played around with one of these values and want to save it to the preset file, you would add something like this at the end of your comp shader definition: 

```
#ifdef colshift_saturation
  colshift_saturation += 0.5;
#endif
```

Note that this would be a Milkwave specific change that would not port over to other MilkDrop variants since they use a different _include.fx_. If you want a portable version, include your own version of _shiftHSV()_ in your preset and call that. See the "SnoopethDuckDuck + IkeC - Breeze" preset for an example.

# Closing Notes

This manual was written by IkeC. If you want to contribute to this manual or need further help, it's a good idea to [open an issue on GitHub](https://github.com/IkeC/Milkwave/issues) or [join the Milkwave Discord server](https://bit.ly/Ikes-Discord). The latter is also a good place to share your presets or discuss with other users.

# Appendix: Milkwave IPC Protocol (MIP)

MIP is the text protocol Milkwave Remote uses to control the Visualizer, the MDropDX12 fork and other compatible MilkDrop variants. It runs over a local named pipe or over TCP, carries one command per message, and reports state back to connected clients. This appendix documents the wire protocol itself, so you can drive the Visualizer from your own script, tool or integration without the Remote.

MIP has no version negotiation and no request/response correlation; the TCP transport only adds an `AUTH` handshake before the first command. A client connects, sends commands, and listens if it wants state updates. Commands are matched by name and executed on the Visualizer's UI thread, in the order they were sent per connection; unknown commands and unknown `SIGNAL|` names are ignored without an error, which is what lets a client written against an older command set keep working when commands are added. Work that takes longer than one frame - loading a preset, compiling shaders, precompiling the cache - continues in the background, so it is not necessarily finished when the next command is processed.

## Transports

### Named pipe

Each running Visualizer hosts exactly one pipe, named after its process ID:

```
\\.\pipe\Milkwave_<PID>
```

| Property | Value |
| --- | --- |
| Direction | Duplex: commands in, notifications out |
| Framing | Message mode, one `Write` is one message |
| Encoding | UTF-16LE, terminated with a NUL `wchar_t` |
| Clients | Several concurrent clients (Remote, MCP server and test tools at the same time) |
| Message size | A write may exceed 64 KB; the server assembles the parts (hard cap 4 million characters) |
| Access | Full access for `Everyone`, so the machine itself is the trust boundary |

Discover instances by enumerating the pipe namespace (`\\.\pipe\*`) for names starting with `Milkwave_` and taking the number as the PID - that is how the Remote fills its instance list. A name whose process no longer exists is stale and fails to open.

A client that connects and writes `STATE` receives the complete state snapshot, which is the simplest way to learn the current state after connecting.

### TCP

The TCP transport carries the same messages to remote machines. It is off by default; start it with Ctrl+Shift+N in the Visualizer, or set `TcpEnabled=1` in the `[Network]` section of _settings.ini_. `TcpPort` (default 9270) and `TcpPIN` configure port and PIN.

| Property | Value |
| --- | --- |
| Framing | 4-byte little-endian payload length, then the UTF-8 payload |
| Max frame | 4 MB; a larger frame closes the connection |
| Clients | 16 concurrent connections, 60 s idle timeout, TCP keepalive |
| Authentication | Required before any command |

Every connection starts with the handshake:

```
AUTH|<pin>|<deviceId>|<deviceName>
```

`deviceId` identifies the client install and is used to evict an older connection of the same device, `deviceName` is what the Visualizer lists as a client. The server answers with one of:

| Reply | Meaning |
| --- | --- |
| `AUTH_OK` | Authenticated, commands may follow |
| `AUTH_FAIL\|WRONG_PIN` | PIN mismatch; the connection is closed |
| `AUTH_FAIL\|MALFORMED` | Fewer than four fields; the connection stays open, so retry |

Milkwave accepts a client as soon as the PIN matches, or immediately when no PIN is configured. MDropDX12 can require manual approval and may answer `AUTH_PENDING` first, so a client should tolerate that reply. After authentication `PING` is answered with `PONG`, and every other frame is treated as a MIP command.

Two differences from the pipe matter when writing a network client:

- Commands over TCP are fire-and-forget. This Milkwave build pushes state notifications to named pipe clients only, so a network client cannot wait for `SETTINGS|` or `PRESET=` replies. Read state over a local pipe with `STATE` when you need it.
- `SPOUT_SENDER=` is handled by the pipe transport directly and therefore has no effect over TCP.

## Message format

| Form | Example | Used for |
| --- | --- | --- |
| Bare name | `CLEARPRESET` | Commands without an argument, and toggles |
| `NAME=value` | `VAR_QUALITY=0.5` | A single argument, taken verbatim to the end of the message |
| `PREFIX\|key=value\|key=value` | `WAVE\|MODE=1\|ALPHA=0.8` | Grouped parameters |

- Command names and keys are case-sensitive and uppercase on the wire.
- Only the first `=` separates a name from its value, so a value may contain further `=` characters.
- `|` separates the fields of a grouped message, so text values (messages, paths, lyrics) must not contain a literal `|`; the Visualizer sanitizes the state strings it emits the same way. The one exception is `SIGNAL|SET_INPUTMIX_LUMAKEY=<threshold>|<softness>`, which uses `|` between its two values.
- Numbers always use `.` as the decimal separator - a localized comma is not accepted.
- Booleans are `1` and `0`.
- Paths are absolute Windows paths. `PRESET=` switches the preset directory to the file's folder before loading, so presets still resolve their neighbours correctly.
- Names are matched as prefixes, so extra text appended to a command that takes no value is not rejected. Send exactly what the tables below name.

## Commands

### Signals

`SIGNAL|<name>` triggers a fixed action instead of setting a value. These signals are handled by Milkwave Visualizer:

| Signal | Effect |
| --- | --- |
| `SIGNAL\|FULLSCREEN` | Toggle fullscreen (Alt+Enter), leaving borderless fullscreen if that is active |
| `SIGNAL\|BORDERLESS_FS` | Toggle borderless fullscreen covering the work area (Ctrl+Alt+Enter) |
| `SIGNAL\|WATERMARK` | Toggle Watermark Mode: borderless, always on top, click-through and semi-transparent (Ctrl+Shift+F9) |
| `SIGNAL\|DESKTOP_MODE` | Toggle Desktop Mode behind the desktop icons (Ctrl+F9) |
| `SIGNAL\|OPEN_REMOTE` | Show the Milkwave Remote window, starting it if necessary |
| `SIGNAL\|SETVIDEODEVICE=<index>` | Select the video input device by index |
| `SIGNAL\|ENABLEVIDEOMIX=<0\|1>` | Enable or disable webcam mixing |
| `SIGNAL\|ENABLESPOUTMIX=<0\|1>` | Enable or disable Spout input mixing |
| `SIGNAL\|SET_INPUTMIX_ONTOP=<0\|1>` | Put the input source above (`1`) or below the preset |
| `SIGNAL\|SET_INPUTMIX_OPACITY=<0-100>` | Opacity of the input layer, in percent |
| `SIGNAL\|SET_INPUTMIX_LUMAKEY=<threshold>\|<softness>` | Luma key; both values in percent, a threshold of `-1` turns the key off |

MDropDX12 understands a larger signal table (extra display signals plus `NEXT_PRESET`, `PREV_PRESET`, `SHOW_COVER`, `STRETCH`, `MIRROR`, `ALWAYS_ON_TOP`) and dispatches from a different message base. Milkwave accepts the five notification names listed further below on the wire for compatibility, but has no handler for them - send `SEND=0x20` (Space) instead of `SIGNAL|NEXT_PRESET`, for example. `SIGNAL|CAPTURE` is accepted as well and also does nothing; use the `CAPTURE` command in the diagnostics section.

### Presets and playback

| Command | Effect |
| --- | --- |
| `PRESET=<path>` | Switch to the preset's folder and load the file with a one second blend |
| `CLEARPRESET` | Unload the current preset (blank output) |
| `QUICKSAVE` | Save the running preset to `resources\presets\Quicksave` |
| `QUICKSAVE2` | Save the running preset to `resources\presets\Quicksave2` |
| `LINK=<0\|1>` | Preset link: while enabled, automatic preset changes are delegated to the Remote |
| `SEND=<text>` or `SEND=0x<hex>` | Send keystrokes: with `0x` the value is posted as a virtual-key code, otherwise each character is posted as a key |

`SEND=0x20` (Space) moves to the next (or a random) preset, `SEND=0x52` (R) switches between random and sequential order, `SEND=~` toggles the preset lock - exactly what pressing those keys does.

### Messages and sprites

| Command | Effect |
| --- | --- |
| `MSG\|<key>=<value>\|...` | Show a message. `text` is required; the other keys are optional |
| `AMP\|l=<left>\|r=<right>` | Set the horizontal amplification of the left and right channel |
| `CLEARSPRITES` | Kill all sprites |
| `CLEARTEXTS` | Kill all messages |
| `SPRITE_KILL_MODE` | Switch numeric input to sprite-kill mode (Shift+K) |
| `CLEAR_OLDEST_SPRITE` | Kill the sprite that was started first (Shift+Delete) |
| `CLEAR_ALL_SPRITES_MESSAGES` | Kill every sprite (Shift+Ctrl+Delete) |

`MSG|` keys:

| Key | Meaning |
| --- | --- |
| `text` | Message text (required) |
| `font`, `size` | Font face and size |
| `x`, `y` | Position, 0-1 of the window |
| `startx`, `starty`, `movetime`, `easemode`, `easefactor` | Entry animation start, duration and easing |
| `randx`, `randy` | Random offset added to the position |
| `growth` | Size change over the message's lifetime |
| `time`, `fade`, `fadeout`, `burntime` | Duration, fade-in, fade-out and burn-in time |
| `bold`, `ital` | Font style flags |
| `r`, `g`, `b` | Text color, 0-255 |
| `randr`, `randg`, `randb` | Random component added to the color |
| `shadowoffset` | Drop shadow offset |
| `box_alpha`, `box_col=r,g,b` | Background box opacity and color |
| `box_left`, `box_right`, `box_top`, `box_bottom` | Background box bounds |

### Audio and equalizer

| Command | Effect |
| --- | --- |
| `DEVICE=<name>`, `DEVICE=IN\|<name>`, `DEVICE=OUT\|<name>` | Switch the audio device; `IN` selects a capture device, `OUT` a render device |
| `AUDIO_DEVICE_DEFAULT` | Return to the system default audio device (Ctrl+D) |
| `AUDIO_DEVICE_INFO` | Show the active device name as a notification (Shift+Ctrl+D) |
| `AUDIO_TEST=<bass>,<mid>,<treb>[,notify][,wave]` | Replace the analysed levels with fixed relative values (`1.0` = track average); `AUDIO_TEST=off` returns to live audio |
| `EQ_ATTACK=<0-1>`, `FFT_ATTACK=<0-1>` | Equalizer attack; both names set the same value |
| `EQ_DECAY=<0-1>`, `FFT_DECAY=<0-1>` | Equalizer decay |
| `EQ_BOOST=<0-9.99>`, `FFT_BOOST=<0-9.99>` | Equalizer boost |

### Wave, color and quality

| Command | Effect |
| --- | --- |
| `WAVE\|<key>=<value>\|...` | Change wave and warp parameters; keys are `MODE`, `ALPHA`, `COLORR`, `COLORG`, `COLORB` (0-255), `PUSHX`, `PUSHY`, `ZOOM`, `WARP`, `ROTATION`, `DECAY`, `SCALE`, `ECHO`, `BRIGHTEN`, `DARKEN`, `SOLARIZE`, `INVERT`, `ADDITIVE`, `DOTTED`, `THICK`, `VOLALPHA` (0/1) |
| `COL_HUE=<float>`, `COL_SATURATION=<float>`, `COL_BRIGHTNESS=<float>` | Color shift applied to the output |
| `HUE_AUTO=<0\|1>`, `HUE_AUTO_SECONDS=<seconds>` | Automatic hue cycling and its period |
| `HUE_SHIFT_UP`, `HUE_SHIFT_DOWN` | Nudge the hue by 0.02 (Ctrl+H / Shift+Ctrl+H) |
| `VAR_TIME=<float>`, `VAR_FRAME=<float>`, `VAR_FPS=<float>` | Time, frame and FPS factors handed to the preset |
| `VAR_INTENSITY=<float>`, `VAR_SHIFT=<float>`, `VAR_VERSION=<float>` | The Milkwave preset variables |
| `CYCLE_VERSION` | Cycle `vis_version` 1 → 2 → 3 (Ctrl+V) |
| `VAR_QUALITY=<0.01-1>`, `VAR_AUTO=<0\|1>` | Render quality factor, or automatic quality |
| `QUALITY_HALF`, `QUALITY_DOUBLE` | Halve or double the current quality (Shift+Ctrl+Q / Ctrl+Q) |

### Window, Spout and input mixing

| Command | Effect |
| --- | --- |
| `OPACITY=<0-1>` | Window opacity. Note the asymmetry: commands take 0-1 while the `OPACITY=` notification reports 0-100 |
| `OPACITY_UP`, `OPACITY_DOWN` | One opacity step brighter or dimmer (Shift+Up / Shift+Down, or Shift+mouse wheel) |
| `STRETCH_MONITORS` | Toggle stretching across all monitors (Alt+S) |
| `WINDOW_RESET` | Reset window size and position (Ctrl+F2) |
| `WINDOW_FIXED_SIZE` | Toggle the fixed window size (Shift+Ctrl+F2) |
| `SHOW_FPS_SETTING` | Show the current FPS limit as a notification (Ctrl+F3) |
| `BLACKMODE` | Toggle black mode, which blanks the output (Ctrl+F12) |
| `SPOUT_ACTIVE=<0\|1>` | Turn the Spout sender on or off |
| `SPOUT_FIXEDSIZE=<0\|1>` | Set the fixed Spout resolution on or off |
| `SPOUT_RESOLUTION=<width>x<height>` | Set the fixed Spout resolution |
| `SPOUT_SENDER=<name>` | Rename the Spout sender (named pipe only) |
| `VIDEOINPUT=<0\|1>\|<device>` | Enable or disable webcam mixing; the device part is currently ignored, use `SIGNAL\|SETVIDEODEVICE=` |
| `SPOUTINPUT=<0\|1>\|<sender>` | Select the Spout sender and enable or disable Spout input mixing |

### Toggles

These accept `=1` or `=0` to set the state, or the bare name to flip it:

| Command | Toggles |
| --- | --- |
| `PRESET_CHANGE_WITH_SONG` | Change the preset when the track changes (Ctrl+A) |
| `DISPLAY_COVER` | Show cover art when the song changes (Ctrl+C) |
| `SONGINFO_POLLING` | Show song info (Ctrl+B) |
| `PRESETINFO_ON_CHANGE` | Flash the preset info on preset change (Ctrl+F4) |
| `INVERT_PRESET` | Invert the preset output |
| `SPOUT_FIXEDSIZE` | Fixed Spout resolution |
| `LYRICS_ACTIVE` | Lyrics overlay (Ctrl+L) |

### Lyrics

| Command | Effect |
| --- | --- |
| `LYRICS_AUTO=<0\|1>` | Look up lyrics for the current track automatically |
| `LYRICS_LOAD=<path>` | Load a lyrics file |
| `LYRICS_RESTART` | Restart the lyrics timeline |
| `LYRICS_CLEAR_LINE` | Clear the current line |
| `LYRICS_FONT=<face>`, `LYRICS_FONTSIZE=<8-256>` | Font face and size |
| `LYRICS_FONTBOLD=<0\|1>`, `LYRICS_FONTITALIC=<0\|1>`, `LYRICS_FONTAA=<0\|1>` | Font style and antialiasing |
| `LYRICS_COLOR=<r>,<g>,<b>`, `LYRICS_OUTLINE_COLOR=<r>,<g>,<b>` | Text color and outline/box color |
| `LYRICS_POSX=`, `LYRICS_POSY=`, `LYRICS_STARTX=`, `LYRICS_STARTY=` | Position and start position of the entry animation, 0-1 |
| `LYRICS_ZOOM=<0.05-10>`, `LYRICS_FADE=<0-10>`, `LYRICS_WIDTH=<0.05-1>` | Zoom, fade time and maximum line width |
| `LYRICS_OFFSET=<seconds>` | Timing offset, -600 to 600 |
| `LYRICS_BURN=<seconds>`, `LYRICS_BURNTYPE=<0-3>` | Burn-in time and mode |
| `LYRICS_SHADOW=<0-16>`, `LYRICS_OUTLINE=<0-5>`, `LYRICS_OUTLINE_AMOUNT=<0-16>` | Shadow and outline |
| `LYRICS_AUTOSCALE=<0\|1>` | Fit the text to the window |

### Settings, diagnostics and housekeeping

| Command | Effect |
| --- | --- |
| `STATE` | Request a state snapshot (see below) |
| `SETTINGS` | Re-read the preset timing from _settings.ini_ and re-arm the change timer |
| `CONFIG` | Reload _settings.ini_, fonts included |
| `TESTFONTS` | Show sample song info, preset info and a notification |
| `CAPTURE` | Save the current frame as `capture\<yyyyMMdd-HHmmss>-<preset>.png` |
| `PRECOMPILE_CACHE` | Start precompiling the shaders from _precompile.txt_ |
| `TEXTBORDER=<0-16>`, `TEXTBORDER_COLOR=<r>,<g>,<b>` | Outline thickness and color for messages and notifications |
| `MOUSE_INTERACTION` | Toggle mouse interaction (Ctrl+M) |
| `NETWORK_SERVER` | Start or stop the TCP server (Ctrl+Shift+N) |
| `NETWORK_SERVER_STATUS` | Show the TCP server state and its clients (Ctrl+N) |

## Notifications

The Visualizer pushes state to every connected named pipe client:

| Message | Content |
| --- | --- |
| `PRESET=<path>` | The preset that is now running, immediately followed by a `WAVE\|` snapshot |
| `WAVE\|...` | The active wave and warp parameters, with the same keys as the command |
| `SETTINGS\|<key>=<value>\|...` | Full settings snapshot |
| `OPACITY=<0-100>` | Window opacity, in percent |
| `DEVICE=<name>` | The audio device the Visualizer fell back to after a device change or a failed capture start |
| `VERSION=<1-3>` | The active `vis_version` after cycling it in the Visualizer with Ctrl+V |
| `STATUS=<text>` | Status text, including shader compile errors |
| `LYRICSSTATUS=`, `LYRICSLINE=`, `LYRICSFILE=` | Lyrics status, current line and file |
| `SIGNAL\|NEXT_PRESET`, `SIGNAL\|PREV_PRESET` | The Visualizer asks the Remote to move the preset selection (preset link) |
| `SIGNAL\|COVER_CHANGED` | Cover art was refreshed |
| `SIGNAL\|SPRITE_MODE`, `SIGNAL\|MESSAGE_MODE` | The numeric input mode changed |

`SETTINGS|` keys: `ACTIVE`, `FIXEDSIZE`, `FIXEDWIDTH`, `FIXEDHEIGHT`, `QUALITY`, `AUTO`, `HUE`, `LOCKED`, `RANDOM`, `CHANGEWITHSONG`, `DISPLAYCOVER`, `SONGINFOPOLLING`, `PRESETINFOONCHANGE`, `PRESETINFO`, `INVERTPRESET`, `INPUTTOP`, `LUMAACTIVE`, `LUMATHR`, `LUMASOFT`, `EQATTACK`, `EQDECAY`, `EQBOOST`, `LYRICSACTIVE`, `LYRICSAUTO`, `LYRICSSTATUS`, `LYRICSLINE`, `LYRICSFILE`, `LYRICSFONT`, `LYRICSFONTSIZE`, `LYRICSFONTBOLD`, `LYRICSFONTITALIC`, `LYRICSFONTAA`, `LYRICSCOLORR/G/B`, `LYRICSPOSX/Y`, `LYRICSSTARTX/Y`, `LYRICSZOOM`, `LYRICSFADE`, `LYRICSOFFSET`, `LYRICSWIDTH`, `LYRICSBURN`, `LYRICSBURNTYPE`, `LYRICSSHADOW`, `LYRICSOUTLINE`, `LYRICSOUTLINEAMOUNT`, `LYRICSOUTLINECOLORR/G/B`, `LYRICSAUTOSCALE`, `SHOWTABPRESET` … `SHOWTABSHADER`, `TEXTBORDER`, `TEXTBORDERCOLORR/G/B`.

Match notifications on the prefix and the delimiter, and ignore keys you do not know, so added keys cannot break your client. Messages that report a single change are dropped when they would arrive less than 100 ms after the previous one; the consolidated `PRESET=`/`WAVE|` and `SETTINGS|` snapshots and the lyrics updates are always sent. A burst of messages after a command, or one message type arriving without another, is therefore normal - do not use notifications to count events.

`STATE` reproduces a fresh connection's snapshot: `OPACITY=`, then `PRESET=` with its `WAVE|`, then `SETTINGS|`, then a `SIGNAL|` for the current input mode.

## Example: control the Visualizer with a script

PowerShell - connect to a Visualizer pipe and load a preset:

```powershell
$name = (Get-ChildItem '\\.\pipe\' | Where-Object Name -like 'Milkwave_*' | Select-Object -First 1).Name
$pipe = [System.IO.Pipes.NamedPipeClientStream]::new('.', $name, 'InOut')
$pipe.Connect(2000)

$message = 'PRESET=C:\Milkwave\resources\presets\Milkwave\My Preset.milk'
$bytes = [Text.Encoding]::Unicode.GetBytes($message + "`0")
$pipe.Write($bytes, 0, $bytes.Length)
$pipe.Flush()
$pipe.Dispose()
```

Notifications arrive on the same pipe as UTF-16LE text terminated by a NUL character, so a reader can split the stream on `\0`.

PowerShell - connect to a remote Visualizer over TCP and advance the preset:

```powershell
$client = [Net.Sockets.TcpClient]::new('192.168.1.100', 9270)
$stream = $client.GetStream()

function Send-Mip([string]$text) {
  $payload = [Text.Encoding]::UTF8.GetBytes($text)
  $length = [BitConverter]::GetBytes([uint32]$payload.Length)
  $stream.Write($length, 0, 4)
  $stream.Write($payload, 0, $payload.Length)
  $stream.Flush()
}

function Read-Mip {
  $header = [byte[]]::new(4); $read = 0
  while ($read -lt 4) { $read += $stream.Read($header, $read, 4 - $read) }
  $size = [BitConverter]::ToUInt32($header, 0)
  $payload = [byte[]]::new($size); $read = 0
  while ($read -lt $size) { $read += $stream.Read($payload, $read, $size - $read) }
  [Text.Encoding]::UTF8.GetString($payload)
}

Send-Mip 'AUTH||my-tool|My Tool'   # empty PIN
Read-Mip                           # AUTH_OK
Send-Mip 'SEND=0x20'               # Space: next preset
$client.Dispose()
```

## Security notes

- The named pipe is created with a security descriptor that grants full access to `Everyone`. The name contains the PID and is not authenticated, so treat any local process as able to send commands.
- The TCP server listens on all interfaces and encrypts nothing: the PIN and every command travel in clear text. Enable it only on a trusted network, set a PIN, and restrict the port in Windows Firewall.
- `TcpPIN` is stored as plain text in _settings.ini_.
- MIP has no sandboxing: a client that can send commands can also save files (`CAPTURE`, `QUICKSAVE`), load arbitrary presets and post keystrokes (`SEND=`).