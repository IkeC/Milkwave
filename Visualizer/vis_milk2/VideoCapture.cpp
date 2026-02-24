#include "VideoCapture.h"
#include "plugin.h"
#include <comdef.h>
#include <dshow.h>
#include <d3d9.h>
#include <vector>
#include <string>

// Forward declare IAMStreamConfig and AM_MEDIA_TYPE if not already in headers
// Actually it's in dshow.h

// Define VFW error codes if not already defined
#ifndef VFW_E_WRONG_STATE
#define VFW_E_WRONG_STATE 0x80040227
#endif

// Define IID for ISampleGrabberCB if not already defined
static const IID IID_ISampleGrabberCB = { 0x0579154A, 0x2B53, 0x4994, { 0xB0, 0xD0, 0xE7, 0x73, 0x14, 0x8E, 0xFF, 0x85 } };

extern CPlugin g_plugin;

// Helper to log to milkwave if available
void LogToMilkwave(const wchar_t* msg) {
    if (g_plugin.milkwave) {
        g_plugin.milkwave->LogInfo(msg);
    }
    OutputDebugStringW(msg);
}

// Function to find a pin on a filter
IPin* GetFilterPin(IBaseFilter* pFilter, PIN_DIRECTION dir) {
    IEnumPins* pEnum = nullptr;
    if (FAILED(pFilter->EnumPins(&pEnum))) return nullptr;

    IPin* pPin = nullptr;
    while (pEnum->Next(1, &pPin, nullptr) == S_OK) {
        PIN_DIRECTION pinDir;
        pPin->QueryDirection(&pinDir);
        if (pinDir == dir) {
            pEnum->Release();
            return pPin;
        }
        pPin->Release();
    }
    pEnum->Release();
    return nullptr;
}

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
    LogToMilkwave(L"VideoCaptureCallback: Created");
}

VideoCaptureCallback::~VideoCaptureCallback() {
    LogToMilkwave(L"VideoCaptureCallback: Destroyed");
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

    if (m_framesReceived < 5) {
        wchar_t dBuf[256];
        swprintf_s(dBuf, L"VideoCaptureCallback::SampleCB called! Frame time: %f", SampleTime);
        LogToMilkwave(dBuf);
    }

    static int sampleCount = 0;
    if (sampleCount++ % 300 == 0) {
        LogToMilkwave(L"VideoCaptureCallback::SampleCB called!");
    }

    BYTE* pBuffer = nullptr;
    HRESULT hr = pSample->GetPointer(&pBuffer);
    if (FAILED(hr) || !pBuffer) {
        return E_FAIL;
    }

    long len = pSample->GetActualDataLength();
    if (len <= 0) {
        return E_FAIL;
    }

    OnFrameReceived(pBuffer, len);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE VideoCaptureCallback::BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen) {
    if (m_framesReceived < 5) {
        wchar_t dBuf[256];
        swprintf_s(dBuf, L"VideoCaptureCallback::BufferCB called! Frame time: %f", SampleTime);
        LogToMilkwave(dBuf);
    }

    static int bufferCount = 0;
    if (bufferCount++ % 300 == 0) {
        LogToMilkwave(L"VideoCaptureCallback::BufferCB called!");
    }

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

    if (m_framesReceived == 0) {
        LogToMilkwave(L"VideoCaptureCallback::OnFrameReceived - DATA RECEIVED!");
    }

    bool firstFrame = (m_framesReceived == 0);

    // Try to lock the frame buffer (non-blocking) to prevent deadlock with render thread
    // DirectShow callbacks must not block
    if (m_pFrameMutex) {
        // Use try_lock to prevent blocking the DirectShow callback thread
        std::unique_lock<std::mutex> lock(*m_pFrameMutex, std::try_to_lock);
        if (lock.owns_lock()) {
            // Successfully acquired lock, copy frame data
            // Use the provided width/height to limit copy, or bufferLen if smaller
            int expectedSize = m_width * m_height * 4;
            int copySize = min((long)expectedSize, bufferLen);
            memcpy(m_pFrameBuffer, pBuffer, copySize);
        } else {
            // Couldn't acquire lock (render thread is reading) - skip this frame
            if (m_framesReceived % 300 == 1) {
                LogToMilkwave(L"VideoCaptureCallback: Lock contention, skipping frames...");
            }
        }
    } else {
        // Fallback if mutex not provided (shouldn't happen)
        int expectedSize = m_width * m_height * 4;
        int copySize = min((long)expectedSize, bufferLen);
        memcpy(m_pFrameBuffer, pBuffer, copySize);
    }

    m_framesReceived++;

    // Log EVERY first frame and then every 300 frames
    if (firstFrame || m_framesReceived % 300 == 0) {
        wchar_t buf[512];
        swprintf_s(buf, L"VideoCaptureCallback::OnFrameReceived - Frames=%d, bufferLen=%ld", m_framesReceived, bufferLen);
        LogToMilkwave(buf);

        // Check for completely black frames (all zeros) for debugging
        bool allZeros = true;
        // Check a decent sample of the buffer
        int checkLen = min(4096, (int)bufferLen);
        for (int i = 0; i < checkLen; i++) {
            if (pBuffer[i] != 0) {
                allZeros = false;
                break;
            }
        }
        if (allZeros) {
            LogToMilkwave(L"  - WARNING: Frame data looks like it is all zeros (black)");
        }
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
    LogToMilkwave(logBuf);

    HRESULT hrCo = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hrCo) && hrCo != RPC_E_CHANGED_MODE) {
        swprintf_s(logBuf, L"VideoCapture: CoInitializeEx failed (0x%08X)", hrCo);
        LogToMilkwave(logBuf);
    }

    m_nWidth = width;
    m_nHeight = height;
    m_nBufferSize = width * height * 4; // RGBA
    m_pFrameBuffer = new BYTE[m_nBufferSize];
    memset(m_pFrameBuffer, 0, m_nBufferSize);

    HRESULT hr = CoCreateInstance(CLSID_FilterGraph, nullptr, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&m_pGraphBuilder));
    if (FAILED(hr)) {
        LogToMilkwave(L"VideoCapture: Failed to create FilterGraph");
        return false;
    }
    LogToMilkwave(L"VideoCapture: FilterGraph created");

    hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, nullptr, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&m_pCaptureBuilder));
    if (FAILED(hr)) {
        LogToMilkwave(L"VideoCapture: Failed to create CaptureGraphBuilder2");
        return false;
    }
    LogToMilkwave(L"VideoCapture: CaptureGraphBuilder2 created");

    m_pCaptureBuilder->SetFiltergraph(m_pGraphBuilder);

    // Enumerate devices and get the one at deviceIndex
    ICreateDevEnum* pDevEnum = nullptr;
    IEnumMoniker* pEnum = nullptr;

    hr = CoCreateInstance(CLSID_SystemDeviceEnum, nullptr, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pDevEnum));
    if (FAILED(hr)) {
        LogToMilkwave(L"VideoCapture: Failed to create SystemDeviceEnum");
        return false;
    }

    hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, 0);
    if (hr == S_OK) {
        IMoniker* pMoniker = nullptr;
        int currentIndex = 0;
        while (pEnum->Next(1, &pMoniker, nullptr) == S_OK) {
            if (currentIndex == deviceIndex) {
                swprintf(logBuf, 512, L"VideoCapture: Binding to device index %d", deviceIndex);
                LogToMilkwave(logBuf);

                // Get display name
                IPropertyBag* pPropBag = nullptr;
                if (SUCCEEDED(pMoniker->BindToStorage(0, 0, IID_PPV_ARGS(&pPropBag)))) {
                    VARIANT var;
                    VariantInit(&var);
                    if (SUCCEEDED(pPropBag->Read(L"FriendlyName", &var, 0))) {
                        swprintf(logBuf, 512, L"VideoCapture: Device name: %s", var.bstrVal);
                        LogToMilkwave(logBuf);
                        VariantClear(&var);
                    }
                    pPropBag->Release();
                }

                hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&m_pVideoCapture);
                if (FAILED(hr)) {
                    swprintf(logBuf, 512, L"VideoCapture: Failed to bind device, hr=0x%08X", hr);
                    LogToMilkwave(logBuf);
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
    if (FAILED(hr)) {
        LogToMilkwave(L"VideoCapture: AddFilter(VideoCapture) failed");
        return false;
    }

    // TRY TO CONFIGURE STREAM (FRAME RATE AND RESOLUTION)
    LogToMilkwave(L"VideoCapture: Attempting to configure source stream...");
    IAMStreamConfig* pConfig = nullptr;
    hr = m_pCaptureBuilder->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video,
        m_pVideoCapture, IID_PPV_ARGS(&pConfig));

    if (FAILED(hr)) {
        // Try preview pin
        hr = m_pCaptureBuilder->FindInterface(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video,
            m_pVideoCapture, IID_PPV_ARGS(&pConfig));
    }

    if (SUCCEEDED(hr)) {
        LogToMilkwave(L"VideoCapture: Found IAMStreamConfig interface");
        AM_MEDIA_TYPE* pmt = nullptr;
        hr = pConfig->GetFormat(&pmt);
        if (SUCCEEDED(hr)) {
            VIDEOINFOHEADER* pVih = (VIDEOINFOHEADER*)pmt->pbFormat;
            if (pmt->formattype == FORMAT_VideoInfo && pmt->cbFormat >= sizeof(VIDEOINFOHEADER)) {
                // Set frame rate to 30fps (33.3ms) or 60fps
                // Many virtual cameras require a valid interval
                if (pVih->AvgTimePerFrame == 0) {
                    pVih->AvgTimePerFrame = 333333; // 30 fps
                    LogToMilkwave(L"VideoCapture: Corrected frame interval to 30fps");
                }
                
                // Try to set our requested resolution directly on the source
                pVih->bmiHeader.biWidth = width;
                pVih->bmiHeader.biHeight = height;
                pVih->bmiHeader.biSizeImage = width * height * 4;
                
                hr = pConfig->SetFormat(pmt);
                if (SUCCEEDED(hr)) {
                    LogToMilkwave(L"VideoCapture: Successfully set source format resolution/rate");
                }
            }
            if (pmt->cbFormat != 0) CoTaskMemFree((PVOID)pmt->pbFormat);
            if (pmt->pUnk != nullptr) pmt->pUnk->Release();
            CoTaskMemFree(pmt);
        }
        pConfig->Release();
    } else {
        LogToMilkwave(L"VideoCapture: IAMStreamConfig not supported by source pin");
    }

    // Create Sample Grabber
    IBaseFilter* pGrabberFilter = nullptr;
    hr = CoCreateInstance(CLSID_SampleGrabber, nullptr, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pGrabberFilter));
    if (FAILED(hr)) {
        LogToMilkwave(L"VideoCapture: Failed to create SampleGrabber instance");
        return false;
    }

    hr = m_pGraphBuilder->AddFilter(pGrabberFilter, L"Sample Grabber");
    if (FAILED(hr)) {
        LogToMilkwave(L"VideoCapture: AddFilter(SampleGrabber) failed");
        pGrabberFilter->Release();
        return false;
    }

    hr = pGrabberFilter->QueryInterface(IID_PPV_ARGS(&m_pSampleGrabber));
    if (FAILED(hr)) {
        LogToMilkwave(L"VideoCapture: QI(ISampleGrabber) failed");
        pGrabberFilter->Release();
        return false;
    }

    // Request RGB32 format for proper color rendering
    // Don't fail if unsupported - the device will use its default
    LogToMilkwave(L"VideoCapture: Requesting RGB32 media type");
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
            LogToMilkwave(L"VideoCapture: RGB32 media type accepted by SampleGrabber hint");
        } else {
            LogToMilkwave(L"VideoCapture: RGB32 hint not supported by SampleGrabber, using defaults");
        }

        CoTaskMemFree(pVih);
    }

    // Create and add null renderer
    hr = CoCreateInstance(CLSID_NullRenderer, nullptr, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&m_pNullRenderer));
    if (FAILED(hr)) {
        swprintf(logBuf, 512, L"VideoCapture: Failed to create null renderer (0x%08X)", hr);
        LogToMilkwave(logBuf);
        pGrabberFilter->Release();
        return false;
    }

    hr = m_pGraphBuilder->AddFilter(m_pNullRenderer, L"Null Renderer");
    if (FAILED(hr)) {
        swprintf(logBuf, 512, L"VideoCapture: Failed to add null renderer to graph (0x%08X)", hr);
        LogToMilkwave(logBuf);
        pGrabberFilter->Release();
        return false;
    }
    LogToMilkwave(L"VideoCapture: Null renderer added to graph");

    // Connect the entire chain: Capture -> Sample Grabber -> Null Renderer
    LogToMilkwave(L"VideoCapture: Attempting to connect Source -> Grabber -> NullRenderer");
    
    // Attempt 1: Default RenderStream with PIN_CATEGORY_CAPTURE
    hr = m_pCaptureBuilder->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, 
        m_pVideoCapture, pGrabberFilter, m_pNullRenderer);

    if (FAILED(hr)) {
        LogToMilkwave(L"VideoCapture: PIN_CATEGORY_CAPTURE failed, trying PIN_CATEGORY_PREVIEW...");
        hr = m_pCaptureBuilder->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video,
            m_pVideoCapture, pGrabberFilter, m_pNullRenderer);
    }

    if (FAILED(hr)) {
        LogToMilkwave(L"VideoCapture: Standard RenderStream failed, trying manual connection...");
        
        // Find pins
        IPin* pSrcOut = GetFilterPin(m_pVideoCapture, PINDIR_OUTPUT);
        IPin* pGrabIn = GetFilterPin(pGrabberFilter, PINDIR_INPUT);
        IPin* pGrabOut = GetFilterPin(pGrabberFilter, PINDIR_OUTPUT);
        IPin* pNullIn = GetFilterPin(m_pNullRenderer, PINDIR_INPUT);

        if (pSrcOut && pGrabIn) {
            hr = m_pGraphBuilder->Connect(pSrcOut, pGrabIn);
            LogToMilkwave(hr == S_OK ? L"VideoCapture: Source -> Grabber connected manually" : L"VideoCapture: Source -> Grabber manual connection failed");
        }

        if (SUCCEEDED(hr) && pGrabOut && pNullIn) {
            hr = m_pGraphBuilder->Connect(pGrabOut, pNullIn);
            LogToMilkwave(hr == S_OK ? L"VideoCapture: Grabber -> NullRenderer connected manually" : L"VideoCapture: Grabber -> NullRenderer manual connection failed");
        }

        if (pSrcOut) pSrcOut->Release();
        if (pGrabIn) pGrabIn->Release();
        if (pGrabOut) pGrabOut->Release();
        if (pNullIn) pNullIn->Release();
    }

    LogToMilkwave(L"VideoCapture: Connection attempt finished");
    swprintf_s(logBuf, 512, L"VideoCapture: Connection result hr=0x%08X", hr);
    LogToMilkwave(logBuf);

    if (FAILED(hr)) {
        LogToMilkwave(L"VideoCapture: ERROR - All connection attempts failed");
        pGrabberFilter->Release();
        return false;
    }

    // CREATE AND SET CALLBACK AFTER CONNECTING
    LogToMilkwave(L"VideoCapture: Creating callback interface");
    m_pCallback = new VideoCaptureCallback(m_pFrameBuffer, m_nWidth, m_nHeight, &m_frameMutex);

    m_pSampleGrabber->SetBufferSamples(TRUE); 
    m_pSampleGrabber->SetOneShot(FALSE);

    // Using BufferCB mode (1) - often more robust for virtual cameras when buffering is enabled
    hr = m_pSampleGrabber->SetCallback(m_pCallback, 1); 
    if (SUCCEEDED(hr)) {
        LogToMilkwave(L"VideoCapture: Callback set successfully (BufferCB mode)");
        m_bUsingCallback = true;
    } else {
        LogToMilkwave(L"VideoCapture: BufferCB failed, trying SampleCB (mode 0)...");
        hr = m_pSampleGrabber->SetCallback(m_pCallback, 0);
        if (SUCCEEDED(hr)) {
            LogToMilkwave(L"VideoCapture: Callback set successfully (SampleCB mode)");
            m_bUsingCallback = true;
        }
    }

    // NOW DETECT ACTUAL RESOLUTION AND FORMAT
    AM_MEDIA_TYPE connectedMt;
    hr = m_pSampleGrabber->GetConnectedMediaType(&connectedMt);
    if (SUCCEEDED(hr)) {
        if (connectedMt.formattype == FORMAT_VideoInfo && connectedMt.cbFormat >= sizeof(VIDEOINFOHEADER)) {
            VIDEOINFOHEADER* pVih2 = (VIDEOINFOHEADER*)connectedMt.pbFormat;
            int actualWidth = pVih2->bmiHeader.biWidth;
            int actualHeight = abs(pVih2->bmiHeader.biHeight);
            int actualBpp = pVih2->bmiHeader.biBitCount;

            swprintf_s(logBuf, 512, L"VideoCapture: Negotiated format - %dx%d @ %dbpp, Subtype=0x%08X",
                actualWidth, actualHeight, actualBpp, connectedMt.subtype.Data1);
            LogToMilkwave(logBuf);

            // Re-allocate frame buffer if resolution changed from requested
            if (actualWidth != m_nWidth || actualHeight != m_nHeight) {
                LogToMilkwave(L"VideoCapture: Updating internal buffer size to match negotiated resolution");
                m_nWidth = actualWidth;
                m_nHeight = actualHeight;
                delete[] m_pFrameBuffer;
                m_nBufferSize = m_nWidth * m_nHeight * 4; // Always allocate for 4 bytes per pixel
                m_pFrameBuffer = new BYTE[m_nBufferSize];
                memset(m_pFrameBuffer, 0, m_nBufferSize);

                // CRITICAL: Synchronize the callback with the new buffer and resolution
                if (m_pCallback) {
                    m_pCallback->UpdateBuffer(m_pFrameBuffer, m_nWidth, m_nHeight);
                }
            }
        }

        if (connectedMt.cbFormat != 0) CoTaskMemFree((PVOID)connectedMt.pbFormat);
        if (connectedMt.pUnk != nullptr) connectedMt.pUnk->Release();
    } else {
        LogToMilkwave(L"VideoCapture: Could not get connected media type");
    }

    pGrabberFilter->Release();

    hr = m_pGraphBuilder->QueryInterface(IID_PPV_ARGS(&m_pMediaControl));
    if (FAILED(hr)) {
        LogToMilkwave(L"VideoCapture: Failed to get IMediaControl");
        return false;
    }

    m_bInitialized = true;
    LogToMilkwave(L"VideoCapture::Initialize() completed successfully!");
    return true;
}

bool VideoCapture::Start() {
    if (!m_bInitialized || !m_pMediaControl) {
        LogToMilkwave(L"VideoCapture::Start() - not initialized or no media control");
        return false;
    }

    wchar_t logBuf[256];
    LogToMilkwave(L"VideoCapture::Start() - calling IMediaControl::Run()");
    HRESULT hr = m_pMediaControl->Run();
    if (FAILED(hr)) {
        swprintf_s(logBuf, 256, L"VideoCapture::Start() - Run() failed, hr=0x%08X", hr);
        LogToMilkwave(logBuf);
        return false;
    }

    // Give the graph a moment to transition to running state
    // This helps prevent VFW_E_WRONG_STATE errors immediately after starting
    // Use multiple small waits to allow graph to actually start delivering frames
    for (int i = 0; i < 25; i++) {
        Sleep(20);
        OAFilterState state;
        HRESULT stateCheck = m_pMediaControl->GetState(0, &state);
        if (SUCCEEDED(stateCheck) && state == State_Running) {
            swprintf_s(logBuf, 256, L"VideoCapture::Start() - Graph reached Running state in %d ms", (i + 1) * 20);
            LogToMilkwave(logBuf);
            break;  
        }
    }

    LogToMilkwave(L"VideoCapture::Start() - graph start operation completed");
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
wchar_t logBuf[256];

if (!m_bInitialized || !m_pSampleGrabber || !pTexture || !pDevice) {
    if (callCount % 1000 == 1) {
        LogToMilkwave(L"CopyFrameToTexture: Not initialized or invalid parameters");
    }
    return false;
}

// Validate texture by attempting to query its surface level info
if (pTexture) {
    IDirect3DSurface9* pSurface = nullptr;
    HRESULT hrSurface = pTexture->GetSurfaceLevel(0, &pSurface);
    if (FAILED(hrSurface)) {
        if (callCount % 1000 == 1) {
            LogToMilkwave(L"CopyFrameToTexture: Texture validation failed - cannot get surface");
        }
        return false;
    }
    if (pSurface) {
        pSurface->Release();
    }
}


//    Update callback frame count for diagnostics
    if (m_bUsingCallback && m_pCallback) {
        m_callbackFramesReceived = m_pCallback->GetFramesReceived();
        if (m_callbackFramesReceived > 0 && m_frameAttempts % 300 == 1) {
            swprintf_s(logBuf, 256, L"VideoCapture::CopyFrameToTexture - Diagnostic: %d frames received", m_callbackFramesReceived);
            LogToMilkwave(logBuf);
        }
    }

    // Check if media is running and wait for it to be in running state
    if (m_pMediaControl) {
        OAFilterState state;
        HRESULT hrState = m_pMediaControl->GetState(0, &state); 
        if (FAILED(hrState)) {
            if (callCount % 300 == 0) {
                swprintf_s(m_szLastError, 256, L"GetState() failed: 0x%08x", hrState);
                LogToMilkwave(m_szLastError);
            }
            return false;
        }
        if (state != State_Running) {
            if (callCount % 60 == 0) {
                swprintf_s(m_szLastError, 256, L"Graph state=%s (%d), attempts=%d", 
                    (state == State_Paused ? L"PAUSED" : (state == State_Stopped ? L"STOPPED" : L"UNKNOWN")), 
                    state, m_frameAttempts);
                LogToMilkwave(m_szLastError);
            }
            return false;
        }
    }

    // If using callback mode and frames are being received
    if (m_bUsingCallback && m_pCallback && m_pCallback->GetFramesReceived() > 0) {
        if (callCount % 300 == 0) {
            // Only log status if it's the first successful frame or periodically
            if (m_successfulFrames == 0) {
                swprintf_s(m_szLastError, 256, L"Using callback mode - %d frames received so far", m_pCallback->GetFramesReceived());
                LogToMilkwave(m_szLastError);
            }
        }

        // Lock texture - use D3DLOCK_DISCARD to avoid synchronization issues
        D3DLOCKED_RECT lockedRect;
        HRESULT hr = pTexture->LockRect(0, &lockedRect, nullptr, D3DLOCK_DISCARD);
        if (FAILED(hr)) {
            if (callCount % 300 == 0) {
                swprintf_s(m_szLastError, 256, L"LockRect failed: 0x%08x", hr);
                LogToMilkwave(m_szLastError);
            }
            return false;
        }

        // Lock the frame buffer ONLY WHILE COPYING TO TEXTURE
        // Use try_lock to avoid blocking the render thread
        std::unique_lock<std::mutex> lock(m_frameMutex, std::try_to_lock);
        if (lock.owns_lock()) {
            // Copy and FLIP vertically (most webcams are bottom-up DIBs)
            BYTE* pDest = (BYTE*)lockedRect.pBits;
            int srcStride = m_nWidth * 4;
            int copyWidth = min(srcStride, (int)lockedRect.Pitch);

            for (int y = 0; y < m_nHeight; y++) {
                int srcY = m_nHeight - 1 - y;
                memcpy(pDest + y * lockedRect.Pitch, m_pFrameBuffer + srcY * srcStride, copyWidth);
            }
        } else {
            // Couldn't acquire lock - copy zeros instead of blocking
            BYTE* pDest = (BYTE*)lockedRect.pBits;
            ZeroMemory(pDest, lockedRect.Pitch * m_nHeight);
        }

        HRESULT hrUnlock = pTexture->UnlockRect(0);
        if (FAILED(hrUnlock)) {
            if (callCount % 300 == 0) {
                swprintf_s(m_szLastError, 256, L"UnlockRect failed: 0x%08x", hrUnlock);
                LogToMilkwave(m_szLastError);
            }
            return false;
        }

        m_successfulFrames++;
        if (m_successfulFrames == 1) {
            LogToMilkwave(L"VideoCapture: FIRST FRAME COPIED TO TEXTURE VIA CALLBACK!");
        }

        return true;
    }

    // FALLBACK: Only try polling if callback not in use
    if (m_bUsingCallback) {
        // We're in callback mode but no frames received yet
        if (callCount % 300 == 1) {
            swprintf_s(m_szLastError, 256, L"Callback mode active but no frames received yet (attempts=%d)", m_frameAttempts);
            LogToMilkwave(m_szLastError);
        }
        return false;
    }

    // Polling mode fallback
    long bufferSize = 0;
    HRESULT hr = m_pSampleGrabber->GetCurrentBuffer(&bufferSize, nullptr);

    if (hr == VFW_E_WRONG_STATE) {
        if (callCount % 1000 == 1) {
            LogToMilkwave(L"GetCurrentBuffer: VFW_E_WRONG_STATE");
        }
        return false;
    }

    if (FAILED(hr)) {
        if (callCount % 300 == 0) {
            swprintf_s(m_szLastError, 256, L"GetCurrentBuffer failed: 0x%08x", hr);
            LogToMilkwave(m_szLastError);
        }
        return false;
    }

    if (bufferSize == 0) {
        return false;
    }

    BYTE* pBuffer = new BYTE[bufferSize];
    hr = m_pSampleGrabber->GetCurrentBuffer(&bufferSize, (long*)pBuffer);
    if (FAILED(hr)) {
        delete[] pBuffer;
        return false;
    }

    D3DLOCKED_RECT lockedRect;
    hr = pTexture->LockRect(0, &lockedRect, nullptr, D3DLOCK_DISCARD);
    if (FAILED(hr)) {
        delete[] pBuffer;
        return false;
    }

    // Copy and flip
    AM_MEDIA_TYPE mt;
    hr = m_pSampleGrabber->GetConnectedMediaType(&mt);
    if (SUCCEEDED(hr)) {
        VIDEOINFOHEADER* pVih = (VIDEOINFOHEADER*)mt.pbFormat;
        int frameWidth = pVih->bmiHeader.biWidth;
        int frameHeight = abs(pVih->bmiHeader.biHeight);

        if (frameWidth > 0 && frameHeight > 0) {
            BYTE* pDest = (BYTE*)lockedRect.pBits;
            int srcStride = frameWidth * 4;
            for (int y = 0; y < frameHeight && y < m_nHeight; y++) {
                int srcY = frameHeight - 1 - y;
                memcpy(pDest + y * lockedRect.Pitch, pBuffer + srcY * srcStride, min(srcStride, lockedRect.Pitch));
            }
        }
        if (mt.cbFormat != 0) CoTaskMemFree((PVOID)mt.pbFormat);
        if (mt.pUnk != nullptr) mt.pUnk->Release();
    }

    pTexture->UnlockRect(0);
    delete[] pBuffer;

    m_successfulFrames++;
    if (m_successfulFrames == 1) {
        LogToMilkwave(L"VideoCapture: FIRST FRAME CAPTURED SUCCESSFULLY (polling mode)!");
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



