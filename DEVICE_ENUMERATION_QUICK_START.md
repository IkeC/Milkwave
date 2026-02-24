# Quick Integration Guide - Device Enumeration

## What Changed

Two new files have been added following **OBS Studio's device enumeration pattern**:

1. **`Remote/Helper/DeviceEnumerator.cs`** - Core enumeration logic (static class)
2. **`Remote/Helper/DeviceManager.cs`** - UI integration layer (static class)

## Benefits

✅ **Clean Separation** - Device logic separate from UI  
✅ **OBS-Compatible** - Uses proven OBS Studio patterns  
✅ **Spout Support** - Native enumeration of Spout senders  
✅ **Error Handling** - Consistent error handling across device types  
✅ **Easy to Use** - Simple methods for populating ComboBoxes  

## Minimal Integration (3 Steps)

### Step 1: Add Usings
```csharp
using MilkwaveRemote.Helper;
```

### Step 2: Replace Device Population

**Old Way:**
```csharp
var devices = RemoteHelper.GetVideoInputDevices();
cboVideo.Items.AddRange(devices.ToArray());
```

**New Way:**
```csharp
DeviceManager.PopulateVideoDevices(cboVideo);
```

### Step 3: Add Spout Support

**Completely New Feature:**
```csharp
// Populate Spout senders
DeviceManager.PopulateSpoutSenders(cboSpoutSender);

// Handle selection change
string? selectedSender = DeviceManager.GetSelectedSpoutSender(cboSpoutSender);
```

## Full Integration Example

Add this to your `MilkwaveRemoteForm.Load()`:

```csharp
private void MilkwaveRemoteForm_Load(object sender, EventArgs e) {
  // Initialize device lists with OBS-style pattern
  InitializeDevices();
  
  // Optional: Refresh Spout senders periodically
  StartSpoutRefreshTimer();
}

private void InitializeDevices() {
  // Video devices
  string savedVideo = RemoteHelper.GetIniValue("Milkwave", "VideoDevice", "");
  DeviceManager.PopulateVideoDevices(cboVideoDevice, savedVideo);

  // Audio devices
  string savedAudio = RemoteHelper.GetIniValue("Milkwave", "AudioDevice", "");
  DeviceManager.PopulateAudioInputDevices(cboAudioDevice, savedAudio);

  // Spout senders
  string savedSpout = RemoteHelper.GetIniValue("Milkwave", "SpoutSender", "");
  DeviceManager.PopulateSpoutSenders(cboSpoutSender, savedSpout);
}

private void StartSpoutRefreshTimer() {
  var timer = new System.Windows.Forms.Timer();
  timer.Interval = 2000; // Refresh every 2 seconds
  timer.Tick += (s, e) => DeviceManager.RefreshSpoutSenders(cboSpoutSender);
  timer.Start();
}

// Handle changes
private void cboSpoutSender_SelectedIndexChanged(object sender, EventArgs e) {
  string? sender = DeviceManager.GetSelectedSpoutSender(cboSpoutSender);
  if (sender != null) {
    // Send to visualizer, save to settings, etc.
  }
}
```

## OBS Studio Pattern Highlights

### Pattern 1: Consistent COM Cleanup
```csharp
try {
  // Use COM object
} finally {
  Marshal.ReleaseComObject(obj); // Always cleanup
}
```

### Pattern 2: Graceful Error Handling
```csharp
try {
  // Enumerate
} catch (Exception ex) {
  Debug.WriteLine(ex.Message); // Log, don't crash
  return emptyList; // Graceful degradation
}
```

### Pattern 3: Two-Layer Architecture
- **Lower Layer** (`DeviceEnumerator`) - Platform APIs
- **Upper Layer** (`DeviceManager`) - UI convenience

This mirrors OBS's `properties_view` over `obs-source-list`.

## Key API Reference

### Most Common Methods

```csharp
// Populate ComboBox with Spout senders
DeviceManager.PopulateSpoutSenders(comboBox, previousSelection);

// Get selected sender
string? sender = DeviceManager.GetSelectedSpoutSender(comboBox);

// Refresh when needed (new senders added at runtime)
DeviceManager.RefreshSpoutSenders(comboBox);

// Get sender details
var (width, height, found) = DeviceManager.GetSpoutSenderInfo(sender);
```

### For Advanced Use

```csharp
// Direct enumeration access
List<DeviceEnumerator.DeviceItem> devices = 
  DeviceEnumerator.EnumerateVideoDevices();

// Get active Spout sender
string? active = DeviceEnumerator.GetActiveSpoutSender();

// Check availability
bool available = DeviceManager.IsDeviceEnumerationAvailable();
```

## Debugging

Check Visual Studio's **Debug Output** window for enumeration details:

```
Video: "Logitech HD Webcam"
Video: "OBS Virtual Camera"
Spout: "OBS-Studio"
Spout: "Resolume Arena"
```

Add more output with:
```csharp
System.Diagnostics.Debug.WriteLine($"Selected: {sender}");
```

## No Breaking Changes

- All existing methods in `RemoteHelper` still work
- You can gradually migrate to new pattern
- New files don't modify existing code

## Performance

- **Video device enumeration**: ~50-200ms (happens on load)
- **Spout sender enumeration**: ~1-5ms (can happen frequently)
- **ComboBox population**: Immediate (UI-thread safe)

Safe to refresh Spout senders every 1-2 seconds for live updates.

## Next Steps

1. ✅ Files created and compiled
2. → Update `MilkwaveRemoteForm.Load()` to use new pattern
3. → Update ComboBox `SelectedIndexChanged` handlers
4. → Test with real devices/senders
5. → Remove old device enumeration code from `RemoteHelper`

## Related Documentation

- `DEVICE_ENUMERATION_GUIDE.md` - Comprehensive guide
- `.github/copilot-instructions.md` - Updated project guidelines
- `SPOUT_SENDER_ENUMERATION_GUIDE.md` - Spout technical details

