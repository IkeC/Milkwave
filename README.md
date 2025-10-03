Milkwave is:
- **Milkwave Visualizer**:
    A feature-enhanced version of [MilkDrop2](https://www.geisswerks.com/milkdrop/) visualizer
- **Milkwave Remote**:
    A standalone window to do things like sending messages (either directly by typing or from a script file), load and change presets, send common key combinations using buttons and more

[**Click here**](https://github.com/IkeC/Milkwave/releases/latest) to get the latest version.

![Milkwave 3.2](https://github.com/user-attachments/assets/c828786e-d4fc-40bf-a84b-ec996eeff1b8)

## Visualizer Features
 
* Display current track information and artwork from Spotify, YouTube or other media sources playing on your PC
* Change preset on track change
* Set window transparency, borderless, and clickthrough ("watermark mode")
* Use cursor keys for media playback control
* Over 5000 presets from skilled artists (more presets [here](https://github.com/projectM-visualizer/projectm?tab=readme-ov-file#presets))
* Improved window handling, input methods and stability
* Use independently or in combination with Milkwave Remote
 
## Remote Features

* Tabbed interface with hideable top and bottom panel

![Milkwave 3.2 Tabs](https://github.com/user-attachments/assets/a244f59b-8070-4314-be2d-ceb3259b33c5)

* Preset tagging
  - Tag your presets with any number of words of your choice
  - Dynamic buttons for your most used tags
  - Load preset lists based on tags
  - Saved in human-readable json file for easy sharing or backup
* Messaging
  - Send text to Visualizer window
  - Display multiple messages at once
  - Set color, font size, position and other parameters
  - Move text around using start and end coordinates
  - Save parameter definitions as named styles for quick access
  - Send messages, parameters, presets and other commands automatically from a script file at configurable intervals based on BPM setting (see script-default.txt)
* Shader code conversion
  - Convert GLSL shader code to HLSL
  - Preview converted code in the Visualizer instantly
  - Load shaders by id or query from Shadertoy.com using Shadertoy API
  - Load and save HLSL code to a file
* Wave manipulation
  - Clear current preset and start with a plain wave
  - Set wave type, zoom, warp, rotation etc. in the running preset
  - Quicksave manipulated presets to a new file
* Font manipulation
  - Customize display of song information, preset name and notifications
  - Change and preview font face, style and size instantly
* MIDI automation
  - Assign up to 50 actions to your MIDI controller
  - Select/change presets, send messages, trigger sprites, start external programs etc.
  - Trigger command chains from script
* Settings
  - Change internal time, FPS and frame counters to slow down or speed up some presets
  - Adjust "Intensity", "Shift" and "Version" live for supported presets (eg. Milkwaves shader presets)
  - Control Spout output settings
  - Adjust render quality to improve performance and/or pixellize output
  - Quick access to configuration files
* Customizable interface - show only what you need
* Send common key combinations using buttons
* Set audio device without restarting Visualizer
* Support for input devices (eg. microphones)
* Amplify virtual audio signal to make Visualizer more (or less) responsive

There are many tooltips explaining all features when you hover over the form elements. Clicking the "Autoplay" button on the Message tab will run the default script file, demonstrating many Milkwave features.

If you have any questions, don't be afraid to [ask for support](#support)!

## History

The original [MilkDrop2](https://www.geisswerks.com/milkdrop/) WinAmp plugin created by Ryan Geiss was turned into a Windows standalone application by Maxim Volskiy as [BeatDrop](https://github.com/mvsoft74/BeatDrop) and has since been improved upon eg. in the [BeatDrop-Music-Visualizer](https://github.com/OfficialIncubo/BeatDrop-Music-Visualizer) and MilkDrop3 projects.

For a more detailed explanation of all features, please read the [Manual](https://github.com/IkeC/Milkwave/blob/main/Build/Manual.md). 

For a chronological list of Milkwave releases and features, read the [Changes](Changes.md).

## System Requirements

* Windows 10 or later
* DirectX 9 or higher - compatible GPU
* [DirectX End-User Legacy Runtimes](https://www.microsoft.com/en-us/download/details.aspx?id=35)
* [Microsoft .NET Desktop Runtime 8](https://dotnet.microsoft.com/en-us/download/dotnet/8.0) (for Remote)

## Support

This project incorporates the work of many different authors over the years, as listed below. Naturally, the entirety of this project is Open Source and there will never be a paid version of it.

However, if you're satisfied with the additions I made and you want to support my work, you may do so using [**Ko-fi**](https://ko-fi.com/ikeserver) or [**PayPal**](https://www.paypal.com/ncp/payment/5XMP3S69PJLCU). You'll be listed as a supporter within the next program release unless you don't want to.

Many thanks to **Shanev** and **Tures1955** for supporting the development of Milkwave. ❤️

I may add new features or fix bugs if people are actually using this, so don't be shy to [open an issue](https://github.com/IkeC/Milkwave/issues) or join my [**Discord**](https://bit.ly/Ikes-Discord)!

## Acknowledgements

Many thanks to:

* Ryan Geiss - [MilkDrop2](https://www.geisswerks.com/milkdrop/)
* Maxim Volskiy - [BeatDrop](https://github.com/mvsoft74/BeatDrop)
* oO-MrC-Oo - [XBMC plugin](https://github.com/oO-MrC-Oo/Milkdrop2-XBMC)
* Casey Langen - [milkdrop2-musikcube](https://github.com/clangen/milkdrop2-musikcube)
* Matthew van Eerde - [loopback-capture](https://github.com/mvaneerde/blog)
* projectM - [projectm-eval](https://github.com/projectM-visualizer/projectm-eval)
* Incubo_ - [BeatDrop-Music-Visualizer](https://github.com/OfficialIncubo/BeatDrop-Music-Visualizer)
* milkdrop2077 - [MilkDrop3](https://github.com/milkdrop2077/MilkDrop3)
* podenthusiast - [Milkwave Logo](https://www.freepik.com/author/podenthusiast/icons)
* and all the preset authors!

If you believe you or someone else should be mentioned here, please let me know.

## License

[license]: #license

Milkwave Remote is licensed under the [Attribution-NonCommercial 4.0 International License](https://creativecommons.org/licenses/by-nc/4.0/), meaning you may not use the material for commercial purposes. See LICENSE.txt for details.

Milkwave Visualizer (as a BeatDrop fork) is licensed under the [3-Clause BSD License](https://opensource.org/licenses/BSD-3-Clause) with the exception of where otherwise noted. See LICENSE.txt for details.

Although the original Matthew van Eerde's [loopback-capture](https://github.com/mvaneerde/blog) project didn't explicitly state the license, the author has been kind enough to provide a license clarification [here](
https://blogs.msdn.microsoft.com/matthew_van_eerde/2014/11/05/draining-the-wasapi-capture-buffer-fully/). All changes in this repository to the original Matthew's code are published either under the terms of BSD license or the license provided by original author.

## Contributions

Unless you explicitly state otherwise, any contribution intentionally submitted for inclusion in the work by you, shall be licensed as above.
