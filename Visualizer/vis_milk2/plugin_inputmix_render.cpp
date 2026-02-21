// Input mixing rendering/compositing implementation
#include "plugin.h"

// Update video capture texture with latest frame
void CPlugin::UpdateVideoInputTexture() {
    static bool firstCall = true;
    static int frameCount = 0;

    if (firstCall) {
        wchar_t buf[512];
        swprintf_s(buf, L"========================= UpdateVideoInputTexture START (First Call) =========================");
        if (milkwave) milkwave->LogInfo(buf);
        swprintf_s(buf, L"  m_bVideoInputEnabled=%d, index=%d", m_bVideoInputEnabled, m_nVideoDeviceIndex);
        if (milkwave) milkwave->LogInfo(buf);
        swprintf_s(buf, L"  m_pVideoCapture=0x%p, m_pVideoCaptureTexture=0x%p", m_pVideoCapture, m_pVideoCaptureTexture);
        if (milkwave) milkwave->LogInfo(buf);
        firstCall = false;
    }

    if (!m_bVideoInputEnabled || !m_pVideoCaptureTexture || !m_pVideoCapture) {
        return;
    }

    LPDIRECT3DDEVICE9 lpDevice = GetDevice();
    if (!lpDevice) {
        return;
    }

    // Copy latest frame from video capture to D3D texture
    bool success = m_pVideoCapture->CopyFrameToTexture(m_pVideoCaptureTexture, lpDevice);

    if (frameCount % 60 == 0) {
        wchar_t buf[512];
        swprintf_s(buf, L"UpdateVideoInputTexture: Frame %d - success=%d, attempts=%d, success_frames=%d, callback_frames=%d",
            frameCount, success, 
            m_pVideoCapture->GetFrameAttempts(),
            m_pVideoCapture->GetSuccessfulFrames(),
            m_pVideoCapture->GetCallbackFramesReceived());
        if (milkwave) milkwave->LogInfo(buf);
    }
    frameCount++;
}

// Update Spout input texture with latest frame
void CPlugin::UpdateSpoutInputTexture() {
    static bool firstCall = true;
    static int frameCount = 0;

    if (firstCall) {
        wchar_t buf[512];
        swprintf_s(buf, L"========================= UpdateSpoutInputTexture START (First Call) =========================");
        milkwave->LogInfo(buf);
        swprintf_s(buf, L"  m_bSpoutInputEnabled=%d", m_bSpoutInputEnabled);
        milkwave->LogInfo(buf);
        swprintf_s(buf, L"  m_pSpoutReceiver=0x%p", m_pSpoutReceiver);
        milkwave->LogInfo(buf);
        swprintf_s(buf, L"  m_pSpoutInputTexture=0x%p", m_pSpoutInputTexture);
        milkwave->LogInfo(buf);
        firstCall = false;
    }

    if (!m_bSpoutInputEnabled) {
        if (frameCount == 0) {
            wchar_t buf[256];
            swprintf_s(buf, L"UpdateSpoutInputTexture: Spout input not enabled - returning");
            milkwave->LogInfo(buf);
        }
        return;
    }

    if (!m_pSpoutReceiver) {
        if (frameCount == 0) {
            wchar_t buf[256];
            swprintf_s(buf, L"UpdateSpoutInputTexture: ERROR - no receiver - returning");
            milkwave->LogInfo(buf);
        }
        return;
    }

    // Use the public ReceiveDX9Texture method which handles everything internally:
    // - Connects to sender
    // - Checks for new frames
    // - Creates/updates the texture
    // - Returns texture via reference parameter
    bool received = m_pSpoutReceiver->ReceiveDX9Texture(m_pSpoutInputTexture);

    // Debug: Check connection status
    static bool wasConnected = false;
    bool isConnected = m_pSpoutReceiver->IsConnected();

    if (isConnected && !wasConnected) {
        // Just connected
        wchar_t buf[512];
        swprintf_s(buf, L"[SPOUT CONNECTION] Connected to: %S (%dx%d)", 
            m_pSpoutReceiver->GetSenderName(),
            m_pSpoutReceiver->GetSenderWidth(),
            m_pSpoutReceiver->GetSenderHeight());
        AddNotification(buf);
        milkwave->LogInfo(buf);
        wasConnected = true;
    }
    else if (!isConnected && wasConnected) {
        // Disconnected
        wchar_t buf[256];
        swprintf_s(buf, L"[SPOUT DISCONNECT] Sender disconnected");
        AddNotification(buf);
        milkwave->LogInfo(buf);
        wasConnected = false;
    }

    // Log every 30 frames
    if (frameCount % 30 == 0) {
        wchar_t buf[512];
        swprintf_s(buf, L"UpdateSpoutInputTexture: Frame %d - received=%d, connected=%d, texture=0x%p", 
            frameCount, received, isConnected, m_pSpoutInputTexture);
        milkwave->LogInfo(buf);

        if (isConnected) {
            swprintf_s(buf, L"  [SENDER] Name: %S, Size: %dx%d, NewFrame: %d", 
                m_pSpoutReceiver->GetSenderName(),
                m_pSpoutReceiver->GetSenderWidth(),
                m_pSpoutReceiver->GetSenderHeight(),
                m_pSpoutReceiver->IsFrameNew());
            milkwave->LogInfo(buf);
        }
    }
    frameCount++;

    if (received && m_pSpoutInputTexture) {
        // Texture updated successfully
        // Get dimensions from the received texture
        D3DSURFACE_DESC desc;
        if (SUCCEEDED(m_pSpoutInputTexture->GetLevelDesc(0, &desc))) {
            if (m_nSpoutInputWidth != desc.Width || m_nSpoutInputHeight != desc.Height) {
                wchar_t buf[512];
                swprintf_s(buf, L"[SPOUT RESIZE] Texture size changed: %dx%d -> %dx%d",
                    m_nSpoutInputWidth, m_nSpoutInputHeight, desc.Width, desc.Height);
                milkwave->LogInfo(buf);
                m_nSpoutInputWidth = desc.Width;
                m_nSpoutInputHeight = desc.Height;
            }
        }
    }
}

// Composite input texture onto backbuffer
void CPlugin::CompositeInputMixing(bool isBackground) {
    static bool firstCall = true;
    static int frameCount = 0;

    // Update input textures only when NOT rendering background (or only on first call)
    // Actually, Update*InputTexture should probably only be called ONCE per frame.
    // In RenderFrame, it was already called inside CompositeInputMixing previously.
    // We'll keep it simple for now and update every time it's called, relying on internal checks.
    if (m_bVideoInputEnabled) {
        UpdateVideoInputTexture();
    }
    else if (m_bSpoutInputEnabled) {
        UpdateSpoutInputTexture();
    }

    // Only one input source can be active at a time
    IDirect3DTexture9* pInputTexture = nullptr;

    if (m_bVideoInputEnabled && m_pVideoCaptureTexture) {
        pInputTexture = m_pVideoCaptureTexture;
    }
    else if (m_bSpoutInputEnabled && m_pSpoutInputTexture) {
        pInputTexture = m_pSpoutInputTexture;

        // Debug: Show when we actually have a Spout texture to composite
        if (firstCall) {
            wchar_t buf[256];
            swprintf_s(buf, L"CompositeInputMixing: First call with Spout texture=0x%p", pInputTexture);
            milkwave->LogInfo(buf);
            AddNotification(L"Spout texture ready for compositing");
            firstCall = false;
        }
    }

    if (!pInputTexture) {
        return; // No input to composite
    }

    // Log every 60 frames
    if (frameCount % 60 == 0) {
        wchar_t buf[256];
        swprintf_s(buf, L"CompositeInputMixing: Frame %d - bg=%d, texture=0x%p, opacity=%.2f", 
            frameCount, isBackground, pInputTexture, m_fInputMixOpacity);
        milkwave->LogInfo(buf);
    }
    frameCount++;


    LPDIRECT3DDEVICE9 lpDevice = GetDevice();
    if (!lpDevice) {
        return;
    }

    // Set up for alpha blending
    if (isBackground) {
        // Background should be opaque
        lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    } else {
        // Overlay needs alpha blending
        lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        lpDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        lpDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    }

    // Explicitly disable alpha test to ensure the quad is not accidentally rejected
    lpDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);

    // Disable Z-buffer and culling for the composite pass
    lpDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
    lpDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    lpDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

    // Set the texture
    lpDevice->SetTexture(0, pInputTexture);
    lpDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    lpDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

    // Set texture stage states for TINTING: Color = Texture * TintColor
    lpDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    lpDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    lpDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

    // If background, use full alpha. If overlay, use m_fInputMixOpacity
    lpDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
    lpDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

    // Get dimensions
    float targetWidth = (float)GetWidth();
    float targetHeight = (float)GetHeight();

    if (targetWidth <= 0 || targetHeight <= 0) {
        lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        return;
    }

    // Calculate destination rect to maintain aspect ratio
    D3DSURFACE_DESC inputDesc;
    pInputTexture->GetLevelDesc(0, &inputDesc);

    float backbufferAspect = targetWidth / targetHeight;
    float inputAspect = (float)inputDesc.Width / (float)inputDesc.Height;

    float left = 0, top = 0, right = targetWidth, bottom = targetHeight;

    if (inputAspect > backbufferAspect) {
        // Input is wider - cover the window by fitting height and letting width overflow (to be cropped)
        float scaledWidth = targetHeight * inputAspect;
        float xOffset = (targetWidth - scaledWidth) / 2.0f;
        left = xOffset;
        right = xOffset + scaledWidth;
        top = 0;
        bottom = targetHeight;
    }
    else {
        // Input is taller - cover the window by fitting width and letting height overflow (to be cropped)
        float scaledHeight = targetWidth / inputAspect;
        float yOffset = (targetHeight - scaledHeight) / 2.0f;
        left = 0;
        right = targetWidth;
        top = yOffset;
        bottom = yOffset + scaledHeight;
    }

    // Calculate combined color (Tint + Opacity)
    DWORD r = (m_cInputMixTint >> 16) & 0xFF;
    DWORD g = (m_cInputMixTint >> 8) & 0xFF;
    DWORD b = m_cInputMixTint & 0xFF;

    // Background is 100% alpha (preset on top will handle its own transparency)
    // Overlay uses effectiveOpacity
    float effectiveOpacity = isBackground ? 1.0f : m_fInputMixOpacity;

    DWORD alphaValue = (DWORD)(effectiveOpacity * 255.0f);
    DWORD vertexColor = D3DCOLOR_ARGB(alphaValue, r, g, b);

    struct CUSTOMVERTEX {
        float x, y, z, rhw;
        DWORD color;
        float tu, tv;
    };

    #define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)

    CUSTOMVERTEX vertices[] = {
        { left - 0.5f,  top - 0.5f,    0.5f, 1.0f, vertexColor, 0.0f, 0.0f },
        { right - 0.5f, top - 0.5f,    0.5f, 1.0f, vertexColor, 1.0f, 0.0f },
        { left - 0.5f,  bottom - 0.5f, 0.5f, 1.0f, vertexColor, 0.0f, 1.0f },
        { right - 0.5f, bottom - 0.5f, 0.5f, 1.0f, vertexColor, 1.0f, 1.0f },
    };

    lpDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
    lpDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(CUSTOMVERTEX));

    // Clean up
    lpDevice->SetTexture(0, NULL);
    lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    lpDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
    lpDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
}
