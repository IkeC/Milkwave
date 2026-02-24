# OBS-Style Device Enumeration in Milkwave Remote

## Overview

Milkwave Remote now implements device enumeration using the same patterns as OBS Studio. This provides:

- **DirectShow video device enumeration** (cameras, capture cards)
- **DirectShow audio device enumeration** (microphones)
- **Spout sender enumeration** (video texture sources)
- **Clean separation of concerns** with `DeviceEnumerator` and `DeviceManager` classes
- **Robust error handling** following OBS patterns

## Architecture

### Two-Layer Design (OBS Pattern)

```
┌─────────────────────────────────────┐
│  MilkwaveRemoteForm (UI)            │
├─────────────────────────────────────┤
│  DeviceManager (UI Integration)     │ ← Easy to use methods for ComboBoxes
├─────────────────────────────────────┤
│  DeviceEnumerator (Core Logic)      │ ← Platform/COM interaction
├─────────────────────────────────────┤
│  Windows API / DirectShow / Registry│ ← System interfaces
└─────────────────────────────────────┘
```

## Classes

### DeviceEnumerator (Static Utility)
**File**: `Remote/Helper/DeviceEnumerator.cs`

Core device enumeration logic using COM interfaces and Windows registry.

#### Methods

```csharp
// DirectShow Video Devices
List<DeviceItem> EnumerateVideoDevices()

// DirectShow Audio Devices  
List<DeviceItem> EnumerateAudioInputDevices()

// Spout Senders
List<DeviceItem> EnumerateSpoutSenders()
string? GetActiveSpoutSender()
(int Width, int Height, bool Success) GetSpoutSenderInfo(string senderName)
```

#### DeviceItem Class
```csharp
public class DeviceItem {
  public string Name { get; set; }          // Display name
  public string? DevicePath { get; set; }   // System path (DirectShow)
  public string? DeviceID { get; set; }     // Unique identifier
  public string? FriendlyName { get; set; } // Long friendly name
  public bool IsDefault { get; set; }       // Is default device
}
```

### DeviceManager (UI Integration)
**File**: `Remote/Helper/DeviceManager.cs`

High-level UI convenience methods.

#### Methods

```csharp
// Populate ComboBoxes
void PopulateVideoDevices(ComboBox comboBox, string? selectedDeviceName = null)
void PopulateAudioInputDevices(ComboBox comboBox, string? selectedDeviceName = null)
void PopulateSpoutSenders(ComboBox comboBox, string? selectedSenderName = null)

// Manage Spout senders
void RefreshSpoutSenders(ComboBox comboBox)
string? GetSelectedSpoutSender(ComboBox comboBox)
(int Width, int Height, bool Success) GetSpoutSenderInfo(string senderName)

// Utilities
DeviceItem? FindDeviceByName(List<DeviceItem> devices, string deviceName)
bool IsDeviceEnumerationAvailable()
```

## Usage Examples

### Example 1: Populate Video Device ComboBox

```csharp
// In form initialization
DeviceManager.PopulateVideoDevices(cboVideoDevice);

// Save selection on close
string selectedVideo = DeviceManager.GetSelectedSpoutSender(cboVideoDevice);
settings.SaveVideoDevice(selectedVideo);

// Restore on reopen
DeviceManager.PopulateVideoDevices(cboVideoDevice, savedSelection);
```

### Example 2: Populate Spout Senders

```csharp
// Initial population
DeviceManager.PopulateSpoutSenders(cboSpoutSender);

// Refresh periodically (new senders may appear)
DeviceManager.RefreshSpoutSenders(cboSpoutSender);

// Get selected sender
string? selectedSender = DeviceManager.GetSelectedSpoutSender(cboSpoutSender);

// Get sender info
var (width, height, success) = DeviceManager.GetSpoutSenderInfo(selectedSender);
if (success) {
  MessageBox.Show($"Sender resolution: {width}x{height}");
}
```

### Example 3: Handle Device Enumeration Errors

```csharp
try {
  if (DeviceManager.IsDeviceEnumerationAvailable()) {
    DeviceManager.PopulateVideoDevices(cboVideoDevice);
  } else {
    cboVideoDevice.Items.Add("Device enumeration not available");
    cboVideoDevice.Enabled = false;
  }
} catch (Exception ex) {
  MessageBox.Show($"Error loading devices: {ex.Message}");
}
```

## Implementation in MilkwaveRemoteForm

### Initialization

Add to form Load event:

```csharp
private void MilkwaveRemoteForm_Load(object sender, EventArgs e) {
  // Initialize device lists
  InitializeDeviceLists();
}

private void InitializeDeviceLists() {
  // Video devices
  string savedVideoDevice = RemoteHelper.GetIniValue("Milkwave", "VideoDevice", "");
  DeviceManager.PopulateVideoDevices(cboVideoDevice, savedVideoDevice);

  // Spout senders
  string savedSpoutSender = RemoteHelper.GetIniValue("Milkwave", "SpoutSender", "");
  DeviceManager.PopulateSpoutSenders(cboSpoutSender, savedSpoutSender);
}
```

### Periodic Refresh

Add timer to refresh Spout senders (new sources may appear at runtime):

```csharp
// In form initialization
var spoutRefreshTimer = new System.Windows.Forms.Timer();
spoutRefreshTimer.Interval = 2000; // Refresh every 2 seconds
spoutRefreshTimer.Tick += (s, e) => {
  DeviceManager.RefreshSpoutSenders(cboSpoutSender);
};
spoutRefreshTimer.Start();
```

### Handle Selection Changes

```csharp
private void cboSpoutSender_SelectedIndexChanged(object sender, EventArgs e) {
  string? selectedSender = DeviceManager.GetSelectedSpoutSender(cboSpoutSender);
  
  if (selectedSender != null) {
    var (width, height, success) = DeviceManager.GetSpoutSenderInfo(selectedSender);
    if (success) {
      labelSenderInfo.Text = $"{width}x{height}";
    }
    
    // Send to visualizer
    SendSpoutSenderToVisualizer(selectedSender);
  }
}
```

## Comparison with Previous Implementation

### Before (Direct P/Invoke in RemoteHelper)
```csharp
// Scattered COM interfaces in RemoteHelper
// DirectShow logic mixed with audio device logic
// Limited error handling
public static List<string> GetVideoInputDevices() { ... }
```

### After (OBS-Style Pattern)
```csharp
// Separated concerns
// Dedicated classes for enumeration (DeviceEnumerator) 
// and UI integration (DeviceManager)
// Consistent error handling across all device types
// Support for Spout senders with registry access

DeviceManager.PopulateSpoutSenders(comboBox);
```

## Key Differences from Original RemoteHelper

| Aspect | RemoteHelper | New Pattern |
|--------|--------------|-----------|
| Organization | Single file, mixed concerns | Separate files per responsibility |
| Device Types | Audio + Video | Audio + Video + Spout |
| UI Integration | Manual marshaling | High-level DeviceManager methods |
| Error Handling | Inconsistent | Centralized with Debug output |
| COM Cleanup | Manual | Automatic with try/finally |
| Registry Access | None | Full Spout sender registry support |

## Windows API Details

### DirectShow COM Interfaces Used
- **ICreateDevEnum** - Device enumerator factory
- **IEnumMoniker** - Moniker enumerator for devices
- **IMoniker** - Individual device identifier
- **IPropertyBag** - Device property storage

### Registry Locations

**Spout Configuration**:
```
HKEY_CURRENT_USER\Software\Leading Edge\Spout
├── MaxSenders         (REG_DWORD, default: 64)
├── ActiveSender       (REG_SZ, current active sender)
└── Senders\
    ├── [SenderName1]\
    │   ├── Width     (REG_DWORD)
    │   ├── Height    (REG_DWORD)
    │   └── Handle    (shared memory handle)
    └── [SenderName2]\...
```

## Error Handling Strategy

Following OBS pattern:
1. **Graceful degradation** - Device unavailability doesn't crash app
2. **Debug output** - Errors logged to Debug console (not shown to users)
3. **Empty fallback** - Show "No devices found" rather than crashing
4. **COM cleanup** - Always release COM objects in finally blocks

```csharp
try {
  // Enumerate devices
} catch (Exception ex) {
  System.Diagnostics.Debug.WriteLine($"Error: {ex.Message}");
  // Return empty list or show fallback UI
}
```

## Performance Considerations

### Caching
Currently, devices are enumerated on-demand. For frequent queries:
```csharp
// Option: Cache results
private static List<DeviceItem>? cachedVideoDevices;
private static DateTime lastVideoEnumeration = DateTime.MinValue;

if (DateTime.Now - lastVideoEnumeration > TimeSpan.FromSeconds(5)) {
  cachedVideoDevices = EnumerateVideoDevices();
  lastVideoEnumeration = DateTime.Now;
}
```

### Spout Sender Enumeration
Registry-based enumeration is fast (~1-5ms). Can safely refresh every 1-2 seconds.

## Registry Access Permissions

**No admin required** - All operations use `HKEY_CURRENT_USER`, not `HKEY_LOCAL_MACHINE`.

This follows OBS's approach and works for all users without elevation.

## Thread Safety

Currently **not thread-safe**. For multi-threaded use:

```csharp
private static readonly object deviceEnumerationLock = new object();

public static List<DeviceItem> EnumerateVideoDevices() {
  lock (deviceEnumerationLock) {
    // Enumeration code
  }
}
```

## Future Improvements

1. **Caching with TTL** - Cache results with time-to-live
2. **Device change notifications** - WM_DEVICECHANGE message handling
3. **Async enumeration** - Background threading for large device lists
4. **Spout receiver integration** - Directly receive textures without visualizer
5. **Property dialogs** - OBS-style device property UI

## Related Files

- `Remote/Helper/DeviceEnumerator.cs` - Core enumeration logic
- `Remote/Helper/DeviceManager.cs` - UI integration layer
- `SPOUT_SENDER_ENUMERATION_GUIDE.md` - Spout technical details
- `.github/copilot-instructions.md` - Project guidelines

## Testing

To test device enumeration:

```csharp
// List video devices
var videos = DeviceEnumerator.EnumerateVideoDevices();
foreach (var v in videos) {
  Debug.WriteLine($"Video: {v.Name}");
}

// List Spout senders
var spout = DeviceEnumerator.EnumerateSpoutSenders();
foreach (var s in spout) {
  Debug.WriteLine($"Spout: {s.Name}");
}
```

Check Output window in Visual Studio for debug messages.

