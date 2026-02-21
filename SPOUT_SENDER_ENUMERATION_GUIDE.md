# Spout Sender Enumeration - Implementation Guide

## Current Milkwave Implementation

### Architecture Overview
Milkwave uses a **shared memory-based** approach to enumerate Spout senders via the `SpoutSenderNames` class.

### Key C++ Implementation Files
- **`Visualizer/spoutDX9/SpoutSenderNames.h`** - Header with all public functions
- **`Visualizer/spoutDX9/SpoutSenderNames.cpp`** - Implementation
- **`Visualizer/spoutDX9/SpoutSharedMemory.h`** - Shared memory management
- **`Visualizer/spoutDX9/SpoutSharedMemory.cpp`** - Shared memory operations

### Main Public Functions for Remote

#### Get All Sender Names
```cpp
bool GetSenderNames(std::set<std::string> *sendernames)
```
- Returns a set of all registered sender names
- **Best for**: Initial population of sender list

#### Get Sender Count
```cpp
int GetSenderCount()
```
- Returns total number of active Spout senders
- **Use case**: Knowing if senders exist before enumeration

#### Get Sender by Index
```cpp
bool GetSender(int index, char* sendername, int MaxSize = 256)
```
- Retrieves a single sender name by index (0-based)
- **Max senders**: 64 (default, configurable via registry)

#### Get Sender Details by Index
```cpp
bool GetSenderNameInfo(int index, char* sendername, int sendernameMaxSize, 
                       unsigned int &width, unsigned int &height, 
                       HANDLE &dxShareHandle)
```
- Returns sender name AND metadata (resolution, DirectX share handle)
- **Best for**: Displaying sender info in UI

#### Get Active Sender Information
```cpp
bool FindActiveSender(char *activename, unsigned int& width, unsigned int& height, 
                      HANDLE& hSharehandle, DWORD& dwFormat, 
                      const int maxlength = SpoutMaxSenderNameLen)
```
- Gets the currently active (last used) sender
- **Use case**: Default selection in dropdown

---

## obs-studio Code Location

### Windows Capture Plugin Structure
- **Main plugin directory**: `C:\Source\obs-studio\plugins\win-capture\`
- **Plugin entry point**: `plugins\win-capture\win-capture-plugin.c` (or similar)

### Relevant OBS Plugin Directories

#### 1. **win-capture** - DirectX Capture
```
C:\Source\obs-studio\plugins\win-capture\
├── d3d8-capture.cpp
├── d3d9-capture.cpp          ← Similar to Milkwave DX9
├── d3d10-capture.cpp
├── d3d11-capture.cpp
├── d3d12-capture.cpp
├── dxgi-capture.cpp
└── graphics-hook.h
```
**Note**: OBS uses **hook-based capture**, not Spout-specific code directly.

#### 2. **win-dshow** - DirectShow Devices
```
C:\Source\obs-studio\plugins\win-dshow\
├── dshow-plugin.cpp          ← Device enumeration
├── win-dshow.cpp             ← Main implementation
└── win-dshow-encoder.cpp
```
**Purpose**: Device enumeration patterns (cameras, audio devices)

#### 3. **win-wasapi** - Audio API
```
C:\Source\obs-studio\plugins\win-wasapi\
└── [Audio device enumeration]
```

---

## Comparison: Milkwave vs OBS Approach

| Aspect | Milkwave | OBS Studio |
|--------|----------|------------|
| **Method** | Shared memory enumeration | Hook-based + Plugin system |
| **Spout Specific** | Yes (via SpoutSenderNames) | Hook-based (not Spout-specific) |
| **Language** | C++ (native) | C (with native plugins) |
| **Device Discovery** | Registry + shared memory maps | DirectShow/DXGI enumeration |
| **Sender Sync** | Manual registration via maps | Automatic via hook detection |

---

## Current Milkwave Remote Implementation

### Where Remote Gets Spout Senders
- **File**: `Remote/MilkwaveRemoteForm.cs` (C# .NET 8)
- **Method**: P/Invoke calls to C++ SpoutSenderNames through visualizer process

### Current Flow
1. Remote sends message to Visualizer (named pipe or window message)
2. Visualizer calls `SpoutSenderNames::GetSenderNames()`
3. Sender list is marshaled back to C# Remote
4. Remote populates dropdown UI

**Challenge**: This requires serialization between C# and C++

---

## Recommendations for Improvement

### Option 1: Direct Spout Memory Map Access (C#)
**Pros:**
- No dependency on visualizer running
- Faster enumeration
- Independent Remote operation

**Implementation:**
```csharp
// Create P/Invoke bindings for:
// - OpenFileMapping() - Access shared memory
// - MapViewOfFile() - Map memory to C# process
// - Parse Spout shared memory format directly
```

**Files to Reference:**
- `Visualizer/spoutDX9/SpoutSharedMemory.h` - Memory format
- `Visualizer/spoutDX9/SpoutSenderNames.cpp:readSenderSetFromBuffer()` - Parsing logic

### Option 2: Wrapper DLL Export
**Pros:**
- Clean abstraction
- Better error handling
- Reusable from other apps

**Implementation:**
```cpp
// Export from Visualizer DLL:
extern "C" {
    int __stdcall GetAvailableSpoutSenders(wchar_t** senderArray, int maxSenders);
    void __stdcall FreeSpoutSenderArray(wchar_t** array);
}
```

### Option 3: Study OBS Pattern
**Purpose:** Understand how OBS handles device enumeration
**Key Files to Review:**
- `plugins\win-dshow\win-dshow.cpp` - DirectShow device enumeration pattern
- `plugins\win-capture\dxgi-capture.cpp` - DXGI resource enumeration

---

## Implementation Steps

### For C# .NET 8 Improvement
1. **Understand shared memory format** from `SpoutSharedMemory.cpp`
2. **Create P/Invoke bindings** for Windows shared memory APIs
3. **Parse sender data** directly in C# without visualizer dependency
4. **Test with multiple Spout sources** running

### Testing Checklist
- [ ] Enumerate senders when visualizer is OFF
- [ ] Handle missing Spout registry entries
- [ ] Support up to 64 concurrent senders
- [ ] Handle sender connect/disconnect in real-time
- [ ] Properly clean up memory map handles

---

## Key OBS Studio Files to Review

### Device Enumeration Pattern
```
C:\Source\obs-studio\plugins\win-dshow\win-dshow.cpp
  └─ enum_devices() function
     └─ Shows pattern for DirectShow device enumeration
```

### Error Handling
```
C:\Source\obs-studio\libobs\util\platform.h
  └─ Common Windows platform utilities
```

---

## Registry Information

### Spout Registry Location
```
HKEY_CURRENT_USER\Software\Leading Edge\Spout
├── MaxSenders = 64 (default)
└── [Sender-specific entries]
```

### Shared Memory Map Name
```
"SpoutSharedMemory" + "[SenderName]"
```

---

## References

- **Milkwave Spout Code**: `Visualizer/spoutDX9/`
- **Spout Official**: http://spout.zeal.co/
- **OBS Studio**: https://github.com/obsproject/obs-studio
- **Windows Shared Memory**: `CreateFileMapping()`, `MapViewOfFile()`

