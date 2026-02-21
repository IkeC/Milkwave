// Input mixing implementation for video and Spout inputs
#include "plugin.h"
#include <string.h>

void CPlugin::SetVideoDevice(int deviceIndex) {
    m_nVideoDeviceIndex = deviceIndex;
    
    // If video mixing is currently enabled, restart capture with new device
    if (m_bVideoInputEnabled && m_pVideoCapture) {
        m_pVideoCapture->Stop();
        delete m_pVideoCapture;
        m_pVideoCapture = nullptr;
        
        if (m_pVideoCaptureTexture) {
            m_pVideoCaptureTexture->Release();
            m_pVideoCaptureTexture = nullptr;
        }
        
        // Reinitialize with new device
        m_pVideoCapture = new VideoCapture();
        int requestW = GetWidth();
        int requestH = GetHeight();
        if (requestW <= 0) requestW = 640;
        if (requestH <= 0) requestH = 480;

        if (m_pVideoCapture->Initialize(deviceIndex, requestW, requestH)) {
            if (m_pVideoCapture->Start()) {
                m_nVideoCaptureWidth = m_pVideoCapture->GetWidth();
                m_nVideoCaptureHeight = m_pVideoCapture->GetHeight();

                // Create D3D texture for video frames
                if (m_lpDX && m_lpDX->m_lpDevice) {
                    HRESULT hr = m_lpDX->m_lpDevice->CreateTexture(
                        m_nVideoCaptureWidth,
                        m_nVideoCaptureHeight,
                        1,
                        D3DUSAGE_DYNAMIC,
                        D3DFMT_X8R8G8B8,
                        D3DPOOL_DEFAULT,
                        &m_pVideoCaptureTexture,
                        NULL
                    );
                }
            }
        }
    }
}

void CPlugin::EnableVideoMixing(bool enable) {
    if (enable == m_bVideoInputEnabled) {
        return; // Already in the requested state
    }
    
    m_bVideoInputEnabled = enable;
    
    if (enable) {
        // Disable Spout mixing if it's enabled (mutually exclusive)
        if (m_bSpoutInputEnabled) {
            EnableSpoutMixing(false);
        }
        
        // Initialize video capture if not already done
        if (!m_pVideoCapture) {
            m_pVideoCapture = new VideoCapture();
            int requestW = GetWidth();
            int requestH = GetHeight();
            if (requestW <= 0) requestW = 640;
            if (requestH <= 0) requestH = 480;

            if (m_pVideoCapture->Initialize(m_nVideoDeviceIndex, requestW, requestH)) {
                if (m_pVideoCapture->Start()) {
                    m_nVideoCaptureWidth = m_pVideoCapture->GetWidth();
                    m_nVideoCaptureHeight = m_pVideoCapture->GetHeight();

                    // Create D3D texture
                    if (m_lpDX && m_lpDX->m_lpDevice) {
                        HRESULT hr = m_lpDX->m_lpDevice->CreateTexture(
                            m_nVideoCaptureWidth,
                            m_nVideoCaptureHeight,
                            1,
                            D3DUSAGE_DYNAMIC,
                            D3DFMT_X8R8G8B8,
                            D3DPOOL_DEFAULT,
                            &m_pVideoCaptureTexture,
                            NULL
                        );
                        
                        if (SUCCEEDED(hr)) {
                            AddNotification(L"Video mixing enabled");
                        } else {
                            m_bVideoInputEnabled = false;
                        }
                    }
                }
            }
        }
    } else {
        // Disable video mixing
        if (m_pVideoCapture) {
            m_pVideoCapture->Stop();
            delete m_pVideoCapture;
            m_pVideoCapture = nullptr;
        }
        
        if (m_pVideoCaptureTexture) {
            m_pVideoCaptureTexture->Release();
            m_pVideoCaptureTexture = nullptr;
        }
        
        AddNotification(L"Video mixing disabled");
    }
}

void CPlugin::SetSpoutSender(const wchar_t* senderName) {
    if (milkwave) {
        wchar_t buf[512];
        swprintf_s(buf, L"========================= SetSpoutSender START =========================");
        milkwave->LogInfo(buf);
        if (senderName) {
            swprintf_s(buf, L"  Setting sender: '%s'", senderName);
            milkwave->LogInfo(buf);
        }
    }

    if (!senderName || senderName[0] == L'\0') {
        if (milkwave) milkwave->LogInfo(L"  ERROR: senderName is NULL or empty!");
        return;
    }

    // Store the wide string safely
    wcsncpy_s(this->m_szSpoutSenderName, _countof(this->m_szSpoutSenderName), senderName, _TRUNCATE);

    // Convert to narrow string for spoutDX9 API
    char narrowSenderName[256] = {0};
    size_t convertedChars = 0;
    wcstombs_s(&convertedChars, narrowSenderName, sizeof(narrowSenderName), senderName, _TRUNCATE);

    if (m_pSpoutReceiver) {
        m_pSpoutReceiver->SetReceiverName(narrowSenderName);
        if (milkwave) milkwave->LogInfo(L"  Receiver updated with new name");
    }

    if (milkwave) milkwave->LogInfo(L"========================= SetSpoutSender END =========================");
}

void CPlugin::EnableSpoutMixing(bool enable) {
    if (enable == m_bSpoutInputEnabled) {
        return; 
    }

    m_bSpoutInputEnabled = enable;

    if (enable) {
        // Disable video mixing if it's enabled (mutually exclusive)
        if (m_bVideoInputEnabled) {
            EnableVideoMixing(false);
        }

        // Initialize Spout receiver if not already done
        if (!m_pSpoutReceiver) {
            m_pSpoutReceiver = new spoutDX9();

            // Set the DX9 device from the plugin
            if (m_lpDX && m_lpDX->m_lpDevice) {
                m_pSpoutReceiver->SetDX9device(m_lpDX->m_lpDevice);
            }

            // If a specific sender was set, configure it
            if (m_szSpoutSenderName[0] != L'\0') {
                char narrowSenderName[256] = {0};
                size_t convertedChars = 0;
                wcstombs_s(&convertedChars, narrowSenderName, sizeof(narrowSenderName), m_szSpoutSenderName, _TRUNCATE);
                m_pSpoutReceiver->SetReceiverName(narrowSenderName);
            }
        }
        AddNotification(L"Spout mixing enabled");
    } else {
        if (m_pSpoutReceiver) {
            if (m_pSpoutInputTexture) {
                m_pSpoutInputTexture->Release();
                m_pSpoutInputTexture = nullptr;
            }
            m_pSpoutReceiver->ReleaseReceiver();
            delete m_pSpoutReceiver;
            m_pSpoutReceiver = nullptr;
        }
        AddNotification(L"Spout mixing disabled");
    }
}

void CPlugin::SetInputMixOnTop(bool onTop) {
    if (milkwave) {
        wchar_t buf[256];
        swprintf_s(buf, L"SetInputMixOnTop: %s", onTop ? L"TRUE" : L"FALSE");
        milkwave->LogInfo(buf);
    }
    m_bInputMixOnTop = onTop;
    if (onTop) {
        AddNotification(L"Input Layer: Top (Overlay)", 2.0f);
    } else {
        AddNotification(L"Input Layer: Background", 2.0f);
    }
}
