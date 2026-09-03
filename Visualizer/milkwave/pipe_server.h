#pragma once
// pipe_server.h — Named Pipe IPC server for Milkwave visualizers
// Replaces the hidden WM_COPYDATA IPC window with a named pipe.
// Pipe name: \\.\pipe\Milkwave_<PID>
// Supports multiple concurrent clients (e.g., MilkwaveRemote + MCP).

#ifndef PIPE_SERVER_H
#define PIPE_SERVER_H

#include <windows.h>
#include <atomic>
#include <mutex>
#include <queue>
#include <string>
#include <vector>

// WM_MW_FULLSCREEN wParam convention (forgejo#23).
//
// The Displays tab needs absolute enter/exit, so 1 and 0 were given those
// meanings -- which silently redefined the 0 that every SIGNAL| and script
// caller was already sending. DispatchSignal can only send a wParam a table
// entry carries, so SIGNAL|FULLSCREEN meant "ensure exit" and could never
// enter fullscreen. The toggle needs a value of its own, and every sender has
// to name the same one, hence these live here rather than at each call site.
#define MW_FS_EXIT    0  // ensure exit, whatever the current state
#define MW_FS_ENTER   1  // ensure enter
#define MW_FS_TOGGLE  2  // toggle (SIGNAL|FULLSCREEN, the FULLSCREEN script command)

class PipeServer;

// Per-client connection context — one per connected client, owned by PipeServer.
struct PipeClientContext {
    HANDLE                    hPipe = INVALID_HANDLE_VALUE;
    HANDLE                    hThread = nullptr;
    HANDLE                    hOutEvent = nullptr;   // auto-reset, wakes handler for outgoing
    std::queue<std::wstring>  outQueue;
    std::mutex                outMutex;
    wchar_t                   szClientExePath[MAX_PATH] = {};
    PipeServer*               pServer = nullptr;     // back-pointer for dispatch/shutdown
    int                       nClientId = 0;         // monotonic ID for logging
};

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

    // Enqueue an outgoing message to all connected clients (fire-and-forget).
    void Send(const wchar_t* message);
    void Send(const std::wstring& message);

    bool IsRunning() const { return m_bRunning.load(); }
    bool IsConnected() const { return m_bClientConnected.load(); }
    int  GetClientCount() const;

    // Get the pipe name (for display in settings UI)
    const wchar_t* GetPipeName() const { return m_szPipeName; }

    // Get the full exe path of the last connected client (empty if never connected)
    const wchar_t* GetLastClientExePath() const { return m_szLastClientExePath; }

    // Parse SIGNAL| messages into PostMessage calls (public for LaunchMessage routing)
    bool DispatchSignal(const wchar_t* signal);

private:
    static unsigned __stdcall ServerThread(void* pParam);
    static unsigned __stdcall ClientThread(void* pParam);
    void ServerLoop();
    void ClientHandler(PipeClientContext* ctx);

    // Parse incoming pipe messages and dispatch to the target window
    void DispatchMessage(const wchar_t* message, size_t len);

    // Move client from active to finished list (called by handler on disconnect)
    void RemoveClient(PipeClientContext* ctx);

    // Join and delete finished client handler threads
    void SweepFinished();

    HWND   m_hTargetWindow = nullptr;
    UINT   m_wmIPCMessage = 0;      // WM_MW_IPC_MESSAGE equivalent
    UINT   m_wmSignalBase = 0x8000; // base for SIGNAL| dispatch (WM_APP or WM_USER)

    std::atomic<bool> m_bRunning{false};
    std::atomic<bool> m_bShutdown{false};
    std::atomic<bool> m_bClientConnected{false};

    // Client tracking (protected by m_clientsMutex)
    std::vector<PipeClientContext*> m_clients;          // active clients
    std::vector<PipeClientContext*> m_finishedClients;  // awaiting cleanup
    mutable std::mutex              m_clientsMutex;
    int                             m_nNextClientId = 0;

    // Shutdown event (manual-reset, shared across all threads)
    HANDLE m_hShutdownEvent = nullptr;

    HANDLE m_hServerThread = nullptr;

    wchar_t m_szPipeName[64] = {};
    wchar_t m_szLastClientExePath[MAX_PATH] = {};
};

// Connect to a running visualizer's pipe by PID and send a message.
// Used for second-instance forwarding (e.g., double-click .milk file).
// Returns true if message was sent successfully.
bool PipeSendToExistingInstance(const wchar_t* message);

#endif // PIPE_SERVER_H
