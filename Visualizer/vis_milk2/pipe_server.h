#pragma once
// pipe_server.h — Named Pipe IPC server for Milkwave visualizers
// Replaces the hidden WM_COPYDATA IPC window with a named pipe.
// Pipe name: \\.\pipe\Milkwave_<PID>

#ifndef PIPE_SERVER_H
#define PIPE_SERVER_H

#include <windows.h>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <string>

class PipeServer {
public:
    PipeServer();
    ~PipeServer();

    // Start the pipe server. hTargetWindow receives wmIPCMessage posts.
    // wmIPCMessage: the custom IPC message constant (WM_APP+9 in MDropDX12, WM_USER+200 in Milkwave).
    // wmSignalBase: base for SIGNAL| dispatch (WM_APP in MDropDX12, WM_USER in Milkwave).
    //   SIGNAL|NEXT_PRESET → PostMessage(wmSignalBase + 100), etc.
    void Start(HWND hTargetWindow, UINT wmIPCMessage, UINT wmSignalBase = 0x8000 /*WM_APP*/);
    void Stop();

    // Enqueue an outgoing message to the connected client (fire-and-forget).
    void Send(const wchar_t* message);
    void Send(const std::wstring& message);

    bool IsRunning() const { return m_bRunning.load(); }
    bool IsConnected() const { return m_bClientConnected.load(); }

    // Get the pipe name (for display in settings UI)
    const wchar_t* GetPipeName() const { return m_szPipeName; }

private:
    static unsigned __stdcall ServerThread(void* pParam);
    void ServerLoop();

    // Parse incoming pipe messages and dispatch to the target window
    void DispatchMessage(const wchar_t* message, size_t len);

    // Parse SIGNAL| messages into PostMessage calls
    bool DispatchSignal(const wchar_t* signal);

    HWND   m_hTargetWindow = nullptr;
    UINT   m_wmIPCMessage = 0;      // WM_MW_IPC_MESSAGE equivalent
    UINT   m_wmSignalBase = 0x8000; // base for SIGNAL| dispatch (WM_APP or WM_USER)
    HANDLE m_hPipe = INVALID_HANDLE_VALUE;

    std::atomic<bool> m_bRunning{false};
    std::atomic<bool> m_bShutdown{false};
    std::atomic<bool> m_bClientConnected{false};

    // Outgoing message queue
    std::queue<std::wstring> m_outQueue;
    std::mutex               m_outMutex;
    HANDLE                   m_hOutEvent = nullptr;   // signaled when queue has data

    // Shutdown event (cancels ConnectNamedPipe / ReadFile waits)
    HANDLE m_hShutdownEvent = nullptr;

    HANDLE m_hServerThread = nullptr;

    wchar_t m_szPipeName[64] = {};
};

// Connect to a running visualizer's pipe by PID and send a message.
// Used for second-instance forwarding (e.g., double-click .milk file).
// Returns true if message was sent successfully.
bool PipeSendToExistingInstance(const wchar_t* message);

#endif // PIPE_SERVER_H
