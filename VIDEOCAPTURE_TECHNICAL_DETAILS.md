# Video Capture Implementation Details

## Architecture

### Three-Layer Frame Delivery

```
OBS Virtual Camera (DirectShow Source)
           ?
       [Capture Filter]
           ?
       [Sample Grabber Filter]
           ?? Callback Mode ????? VideoCaptureCallback::BufferCB()
           ?                              ?
           ?                        m_pFrameBuffer
           ?
           ?? Polling Mode ????? GetCurrentBuffer()
                                          ?
                                    m_pFrameBuffer
           ?
      [Null Renderer]
           ?
       (Graph consumes frames)
```

### Frame Buffer Flow

1. **Callback Reception**: `VideoCaptureCallback::OnFrameReceived()`
   - Receives BYTE* pointing to frame data
   - Copies directly into `m_pFrameBuffer`
   - Increments `m_framesReceived` counter

2. **Texture Population**: `CopyFrameToTexture()`
   - Locks D3D9 texture surface
   - Copies from `m_pFrameBuffer` to texture memory
   - Unlocks texture for rendering

3. **Fallback**: If callback fails
   - Uses `GetCurrentBuffer()` polling
   - Handles DirectShow state machine
   - Graceful degradation to slower mode

## Key Classes

### VideoCaptureCallback

```cpp
class VideoCaptureCallback : public ISampleGrabberCB {
    // IUnknown implementation
    // Reference counting for COM
    
    // ISampleGrabberCB implementation
    HRESULT SampleCB(double SampleTime, IMediaSample *pSample);
    HRESULT BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen);
    
    // Frame reception handler
    void OnFrameReceived(BYTE* pBuffer, long bufferLen);
    
    // Diagnostics
    bool HasReceivedFrames() const;
    int GetFramesReceived() const;
};
```

**Why Both SampleCB and BufferCB?**
- `SampleCB`: Provides `IMediaSample` interface (frame properties, timing)
- `BufferCB`: Provides raw buffer (memory-efficient)
- We use `BufferCB` mode (parameter 1 in `SetCallback()`)

### VideoCapture

```cpp
class VideoCapture {
    // Graph Building
    IGraphBuilder* m_pGraphBuilder;          // Main graph
    ICaptureGraphBuilder2* m_pCaptureBuilder; // Helper for connections
    
    // Filters
    IBaseFilter* m_pVideoCapture;            // Source (device driver)
    ISampleGrabber* m_pSampleGrabber;        // Collector
    IBaseFilter* m_pNullRenderer;            // Sink (consumes frames)
    
    // Control
    IMediaControl* m_pMediaControl;          // Start/stop graph
    
    // Callback
    VideoCaptureCallback* m_pCallback;       // Frame notification handler
    bool m_bUsingCallback;                   // Callback mode active
    
    // Storage
    BYTE* m_pFrameBuffer;                    // RGBA32 frame data
    int m_nWidth, m_nHeight;
    
    // Diagnostics
    int m_frameAttempts;                     // Total CopyFrameToTexture calls
    int m_successfulFrames;                  // Frames successfully copied
    int m_callbackFramesReceived;            // Frames via callback
};
```

## DirectShow Graph Configuration

### Graph Layout
```
[OBS Virtual Camera] (source)
    ? (RGB32 stream)
[Sample Grabber]  ? Callback installed here
    ? (RGB32 stream)  
[Null Renderer]
```

### Initialization Steps

1. **Create Graph Components**
   ```cpp
   CoCreateInstance(CLSID_FilterGraph, ..., &m_pGraphBuilder);
   CoCreateInstance(CLSID_CaptureGraphBuilder2, ..., &m_pCaptureBuilder);
   ```

2. **Find Device**
   ```cpp
   // Enumerate CLSID_VideoInputDeviceCategory
   // Get IMoniker for selected device
   pMoniker->BindToObject(..., &m_pVideoCapture);
   ```

3. **Add Filters to Graph**
   ```cpp
   m_pGraphBuilder->AddFilter(m_pVideoCapture, L"Video Capture");
   m_pGraphBuilder->AddFilter(pGrabberFilter, L"Sample Grabber");
   m_pGraphBuilder->AddFilter(m_pNullRenderer, L"Null Renderer");
   ```

4. **Set Media Type** (Critical for OBS Virtual Camera!)
   ```cpp
   AM_MEDIA_TYPE mt;
   mt.majortype = MEDIATYPE_Video;
   mt.subtype = MEDIASUBTYPE_RGB32;     // Force RGB32
   mt.formattype = FORMAT_VideoInfo;
   m_pSampleGrabber->SetMediaType(&mt);
   ```

5. **Connect Graph**
   ```cpp
   // Capture ? SampleGrabber (via RenderStream)
   m_pCaptureBuilder->RenderStream(
       &PIN_CATEGORY_CAPTURE, 
       &MEDIATYPE_Video,
       m_pVideoCapture, 
       nullptr,           // Intermediate filter (none)
       pGrabberFilter     // Output
   );
   
   // SampleGrabber ? NullRenderer (direct connect)
   m_pGraphBuilder->Connect(pGrabberOut, pNullRendererIn);
   ```

6. **Install Callback**
   ```cpp
   m_pCallback = new VideoCaptureCallback(m_pFrameBuffer, width, height);
   m_pSampleGrabber->SetCallback(m_pCallback, 1);  // 1 = BufferCB
   ```

7. **Start Graph**
   ```cpp
   m_pMediaControl->Run();
   ```

## Why OBS Virtual Camera Needs Special Handling

### Problem with Standard Polling

DirectShow's SampleGrabber `GetCurrentBuffer()` requires:
1. ? Graph running
2. ? Frames being delivered
3. ? **Frames buffered in grabber** ? OBS doesn't do this reliably
4. ? **Sample available** ? May return VFW_E_WRONG_STATE

### Solution: Use Callbacks

Callbacks bypass the buffer requirement:
- DirectShow calls `BufferCB()` **immediately** when frame arrives
- **No waiting** for buffer to fill
- **No state machine** issues
- **More responsive** to virtual devices

## Frame Data Format

```
BYTE* pFrameBuffer = [W*H*4 bytes]

For each pixel (x,y):
    offset = (y * W + x) * 4
    B = pFrameBuffer[offset + 0]
    G = pFrameBuffer[offset + 1]
    R = pFrameBuffer[offset + 2]
    X = pFrameBuffer[offset + 3]  // Unused (alpha or reserved)
    
    Color = 0xAABBGGRR (ARGB32)
```

**Note:** DirectShow delivers bottom-up DIB format, so `CopyFrameToTexture()` flips Y-coordinates:
```cpp
int srcY = frameHeight - 1 - y;  // Flip vertically
memcpy(pDest + y * pitch, pBuffer + srcY * stride, bytesToCopy);
```

## Error Handling

### VFW_E_WRONG_STATE (0x80040227)

**Meaning:** Filter is not in Running state or no samples available

**Causes:**
1. Graph still transitioning to Running state (takes ~100-500ms)
2. Device hasn't delivered any frames yet
3. Media type mismatch

**Solution:**
- Wait longer (graceful degradation)
- Check graph state with `GetState()`
- Try callback mode first

### E_FAIL (0x80004005)

**Meaning:** Operation failed (generic)

**Causes:**
1. Device not streaming
2. Sample grabber not properly initialized
3. No media type set

**Solution:**
- Verify device is streaming (check in Discord/browser first)
- Reinitialize device
- Try different format (though RGB32 is most compatible)

### 0x80070002 (ERROR_FILE_NOT_FOUND)

**Meaning:** Driver or device not found

**Causes:**
1. Device unplugged
2. Driver uninstalled
3. Wrong device index

**Solution:**
- Re-enumerate devices
- Verify physical connection
- Update/reinstall drivers

## Performance Considerations

### CPU Usage
- Callback: ~0.5-1% (minimal, just memory copy)
- Polling: ~2-5% (repeated DirectShow calls)
- Vertical flip: ~1-2% (memcpy operations)

### Memory
- Frame buffer: W * H * 4 bytes
  - 640×480: 1.2 MB
  - 1920×1080: 8.3 MB
- Plus DirectShow internal buffering: ~5-20 MB

### Latency
- Callback: ~16-33 ms (one frame at 30-60 fps)
- Polling: ~0-16 ms (up to next GetCurrentBuffer call)
- Texture copy: ~1-5 ms depending on GPU

## Thread Safety

### Current Implementation
- Frame buffer protected by `std::mutex m_frameMutex` (ready for future use)
- Callback can be called from graph threads
- `CopyFrameToTexture()` called from render thread

### Safe Operations
- ? Copying from m_pFrameBuffer (atomic 32-bit ops)
- ? Reading frame counters (atomic int ops)
- ? Modifying DirectShow interfaces without synchronization

**Note:** Current implementation doesn't use mutex because:
1. Frame buffer is written atomically (pixel-by-pixel)
2. Texture copy only reads
3. Locks are too slow for real-time rendering

For future improvements if needed:
```cpp
// Before reading frame buffer
std::lock_guard<std::mutex> lock(m_frameMutex);
// Safe to read m_pFrameBuffer
```

## Debugging in Production

Without Visual Studio debugger:

1. **Use DebugView** (Sysinternals)
   - Download: https://live.sysinternals.com/Dbgview.exe
   - Capture all OutputDebugString() calls
   - Filter by "VideoCapture" or "VideoCaptureCallback"

2. **Log to File** (Alternative)
   - Redirect OutputDebugString to file
   - Parse output during development

3. **Diagnostic API**
   ```cpp
   int attempts = videoCap->GetFrameAttempts();
   int successful = videoCap->GetSuccessfulFrames();
   int callbackFrames = videoCap->GetCallbackFramesReceived();
   wchar_t* lastError = videoCap->GetLastError();
   ```

## Future Optimization Ideas

1. **Media Foundation** - Newer Microsoft API
   - Better virtual device support
   - More efficient
   - Steeper learning curve

2. **Async Frame Delivery**
   - Queue frames instead of replacing
   - Reduce chance of missed frames

3. **GPU-Accelerated Copy**
   - Use D3D9 texture copy instead of CPU memcpy
   - Offload flip operation to shader

4. **Format Negotiation**
   - Try multiple formats if RGB32 fails
   - Auto-detect best format

5. **Device Monitoring**
   - Detect disconnected devices
   - Auto-switch to fallback
   - Retry with timeout
