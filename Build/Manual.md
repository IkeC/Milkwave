# Milkwave Manual

Welcome to the Milkwave manual! This document will help you get started with the Milkwave Remote, a tool to control the Milkwave Visualizer.

In general, make sure to hover the mouse over all labels and buttons to see some inline help tooltips describing most of the features and displaying keyboard shortcuts.

If you need help with the Visualizer itself, press F1 there to see the on-screen help pages.

# Interface

The Tabs panel holds most features available in the Remote, described in detail below.

The Buttons panel allows to trigger some commonly used functions in the Visualizer without having to focus the Visualizer window. The Buttons labeled "00" to "99" will either trigger the sprites from _sprites.ini_ when Visualizer is in sprite mode (default), or the messages from _messages.ini_ when Visualizer is in message mode. Press K in Visualizer to change modes.

Note that you can choose to hide either the Tabs or the Button panel with  the popup menu that opens by clicking "Milkwave" in the bottom right corner. You may also use the menu to access help resources, find or open the Visualizer window and switch between light and dark mode.

## Tab "Presets"

Send a preset from the drop-down list to display in the Visualizer using "Send". The list will show the presets from _presets/Milkwave_ by default. You can fill the list using the buttons "Tags", "File" and "Dir".

The currently playing Visualizer preset is displayed after the "Running" label. After "Tags" you can supply your own set of tags describing the current preset, separated by comma. If "Running" is checked, the tags are loaded (and saved to) the currently running preset, if it is unchecked the preset from the drop-down list in the first line is used. Click "Save" to save the tag information to tags-remote.json.

Your most used tags are displayed as buttons automatically after the "Most used" label. Note that you may delete the file _tags-remote.json_ anytime if you want to start from scratch.

Your audio input and output devices are displayed after the "Device" label. If you don't wish to see input devices (such as microphones), set _settings.ini:IncludeInputDevices=0_. Use "Set" to set the Visualizer input source to the selected device. Use Ctrl+D in the Visualizer window to display the currently active device.

Use "Amp" to amplify the virtual audio level for the Visualizer, eg. if you feel the preset should react more to your music. If "Link" is checked, left and right audio channels are amplified equally.

## Tab "Message"

Use "Send" to send the text after "Message" to display in the Visualizer window. Toggle "Preview" to see a "what you see is what you get" version of your text. If "Wrap" is checked, text will be wrapped to two lines if the text is longer than the number in the drop-down box. You can manually define a line break within your text by writing "//". Note that this feature is rather experimental and may clip your text sometimes depending on your used font face and size.

The "Window" label shows the window title of the Visualizer that the Remote will send its commands to. You may start multiple Visualizer windows and control them all by changing the target window. Change the opacity of the Visualizer window using the "%" box.

Right-Click the "Parameters" label to display a list of all available parameters for the text message display. You can save a parameter line to the named slot after "Style" by pressing "Save".

Changing "Font", "Size" and "Color" parameters will change the text display. Note that these definitions can be "Set" to the "Parameters" line. If any of these values are defined in "Parameters", they will override your UI settings. To remove a set definition from "Parameters", you may double-click the "Font", "Size" and "Color" labels.

You can load a list of parameter lines from a file and send them to the Visualizer individually by pressing the "Send" button in the bottom right. By default, the file _script-default.txt_ from the Milkwave main directory is loaded, but you can load any file by right-clicking the "From File" label. Please read the documentation in _script-default.txt_ for a detailed description on possible commands.

Instead of selecting and sending lines from the fine manually, you may check the "Autoplay" to play them sequentially. Use the "BPM" and "Beats" parameters to control the interval between sending each line. Check the "Rand" button to send a random line on each interval (excluding the last sent line).

### Fade-in, Fade-Out and Burntime

You can define default values for fade-in ("fade", default 0.2), fade-out ("fadeout", default 0.0) and burntime ("burntime", default 0.1) in settings.ini, and override them per message as a parameter.

If a burntime > 0 is defined, the message will be "baked" into the background and slowly fade away for the defined burntime duration. Note that this won't work with all presets, and if you define a fadeout > 0, the burntime will be irrelevant because the message will fade out before there's something to "burn in".

## Tab "Shader"

This tab can convert GLSL shader code (eg. from Shadertoy.com) to HLSL shader code, save it to a preset file and display it in the Visualizer instantly.

You may load code from Shadertoy.com by entering an ID or URL and click "Load". You may load the n-th "newest", "hot" etc. entry from Shadertoy.com, or load code from a file. Of course, you can also simply paste code into the left pane. If loaded from Shadertoy, the shader author, name and URL will be put into the shaderinfo box below the panes.

You can show/hide the left pane with the "<" button. Click "Convert" to convert the GLSL code from the left pane to HLSL code in the right pane. Use "Send" to save a preset from the code and send it to the Visualizer.

The "File" checkbox determines whether the preset name is built from the shaderinfo line or not. If you test many different shader code snippets, you may not want to produce a (possibly not working) preset file everytime, so if "File" is unchecked, the default filename "Shader.milk" will be used (and overwritten each time). You can set the pixel shader version written to the preset file using the number box before the "Send" button. If unsure, leave it at 4.

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

## Tab "Wave"

Manipulate factors of the default wave of the current preset and display the changes instantly in the Visualizer. If the "Link" button is checked, values are received from and sent to the currently running preset instantly, if not click "Send" to send your values. Use "Clear" to display a default wave. 

Use the "Quicksave" button or press Ctrl+S in the Visualizer to save the current preset instantly to the _presets/Quicksave_ folder as a new file. Press Shift+Ctrl+S to save to _presets/Quicksave2_ instead.

## Tab "Fonts"

Modify most of the fonts used to display information in the Visualizer window. Use "Save" and "Test" to see your changes. You can save and preview changes instantly if you hold the ALT key while changing fonts or sizes.

Changes are saved to the _Fonts_ section in _settings.ini_. Of course you may edit them there manually as well.

## Tab "Settings"

Change the internal "Time", "FPS" and "Frame" values that the Visualizer sends to the preset. This may speed up, slow down or otherwise change the behaviour of the preset, depending on how the preset is built and how (or if) it uses any of these variables.

The "Intensity", "Shift" and "Version" values can be read by presets that support the Milkwave specific vis_intensity, vis_shift and vis_version variables (see below). As above, you can change these values live while a preset is running.

For [Spout](https://spout.zeal.co/), you can set the output to a "Fixed" resolution instead of the Visualizer window size. This may be useful if you want to use Milkwave as a source for other applications that expect a certain resolution. The Visualizer window will then use the fixed backbuffer size and aspect ratio for display.

With the "Quality" setting, you can reduce the size of the backbuffer used for rendering, eg. a quality factor of 0.5 will render to an internal buffer with half the width and height of your Visualizer window. This will improve performance on slower systems, but will also reduce visual quality. A low quality may also yield in a pixellated look, giving a nice retro effect. Note that the quality setting will be ignored if "Fixed" Spout resolution is used.

Use the buttons on the right side to open some commonly used files instantly in your associated text editor.

Keep in mind that most settings can be automated using script commands in the _script-default.txt_ file or your own script files. See the comments in _script-default.txt_ for details. They can also be MIDI-controlled (see below).

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

Presets can use these variables to modify the intensity or other appearance aspects of a preset. The default values are 1.0 (float) for vis_intensity, 0.0 for vis_shift (float) and 1 for version (integer). Users can adjust these values live from the "Settings" tab in the Remote while the preset is running. They can also be assigned to MIDI controls.

See the Shader presets included with Milkwave for examples that make use of these parameters.

As with the _smooth parameters above, you may want to use defines like this in your preset to stay compatible with other MilkDrop based visualizers:
```
#ifndef vis_intensity
#define vis_intensity 1
#endif
```

# Closing Notes

This manual was written by IkeC. If you want to contribute to this manual or need further help, it's a good idea to [open an issue on GitHub](https://github.com/IkeC/Milkwave/issues) or [join the Milkwave Discord server](https://bit.ly/Ikes-Discord). The latter is also a good place to share your presets or discuss with other users.