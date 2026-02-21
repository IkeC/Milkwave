#include "VideoCapture.h"
#include "plugin.h"
#include <comdef.h>

// Define VFW error codes if not already defined
#ifndef VFW_E_WRONG_STATE
#define VFW_E_WRONG_STATE 0x80040227
#endif

// Define IID for ISampleGrabberCB if not already defined
static const IID IID_ISampleGrabberCB = { 0x0579154A, 0x2B53, 0x4994, { 0xB0, 0xD0, 0xE7, 0x73, 0x14, 0x8E, 0xFF, 0x85 } };

//============================================================
// VideoCaptureCallback Implementation
//============================================================

VideoCaptureCallback::VideoCaptureCallback(BYTE* pFrameBuffer, int width, int height, std::mutex* pFrameMutex)
    : m_pFrameBuffer(pFrameBuffer)
    , m_width(width)
    , m_height(height)
    , m_refCount(1)
    , m_framesReceived(0)
    , m_pFrameMutex(pFrameMutex) {
    OutputDebugStringW(L"VideoCaptureCallback: Created");
}

VideoCaptureCallback::~VideoCaptureCallback() {
    OutputDebugStringW(L"VideoCaptureCallback: Destroyed");
}

HRESULT STDMETHODCALLTYPE VideoCaptureCallback::QueryInterface(REFIID riid, void** ppvObject) {
    if (riid == IID_IUnknown || riid == IID_ISampleGrabberCB) {
        *ppvObject = this;
        AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE VideoCaptureCallback::AddRef() {
    return InterlockedIncrement(&m_refCount);
}

ULONG STDMETHODCALLTYPE VideoCaptureCallback::Release() {
    LONG refCount = InterlockedDecrement(&m_refCount);
    if (refCount == 0) {
        delete this;
    }
    return refCount;
}

HRESULT STDMETHODCALLTYPE VideoCaptureCallback::SampleCB(double SampleTime, IMediaSample *pSample) {
    if (!pSample) {
        return E_POINTER;
    }
    
    BYTE* pBuffer = nullptr;
    HRESULT hr = pSample->GetPointer(&pBuffer);
    if (FAILED(hr) || !pBuffer) {
        OutputDebugStringW(L"VideoCaptureCallback::SampleCB: GetPointer failed");
        return E_FAIL;
    }
    
    long len = pSample->GetActualDataLength();
    if (len <= 0) {
        return E_FAIL;
    }
    
    // Frame data obtained - pass to handler
    OnFrameReceived(pBuffer, len);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE VideoCaptureCallback::BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen) {
    if (!pBuffer || BufferLen <= 0) {
        return E_FAIL;
    }
    
    OnFrameReceived(pBuffer, BufferLen);
    return S_OK;
}

void VideoCaptureCallback::OnFrameReceived(BYTE* pBuffer, long bufferLen) {
    if (!m_pFrameBuffer || bufferLen <= 0) {
        return;
    }
    
    // Try to lock the frame buffer (non-blocking) to prevent deadlock with render thread
    // DirectShow callbacks must not block
    if (m_pFrameMutex) {
        // Use try_lock to prevent blocking the DirectShow callback thread
        std::unique_lock<std::mutex> lock(*m_pFrameMutex, std::try_to_lock);
        if (lock.owns_lock()) {
            // Successfully acquired lock, copy frame data
            int expectedSize = m_width * m_height * 4;
            int copySize = min((long)expectedSize, bufferLen);
            memcpy(m_pFrameBuffer, pBuffer, copySize);
        } else {
            // Couldn't acquire lock (render thread is reading) - skip this frame
            // This is safe because the next frame will update shortly
        }
    } else {
        // Fallback if mutex not provided (shouldn't happen)
        int expectedSize = m_width * m_height * 4;
        int copySize = min((long)expectedSize, bufferLen);
        memcpy(m_pFrameBuffer, pBuffer, copySize);
    }
    
    m_framesReceived++;
    
    // Log EVERY first frame
    if (m_framesReceived == 1) {
        wchar_t buf[256];
        swprintf_s(buf, L"VideoCaptureCallback::OnFrameReceived - FIRST FRAME! bufferLen=%ld", bufferLen);
        OutputDebugStringW(buf);
    }
}

//============================================================
// VideoCapture Implementation
//============================================================

VideoCapture::VideoCapture()
    : m_pGraphBuilder(nullptr)
    , m_pCaptureBuilder(nullptr)
    , m_pVideoCapture(nullptr)
    , m_pSampleGrabber(nullptr)
    , m_pMediaControl(nullptr)
    , m_pNullRenderer(nullptr)
    , m_pCallback(nullptr)
    , m_pFrameBuffer(nullptr)
    , m_nWidth(0)
    , m_nHeight(0)
    , m_nBufferSize(0)
    , m_bNewFrame(false)
    , m_bInitialized(false)
    , m_bUsingCallback(false)
    , m_frameAttempts(0)
    , m_successfulFrames(0)
    , m_callbackFramesReceived(0) {
    memset(m_szLastError, 0, sizeof(m_szLastError));
}

VideoCapture::~VideoCapture() {
    Release();
}

std::vector<std::wstring> VideoCapture::EnumerateDevices() {
    std::vector<std::wstring> devices;
    
    CoInitialize(nullptr);
    
    ICreateDevEnum* pDevEnum = nullptr;
    IEnumMoniker* pEnum = nullptr;
    
    HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, nullptr, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pDevEnum));
    
    if (SUCCEEDED(hr)) {
        hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, 0);
        if (hr == S_OK) {
            IMoniker* pMoniker = nullptr;
            while (pEnum->Next(1, &pMoniker, nullptr) == S_OK) {
                IPropertyBag* pPropBag = nullptr;
                hr = pMoniker->BindToStorage(0, 0, IID_PPV_ARGS(&pPropBag));
                if (SUCCEEDED(hr)) {
                    VARIANT var;
                    VariantInit(&var);
                    hr = pPropBag->Read(L"FriendlyName", &var, 0);
                    if (SUCCEEDED(hr)) {
                        devices.push_back(var.bstrVal);
                        VariantClear(&var);
                    }
                    pPropBag->Release();
                }
                pMoniker->Release();
            }
            pEnum->Release();
        }
        pDevEnum->Release();
    }
    
    return devices;
}

bool VideoCapture::Initialize(int deviceIndex, int width, int height) {
Release();
    
wchar_t logBuf[512];
swprintf(logBuf, 512, L"VideoCapture::Initialize - deviceIndex=%d, size=%dx%d", deviceIndex, width, height);
OutputDebugStringW(logBuf);

CoInitialize(nullptr);
    
m_nWidth = width;
m_nHeight = height;
m_nBufferSize = width * height * 4; // RGBA
m_pFrameBuffer = new BYTE[m_nBufferSize];
memset(m_pFrameBuffer, 0, m_nBufferSize);
    
HRESULT hr = CoCreateInstance(CLSID_FilterGraph, nullptr, CLSCTX_INPROC_SERVER,
    IID_PPV_ARGS(&m_pGraphBuilder));
if (FAILED(hr)) {
    OutputDebugStringW(L"VideoCapture: Failed to create FilterGraph");
    return false;
}
OutputDebugStringW(L"VideoCapture: FilterGraph created");
    
hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, nullptr, CLSCTX_INPROC_SERVER,
    IID_PPV_ARGS(&m_pCaptureBuilder));
if (FAILED(hr)) {
    OutputDebugStringW(L"VideoCapture: Failed to create CaptureGraphBuilder2");
    return false;
}
OutputDebugStringW(L"VideoCapture: CaptureGraphBuilder2 created");
    
m_pCaptureBuilder->SetFiltergraph(m_pGraphBuilder);
    
// Enumerate devices and get the one at deviceIndex
ICreateDevEnum* pDevEnum = nullptr;
IEnumMoniker* pEnum = nullptr;
    
hr = CoCreateInstance(CLSID_SystemDeviceEnum, nullptr, CLSCTX_INPROC_SERVER,
    IID_PPV_ARGS(&pDevEnum));
if (FAILED(hr)) {
    OutputDebugStringW(L"VideoCapture: Failed to create SystemDeviceEnum");
    return false;
}
    
    hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, 0);
    if (hr == S_OK) {
        IMoniker* pMoniker = nullptr;
        int currentIndex = 0;
        while (pEnum->Next(1, &pMoniker, nullptr) == S_OK) {
            if (currentIndex == deviceIndex) {
                swprintf(logBuf, 512, L"VideoCapture: Binding to device index %d", deviceIndex);
                OutputDebugStringW(logBuf);
                hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&m_pVideoCapture);
                if (FAILED(hr)) {
                    swprintf(logBuf, 512, L"VideoCapture: Failed to bind device, hr=0x%08X", hr);
                    OutputDebugStringW(logBuf);
                }
                pMoniker->Release();
                break;
            }
            pMoniker->Release();
            currentIndex++;
        }
        pEnum->Release();
    }
    pDevEnum->Release();
    
    if (!m_pVideoCapture) return false;
    
    hr = m_pGraphBuilder->AddFilter(m_pVideoCapture, L"Video Capture");
    if (FAILED(hr)) return false;
    
    // Create Sample Grabber
    IBaseFilter* pGrabberFilter = nullptr;
    hr = CoCreateInstance(CLSID_SampleGrabber, nullptr, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pGrabberFilter));
    if (FAILED(hr)) return false;
    
    hr = m_pGraphBuilder->AddFilter(pGrabberFilter, L"Sample Grabber");
    if (FAILED(hr)) {
        pGrabberFilter->Release();
        return false;
    }
    
    hr = pGrabberFilter->QueryInterface(IID_PPV_ARGS(&m_pSampleGrabber));
    if (FAILED(hr)) {
        pGrabberFilter->Release();
        return false;
    }
    
    // Request RGB32 format for proper color rendering
    // Don't fail if unsupported - the device will use its default
    OutputDebugStringW(L"VideoCapture: Requesting RGB32 media type");
    AM_MEDIA_TYPE mt;
    ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
    mt.majortype = MEDIATYPE_Video;
    mt.subtype = MEDIASUBTYPE_RGB32;
    mt.formattype = FORMAT_VideoInfo;
    
    VIDEOINFOHEADER* pVih = (VIDEOINFOHEADER*)CoTaskMemAlloc(sizeof(VIDEOINFOHEADER));
    if (pVih) {
        ZeroMemory(pVih, sizeof(VIDEOINFOHEADER));
        pVih->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        pVih->bmiHeader.biWidth = width;
        pVih->bmiHeader.biHeight = height;
        pVih->bmiHeader.biBitCount = 32;
        pVih->bmiHeader.biPlanes = 1;
        pVih->bmiHeader.biCompression = BI_RGB;
        pVih->bmiHeader.biSizeImage = width * height * 4;
        
        mt.pbFormat = (PBYTE)pVih;
        mt.cbFormat = sizeof(VIDEOINFOHEADER);
        mt.bFixedSizeSamples = TRUE;
        mt.lSampleSize = width * height * 4;
        
        HRESULT hrMediaType = m_pSampleGrabber->SetMediaType(&mt);
        if (SUCCEEDED(hrMediaType)) {
            OutputDebugStringW(L"VideoCapture: RGB32 media type accepted by device");
        } else {
            OutputDebugStringW(L"VideoCapture: RGB32 not supported, using device default");
        }
        
        CoTaskMemFree(pVih);
    }
    
    // Create and add null renderer BEFORE connecting filters
    // This is critical for virtual cameras like OBS Virtual Camera
    hr = CoCreateInstance(CLSID_NullRenderer, nullptr, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&m_pNullRenderer));
    if (FAILED(hr)) {
        wchar_t logBuf[256];
        swprintf(logBuf, 256, L"VideoCapture: Failed to create null renderer (0x%08X)", hr);
        OutputDebugStringW(logBuf);
        pGrabberFilter->Release();
        return false;
    }
    OutputDebugStringW(L"VideoCapture: Null renderer created");
    
    hr = m_pGraphBuilder->AddFilter(m_pNullRenderer, L"Null Renderer");
    if (FAILED(hr)) {
        wchar_t logBuf[256];
        swprintf(logBuf, 256, L"VideoCapture: Failed to add null renderer to graph (0x%08X)", hr);
        OutputDebugStringW(logBuf);
        pGrabberFilter->Release();
        return false;
    }
    OutputDebugStringW(L"VideoCapture: Null renderer added to graph");
    
    // Connect the entire chain: Capture -> Sample Grabber -> Null Renderer
    // Try with PIN_CATEGORY_CAPTURE first (standard webcams)
    OutputDebugStringW(L"VideoCapture: Attempting RenderStream with PIN_CATEGORY_CAPTURE");
    hr = m_pCaptureBuilder->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video,
        m_pVideoCapture, nullptr, pGrabberFilter);
    
    // If that fails, try without category (works better for virtual cameras like OBS)
    if (FAILED(hr)) {
        wchar_t logBuf[512];
        swprintf(logBuf, 512, L"VideoCapture: RenderStream with category failed (0x%08X), trying without category", hr);
        OutputDebugStringW(logBuf);
        hr = m_pCaptureBuilder->RenderStream(nullptr, &MEDIATYPE_Video,
            m_pVideoCapture, nullptr, pGrabberFilter);
    }
    
    if (FAILED(hr)) {
        wchar_t logBuf[512];
        swprintf(logBuf, 512, L"VideoCapture: RenderStream failed completely, hr=0x%08X - THIS IS THE CRITICAL ERROR", hr);
        OutputDebugStringW(logBuf);
        
        // Additional debug: Check if device is actually streaming
        OutputDebugStringW(L"VideoCapture: Checking device pins for issues...");
        
        pGrabberFilter->Release();
        return false;
    }
    OutputDebugStringW(L"VideoCapture: RenderStream succeeded - Capture->SampleGrabber connected");
    
    // Now check what media type was actually negotiated
    AM_MEDIA_TYPE connectedMt;
    hr = m_pSampleGrabber->GetConnectedMediaType(&connectedMt);
    if (SUCCEEDED(hr)) {
        VIDEOINFOHEADER* pVih = (VIDEOINFOHEADER*)connectedMt.pbFormat;
        swprintf(logBuf, 512, L"VideoCapture: Connected media type - Width=%d, Height=%d, Format=0x%08X",
            pVih->bmiHeader.biWidth, abs(pVih->bmiHeader.biHeight), connectedMt.subtype.Data1);
        OutputDebugStringW(logBuf);
        
        if (connectedMt.cbFormat != 0) CoTaskMemFree((PVOID)connectedMt.pbFormat);
        if (connectedMt.pUnk != nullptr) connectedMt.pUnk->Release();
    } else {
        OutputDebugStringW(L"VideoCapture: Could not get connected media type");
    }
    
    // Now connect sample grabber output to null renderer input
    // Get the output pin from the sample grabber
    OutputDebugStringW(L"VideoCapture: Looking for sample grabber output pin");
    IPin* pGrabberOut = nullptr;
    IEnumPins* pPinEnum = nullptr;
    hr = pGrabberFilter->EnumPins(&pPinEnum);
    if (SUCCEEDED(hr)) {
        IPin* pPin = nullptr;
        while (pPinEnum->Next(1, &pPin, nullptr) == S_OK) {
            PIN_DIRECTION dir;
            pPin->QueryDirection(&dir);
            if (dir == PINDIR_OUTPUT) {
                pGrabberOut = pPin;
                OutputDebugStringW(L"VideoCapture: Found sample grabber output pin");
                break;
            }
            pPin->Release();
        }
        pPinEnum->Release();
    }
    
    if (!pGrabberOut) {
        OutputDebugStringW(L"VideoCapture: ERROR - Could not find sample grabber output pin!");
    }
    
    // Get the input pin from the null renderer
    IPin* pNullIn = nullptr;
    if (pGrabberOut) {
        OutputDebugStringW(L"VideoCapture: Looking for null renderer input pin");
        pPinEnum = nullptr;
        hr = m_pNullRenderer->EnumPins(&pPinEnum);
        if (SUCCEEDED(hr)) {
            pPinEnum->Next(1, &pNullIn, nullptr);
            pPinEnum->Release();
            if (pNullIn) {
                OutputDebugStringW(L"VideoCapture: Found null renderer input pin");
            }
        }
        
        // Connect them - this is critical for the graph to run!
        if (pNullIn) {
            OutputDebugStringW(L"VideoCapture: Attempting to connect SampleGrabber->NullRenderer");
            hr = m_pGraphBuilder->Connect(pGrabberOut, pNullIn);
            if (FAILED(hr)) {
                swprintf(logBuf, 512, L"VideoCapture: Connect() failed (0x%08X), trying ConnectDirect", hr);
                OutputDebugStringW(logBuf);
                // If Connect fails, try ConnectDirect with the media type
                AM_MEDIA_TYPE grabberMt;
                if (SUCCEEDED(m_pSampleGrabber->GetConnectedMediaType(&grabberMt))) {
                    hr = m_pGraphBuilder->ConnectDirect(pGrabberOut, pNullIn, &grabberMt);
                    if (SUCCEEDED(hr)) {
                        OutputDebugStringW(L"VideoCapture: ConnectDirect succeeded!");
                    } else {
                        swprintf(logBuf, 512, L"VideoCapture: ConnectDirect also failed (0x%08X)", hr);
                        OutputDebugStringW(logBuf);
                    }
                    if (grabberMt.cbFormat != 0) CoTaskMemFree((PVOID)grabberMt.pbFormat);
                    if (grabberMt.pUnk != nullptr) grabberMt.pUnk->Release();
                }
            } else {
                OutputDebugStringW(L"VideoCapture: SampleGrabber->NullRenderer connected successfully!");
            }
            pNullIn->Release();
        }
        pGrabberOut->Release();
    }
    
    // Now safe to release the grabber filter interface (we still have m_pSampleGrabber)
    pGrabberFilter->Release();
    
    // CREATE AND SET CALLBACK FOR FRAME NOTIFICATION
    // Try BufferCB mode first (mode 1) - more reliable for data flow notification
    OutputDebugStringW(L"VideoCapture: Creating callback interface for frame notifications");
    m_pCallback = new VideoCaptureCallback(m_pFrameBuffer, m_nWidth, m_nHeight, &m_frameMutex);
    
    // Use BufferCB mode (1) - try this first, it's more reliable
    OutputDebugStringW(L"VideoCapture: Attempting to set callback in BufferCB mode");
    m_pSampleGrabber->SetBufferSamples(TRUE);
    m_pSampleGrabber->SetOneShot(FALSE);
    
    HRESULT hrCallback = m_pSampleGrabber->SetCallback(m_pCallback, 1); // 1 = BufferCB mode
    if (SUCCEEDED(hrCallback)) {
        OutputDebugStringW(L"VideoCapture: Callback interface set successfully (BufferCB mode)!");
        m_bUsingCallback = true;
    } else {
        wchar_t logBuf2[256];
        swprintf(logBuf2, 256, L"VideoCapture: WARNING - SetCallback (BufferCB) failed (0x%08X), falling back to polling only", hrCallback);
        OutputDebugStringW(logBuf2);
        m_pCallback->Release();
        m_pCallback = nullptr;
        m_bUsingCallback = false;
    }
    
    // Get media control interface
    hr = m_pGraphBuilder->QueryInterface(IID_PPV_ARGS(&m_pMediaControl));
    if (FAILED(hr)) {
        OutputDebugStringW(L"VideoCapture: Failed to get IMediaControl interface");
        return false;
    }
    OutputDebugStringW(L"VideoCapture: Got IMediaControl interface");
    
    m_bInitialized = true;
    OutputDebugStringW(L"VideoCapture::Initialize() completed successfully!");
    return true;
}

bool VideoCapture::Start() {
    if (!m_bInitialized || !m_pMediaControl) {
        OutputDebugStringW(L"VideoCapture::Start() - not initialized or no media control");
        return false;
    }
    
    OutputDebugStringW(L"VideoCapture::Start() - calling IMediaControl::Run()");
    HRESULT hr = m_pMediaControl->Run();
    if (FAILED(hr)) {
        wchar_t logBuf[256];
        swprintf(logBuf, 256, L"VideoCapture::Start() - Run() failed, hr=0x%08X", hr);
        OutputDebugStringW(logBuf);
        return false;
    }
    
    // Give the graph a moment to transition to running state
    // This helps prevent VFW_E_WRONG_STATE errors immediately after starting
    // Use multiple small waits to allow graph to actually start delivering frames
    for (int i = 0; i < 5; i++) {
        Sleep(20);
        OAFilterState state;
        HRESULT stateCheck = m_pMediaControl->GetState(0, &state);
        if (SUCCEEDED(stateCheck) && state == State_Running) {
            break;  // Graph is running, exit early
        }
    }
    
    OutputDebugStringW(L"VideoCapture::Start() - graph is running!");
    return SUCCEEDED(hr);
}

void VideoCapture::Stop() {
    if (m_pMediaControl) {
        m_pMediaControl->Stop();
    }
}

bool VideoCapture::CopyFrameToTexture(IDirect3DTexture9* pTexture, IDirect3DDevice9* pDevice) {
static int callCount = 0;
callCount++;
m_frameAttempts++;
    
if (!m_bInitialized || !m_pSampleGrabber || !pTexture || !pDevice) {
    if (callCount % 1000 == 1) {
        OutputDebugStringW(L"CopyFrameToTexture: Not initialized or invalid parameters");
    }
    return false;
}
    
// Validate texture by attempting to query its surface level info
if (pTexture) {
    IDirect3DSurface9* pSurface = nullptr;
    HRESULT hrSurface = pTexture->GetSurfaceLevel(0, &pSurface);
    if (FAILED(hrSurface)) {
        if (callCount % 1000 == 1) {
            OutputDebugStringW(L"CopyFrameToTexture: Texture validation failed - cannot get surface");
        }
        return false;
    }
    if (pSurface) {
        pSurface->Release();
    }
}
    
    
    // Update callback frame count for diagnostics
    if (m_bUsingCallback && m_pCallback) {
        m_callbackFramesReceived = m_pCallback->GetFramesReceived();
    }
    
    // Check if media is running and wait for it to be in running state
    if (m_pMediaControl) {
        OAFilterState state;
        HRESULT hrState = m_pMediaControl->GetState(100, &state);  // Wait up to 100ms for state
        if (FAILED(hrState)) {
            if (callCount % 300 == 0) {
                swprintf_s(m_szLastError, 256, L"GetState() failed: 0x%08x", hrState);
                OutputDebugStringW(m_szLastError);
            }
            return false;
        }
        if (state != State_Running) {
            if (callCount % 300 == 0) {
                swprintf_s(m_szLastError, 256, L"Graph state=%d (not running)", state);
                OutputDebugStringW(m_szLastError);
            }
            return false;
        }
    }
    
    // If using callback mode and frames are being received
    if (m_bUsingCallback && m_pCallback && m_pCallback->GetFramesReceived() > 0) {
        if (callCount % 300 == 0) {
            swprintf_s(m_szLastError, 256, L"Using callback mode - %d frames received", m_pCallback->GetFramesReceived());
            OutputDebugStringW(m_szLastError);
        }
        
        // Lock texture - use D3DLOCK_DISCARD to avoid synchronization issues
        D3DLOCKED_RECT lockedRect;
        HRESULT hr = pTexture->LockRect(0, &lockedRect, nullptr, D3DLOCK_DISCARD);
        if (FAILED(hr)) {
            if (callCount % 300 == 0) {
                swprintf_s(m_szLastError, 256, L"LockRect failed: 0x%08x", hr);
                OutputDebugStringW(m_szLastError);
            }
            return false;
        }
        
        // Lock the frame buffer ONLY WHILE COPYING TO TEXTURE
        // Use try_lock to avoid blocking the render thread
        std::unique_lock<std::mutex> lock(m_frameMutex, std::try_to_lock);
        if (lock.owns_lock()) {
            // Copy frame buffer directly to texture
            BYTE* pDest = (BYTE*)lockedRect.pBits;
            int copySize = min((int)m_nBufferSize, lockedRect.Pitch * m_nHeight);
            memcpy(pDest, m_pFrameBuffer, copySize);
        } else {
            // Couldn't acquire lock - copy zeros instead of blocking
            // This will just show a blank frame for this iteration
            BYTE* pDest = (BYTE*)lockedRect.pBits;
            ZeroMemory(pDest, lockedRect.Pitch * m_nHeight);
        }
        
        HRESULT hrUnlock = pTexture->UnlockRect(0);
        if (FAILED(hrUnlock)) {
            if (callCount % 300 == 0) {
                swprintf_s(m_szLastError, 256, L"UnlockRect failed: 0x%08x", hrUnlock);
                OutputDebugStringW(m_szLastError);
            }
            return false;
        }
        
        m_successfulFrames++;
        if (m_successfulFrames == 1) {
            OutputDebugStringW(L"VideoCapture: FIRST FRAME COPIED TO TEXTURE VIA CALLBACK!");
        }
        
        return true;
    }
    
    // FALLBACK: Only try polling if callback not in use
    // In SampleCB mode, there IS NO BUFFER to poll, so skip this
    
    if (m_bUsingCallback) {
        // We're in callback mode but no frames received yet
        if (callCount % 1000 == 1) {
            swprintf_s(m_szLastError, 256, L"SampleCB mode active but no frames received yet (attempt %d)", m_frameAttempts);
            OutputDebugStringW(m_szLastError);
        }
        return false;  // Don't try polling in callback mode
    }
    
    // Polling mode fallback (for non-callback devices)
    long bufferSize = 0;
    HRESULT hr = m_pSampleGrabber->GetCurrentBuffer(&bufferSize, nullptr);
    
    // Handle the specific VFW_E_WRONG_STATE error more gracefully
    if (hr == VFW_E_WRONG_STATE) {
        if (callCount % 1000 == 1) {
            swprintf_s(m_szLastError, 256, L"GetCurrentBuffer: VFW_E_WRONG_STATE - no samples delivered yet");
            OutputDebugStringW(m_szLastError);
        }
        return false;
    }
    
    // 0x80004005 is E_FAIL - also indicates no samples
    if (hr == E_FAIL) {
        if (callCount % 1000 == 1) {
            swprintf_s(m_szLastError, 256, L"GetCurrentBuffer: E_FAIL - device may not be delivering samples");
            OutputDebugStringW(m_szLastError);
        }
        return false;
    }
    
    if (FAILED(hr)) {
        if (callCount % 300 == 0) {
            swprintf_s(m_szLastError, 256, L"GetCurrentBuffer failed: 0x%08x", hr);
            OutputDebugStringW(m_szLastError);
        }
        return false;
    }
    
    if (bufferSize == 0) {
        if (m_successfulFrames == 0 && m_frameAttempts < 300) {
            return false; // Early startup - be quiet
        }
        
        if (callCount % 1000 == 1) {
            if (m_successfulFrames == 0) {
                swprintf_s(m_szLastError, 256, L"No frames after %d attempts - device may not be working", m_frameAttempts);
            } else {
                swprintf_s(m_szLastError, 256, L"Buffer size is 0 (last frame count: %d)", m_successfulFrames);
            }
            OutputDebugStringW(m_szLastError);
        }
        return false;
    }
    
    BYTE* pBuffer = new BYTE[bufferSize];
    hr = m_pSampleGrabber->GetCurrentBuffer(&bufferSize, (long*)pBuffer);
    if (FAILED(hr)) {
        delete[] pBuffer;
        return false;
    }
    
    
    // Lock texture - use D3DLOCK_DISCARD to avoid synchronization issues
    D3DLOCKED_RECT lockedRect;
    hr = pTexture->LockRect(0, &lockedRect, nullptr, D3DLOCK_DISCARD);
    if (FAILED(hr)) {
        delete[] pBuffer;
        if (callCount % 300 == 0) {
            swprintf_s(m_szLastError, 256, L"LockRect failed: 0x%08x", hr);
            OutputDebugStringW(m_szLastError);
        }
        return false;
    }
    
    // Get actual frame dimensions from media type
    AM_MEDIA_TYPE mt;
    hr = m_pSampleGrabber->GetConnectedMediaType(&mt);
    if (SUCCEEDED(hr)) {
        VIDEOINFOHEADER* pVih = (VIDEOINFOHEADER*)mt.pbFormat;
        int frameWidth = pVih->bmiHeader.biWidth;
        int frameHeight = abs(pVih->bmiHeader.biHeight);
        
        // Validate dimensions
        if (frameWidth > 0 && frameHeight > 0 && frameWidth <= 4096 && frameHeight <= 4096) {
            // Copy frame data to texture
            BYTE* pDest = (BYTE*)lockedRect.pBits;
            int srcStride = frameWidth * 4;
            
            // Copy rows in reverse order to flip the image (DirectShow gives bottom-up)
            for (int y = 0; y < frameHeight && y < m_nHeight; y++) {
                int srcY = frameHeight - 1 - y; // Flip vertically
                memcpy(pDest + y * lockedRect.Pitch, pBuffer + srcY * srcStride, 
                    min(srcStride, lockedRect.Pitch));
            }
        }
        
        if (mt.cbFormat != 0) {
            CoTaskMemFree((PVOID)mt.pbFormat);
        }
        if (mt.pUnk != nullptr) {
            mt.pUnk->Release();
        }
    }
    
    
    pTexture->UnlockRect(0);
    delete[] pBuffer;
    
    // Track successful frame capture
    m_successfulFrames++;
    if (m_successfulFrames == 1) {
        OutputDebugStringW(L"VideoCapture: FIRST FRAME CAPTURED SUCCESSFULLY (polling mode)!");
    }
    
    return true;
}

void VideoCapture::Release() {
    Stop();
    
    if (m_pCallback) {
        m_pCallback->Release();
        m_pCallback = nullptr;
    }
    
    if (m_pMediaControl) {
        m_pMediaControl->Release();
        m_pMediaControl = nullptr;
    }
    
    if (m_pNullRenderer) {
        m_pNullRenderer->Release();
        m_pNullRenderer = nullptr;
    }
    
    if (m_pSampleGrabber) {
        m_pSampleGrabber->Release();
        m_pSampleGrabber = nullptr;
    }
    
    if (m_pVideoCapture) {
        m_pVideoCapture->Release();
        m_pVideoCapture = nullptr;
    }
    
    if (m_pCaptureBuilder) {
        m_pCaptureBuilder->Release();
        m_pCaptureBuilder = nullptr;
    }
    
    if (m_pGraphBuilder) {
        m_pGraphBuilder->Release();
        m_pGraphBuilder = nullptr;
    }
    
    if (m_pFrameBuffer) {
        delete[] m_pFrameBuffer;
        m_pFrameBuffer = nullptr;
    }
    
    m_bInitialized = false;
}

void VideoCapture::GenerateTestPattern(BYTE* pBuffer, int width, int height) {
    if (!pBuffer) return;
    
    // Generate a simple color gradient test pattern (XRGB32 format)
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int offset = (y * width + x) * 4;
            
            // Create a gradient pattern
            pBuffer[offset + 0] = (BYTE)((x * 255) / width);       // Blue
            pBuffer[offset + 1] = (BYTE)((y * 255) / height);      // Green  
            pBuffer[offset + 2] = (BYTE)(((x + y) * 255) / (width + height)); // Red
            pBuffer[offset + 3] = 0;                                // X (unused)
        }
    }
}

