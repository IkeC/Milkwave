// loopback-capture.cpp

#include "common.h"
#include "..\vis_milk2\milkwave.h"
#include <string>
#include <stdexcept>
#include <vector>

DWORD WINAPI LoopbackCaptureThreadFunction(LPVOID pContext);

// Helper function for GetBuffer with SEH protection and retry logic
HRESULT GetBufferWithRetry(
  IAudioCaptureClient* pAudioCaptureClient,
  BYTE** ppData,
  UINT32* pNumFramesToRead,
  DWORD* pFlags,
  Milkwave* pMilkwave
);

HRESULT LoopbackCapture(
  IMMDevice* pMMDevice,
  bool bIsRenderDevice,
  HMMIO hFile,
  bool bInt16,
  Milkwave* pMilkwave,
  HANDLE hStartedEvent,
  HANDLE hStopEvent,
  PUINT32 pnFrames
);

HRESULT WriteWaveHeader(HMMIO hFile, LPCWAVEFORMATEX pwfx, MMCKINFO* pckRIFF, MMCKINFO* pckData);
HRESULT FinishWaveFile(HMMIO hFile, MMCKINFO* pckRIFF, MMCKINFO* pckData);

DWORD WINAPI LoopbackCaptureThreadFunction(LPVOID pContext) {
  LoopbackCaptureThreadFunctionArguments* pArgs =
    (LoopbackCaptureThreadFunctionArguments*)pContext;

  // Log thread start if milkwave provided
  if (pArgs && pArgs->pMilkwave) {
    try {
      pArgs->pMilkwave->LogInfo(L"LoopbackCaptureThreadFunction: thread starting");
    } catch (...) {
      // ignore
    }
  }

  pArgs->hr = CoInitialize(NULL);
  if (FAILED(pArgs->hr)) {
    ERR(L"CoInitialize failed: hr =0x%08x", pArgs->hr);
    if (pArgs && pArgs->pMilkwave) {
      // Also log to Milkwave if available
      wchar_t buf[128];
      swprintf_s(buf, L"CoInitialize failed: hr =0x%08x", pArgs->hr);
      try {
        pArgs->pMilkwave->LogInfo(buf);
      } catch (...) {
        // ignore logging failures
      }
    }
    return 0;
  }
  CoUninitializeOnExit cuoe;

  try {
    pArgs->hr = LoopbackCapture(
      pArgs->pMMDevice,
      pArgs->bIsRenderDevice,
      pArgs->hFile,
      pArgs->bInt16,
      pArgs->pMilkwave,
      pArgs->hStartedEvent,
      pArgs->hStopEvent,
      &pArgs->nFrames
    );
  } catch (const std::exception& e) {
    if (pArgs && pArgs->pMilkwave) {
      try {
        pArgs->pMilkwave->LogException(L"LoopbackCaptureThreadFunction: exception", e, true);
      } catch (...) {
        // ignore logging failures
      }
    }
    pArgs->hr = E_FAIL;
  } catch (...) {
    // Attempt to capture more information about non-std exceptions
    if (pArgs && pArgs->pMilkwave) {
      try {
        try {
          throw; // rethrow to identify
        } catch (const std::exception& e) {
          // This should be handled by the std::exception handler above, but handle defensively
          pArgs->pMilkwave->LogException(L"LoopbackCaptureThreadFunction: rethrown std::exception", e, true);
        } catch (const wchar_t* ws) {
          // convert wchar_t* to UTF-8
          std::string msg;
          if (ws) {
            int size = WideCharToMultiByte(CP_UTF8, 0, ws, -1, nullptr, 0, nullptr, nullptr);
            if (size > 0) {
              msg.resize(size - 1);
              WideCharToMultiByte(CP_UTF8, 0, ws, -1, &msg[0], size, nullptr, nullptr);
            }
          }
          else {
            msg = "(null)";
          }
          std::runtime_error re(msg);
          pArgs->pMilkwave->LogException(L"LoopbackCaptureThreadFunction: non-std exception (wchar_t*)", re, true);
        } catch (const char* cs) {
          std::runtime_error re(cs ? cs : "(null)");
          pArgs->pMilkwave->LogException(L"LoopbackCaptureThreadFunction: non-std exception (char*)", re, true);
        } catch (...) {
          std::runtime_error re("Unknown non-std exception");
          pArgs->pMilkwave->LogException(L"LoopbackCaptureThreadFunction: unknown non-std exception", re, true);
        }
      } catch (...) {
        // If logging itself throws, swallow to avoid termination
      }
    }
    pArgs->hr = E_FAIL;
  }

  return 0;
}

HRESULT LoopbackCapture(
  IMMDevice* pMMDevice,
  bool bIsRenderDevice,
  HMMIO hFile,
  bool bInt16,
  Milkwave* pMilkwave,
  HANDLE hStartedEvent,
  HANDLE hStopEvent,
  PUINT32 pnFrames
) {
  HRESULT hr;

  // Optionally log start of LoopbackCapture if milkwave provided
  if (pMilkwave) {
    try {
      pMilkwave->LogInfo(L"LoopbackCapture: starting capture");
    } catch (...) {
      // ignore logging failures
    }
  }

  if (pMilkwave && pMMDevice) {
    try {
      LPWSTR deviceId = nullptr;
      HRESULT hrDeviceId = pMMDevice->GetId(&deviceId);
      if (SUCCEEDED(hrDeviceId)) {
        CoTaskMemFreeOnExit freeId(deviceId);
        wchar_t buf[512];
        swprintf_s(buf, L"LoopbackCapture: deviceId=%ls isRender=%ls", deviceId, bIsRenderDevice ? L"true" : L"false");
        pMilkwave->LogDebug(buf);
      }
      else {
        wchar_t buf[256];
        swprintf_s(buf, L"LoopbackCapture: GetId failed hr=0x%08x", hrDeviceId);
        pMilkwave->LogDebug(buf);
      }
    } catch (...) {
      // ignore logging failures
    }
  }

  try {
    // activate an IAudioClient
    if (pMilkwave) { try { pMilkwave->LogInfo(L"LoopbackCapture: calling IMMDevice::Activate"); } catch (...) {} }
    IAudioClient* pAudioClient;
    hr = pMMDevice->Activate(
      __uuidof(IAudioClient),
      CLSCTX_ALL, NULL,
      (void**)&pAudioClient
    );
    if (FAILED(hr)) {
      ERR(L"IMMDevice::Activate(IAudioClient) failed: hr =0x%08x", hr);
      if (pMilkwave) { try { wchar_t buf[256]; swprintf_s(buf, L"IMMDevice::Activate failed: hr =0x%08x", hr); pMilkwave->LogInfo(buf); } catch (...) {} }
      return hr;
    }
    ReleaseOnExit releaseAudioClient(pAudioClient);

    // get the default device periodicity
    if (pMilkwave) { try { pMilkwave->LogInfo(L"LoopbackCapture: calling IAudioClient::GetDevicePeriod"); } catch (...) {} }
    REFERENCE_TIME hnsDefaultDevicePeriod;
    hr = pAudioClient->GetDevicePeriod(&hnsDefaultDevicePeriod, NULL);
    if (FAILED(hr)) {
      ERR(L"IAudioClient::GetDevicePeriod failed: hr =0x%08x", hr);
      if (pMilkwave) { try { wchar_t buf[256]; swprintf_s(buf, L"IAudioClient::GetDevicePeriod failed: hr =0x%08x", hr); pMilkwave->LogInfo(buf); } catch (...) {} }
      return hr;
    }

    if (pMilkwave) {
      try {
        double defaultPeriodMs = static_cast<double>(hnsDefaultDevicePeriod) / 10000.0;
        wchar_t buf[256];
        swprintf_s(buf, L"LoopbackCapture: default period %.3f ms", defaultPeriodMs);
        pMilkwave->LogDebug(buf);
      } catch (...) {}
    }

    // get the default device format
    if (pMilkwave) { try { pMilkwave->LogInfo(L"LoopbackCapture: calling IAudioClient::GetMixFormat"); } catch (...) {} }
    WAVEFORMATEX* pwfx;
    hr = pAudioClient->GetMixFormat(&pwfx);
    if (FAILED(hr)) {
      ERR(L"IAudioClient::GetMixFormat failed: hr =0x%08x", hr);
      if (pMilkwave) { try { wchar_t buf[256]; swprintf_s(buf, L"IAudioClient::GetMixFormat failed: hr =0x%08x", hr); pMilkwave->LogInfo(buf); } catch (...) {} }
      return hr;
    }
    CoTaskMemFreeOnExit freeMixFormat(pwfx);

    if (pMilkwave && pwfx) {
      try {
        wchar_t buf[256];
        swprintf_s(
          buf,
          L"LoopbackCapture: mix format rate=%lu channels=%u bits=%u blockAlign=%u avgBytesPerSec=%lu",
          pwfx->nSamplesPerSec,
          pwfx->nChannels,
          pwfx->wBitsPerSample,
          pwfx->nBlockAlign,
          pwfx->nAvgBytesPerSec
        );
        pMilkwave->LogDebug(buf);
      } catch (...) {}
    }

    if (bInt16) {
      // coerce int-16 wave format
      // can do this in-place since we're not changing the size of the format
      // also, the engine will auto-convert from float to int for us
      switch (pwfx->wFormatTag) {
      case WAVE_FORMAT_IEEE_FLOAT:
        pwfx->wFormatTag = WAVE_FORMAT_PCM;
        pwfx->wBitsPerSample = 16;
        pwfx->nBlockAlign = pwfx->nChannels * pwfx->wBitsPerSample / 8;
        pwfx->nAvgBytesPerSec = pwfx->nBlockAlign * pwfx->nSamplesPerSec;
        break;

      case WAVE_FORMAT_EXTENSIBLE:
      {
        // naked scope for case-local variable
        PWAVEFORMATEXTENSIBLE pEx = reinterpret_cast<PWAVEFORMATEXTENSIBLE>(pwfx);
        if (IsEqualGUID(KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, pEx->SubFormat)) {
          pEx->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
          pEx->Samples.wValidBitsPerSample = 16;
          pwfx->wBitsPerSample = 16;
          pwfx->nBlockAlign = pwfx->nChannels * pwfx->wBitsPerSample / 8;
          pwfx->nAvgBytesPerSec = pwfx->nBlockAlign * pwfx->nSamplesPerSec;
        }
        else {
          ERR(L"%s", L"Don't know how to coerce mix format to int-16");
          if (pMilkwave) { try { pMilkwave->LogInfo(L"Don't know how to coerce mix format to int-16"); } catch (...) {} }
          return E_UNEXPECTED;
        }
      }
      break;

      default:
        ERR(L"Don't know how to coerce WAVEFORMATEX with wFormatTag =0x%08x to int-16", pwfx->wFormatTag);
        if (pMilkwave) { try { wchar_t buf[256]; swprintf_s(buf, L"Don't know how to coerce WAVEFORMATEX with wFormatTag =0x%08x to int-16", pwfx->wFormatTag); pMilkwave->LogInfo(buf); } catch (...) {} }
        return E_UNEXPECTED;
      }
    }

    MMCKINFO ckRIFF = { 0 };
    MMCKINFO ckData = { 0 };

    if (NULL != hFile) {
      if (pMilkwave) { try { pMilkwave->LogInfo(L"LoopbackCapture: calling WriteWaveHeader"); } catch (...) {} }
      hr = WriteWaveHeader(hFile, pwfx, &ckRIFF, &ckData);
      if (FAILED(hr)) {
        // WriteWaveHeader does its own logging
        if (pMilkwave) { try { pMilkwave->LogInfo(L"WriteWaveHeader failed"); } catch (...) {} }
        return hr;
      }
    }

    // create a periodic waitable timer
    if (pMilkwave) { try { pMilkwave->LogInfo(L"LoopbackCapture: creating waitable timer"); } catch (...) {} }
    HANDLE hWakeUp = CreateWaitableTimer(NULL, FALSE, NULL);
    if (NULL == hWakeUp) {
      DWORD dwErr = GetLastError();
      ERR(L"CreateWaitableTimer failed: last error = %u", dwErr);
      if (pMilkwave) { try { wchar_t buf[256]; swprintf_s(buf, L"CreateWaitableTimer failed: last error = %u", dwErr); pMilkwave->LogInfo(buf); } catch (...) {} }
      return HRESULT_FROM_WIN32(dwErr);
    }
    CloseHandleOnExit closeWakeUp(hWakeUp);

    UINT32 nBlockAlign = pwfx->nBlockAlign;
    *pnFrames = 0;

    // call IAudioClient::Initialize
    // note that AUDCLNT_STREAMFLAGS_LOOPBACK and AUDCLNT_STREAMFLAGS_EVENTCALLBACK
    // do not work together...
    // the "data ready" event never gets set
    // so we're going to do a timer-driven loop
    if (pMilkwave) { try { pMilkwave->LogInfo(L"LoopbackCapture: calling IAudioClient::Initialize"); } catch (...) {} }
    hr = pAudioClient->Initialize(
      AUDCLNT_SHAREMODE_SHARED,
      bIsRenderDevice ? AUDCLNT_STREAMFLAGS_LOOPBACK : 0,
      0, 0, pwfx, nullptr
    );

    if (FAILED(hr)) {
      ERR(L"IAudioClient::Initialize failed: hr =0x%08x", hr);
      if (pMilkwave) { try { wchar_t buf[256]; swprintf_s(buf, L"IAudioClient::Initialize failed: hr =0x%08x", hr); pMilkwave->LogInfo(buf); } catch (...) {} }
      return hr;
    }

    if (pMilkwave) {
      try {
        wchar_t buf[256];
        swprintf_s(
          buf,
          L"LoopbackCapture: initialize flags loopback=%ls int16=%ls",
          bIsRenderDevice ? L"true" : L"false",
          bInt16 ? L"true" : L"false"
        );
        pMilkwave->LogDebug(buf);
      } catch (...) {}
    }

    // activate an IAudioCaptureClient
    if (pMilkwave) { try { pMilkwave->LogInfo(L"LoopbackCapture: calling IAudioClient::GetService for IAudioCaptureClient"); } catch (...) {} }
    IAudioCaptureClient* pAudioCaptureClient;
    hr = pAudioClient->GetService(
      __uuidof(IAudioCaptureClient),
      (void**)&pAudioCaptureClient
    );
    if (FAILED(hr)) {
      ERR(L"IAudioClient::GetService(IAudioCaptureClient) failed: hr =0x%08x", hr);
      if (pMilkwave) { try { wchar_t buf[256]; swprintf_s(buf, L"IAudioClient::GetService(IAudioCaptureClient) failed: hr =0x%08x", hr); pMilkwave->LogInfo(buf); } catch (...) {} }
      return hr;
    }
    ReleaseOnExit releaseAudioCaptureClient(pAudioCaptureClient);

    // set the waitable timer
    LARGE_INTEGER liFirstFire;
    liFirstFire.QuadPart = -hnsDefaultDevicePeriod / 2; // negative means relative time
    LONG lTimeBetweenFires = (LONG)hnsDefaultDevicePeriod / 2 / (10 * 1000); // convert to milliseconds
    if (pMilkwave) { try { pMilkwave->LogInfo(L"LoopbackCapture: calling SetWaitableTimer"); } catch (...) {} }
    BOOL bOK = SetWaitableTimer(
      hWakeUp,
      &liFirstFire,
      lTimeBetweenFires,
      NULL, NULL, FALSE
    );
    if (!bOK) {
      DWORD dwErr = GetLastError();
      ERR(L"SetWaitableTimer failed: last error = %u", dwErr);
      if (pMilkwave) { try { wchar_t buf[256]; swprintf_s(buf, L"SetWaitableTimer failed: last error = %u", dwErr); pMilkwave->LogInfo(buf); } catch (...) {} }
      return HRESULT_FROM_WIN32(dwErr);
    }
    CancelWaitableTimerOnExit cancelWakeUp(hWakeUp);

    // call IAudioClient::Start
    if (pMilkwave) { try { pMilkwave->LogInfo(L"LoopbackCapture: calling IAudioClient::Start"); } catch (...) {} }
    hr = pAudioClient->Start();
    if (FAILED(hr)) {
      ERR(L"IAudioClient::Start failed: hr =0x%08x", hr);
      if (pMilkwave) { try { wchar_t buf[256]; swprintf_s(buf, L"IAudioClient::Start failed: hr =0x%08x", hr); pMilkwave->LogInfo(buf); } catch (...) {} }
      return hr;
    }
    AudioClientStopOnExit stopAudioClient(pAudioClient);

    SetEvent(hStartedEvent);

    // loopback capture loop
    HANDLE waitArray[2] = { hStopEvent, hWakeUp };
    DWORD dwWaitResult;

    bool bDone = false;
    bool bFirstPacket = true;

    bool bErrorInAudioData = false;

    for (UINT32 nPasses = 0; !bDone; nPasses++) {
      // drain data while it is available
      UINT32 nNextPacketSize;
      // if (pMilkwave) { try { pMilkwave->LogDebug(L"LoopbackCapture: entering packet drain loop / calling GetNextPacketSize"); } catch(...){} }
      for (
        hr = pAudioCaptureClient->GetNextPacketSize(&nNextPacketSize);
        SUCCEEDED(hr) && nNextPacketSize > 0;
        hr = pAudioCaptureClient->GetNextPacketSize(&nNextPacketSize)
        ) {
        // get the captured data
        // if (pMilkwave) { try { pMilkwave->LogDebug(L"LoopbackCapture: calling IAudioCaptureClient::GetBuffer (with retry)"); } catch (...) {} }
        BYTE* pData = nullptr;
        UINT32 nNumFramesToRead = 0;
        DWORD dwFlags = 0;

        hr = GetBufferWithRetry(pAudioCaptureClient, &pData, &nNumFramesToRead, &dwFlags, pMilkwave);

        bErrorInAudioData = false;

        if (FAILED(hr)) {
          bErrorInAudioData = true;
          ERR(L"IAudioCaptureClient::GetBuffer failed on pass %u after %u frames: hr =0x%08x", nPasses, *pnFrames, hr);
          if (pMilkwave) { try { wchar_t buf[256]; swprintf_s(buf, L"IAudioCaptureClient::GetBuffer failed on pass %u after %u frames: hr =0x%08x", nPasses, *pnFrames, hr); pMilkwave->LogInfo(buf); } catch (...) {} }
          return hr;
        }

        const bool bSilentBuffer = (dwFlags & AUDCLNT_BUFFERFLAGS_SILENT) != 0;
        DWORD dwFlagsWithoutSilent = dwFlags & ~AUDCLNT_BUFFERFLAGS_SILENT;

        if (bFirstPacket && AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY == dwFlagsWithoutSilent) {
          //bErrorInAudioData = true;
          LOG(L"%s", L"Probably spurious glitch reported on first packet");
        }
        else if (0 != dwFlagsWithoutSilent) {
          bErrorInAudioData = true;
          LOG(L"IAudioCaptureClient::GetBuffer set flags to0x%08x on pass %u after %u frames", dwFlags, nPasses, *pnFrames);
          if (pMilkwave) { try { wchar_t buf[256]; swprintf_s(buf, L"IAudioCaptureClient::GetBuffer set flags to0x%08x on pass %u after %u frames", dwFlags, nPasses, *pnFrames); pMilkwave->LogInfo(buf); } catch (...) {} }
          //return E_UNEXPECTED;
        }

        if (0 == nNumFramesToRead) {
          bErrorInAudioData = true;
          ERR(L"IAudioCaptureClient::GetBuffer said to read0 frames on pass %u after %u frames", nPasses, *pnFrames);
          if (pMilkwave) { try { wchar_t buf[256]; swprintf_s(buf, L"IAudioCaptureClient::GetBuffer said to read0 frames on pass %u after %u frames", nPasses, *pnFrames); pMilkwave->LogInfo(buf); } catch (...) {} }
          //return E_UNEXPECTED;
        }

        if (bErrorInAudioData) {
          // Glitch in audio detected so we reset audio buffer and avoid writing to the output .wav file
          ResetAudioBuf();
        }
        else {
          // Ensure we have valid audio data even when WASAPI marks the buffer as silent
          std::vector<BYTE> silentBuffer;
          const BYTE* pDataForVisualizer = pData;
          if (bSilentBuffer) {
            silentBuffer.assign(static_cast<size_t>(nNumFramesToRead) * nBlockAlign, 0);
            pDataForVisualizer = silentBuffer.data();
          }

          // Saving audio data for visualizer
          SetAudioBuf(pDataForVisualizer, nNumFramesToRead, pwfx, bInt16);

          if (NULL != hFile) {
            // Writing the buffer captured to the output .wav file
            if (pMilkwave) { try { pMilkwave->LogInfo(L"LoopbackCapture: writing buffer to file via mmioWrite"); } catch (...) {} }
            LONG lBytesToWrite = nNumFramesToRead * nBlockAlign;
            const BYTE* pDataForFile = bSilentBuffer ? pDataForVisualizer : pData;
#pragma prefast(suppress: __WARNING_INCORRECT_ANNOTATION, "IAudioCaptureClient::GetBuffer SAL annotation implies a1-byte buffer")
            LONG lBytesWritten = mmioWrite(hFile, reinterpret_cast<PCHAR>(const_cast<BYTE*>(pDataForFile)), lBytesToWrite);
            if (lBytesToWrite != lBytesWritten) {
              ERR(L"mmioWrite wrote %u bytes on pass %u after %u frames: expected %u bytes", lBytesWritten, nPasses, *pnFrames, lBytesToWrite);
              if (pMilkwave) { try { wchar_t buf[256]; swprintf_s(buf, L"mmioWrite wrote %u bytes on pass %u after %u frames: expected %u bytes", lBytesWritten, nPasses, *pnFrames, lBytesToWrite); pMilkwave->LogInfo(buf); } catch (...) {} }
              return E_UNEXPECTED;
            }
          }
          *pnFrames += nNumFramesToRead;
        }

        // if (pMilkwave) { try { pMilkwave->LogDebug(L"LoopbackCapture: calling IAudioCaptureClient::ReleaseBuffer"); } catch (...) {} }
        hr = pAudioCaptureClient->ReleaseBuffer(nNumFramesToRead);
        if (FAILED(hr)) {
          ERR(L"IAudioCaptureClient::ReleaseBuffer failed on pass %u after %u frames: hr =0x%08x", nPasses, *pnFrames, hr);
          if (pMilkwave) { try { wchar_t buf[256]; swprintf_s(buf, L"IAudioCaptureClient::ReleaseBuffer failed on pass %u after %u frames: hr =0x%08x", nPasses, *pnFrames, hr); pMilkwave->LogInfo(buf); } catch (...) {} }
          return hr;
        }

        bFirstPacket = false;
      }

      if (FAILED(hr)) {
        ERR(L"IAudioCaptureClient::GetNextPacketSize failed on pass %u after %u frames: hr =0x%08x", nPasses, *pnFrames, hr);
        if (pMilkwave) { try { wchar_t buf[256]; swprintf_s(buf, L"IAudioCaptureClient::GetNextPacketSize failed on pass %u after %u frames: hr =0x%08x", nPasses, *pnFrames, hr); pMilkwave->LogInfo(buf); } catch (...) {} }
        return hr;
      }

      dwWaitResult = WaitForMultipleObjects(
        ARRAYSIZE(waitArray), waitArray,
        FALSE, INFINITE
      );

      if (WAIT_OBJECT_0 == dwWaitResult) {
        LOG(L"Received stop event after %u passes and %u frames", nPasses, *pnFrames);
        bDone = true;
        continue; // exits loop
      }

      if (WAIT_OBJECT_0 + 1 != dwWaitResult) {
        ERR(L"Unexpected WaitForMultipleObjects return value %u on pass %u after %u frames", dwWaitResult, nPasses, *pnFrames);
        if (pMilkwave) { try { wchar_t buf[256]; swprintf_s(buf, L"Unexpected WaitForMultipleObjects return value %u on pass %u after %u frames", dwWaitResult, nPasses, *pnFrames); pMilkwave->LogInfo(buf); } catch (...) {} }
        return E_UNEXPECTED;
      }
    } // capture loop

    if (NULL != hFile) {
      if (pMilkwave) { try { pMilkwave->LogInfo(L"LoopbackCapture: calling FinishWaveFile"); } catch (...) {} }
      hr = FinishWaveFile(hFile, &ckData, &ckRIFF);
      if (FAILED(hr)) {
        if (pMilkwave) {
          try { pMilkwave->LogInfo(L"FinishWaveFile failed"); } catch (...) {}
        }
        return hr;
      }
    }

    return hr;
  } catch (const std::exception& e) {
    ERR(L"LoopbackCapture threw exception: %hs", e.what());
    if (pMilkwave) {
      try { pMilkwave->LogException(L"LoopbackCapture: exception", e, true); } catch (...) {}
    }
    return E_FAIL;
  } catch (...) {
    ERR(L"LoopbackCapture threw unknown exception");
    if (pMilkwave) {
      try {
        std::runtime_error re("Unknown non-std exception in LoopbackCapture");
        pMilkwave->LogException(L"LoopbackCapture: unknown exception", re, true);
      } catch (...) {}
    }
    return E_FAIL;
  }
}

// Helper function for GetBuffer with SEH protection and retry logic
// Isolated in its own function to avoid mixing SEH with C++ exception handling
HRESULT GetBufferWithRetry(
  IAudioCaptureClient* pAudioCaptureClient,
  BYTE** ppData,
  UINT32* pNumFramesToRead,
  DWORD* pFlags,
  Milkwave* pMilkwave
) {
  const int kMaxRetries = 5;
  int backoffMs = 100;
  HRESULT hr = S_OK;

#if defined(_WIN32) && defined(_MSC_VER)
  for (int attempt = 0; attempt < kMaxRetries; ++attempt) {
    bool hadSeh = false;
    DWORD sehCode = 0;
    void* vtablePtr = nullptr;
    HRESULT attemptHr = S_OK;
    BYTE* pDataResult = nullptr;
    UINT32 framesResult = 0;
    DWORD flagsResult = 0;

    __try {
      if (nullptr == pAudioCaptureClient) {
        ERR(L"GetBufferWithRetry: pAudioCaptureClient is NULL");
        hr = E_POINTER;
 break;
      }

      // Capture vtable pointer before the call
      vtablePtr = (void*)(*((void**)pAudioCaptureClient));

      attemptHr = pAudioCaptureClient->GetBuffer(
        &pDataResult,
        &framesResult,
      &flagsResult,
        NULL,
        NULL
      );
    } __except (EXCEPTION_EXECUTE_HANDLER) {
    hadSeh = true;
      sehCode = GetExceptionCode();
      attemptHr = E_FAIL;
      ERR(L"GetBufferWithRetry: SEH exception 0x%08x (attempt %d)", sehCode, attempt + 1);
    }

    // Store results if successful
    if (!hadSeh && SUCCEEDED(attemptHr)) {
      *ppData = pDataResult;
      *pNumFramesToRead = framesResult;
      *pFlags = flagsResult;
      hr = attemptHr;
    } else {
      hr = attemptHr;
    }

    // Log AFTER __try/__except to avoid mixing exception handling
  if (pMilkwave) {
      wchar_t buf[256];
      if (hadSeh) {
        swprintf_s(buf, L"GetBufferWithRetry: attempt %d SEH exception 0x%08x (pAudioCaptureClient=%p, vtable=%p)", 
          attempt + 1, sehCode, pAudioCaptureClient, vtablePtr);
 } else {
        swprintf_s(buf, L"GetBufferWithRetry: attempt %d hr=0x%08x (pData=%p, frames=%u, flags=0x%08x)", 
 attempt + 1, hr, pDataResult, framesResult, flagsResult);
      }
      pMilkwave->LogInfo(buf);
    }

    // Success case
    if (!hadSeh && SUCCEEDED(hr) && (pDataResult != nullptr || (flagsResult & AUDCLNT_BUFFERFLAGS_SILENT))) {
      break;
    }

    // If last attempt, break and return failure
    if (attempt == kMaxRetries - 1) {
   ERR(L"GetBufferWithRetry: failed after %d attempts, hr=0x%08x", kMaxRetries, hr);
   if (pMilkwave) {
 wchar_t buf[256];
        swprintf_s(buf, L"GetBufferWithRetry: FAILED after %d attempts, hr=0x%08x", kMaxRetries, hr);
        pMilkwave->LogInfo(buf);
    }
  break;
    }

    // Backoff before retrying
 if (pMilkwave) {
      wchar_t buf[128];
      swprintf_s(buf, L"GetBufferWithRetry: attempt %d failed, backing off %d ms", attempt + 1, backoffMs);
      pMilkwave->LogDebug(buf);
    }
    Sleep(backoffMs);
    backoffMs = backoffMs * 2;
  }
#else
  // Non-MSVC: single attempt
  if (nullptr == pAudioCaptureClient) {
    ERR(L"GetBufferWithRetry: pAudioCaptureClient is NULL");
    return E_POINTER;
  }
  hr = pAudioCaptureClient->GetBuffer(
    ppData,
    pNumFramesToRead,
    pFlags,
    NULL,
    NULL
  );
#endif

  return hr;
}

HRESULT WriteWaveHeader(HMMIO hFile, LPCWAVEFORMATEX pwfx, MMCKINFO* pckRIFF, MMCKINFO* pckData) {
  MMRESULT result;

  // make a RIFF/WAVE chunk
  pckRIFF->ckid = MAKEFOURCC('R', 'I', 'F', 'F');
  pckRIFF->fccType = MAKEFOURCC('W', 'A', 'V', 'E');

  result = mmioCreateChunk(hFile, pckRIFF, MMIO_CREATERIFF);
  if (MMSYSERR_NOERROR != result) {
    ERR(L"mmioCreateChunk(\"RIFF/WAVE\") failed: MMRESULT = 0x%08x", result);
    return E_FAIL;
  }

  // make a 'fmt ' chunk (within the RIFF/WAVE chunk)
  MMCKINFO chunk;
  chunk.ckid = MAKEFOURCC('f', 'm', 't', ' ');
  result = mmioCreateChunk(hFile, &chunk, 0);
  if (MMSYSERR_NOERROR != result) {
    ERR(L"mmioCreateChunk(\"fmt \") failed: MMRESULT = 0x%08x", result);
    return E_FAIL;
  }

  // write the WAVEFORMATEX data to it
  LONG lBytesInWfx = sizeof(WAVEFORMATEX) + pwfx->cbSize;
  LONG lBytesWritten =
    mmioWrite(
      hFile,
      reinterpret_cast<PCHAR>(const_cast<LPWAVEFORMATEX>(pwfx)),
      lBytesInWfx
    );
  if (lBytesWritten != lBytesInWfx) {
    ERR(L"mmioWrite(fmt data) wrote %u bytes; expected %u bytes", lBytesWritten, lBytesInWfx);
    return E_FAIL;
  }

  // ascend from the 'fmt ' chunk
  result = mmioAscend(hFile, &chunk, 0);
  if (MMSYSERR_NOERROR != result) {
    ERR(L"mmioAscend(\"fmt \" failed: MMRESULT = 0x%08x", result);
    return E_FAIL;
  }

  // make a 'fact' chunk whose data is (DWORD)0
  chunk.ckid = MAKEFOURCC('f', 'a', 'c', 't');
  result = mmioCreateChunk(hFile, &chunk, 0);
  if (MMSYSERR_NOERROR != result) {
    ERR(L"mmioCreateChunk(\"fmt \") failed: MMRESULT = 0x%08x", result);
    return E_FAIL;
  }

  // write (DWORD)0 to it
  // this is cleaned up later
  DWORD frames = 0;
  lBytesWritten = mmioWrite(hFile, reinterpret_cast<PCHAR>(&frames), sizeof(frames));
  if (lBytesWritten != sizeof(frames)) {
    ERR(L"mmioWrite(fact data) wrote %u bytes; expected %u bytes", lBytesWritten, (UINT32)sizeof(frames));
    return E_FAIL;
  }

  // ascend from the 'fact' chunk
  result = mmioAscend(hFile, &chunk, 0);
  if (MMSYSERR_NOERROR != result) {
    ERR(L"mmioAscend(\"fact\" failed: MMRESULT = 0x%08x", result);
    return E_FAIL;
  }

  // make a 'data' chunk and leave the data pointer there
  pckData->ckid = MAKEFOURCC('d', 'a', 't', 'a');
  result = mmioCreateChunk(hFile, pckData, 0);
  if (MMSYSERR_NOERROR != result) {
    ERR(L"mmioCreateChunk(\"data\") failed: MMRESULT = 0x%08x", result);
    return E_FAIL;
  }

  return S_OK;
}

HRESULT FinishWaveFile(HMMIO hFile, MMCKINFO* pckRIFF, MMCKINFO* pckData) {
  MMRESULT result;

  result = mmioAscend(hFile, pckData, 0);
  if (MMSYSERR_NOERROR != result) {
    ERR(L"mmioAscend(\"data\" failed: MMRESULT = 0x%08x", result);
    return E_FAIL;
  }

  result = mmioAscend(hFile, pckRIFF, 0);
  if (MMSYSERR_NOERROR != result) {
    ERR(L"mmioAscend(\"RIFF/WAVE\" failed: MMRESULT = 0x%08x", result);
    return E_FAIL;
  }

  return S_OK;
}
