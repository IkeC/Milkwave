// loopback-capture.h

// forward declarations
class Milkwave;

// call CreateThread on this function
// feed it the address of a LoopbackCaptureThreadFunctionArguments
// it will capture via loopback from the IMMDevice
// and dump output to the HMMIO
// until the stop event is set
// any failures will be propagated back via hr

struct LoopbackCaptureThreadFunctionArguments {
  IMMDevice* pMMDevice;
  bool bIsRenderDevice; // true if this is a render device, false if capture
  bool bInt16;
  HMMIO hFile;
  HANDLE hStartedEvent;
  HANDLE hStopEvent;
  UINT32 nFrames;
  HRESULT hr;

  // pointer to milkwave instance for logging from the capture thread
  Milkwave* pMilkwave;
};

DWORD WINAPI LoopbackCaptureThreadFunction(LPVOID pContext);

// Add declaration for LoopbackCapture with Milkwave* parameter
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
