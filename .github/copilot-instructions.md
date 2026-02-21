# GitHub Copilot Instructions for Milkwave Project

## Critical: Windows API Constants

⚠️ **IMPORTANT**: The correct constant is `HWND_NOTOPMOST` (one T), NOT `HWND_NOTTOPMOST` (two T's)

This is a Windows API constant defined in `winuser.h`. Always use the correct spelling.

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
- WPF-based remote control application
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

## External Dependencies

### Spout Sender Enumeration (Current Implementation)
- **Location**: `Visualizer\spoutDX9\SpoutSenderNames.*`
- **Key Classes**: 
  - `spoutSenderNames` - Manages Spout sender registration and discovery
- **Main Functions for Remote**:
  - `GetSenderNames(std::set<std::string> *sendernames)` - Retrieve all registered sender names
  - `GetSenderCount()` - Get number of active senders
  - `GetSender(int index, char* sendername, int MaxSize)` - Get sender by index
  - `GetSenderNameInfo(int index, ...)` - Get sender details (width, height, share handle)
  - `FindActiveSender(...)` - Get currently active sender
- **Shared Memory Access**: Uses Windows named shared memory maps to enumerate senders
- **Max Senders**: Configurable via registry (default: 64)

### obs-studio Integration Reference
- **Location**: `C:\Source\obs-studio`
- **Relevant Plugin**: `plugins\win-capture\` - DirectX capture plugins
- **Other Win Plugins**:
  - `plugins\win-dshow\` - DirectShow (video device enumeration)
  - `plugins\win-wasapi\` - Windows Audio Session API
- **Purpose**: Review OBS implementation for Spout sender enumeration improvements
- **Note**: OBS Studio may handle Spout discovery differently than Milkwave's direct shared memory approach
- **Potential Integration Points**:
  - Study OBS sender enumeration patterns for .NET interop improvements
  - Check if OBS uses registry or alternative methods for sender discovery