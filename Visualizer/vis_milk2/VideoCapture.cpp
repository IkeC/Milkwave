#include "VideoCapture.h"
#include "plugin.h"
#include <comdef.h>

VideoCapture::VideoCapture()
    : m_pGraphBuilder(nullptr)
    , m_pCaptureBuilder(nullptr)
    , m_pVideoCapture(nullptr)
    , m_pSampleGrabber(nullptr)
    , m_pMediaControl(nullptr)
    , m_pFrameBuffer(nullptr)
    , m_nWidth(0)
    , m_nHeight(0)
    , m_nBufferSize(0)
    , m_bNewFrame(false)
    , m_bInitialized(false) {
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
    
    CoInitialize(nullptr);
    
    m_nWidth = width;
    m_nHeight = height;
    m_nBufferSize = width * height * 4; // RGBA
    m_pFrameBuffer = new BYTE[m_nBufferSize];
    memset(m_pFrameBuffer, 0, m_nBufferSize);
    
    HRESULT hr = CoCreateInstance(CLSID_FilterGraph, nullptr, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&m_pGraphBuilder));
    if (FAILED(hr)) return false;
    
    hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, nullptr, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&m_pCaptureBuilder));
    if (FAILED(hr)) return false;
    
    m_pCaptureBuilder->SetFiltergraph(m_pGraphBuilder);
    
    // Enumerate devices and get the one at deviceIndex
    ICreateDevEnum* pDevEnum = nullptr;
    IEnumMoniker* pEnum = nullptr;
    
    hr = CoCreateInstance(CLSID_SystemDeviceEnum, nullptr, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pDevEnum));
    if (FAILED(hr)) return false;
    
    hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, 0);
    if (hr == S_OK) {
        IMoniker* pMoniker = nullptr;
        int currentIndex = 0;
        while (pEnum->Next(1, &pMoniker, nullptr) == S_OK) {
            if (currentIndex == deviceIndex) {
                hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&m_pVideoCapture);
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
    pGrabberFilter->Release();
    if (FAILED(hr)) return false;
    
    // Set media type for RGB32
    AM_MEDIA_TYPE mt;
    ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
    mt.majortype = MEDIATYPE_Video;
    mt.subtype = MEDIASUBTYPE_RGB32;
    mt.formattype = FORMAT_VideoInfo;
    
    hr = m_pSampleGrabber->SetMediaType(&mt);
    if (FAILED(hr)) return false;
    
    // Connect the capture filter to the sample grabber
    hr = m_pCaptureBuilder->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video,
        m_pVideoCapture, nullptr, pGrabberFilter);
    if (FAILED(hr)) return false;
    
    // Set callback (we'll implement this differently - using GetCurrentBuffer instead)
    m_pSampleGrabber->SetBufferSamples(TRUE);
    m_pSampleGrabber->SetOneShot(FALSE);
    
    // Get media control interface
    hr = m_pGraphBuilder->QueryInterface(IID_PPV_ARGS(&m_pMediaControl));
    if (FAILED(hr)) return false;
    
    m_bInitialized = true;
    return true;
}

bool VideoCapture::Start() {
    if (!m_bInitialized || !m_pMediaControl) return false;
    
    HRESULT hr = m_pMediaControl->Run();
    return SUCCEEDED(hr);
}

void VideoCapture::Stop() {
    if (m_pMediaControl) {
        m_pMediaControl->Stop();
    }
}

bool VideoCapture::CopyFrameToTexture(IDirect3DTexture9* pTexture, IDirect3DDevice9* pDevice) {
if (!m_bInitialized || !m_pSampleGrabber || !pTexture) {
    static int logCount = 0;
    if (logCount++ % 120 == 0) { // Log every 2 seconds at 60fps
        wchar_t buf[256];
        swprintf_s(buf, L"VideoCapture: Failed preconditions - init=%d, grabber=%p, texture=%p",
            m_bInitialized, m_pSampleGrabber, pTexture);
        OutputDebugStringW(buf);
    }
    return false;
}
    
// Check if media is running
if (m_pMediaControl) {
    OAFilterState state;
    HRESULT hrState = m_pMediaControl->GetState(0, &state);
    if (SUCCEEDED(hrState) && state != State_Running) {
        static int stateLogCount = 0;
        if (stateLogCount++ % 120 == 0) {
            wchar_t buf[256];
            swprintf_s(buf, L"VideoCapture: Media not running, state=%d", state);
            OutputDebugStringW(buf);
        }
        return false;
    }
}
    
long bufferSize = 0;
HRESULT hr = m_pSampleGrabber->GetCurrentBuffer(&bufferSize, nullptr);
if (FAILED(hr)) {
    static int hrLogCount = 0;
    if (hrLogCount++ % 120 == 0) {
        wchar_t buf[256];
        swprintf_s(buf, L"VideoCapture: GetCurrentBuffer failed with HRESULT=0x%08x", hr);
        OutputDebugStringW(buf);
    }
    return false;
}
if (bufferSize == 0) {
    static int sizeLogCount = 0;
    if (sizeLogCount++ % 120 == 0) {
        OutputDebugStringW(L"VideoCapture: Buffer size is 0 - no frames captured yet");
    }
    return false;
}
    
    BYTE* pBuffer = new BYTE[bufferSize];
    hr = m_pSampleGrabber->GetCurrentBuffer(&bufferSize, (long*)pBuffer);
    if (FAILED(hr)) {
        delete[] pBuffer;
        return false;
    }
    
    // Lock texture
    D3DLOCKED_RECT lockedRect;
    hr = pTexture->LockRect(0, &lockedRect, nullptr, 0);
    if (FAILED(hr)) {
        delete[] pBuffer;
        return false;
    }
    
    // Get actual frame dimensions from media type
    AM_MEDIA_TYPE mt;
    hr = m_pSampleGrabber->GetConnectedMediaType(&mt);
    if (SUCCEEDED(hr)) {
        VIDEOINFOHEADER* pVih = (VIDEOINFOHEADER*)mt.pbFormat;
        int frameWidth = pVih->bmiHeader.biWidth;
        int frameHeight = abs(pVih->bmiHeader.biHeight);
        
        // Copy frame data to texture
        // DirectShow gives us bottom-up DIB data, so we need to flip it for D3D
        BYTE* pDest = (BYTE*)lockedRect.pBits;
        int srcStride = frameWidth * 4;
        
        // Copy rows in reverse order to flip the image
        for (int y = 0; y < frameHeight && y < m_nHeight; y++) {
            int srcY = frameHeight - 1 - y; // Flip vertically
            memcpy(pDest + y * lockedRect.Pitch, pBuffer + srcY * srcStride, 
                min(srcStride, lockedRect.Pitch));
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
    
    return true;
}

void VideoCapture::Release() {
    Stop();
    
    if (m_pMediaControl) {
        m_pMediaControl->Release();
        m_pMediaControl = nullptr;
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

