# Milkwave Project

Milkwave is a feature-enhanced MilkDrop2 music visualizer with a companion remote control app.

## Project Structure

### Visualizer (C++)
- Main visualizer engine based on MilkDrop2/BeatDrop
- **Standard**: C++17, Windows (Win32 API)
- **Graphics**: DirectX 9Ex (not standard DX9 — this matters for performance)
- **Audio**: WASAPI loopback capture
- **Spout** integration for texture sharing
- **Expression eval**: projectM-eval via ns-eel2 shim (see `Visualizer/ns-eel2-shim/`)
- Build outputs: Debug uses `../../Release` as working dir; Release uses exe directory

### Remote (C#/.NET 8)
- WPF-based remote control application
- Communicates with Visualizer via named pipes and window messages

## Critical Warnings

- `HWND_NOTOPMOST` has ONE T — never use `HWND_NOTTOPMOST`
- Use DX9**Ex** APIs, not standard DX9
- All file paths use wide strings (`wchar_t`, `std::wstring`)
- The visualizer should never crash — always handle exceptions gracefully

## Naming Conventions

### C++
- Classes: `PascalCase` (e.g., `CPlugin`, `Milkwave`)
- Member variables: `m_camelCase` (e.g., `m_WindowWidth`)
- Functions: `PascalCase` (e.g., `StartRenderThread`)
- Constants: `UPPER_CASE` (e.g., `SAMPLE_SIZE`)

### C#
- Standard C# conventions (PascalCase for public members)

## Threading Model

- **Render thread**: Main window and DirectX rendering
- **Setup thread**: Shader precompilation
- **Audio thread**: WASAPI loopback capture
- Use `std::atomic` for thread-safe flags

## Error Handling

- C++: `try/catch` for `std::exception`; SEH for low-level exceptions
- Logging via `milkwave.LogInfo()`, `milkwave.LogException()`, etc.
- `settings.ini`: `LogLevel=2` for verbose logging

## Key Features (current: v3.5-dev)

- Track info and artwork from Spotify/YouTube/media sources
- Preset change on track change; preset tagging system
- Window transparency, borderless, clickthrough ("watermark mode")
- GLSL-to-HLSL shader conversion with live preview
- MIDI automation (up to 50 controls)
- Hue/Saturation/Brightness color shifting
- Shader precompiling and caching
- Custom preset variables: `bass_smooth`, `mid_smooth`, `treb_smooth`, `vol_smooth`, `vis_intensity`, `vis_shift`, `vis_version`, `colshift_hue`

## System Requirements

- Windows 10+
- DirectX 9+ compatible GPU
- DirectX End-User Legacy Runtimes
- .NET Desktop Runtime 8 (for Remote)

## Licenses

- **Visualizer**: 3-Clause BSD License (as BeatDrop fork)
- **Remote**: CC BY-NC 4.0 (non-commercial)
