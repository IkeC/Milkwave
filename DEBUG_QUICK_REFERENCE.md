# Quick Debugging Checklist

## How to Access Debug Output

**In Visual Studio:**
- Menu: `View ? Output` (or press `Ctrl+Alt+O`)
- Bottom dropdown: Change from "Debug" to see debug messages

## Key Success Indicators

? **GOOD - Callback Mode Working:**
```
VideoCapture: Callback interface set successfully!
VideoCaptureCallback: FIRST FRAME RECEIVED!
VideoCapture: FIRST FRAME COPIED TO TEXTURE VIA CALLBACK!
```

? **GOOD - Polling Mode Working:**
```
VideoCapture: RenderStream succeeded
VideoCapture: FIRST FRAME CAPTURED SUCCESSFULLY (polling mode)!
```

? **PROBLEM - No Device Connection:**
```
VideoCapture: Failed to bind device
```
? Device not selected or not available

? **PROBLEM - Graph Not Running:**
```
Graph state=X (not running)
```
? Media control failed or graph stopped

? **PROBLEM - No Frames Arriving:**
```
VFW_E_WRONG_STATE - no samples delivered yet
```
? Device not streaming or initialization timeout

## Diagnostic Codes

| Code | Meaning | Solution |
|------|---------|----------|
| `0x00000000` | S_OK | Everything working |
| `0x80040227` | VFW_E_WRONG_STATE | Wait longer or restart device |
| `0x80004005` | E_FAIL | Device not delivering samples |
| `0x80004002` | E_NOINTERFACE | Driver issue |
| `0x80070002` | ERROR_FILE_NOT_FOUND | Device driver missing |

## Timeline of Successful Capture

```
T=0ms    : WM_USER_SETVIDEODEVICE message
T=10ms   : VideoCapture::Initialize() starts
T=50ms   : Device bound and graph built
T=100ms  : "Callback interface set successfully!"
T=150ms  : VideoCapture::Start() calls Run()
T=200ms  : "Start() - graph is running!"
T=1000ms : "VideoCaptureCallback: FIRST FRAME RECEIVED!"
T=1050ms : "FIRST FRAME COPIED TO TEXTURE"
```

If you don't see "FIRST FRAME RECEIVED" within 2-3 seconds, the device isn't streaming.

## Enable Spout Logging (Optional)

In OutputDebugString calls, look for lines containing:
- `[SPOUT` - Spout related messages
- `VideoCapture` - Video capture messages
- `VideoCaptureCallback` - Frame callback messages

## Manual Test

```cpp
// In plugin.cpp, temporarily add:
if (GetAsyncKeyState('T') & 0x8000) {  // Press 'T'
    wchar_t buf[256];
    swprintf_s(buf, L"Diagnostics - Attempts: %d, Successful: %d, Callback Frames: %d",
        m_pVideoCapture->GetFrameAttempts(),
        m_pVideoCapture->GetSuccessfulFrames(),
        m_pVideoCapture->GetCallbackFramesReceived());
    OutputDebugStringW(buf);
}
```

Then press 'T' while running to see statistics.
