# Video Capture Debugging Guide - OBS Virtual Camera Issue

## Problem Description
The OBS Virtual Camera works fine in Discord/browsers but frame grabbing fails in the Milkwave Visualizer with error: `VFW_E_WRONG_STATE - no samples delivered yet`

## Root Cause Analysis

The issue stems from how DirectShow's SampleGrabber works with virtual cameras:

1. **Virtual Camera Compatibility**: OBS Virtual Camera (and similar virtual devices) may not properly signal frame availability through the standard DirectShow polling mechanism
2. **Graph State Issues**: The filter graph might be running, but frames aren't being buffered in a way that `GetCurrentBuffer()` can access
3. **Callback Mechanism**: Virtual cameras often require a callback-based approach to receive frame notifications reliably

## Solution Implemented

### 1. **Added Callback Interface (ISampleGrabberCB)**
   - Implemented `VideoCaptureCallback` class that derives from `ISampleGrabberCB`
   - Provides `BufferCB()` method to receive frame data directly from DirectShow
   - Tracks frame arrival count for diagnostics

### 2. **Dual-Mode Frame Capture**
   - **Callback Mode** (Primary): Frames are delivered via callback
   - **Polling Mode** (Fallback): Standard `GetCurrentBuffer()` polling
   - System automatically tries callback first, falls back to polling if needed

### 3. **Enhanced Diagnostic Logging**
   
All logging goes to **Debug Output** (visible in Visual Studio Debug Window or DebugView):

```
VideoCapture::Initialize - deviceIndex=X, size=WIDTHxHEIGHT
VideoCapture: FilterGraph created
VideoCapture: CaptureGraphBuilder2 created
VideoCapture: Binding to device index X
VideoCapture: Media type set on sample grabber
VideoCapture: Null renderer created
VideoCapture: Attempting RenderStream with PIN_CATEGORY_CAPTURE
VideoCapture: RenderStream succeeded - Capture->SampleGrabber connected
VideoCapture: Creating callback interface for frame notifications
VideoCaptureCallback: Created
VideoCapture: Callback interface set successfully!
VideoCapture::Start() - calling IMediaControl::Run()
VideoCapture::Start() - graph is running!
VideoCapture: FIRST FRAME CAPTURED SUCCESSFULLY!
VideoCaptureCallback: FIRST FRAME RECEIVED!
```

### 4. **Diagnostic Functions Available**

```cpp
// In CPlugin class:
int GetFrameAttempts() const { return m_frameAttempts; }
int GetSuccessfulFrames() const { return m_successfulFrames; }
int GetCallbackFramesReceived() const { return m_callbackFramesReceived; }

// In VideoCapture class:
bool HasReceivedFrames() const { return m_framesReceived > 0; }
int GetFramesReceived() const { return m_framesReceived; }
```

## How to Debug Issues

### Via Debug Output Window

1. **Run with debugger attached** (Visual Studio)
2. **View ? Output** or press `Ctrl+Alt+O`
3. **Show output from:** Select "Debug" 
4. Look for these key messages:

   - `VideoCapture: Callback interface set successfully!` - Callback mode enabled
   - `VideoCaptureCallback: FIRST FRAME RECEIVED!` - Frames are arriving
   - `VideoCapture: FIRST FRAME CAPTURED SUCCESSFULLY!` - Frames successfully copied to texture
   - `VFW_E_WRONG_STATE - no samples delivered yet` - Device still initializing or not streaming

### Via Remote UI (Future Enhancement)

The `Input` tab in Milkwave Remote shows:
- **Video mixing enabled/disabled** message
- Device selection

Future enhancement could add:
- Frame receive count
- Last error message
- Callback mode status

## Troubleshooting Steps

### If you still see "no samples delivered yet":

1. **Check device is streaming:**
   - OBS Virtual Camera must be actively sending frames
   - Test in Discord/browser first to confirm device works

2. **Look for initialization:**
   - If error only appears briefly (first ~5 seconds), that's normal - device is initializing
   - Should see "FIRST FRAME" messages after ~1-2 seconds

3. **Check debug output for exact error:**
   - Look for all messages with "hr=0x" format
   - Search the error code online for meaning

4. **Try different video sources:**
   - Built-in webcam (if available)
   - Another virtual camera software
   - This helps narrow down if issue is OBS Virtual Camera specific

## Code Changes Summary

### VideoCapture.h
- Added `VideoCaptureCallback` class
- Added `m_pCallback` member
- Added `m_bUsingCallback` flag
- Added `m_callbackFramesReceived` counter
- Added `GetCallbackFramesReceived()` method

### VideoCapture.cpp
- Implemented `VideoCaptureCallback` with callback interface
- Updated `Initialize()` to set up callback
- Updated `CopyFrameToTexture()` to try callback first
- Enhanced all logging with detailed status messages
- Added frame count tracking

### plugin.h/plugin.cpp
- Updated to track `m_callbackFramesReceived`
- Can display in debug messages/remote UI

## Expected Behavior

**First Run with OBS Virtual Camera:**
```
VideoCapture::Initialize - deviceIndex=1, size=640x480
VideoCapture: Callback interface set successfully!  ? Callback mode enabled
VideoCapture::Start() - graph is running!
VideoCapture: Using callback mode - 0 frames received   ? Initially no frames
[after ~1 second]
VideoCaptureCallback: FIRST FRAME RECEIVED!           ? Frame arrived!
VideoCapture: FIRST FRAME COPIED TO TEXTURE VIA CALLBACK!
```

## Future Improvements

1. **Remote UI Integration**: Display callback status and frame count in Milkwave Remote UI
2. **Alternative APIs**: Consider Media Foundation (newer API) if DirectShow continues to have issues
3. **Device-Specific Workarounds**: Detect OBS Virtual Camera and apply special handling if needed
4. **Timeout Handling**: Auto-switch to test pattern if no frames after timeout period
5. **Performance Monitoring**: Track frame rate and latency

## Files Modified

- `Visualizer\vis_milk2\VideoCapture.h` - Header with callback interface
- `Visualizer\vis_milk2\VideoCapture.cpp` - Implementation with callback support
- `Visualizer\vis_milk2\plugin.h` - Updated diagnostics
- `Visualizer\vis_milk2\plugin.cpp` - Updated tracking

## Testing

To test the changes:

1. Build the solution
2. Run Milkwave with debugger
3. Select OBS Virtual Camera in Remote UI ? Input tab
4. Click "Mix" to enable video mixing
5. Watch Debug Output window for messages
6. Check if visualization shows video feed

Look for:
- ? "Callback interface set successfully!" - Good sign
- ? "FIRST FRAME RECEIVED!" - Frames are coming through
- ? "VFW_E_WRONG_STATE" repeatedly - Still having issues (see Troubleshooting)
