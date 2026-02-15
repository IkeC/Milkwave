#pragma once

#include <dshow.h>
#include <d3d9.h>
#include <vector>
#include <string>
#include <mutex>

#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "quartz.lib")

// Forward declare ISampleGrabber interfaces (qedit.h is deprecated)
MIDL_INTERFACE("0579154A-2B53-4994-B0D0-E773148EFF85")
ISampleGrabberCB : public IUnknown {
    public:
        virtual HRESULT STDMETHODCALLTYPE SampleCB(double SampleTime, IMediaSample *pSample) = 0;
        virtual HRESULT STDMETHODCALLTYPE BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen) = 0;
};

MIDL_INTERFACE("6B652FFF-11FE-4fce-92AD-0266B5D7C78F")
ISampleGrabber : public IUnknown {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetOneShot(BOOL OneShot) = 0;
        virtual HRESULT STDMETHODCALLTYPE SetMediaType(const AM_MEDIA_TYPE *pType) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetConnectedMediaType(AM_MEDIA_TYPE *pType) = 0;
        virtual HRESULT STDMETHODCALLTYPE SetBufferSamples(BOOL BufferThem) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetCurrentBuffer(long *pBufferSize, long *pBuffer) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetCurrentSample(IMediaSample **ppSample) = 0;
        virtual HRESULT STDMETHODCALLTYPE SetCallback(ISampleGrabberCB *pCallback, long WhichMethodToCallback) = 0;
};

static const CLSID CLSID_SampleGrabber = { 0xC1F400A0, 0x3F08, 0x11d3, { 0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37 } };
static const CLSID CLSID_NullRenderer = { 0xC1F400A4, 0x3F08, 0x11d3, { 0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37 } };
static const IID IID_ISampleGrabber = { 0x6B652FFF, 0x11FE, 0x4fce, { 0x92, 0xAD, 0x02, 0x66, 0xB5, 0xD7, 0xC7, 0x8F } };

class VideoCapture {
public:
    VideoCapture();
    ~VideoCapture();

    // Enumerate available video capture devices
    static std::vector<std::wstring> EnumerateDevices();

    // Initialize video capture with specified device
    bool Initialize(int deviceIndex, int width, int height);

    // Start/stop capture
    bool Start();
    void Stop();

    // Copy latest frame to D3D9 texture
    bool CopyFrameToTexture(IDirect3DTexture9* pTexture, IDirect3DDevice9* pDevice);

    // Release all resources
    void Release();
    
    // Generate test pattern for debugging
    static void GenerateTestPattern(BYTE* pBuffer, int width, int height);

    // Check if a new frame is available
    bool HasNewFrame() const { return m_bNewFrame; }

    // Get current resolution
    int GetWidth() const { return m_nWidth; }
    int GetHeight() const { return m_nHeight; }
    
    // Diagnostic: get the last error encountered
    const wchar_t* GetLastError() const { return m_szLastError; }
    
    // Diagnostic: get statistics
    int GetFrameAttempts() const { return m_frameAttempts; }
    int GetSuccessfulFrames() const { return m_successfulFrames; }

private:
// DirectShow interfaces
IGraphBuilder* m_pGraphBuilder;
ICaptureGraphBuilder2* m_pCaptureBuilder;
IBaseFilter* m_pVideoCapture;
ISampleGrabber* m_pSampleGrabber;
IMediaControl* m_pMediaControl;
IBaseFilter* m_pNullRenderer;

    // Frame buffer
    BYTE* m_pFrameBuffer;
    int m_nWidth;
    int m_nHeight;
    int m_nBufferSize;
    
    // Diagnostics
    wchar_t m_szLastError[256];
    int m_frameAttempts;
    int m_successfulFrames;
    bool m_bNewFrame;
    std::mutex m_frameMutex;
    bool m_bInitialized;
};
