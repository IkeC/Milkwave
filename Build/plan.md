Requirement
- Visualizer should be able to take video input from a webcam and mix it with the existing video output ("preset output").
- Visualizer already has spout integration for output, maybe this can be used for input as well.
- For now, the preset output will be shown with 50% opacity (define a variable for this) and the webcam input will be shown with 100% opacity below that. We'll see if we can add a slider for this later in the Remote.
- In the remote tab "Mix", fill the cboVideoInput with the available video input devices. When the user selects a device and clicks "Set", it should be used as the webcam input for the visualizer.

## Technical Decisions

- **Video Capture API**: DirectShow (broader compatibility, well-established)
- **Video Resolution**: Match visualizer output resolution with scaling
- **Capture Framerate**: 30fps max to preserve performance
- **Persistence**: Save selected device in settings, auto-start on launch
- **Tab Name**: Use "Input" tab (already exists in UI)

## Implementation Plan

### Phase 1: C++ Visualizer - Video Capture Foundation

**1.1 Add DirectShow dependencies**
- Add to `plugin.vcxproj`: `strmiids.lib`, `quartz.lib`
- Include headers: `<dshow.h>`, `<qedit.h>`

**1.2 Create VideoCapture class**

Files: `Visualizer/vis_milk2/VideoCapture.h`, `VideoCapture.cpp`

Class responsibilities:
- Enumerate available DirectShow video capture devices
- Initialize selected device with graph builder
- Capture frames to system memory buffer
- Convert captured frames to `IDirect3DTexture9*`
- Thread-safe frame buffer access
- Stop/release all COM resources

Key members:
```cpp
class VideoCapture {
  IGraphBuilder* m_pGraphBuilder;
  ICaptureGraphBuilder2* m_pCaptureBuilder;
  IBaseFilter* m_pVideoCapture;
  ISampleGrabber* m_pSampleGrabber;
  IMediaControl* m_pMediaControl;
  BYTE* m_pFrameBuffer;
  int m_nWidth, m_nHeight;
  bool m_bNewFrame;
  std::mutex m_frameMutex;
};
```

Methods:
- `static std::vector<std::wstring> EnumerateDevices()`
- `bool Initialize(int deviceIndex, int width, int height)`
- `bool CopyFrameToTexture(IDirect3DTexture9* pTexture)`
- `void Start()`, `void Stop()`, `void Release()`

**1.3 Integration in CPlugin**

Files: `Visualizer/vis_milk2/plugin.h`, `plugin.cpp`

Add members to `CPlugin`:
```cpp
VideoCapture* m_pVideoCapture;
IDirect3DTexture9* m_pVideoCaptureTexture;
float m_fPresetOpacity; // 0.5f default
bool m_bVideoInputEnabled;
int m_nVideoDeviceIndex;
```

Initialization:
- Create `VideoCapture` instance in constructor
- Create texture sized to output resolution
- Load saved device index from config
- Auto-start if previously enabled

Cleanup:
- Stop capture in destructor
- Release texture and VideoCapture instance

### Phase 2: C++ Visualizer - Rendering Integration

**2.1 Modify render pipeline**

File: `plugin.cpp` in render method (after DoCustomSoundAnalysis, before final composite)

Render order (bottom to top):
1. Clear to black
2. Draw video capture texture at 100% opacity (if enabled and frame available)
3. Draw preset output at `m_fPresetOpacity` (50%) on top

**2.2 Rendering implementation**

```cpp
if (m_bVideoInputEnabled && m_pVideoCaptureTexture) {
  // Copy latest frame from VideoCapture to texture
  m_pVideoCapture->CopyFrameToTexture(m_pVideoCaptureTexture);
  
  // Draw video input at full screen, 100% opacity
  DrawTextureFullScreen(m_pVideoCaptureTexture, 1.0f);
}

// Draw preset output with reduced opacity
SetAlphaBlending(true, m_fPresetOpacity);
DrawPresetOutput();
SetAlphaBlending(false, 1.0f);
```

Helper method:
```cpp
void CPlugin::DrawTextureFullScreen(IDirect3DTexture9* pTexture, float alpha) {
  // Set up orthogonal projection
  // Create full-screen quad with texture coordinates
  // Apply alpha blending
  // Draw primitive
}
```

### Phase 3: C# Remote - UI & Device Enumeration

**3.1 Update UI in Input Tab**

File: `MilkwaveRemoteForm.Designer.cs`

Already exists:
- `cboVideoInput` (ComboBox for device selection)
- Button for "Set"

Add later (for future enhancement):
- `chkVideoInputEnabled` (enable/disable)
- `numVideoOpacity` (opacity slider 0-100)

**3.2 Add video device enumeration**

File: `Remote/Helper/RemoteHelper.cs`

Add method:
```csharp
public static List<string> GetVideoInputDevices() {
  // Use DirectShow.NET or Media Foundation API
  // Enumerate video capture devices
  // Return friendly names
}
```

Alternative: Use Windows.Media.Capture (UWP) or OpenCvSharp for enumeration

**3.3 Populate combo box**

File: `MilkwaveRemoteForm.cs`

In form load or tab activation:
```csharp
private void PopulateVideoDevices() {
  cboVideoInput.Items.Clear();
  var devices = RemoteHelper.GetVideoInputDevices();
  foreach (var device in devices) {
    cboVideoInput.Items.Add(device);
  }
  if (cboVideoInput.Items.Count > 0) {
    cboVideoInput.SelectedIndex = 0;
  }
}
```

### Phase 4: C# Remote - Communication with Visualizer

**4.1 Add new command constant**

File: `Remote/Helper/RemoteHelper.cs`

Add to existing window message constants:
```csharp
public const uint WM_SETVIDEODEVICE = WM_USER + 50; // Adjust number
```

Or add to named pipe commands if using that approach.

**4.2 Send device selection**

File: `MilkwaveRemoteForm.cs`

Button click handler:
```csharp
private void btnSetVideoDevice_Click(object sender, EventArgs e) {
  if (cboVideoInput.SelectedIndex >= 0) {
    int deviceIndex = cboVideoInput.SelectedIndex;
    SendMessageToVisualizer(WM_SETVIDEODEVICE, (IntPtr)deviceIndex, IntPtr.Zero);
    statusBar.Text = $"Video device set: {cboVideoInput.Text}";
  }
}
```

### Phase 5: C++ Visualizer - Message Handling

**5.1 Handle enable/disable mixing message**

File: `plugin.cpp` in `MyWindowProc` or message handler

```cpp
case WM_ENABLEVIDEOMIX:
{
  bool enable = (bool)wParam;
  m_bVideoInputEnabled = enable;
  
  if (!enable && m_pVideoCapture) {
    // Stop capture but don't release resources for quick re-enable
    m_pVideoCapture->Stop();
  } else if (enable && m_pVideoCapture && m_nVideoDeviceIndex >= 0) {
    // Re-start capture with last used device
    m_pVideoCapture->Start();
  }
  
  SaveVideoDeviceConfig();
  break;
}
```

**5.2 Handle device selection message**

File: `plugin.cpp` in `MyWindowProc` or message handler

```cpp
case WM_SETVIDEODEVICE:
{
  int deviceIndex = (int)wParam;
  if (m_pVideoCapture) {
    m_pVideoCapture->Stop();
    m_pVideoCapture->Release();
    
    if (m_pVideoCapture->Initialize(deviceIndex, m_nTexSizeX, m_nTexSizeY)) {
      if (m_bVideoInputEnabled) {
        m_pVideoCapture->Start();
      }
      m_nVideoDeviceIndex = deviceIndex;
      SaveVideoDeviceConfig();
    }
  }
  break;
}
```

### Phase 6: Settings Persistence

**6.1 Remote settings**

File: `Remote/Data/Settings.cs`

Add properties:
```csharp
public int VideoInputDeviceIndex { get; set; } = -1;
public bool VideoInputEnabled { get; set; } = false;
public float VideoInputOpacity { get; set; } = 0.5f;
```

**6.2 Visualizer config**

File: `config.ini` or similar

Add section:
```ini
[VideoInput]
DeviceIndex=0
Enabled=1
PresetOpacity=0.5
```

Load in plugin initialization:
```cpp
void CPlugin::LoadVideoConfig() {
  m_nVideoDeviceIndex = GetPrivateProfileInt(L"VideoInput", L"DeviceIndex", -1, m_szConfigIniFile);
  m_bVideoInputEnabled = GetPrivateProfileInt(L"VideoInput", L"Enabled", 0, m_szConfigIniFile);
  m_fPresetOpacity = GetPrivateProfileFloat(L"VideoInput", L"PresetOpacity", 0.5f, m_szConfigIniFile);
}
```

### Phase 7: Error Handling & Edge Cases

**7.1 Error scenarios to handle**

- No webcam available
- Webcam disconnected during operation
- Device initialization failure
- Frame capture failure
- Texture creation failure
- Invalid device index

**7.2 Graceful degradation**

- If video capture fails, continue rendering preset normally
- Log errors to existing Milkwave logging system
- Display error message in Remote status bar
- Disable video input automatically on failure

**7.3 Memory & resource management**

- Ensure all COM objects properly released
- Prevent memory leaks in frame buffer
- Thread-safe access to shared frame buffer
- Proper cleanup on visualizer exit

## File Changes Summary

### New Files
```
Visualizer/vis_milk2/
  ├── VideoCapture.h
  └── VideoCapture.cpp
```

### Modified Files
```
Visualizer/vis_milk2/
  ├── plugin.h (add members)
  ├── plugin.cpp (integrate video capture & rendering)
  ├── plugin.vcxproj (add new files, link libraries)
  └── plugin.vcxproj.filters (add new files to filters)

Remote/
  ├── MilkwaveRemoteForm.cs (video device logic)
  ├── Helper/RemoteHelper.cs (device enumeration, message constants)
  └── Data/Settings.cs (video settings persistence)
```

## Testing Checklist

- [ ] Video device enumeration works with 0, 1, and 2+ devices
- [ ] "Mix" checkbox toggles video mixing on/off correctly
- [ ] Changing video source while "Mix" is checked switches instantly
- [ ] Changing video source while "Mix" is unchecked has no effect
- [ ] Video capture initializes and displays frames
- [ ] Preset output renders at 50% opacity over video
- [ ] Performance acceptable at various resolutions
- [ ] Handles device disconnect gracefully
- [ ] Settings persist and restore on restart (including Mix state)
- [ ] No memory leaks during extended operation
- [ ] Error messages display correctly in Remote
- [ ] Works with built-in and USB webcams
- [ ] ComboBox disabled when "Mix" is unchecked

## Future Enhancements

- [ ] Add opacity slider in Remote
- [ ] Enable/disable toggle in Remote
- [ ] Resolution selection
- [ ] Spout input support (alternative to DirectShow)
- [ ] Video file playback support
- [ ] Flip/mirror video options
- [ ] Color correction/effects on video input
