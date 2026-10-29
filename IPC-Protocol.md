# Milkwave IPC Protocol

Milkwave IPC Protocol (MIP) is the text protocol Milkwave Remote uses to control the Visualizer, the MDropDX12 fork and other compatible MilkDrop variants. It runs over a local named pipe or over TCP, carries one command per message, and reports state back to connected clients. This appendix documents the wire protocol itself, so you can drive the Visualizer from your own script, tool or integration without the Remote.

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
| `MIXTYPE=<value>` | Blend transition type used by the next preset change; `-1` = Random, `0-20` = fixed type |
| `COVER_STYLE=<0-3>` | Cover placement: `0` = corner small, `1` = corner medium, `2` = corner large, `3` = fullscreen zoom |
| `SEND=<text>` or `SEND=0x<hex>` | Send keystrokes: with `0x` the value is posted as a virtual-key code, otherwise each character is posted as a key |

`SEND=0x20` (Space) moves to the next (or a random) preset, `SEND=0x52` (R) switches between random and sequential order, `SEND=~` toggles the preset lock - exactly what pressing those keys does.

`MIXTYPE` values match `[Settings] Mixtype`: `-1` = Random, `0` = Uniform Fade, `1` = Directional Wipe, `2` = Plasma, `3` = Radial, `4` = Clock, `5` = Spiral, `6` = Rhombus, `7` = Nuclear Clock, `8` = Square/Diamond, `9` = Checkerboard, `10` = Curtain, `11` = Bubbles, `12` = Kaleidoscope, `13` = Moebius Strip, `14` = Stars, `15` = Disco Floor, `16` = Fire, `17` = Drain Swirl, `18` = Julia Set, `19` = Vertical Wipe, `20` = Horizontal Wipe. Pressing T in the Visualizer cycles the same values.

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
| `FLASH_INFO` | Re-poll the track info/cover and flash the preset info (Ctrl+Space) |
| `CAPTURE` | Save the current frame as `capture\<yyyyMMdd-HHmmss>-<preset>.png` |
| `PRECOMPILE_CACHE` | Start precompiling the shaders from _precompile.txt_ |
| `TEXTBORDER=<0-16>`, `TEXTBORDER_COLOR=<r>,<g>,<b>` | Outline thickness and color for messages and notifications |
| `MOUSE_INTERACTION` | Toggle mouse interaction (Ctrl+M) |
| `NETWORK_SERVER` | Start or stop the TCP server (Ctrl+Shift+N) |
| `NETWORK_SERVER_STATUS` | Show the TCP server state and its clients (Ctrl+N) |
| `SHOWTAB<Name>=<0\|1>` | Show or hide a Remote tab; `<Name>` is `PRESET`, `MESSAGE`, `INOUT`, `SETTINGS`, `LYRICS`, `FONTS`, `MIDI`, `WAVE` or `SHADER` |

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

`SETTINGS|` keys: `ACTIVE`, `FIXEDSIZE`, `FIXEDWIDTH`, `FIXEDHEIGHT`, `QUALITY`, `AUTO`, `HUE`, `LOCKED`, `RANDOM`, `MIXTYPE`, `CHANGEWITHSONG`, `DISPLAYCOVER`, `COVERSTYLE`, `SONGINFOPOLLING`, `PRESETINFOONCHANGE`, `PRESETINFO`, `INVERTPRESET`, `INPUTTOP`, `LUMAACTIVE`, `LUMATHR`, `LUMASOFT`, `EQATTACK`, `EQDECAY`, `EQBOOST`, `LYRICSACTIVE`, `LYRICSAUTO`, `LYRICSSTATUS`, `LYRICSLINE`, `LYRICSFILE`, `LYRICSFONT`, `LYRICSFONTSIZE`, `LYRICSFONTBOLD`, `LYRICSFONTITALIC`, `LYRICSFONTAA`, `LYRICSCOLORR/G/B`, `LYRICSPOSX/Y`, `LYRICSSTARTX/Y`, `LYRICSZOOM`, `LYRICSFADE`, `LYRICSOFFSET`, `LYRICSWIDTH`, `LYRICSBURN`, `LYRICSBURNTYPE`, `LYRICSSHADOW`, `LYRICSOUTLINE`, `LYRICSOUTLINEAMOUNT`, `LYRICSOUTLINECOLORR/G/B`, `LYRICSAUTOSCALE`, `SHOWTABPRESET` … `SHOWTABSHADER`, `TEXTBORDER`, `TEXTBORDERCOLORR/G/B`.

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