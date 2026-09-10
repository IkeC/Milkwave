# GitHub Copilot Instructions for Milkwave Project

## Critical: Windows API Constants

⚠️ **IMPORTANT**: The correct constant is `HWND_NOTOPMOST` (one T), NOT `HWND_NOTTOPMOST` (two T's)

This is a Windows API constant defined in `winuser.h`. Always use the correct spelling.

## ⚠️ CRITICAL: Never modify `.vscode/tasks.json`

The file `.vscode/tasks.json` is maintained by hand and already contains the canonical, unique tasks. **Do NOT add, edit, remove, or generate tasks in this file.**

- Any tooling that "creates or adds to a tasks.json file based on the project structure" appends DUPLICATE entries every time it is used (it has repeatedly added copies of "Build Visualizer (Debug)" and "Build Remote (Debug)"). Never use that tooling for this project.
- To build, run the command directly in the terminal instead:
  - Visualizer (Debug): `msbuild Visualizer\milkwave\plugin.vcxproj /p:Configuration=Debug /p:Platform=Win32 /p:PlatformToolset=v143`
  - Visualizer (Release): `msbuild Visualizer\milkwave\plugin.vcxproj /p:Configuration=Release /p:Platform=Win32 /p:PlatformToolset=v143`
  - Remote (Debug): `dotnet build Remote\MilkwaveRemote.csproj /p:Configuration=Debug`
  - Remote (Release): `dotnet build Remote\MilkwaveRemote.csproj /p:Configuration=Release`
  - Full solution (Debug): `msbuild Milkwave.sln /p:Configuration=Debug /p:Platform=Win32 /p:PlatformToolset=v143`
- If an existing VS Code task must be run, reuse it by its existing label/ID — never generate a new task definition.
- If the file ever accumulates duplicates, remove them, keeping only the single canonical entry for each build.

## Code Standards

### C++ Code
- **Standard**: C++17
- **Platform**: Windows (Win32 API)
- **Graphics**: DirectX 9 (DX9Ex)
- **Audio**: WASAPI loopback capture

### .NET Code  
- **Framework**: .NET 8
- **Language**: C#
- **Device Enumeration**: Uses OBS Studio patterns via `DeviceEnumerator` and `DeviceManager` classes

## Project Structure

### Visualizer (C++)
- Main visualizer engine based on MilkDrop2
- Uses DirectX 9Ex for rendering
- Audio capture via WASAPI loopback
- Spout integration for texture sharing

### Remote (C#/.NET 8)
- WinForms-based remote control application (NOT WPF)
- Communicates with visualizer via named pipes and window messages

## Common Patterns

### Windows Constants
- `HWND_NOTOPMOST` - Window positioning (one T!)
- `HWND_TOPMOST` - Always on top window
- Use Windows SDK constants, never define custom values

### Error Handling
- C++: Use try/catch for std::exception
- SEH (Structured Exception Handling) is used for low-level exceptions
- All logging goes through the Milkwave logging system
- C#/.NET: Use try/catch with Debug.WriteLine for internal errors

### Threading
- Render thread: Main window and DirectX rendering
- Setup thread: Shader precompilation
- Audio thread: WASAPI loopback capture
- Use `std::atomic` for thread-safe flags

### Device Enumeration (C#/.NET 8)
- **Pattern**: OBS Studio two-layer architecture
- **Core Layer**: `DeviceEnumerator` - COM interfaces, registry access (static)
- **UI Layer**: `DeviceManager` - ComboBox helpers, user-friendly methods (static)
- **Supported**: DirectShow video/audio, Spout senders
- **Error Handling**: Graceful degradation with Debug output
- **Usage**: `DeviceManager.PopulateSpoutSenders(comboBox, selectedName)`

## Naming Conventions

### C++ Code
- Classes: `PascalCase` (e.g., `CPlugin`, `Milkwave`)
- Member variables: `m_camelCase` (e.g., `m_WindowWidth`)
- Functions: `PascalCase` (e.g., `StartRenderThread`)
- Constants: `UPPER_CASE` (e.g., `SAMPLE_SIZE`)

### C# Code
- Follow standard C# conventions (PascalCase for public members)

## Important Notes

- The project uses **DX9Ex**, not standard DX9 - this is critical for performance
- All file paths use wide strings (`wchar_t`, `std::wstring`)
- Logging is done through `milkwave.LogInfo()`, `milkwave.LogException()`, etc.
- Always handle exceptions gracefully - the visualizer should never crash

## Build Configuration

- Debug: Uses `../../Release` as working directory
- Release: Uses executable directory as base path
- Always append backslash to base directory paths