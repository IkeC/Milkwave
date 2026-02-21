// Input mixing rendering/compositing implementation
#include "plugin.h"

// Update video capture texture with latest frame
void CPlugin::UpdateVideoInputTexture() {
    if (!m_bVideoInputEnabled || !m_pVideoCaptureTexture || !m_pVideoCapture) {
        return;
    }
    
    LPDIRECT3DDEVICE9 lpDevice = GetDevice();
    if (!lpDevice) {
        return;
    }
    
    // Copy latest frame from video capture to D3D texture
    m_pVideoCapture->CopyFrameToTexture(m_pVideoCaptureTexture, lpDevice);
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
void CPlugin::CompositeInputMixing() {
    static bool firstCall = true;
    static int frameCount = 0;
    
    // Update input textures with latest frames BEFORE compositing
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
        // Log why we don't have a texture
        static int noTextureLogCounter = 0;
        if (noTextureLogCounter++ % 60 == 0) {
            wchar_t buf[512];
            swprintf_s(buf, L"CompositeInputMixing: No input texture - videoEnabled=%d, spoutEnabled=%d, videoTex=0x%p, spoutTex=0x%p", 
                m_bVideoInputEnabled, m_bSpoutInputEnabled, m_pVideoCaptureTexture, m_pSpoutInputTexture);
            milkwave->LogInfo(buf);
        }
        return; // No input to composite
    }
    
    // Log every 60 frames
    if (frameCount % 60 == 0) {
        wchar_t buf[256];
        swprintf_s(buf, L"CompositeInputMixing: Frame %d - texture=0x%p, opacity=%.2f", 
            frameCount, pInputTexture, m_fPresetOpacity);
        milkwave->LogInfo(buf);
    }
    frameCount++;
    
    
    LPDIRECT3DDEVICE9 lpDevice = GetDevice();
    if (!lpDevice) {
        return;
    }

    // Set up for alpha blending - use vertex alpha, ignore texture alpha
    lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    lpDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    lpDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

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

    // Set texture stage states
    lpDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    lpDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);

    // IGNORE texture alpha channel - use ONLY our global diffuse alpha (m_fPresetOpacity)
    lpDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
    lpDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

    // Get dimensions
    float targetWidth = (float)GetWidth();
    float targetHeight = (float)GetHeight();

    if (targetWidth <= 0 || targetHeight <= 0) {
        // Fallback to backbuffer query if shell metrics are missing
        D3DSURFACE_DESC desc;
        LPDIRECT3DSURFACE9 pBackBuffer = nullptr;
        if (SUCCEEDED(lpDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer))) {
            pBackBuffer->GetDesc(&desc);
            targetWidth = (float)desc.Width;
            targetHeight = (float)desc.Height;
            pBackBuffer->Release();
        } else {
            lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
            return;
        }
    }

    // Calculate destination rect to maintain aspect ratio
    D3DSURFACE_DESC inputDesc;
    pInputTexture->GetLevelDesc(0, &inputDesc);

    float backbufferAspect = targetWidth / targetHeight;
    float inputAspect = (float)inputDesc.Width / (float)inputDesc.Height;

    float left = 0, top = 0, right = targetWidth, bottom = targetHeight;

    if (inputAspect > backbufferAspect) {
        // Input is wider - fit to width
        float scaledHeight = targetWidth / inputAspect;
        float yOffset = (targetHeight - scaledHeight) / 2.0f;
        top = yOffset;
        bottom = yOffset + scaledHeight;
    }
    else {
        // Input is taller - fit to height
        float scaledWidth = targetHeight * inputAspect;
        float xOffset = (targetWidth - scaledWidth) / 2.0f;
        left = xOffset;
        right = xOffset + scaledWidth;
    }

    // Draw a fullscreen quad with the input texture
    // Use preset opacity for blending strength
    float effectiveOpacity = m_fPresetOpacity;
    if (effectiveOpacity < 0.1f) effectiveOpacity = 0.5f; // Debug: ensure visibility if preset opacity is near zero

    DWORD alphaValue = (DWORD)(effectiveOpacity * 255.0f);
    DWORD color = D3DCOLOR_ARGB(alphaValue, 255, 255, 255);

    struct CUSTOMVERTEX {
        float x, y, z, rhw;
        DWORD color;
        float tu, tv;
    };

    #define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)

    CUSTOMVERTEX vertices[] = {
        { left - 0.5f,  top - 0.5f,    0.5f, 1.0f, color, 0.0f, 0.0f },
        { right - 0.5f, top - 0.5f,    0.5f, 1.0f, color, 1.0f, 0.0f },
        { left - 0.5f,  bottom - 0.5f, 0.5f, 1.0f, color, 0.0f, 1.0f },
        { right - 0.5f, bottom - 0.5f, 0.5f, 1.0f, color, 1.0f, 1.0f },
    };

    lpDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
    lpDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(CUSTOMVERTEX));

    // Clean up
    lpDevice->SetTexture(0, NULL);
    lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    lpDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
    lpDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
}
