// pipe_server.cpp — Named Pipe IPC server for Milkwave visualizers
// See pipe_server.h for overview.

#include "pipe_server.h"
#include <process.h>   // _beginthreadex
#include <sddl.h>      // ConvertStringSecurityDescriptorToSecurityDescriptor
#include <tlhelp32.h>  // CreateToolhelp32Snapshot
#include <cstdlib>     // _wtoi, wcstof
#include <cstring>     // wcsncmp, wcsstr
#include <algorithm>   // std::find

// Self-contained logging — uses OutputDebugStringA (works in all projects)
#include <cstdio>  // vsnprintf
#include <cstdarg>

static void PipeLog(const char* fmt, ...) {
    char buf[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    OutputDebugStringA(buf);
}

// ─── PipeServer ────────────────────────────────────────────────────────────────

PipeServer::PipeServer() {}

PipeServer::~PipeServer() {
    Stop();
}

void PipeServer::Start(HWND hTargetWindow, UINT wmIPCMessage, UINT wmSignalBase) {
    if (m_bRunning.load())
        return;

    m_hTargetWindow = hTargetWindow;
    m_wmIPCMessage = wmIPCMessage;
    m_wmSignalBase = wmSignalBase;
    m_bShutdown.store(false);

    // Build pipe name: \\.\pipe\Milkwave_<PID>
    swprintf_s(m_szPipeName, L"\\\\.\\pipe\\Milkwave_%u", GetCurrentProcessId());

    // Create shutdown event (manual-reset, shared across all threads)
    m_hShutdownEvent = CreateEventW(NULL, TRUE, FALSE, NULL);

    // Start accept thread
    m_hServerThread = (HANDLE)_beginthreadex(
        nullptr, 0, &PipeServer::ServerThread, this, 0, nullptr);

    if (m_hServerThread) {
        m_bRunning.store(true);
        PipeLog("PipeServer: started on %ls\n", m_szPipeName);
    } else {
        PipeLog("PipeServer: failed to start thread\n");
    }
}

void PipeServer::Stop() {
    if (!m_bRunning.load())
        return;

    m_bShutdown.store(true);
    if (m_hShutdownEvent)
        SetEvent(m_hShutdownEvent);

    // Wake every handler: cancel its pending read, signal its out-event. Both
    // handles belong to a context, so this runs under the clients lock -- a
    // handler retires its own pipe under that same lock, and cancelling I/O on
    // a handle it had already closed would target whatever the process opened
    // next with the recycled value.
    {
        std::lock_guard<std::mutex> lock(m_clientsMutex);
        for (auto* ctx : m_clients) {
            if (ctx->hPipe != INVALID_HANDLE_VALUE)
                CancelIoEx(ctx->hPipe, NULL);
            if (ctx->hOutEvent)
                SetEvent(ctx->hOutEvent);
        }
    }

    // Join the accept thread FIRST. It owns SweepFinished, which deletes
    // contexts; retiring it is what makes the context pointers read below
    // stable enough to touch at all.
    if (m_hServerThread) {
        WaitForSingleObject(m_hServerThread, 5000);
        CloseHandle(m_hServerThread);
        m_hServerThread = nullptr;
    }

    // Join the handlers against ONE budget rather than 3s each -- with sixteen
    // listeners a single wedged client must not turn into a 48-second exit.
    std::vector<PipeClientContext*> active;
    {
        std::lock_guard<std::mutex> lock(m_clientsMutex);
        active = m_clients;
    }
    const ULONGLONG joinDeadline = GetTickCount64() + 3000;
    for (auto* ctx : active) {
        if (!ctx->hThread)
            continue;
        const ULONGLONG now = GetTickCount64();
        WaitForSingleObject(ctx->hThread,
                            now >= joinDeadline ? 0 : (DWORD)(joinDeadline - now));
    }

    // A handler that returned has already retired its pipe and moved itself to
    // m_finishedClients, so whatever is still in m_clients is a thread that
    // never came back. Its context is memory that thread may still be running
    // inside and its pipe is a handle that thread may still close: freeing the
    // one is a use-after-free and closing the other is a double-close. The
    // process is on its way out, so leak them on purpose.
    std::vector<PipeClientContext*> finished;
    size_t nOrphans = 0;
    {
        std::lock_guard<std::mutex> lock(m_clientsMutex);
        nOrphans = m_clients.size();
        m_clients.clear();
        finished.swap(m_finishedClients);
    }
    if (nOrphans)
        PipeLog("PipeServer: %zu handler(s) never exited; leaking their contexts\n", nOrphans);

    for (auto* ctx : finished) {
        // A context reaches the finished list a few instructions before its
        // thread actually returns, so this join is normally instant -- but its
        // result, not its expiry, is what says the context is ours to free.
        if (ctx->hThread && WaitForSingleObject(ctx->hThread, 1000) != WAIT_OBJECT_0) {
            PipeLog("PipeServer: client #%d still running; leaking its context\n",
                    ctx->nClientId);
            continue;
        }
        if (ctx->hThread) CloseHandle(ctx->hThread);
        if (ctx->hOutEvent) CloseHandle(ctx->hOutEvent);
        delete ctx;
    }

    if (m_hShutdownEvent) { CloseHandle(m_hShutdownEvent); m_hShutdownEvent = nullptr; }

    m_bRunning.store(false);
    m_bClientConnected.store(false);
    PipeLog("PipeServer: stopped\n");
}

void PipeServer::Send(const wchar_t* message) {
    if (!message || !*message || m_bShutdown.load())
        return;

    std::lock_guard<std::mutex> lock(m_clientsMutex);
    for (auto* ctx : m_clients) {
        {
            std::lock_guard<std::mutex> qlock(ctx->outMutex);
            ctx->outQueue.emplace(message);
        }
        SetEvent(ctx->hOutEvent);
    }
}

void PipeServer::Send(const std::wstring& message) {
    Send(message.c_str());
}

int PipeServer::GetClientCount() const {
    std::lock_guard<std::mutex> lock(m_clientsMutex);
    return (int)m_clients.size();
}

void PipeServer::RemoveClient(PipeClientContext* ctx) {
    std::lock_guard<std::mutex> lock(m_clientsMutex);

    // The pipe is retired here rather than at the end of ClientHandler so that
    // its handle is only ever touched while this lock is held. Stop() cancels
    // I/O on the same handle, and a closed handle value is immediately free for
    // any other CreateFile in the process to reuse.
    if (ctx->hPipe != INVALID_HANDLE_VALUE) {
        DisconnectNamedPipe(ctx->hPipe);
        CloseHandle(ctx->hPipe);
        ctx->hPipe = INVALID_HANDLE_VALUE;
    }

    auto it = std::find(m_clients.begin(), m_clients.end(), ctx);
    if (it != m_clients.end())
        m_clients.erase(it);
    m_finishedClients.push_back(ctx);
    m_bClientConnected.store(!m_clients.empty());
}

void PipeServer::SweepFinished() {
    // Take the list, then join outside the lock. A handler on its way out does
    // not need this mutex again, but waiting on a thread while holding a lock
    // that thread might want is the shape deadlocks come in.
    std::vector<PipeClientContext*> done;
    {
        std::lock_guard<std::mutex> lock(m_clientsMutex);
        done.swap(m_finishedClients);
    }

    std::vector<PipeClientContext*> stillRunning;
    for (auto* ctx : done) {
        // A context lands on the finished list a few instructions before its
        // thread returns, so this join almost always completes at once. When it
        // does not, the thread is still executing inside the context and
        // deleting it is a use-after-free -- so put it back and retry on the
        // next sweep. Ignoring the join result is what made this unsafe.
        if (ctx->hThread && WaitForSingleObject(ctx->hThread, 100) != WAIT_OBJECT_0) {
            stillRunning.push_back(ctx);
            continue;
        }
        if (ctx->hThread) CloseHandle(ctx->hThread);
        if (ctx->hOutEvent) CloseHandle(ctx->hOutEvent);
        delete ctx;
    }

    if (!stillRunning.empty()) {
        std::lock_guard<std::mutex> lock(m_clientsMutex);
        m_finishedClients.insert(m_finishedClients.end(),
                                 stillRunning.begin(), stillRunning.end());
    }
}

unsigned __stdcall PipeServer::ServerThread(void* pParam) {
    PipeServer* self = static_cast<PipeServer*>(pParam);
    self->ServerLoop();
    return 0;
}

unsigned __stdcall PipeServer::ClientThread(void* pParam) {
    PipeClientContext* ctx = static_cast<PipeClientContext*>(pParam);
    ctx->pServer->ClientHandler(ctx);
    return 0;
}

// ─── Accept loop ────────────────────────────────────────────────────────────────

void PipeServer::ServerLoop() {
    // Security: allow same-user access (handles admin/non-admin mismatch)
    SECURITY_ATTRIBUTES sa = {};
    PSECURITY_DESCRIPTOR pSD = nullptr;
    // SDDL: D:(A;;GA;;;WD) — grant all access to Everyone
    // This is safe because named pipes are local-only and the pipe name
    // includes the PID, so only someone who can enumerate processes can connect.
    if (ConvertStringSecurityDescriptorToSecurityDescriptorA(
            "D:(A;;GA;;;WD)", SDDL_REVISION_1, &pSD, NULL)) {
        sa.nLength = sizeof(sa);
        sa.lpSecurityDescriptor = pSD;
        sa.bInheritHandle = FALSE;
    }

    // Several listeners, armed at once (forgejo#25).
    //
    // A pipe is connectable only while an unconnected instance of it exists.
    // This loop used to keep exactly ONE, and created the next only after
    // building the client context, querying the client process image and
    // spawning a handler thread -- so every connection left a window of
    // milliseconds in which CreateFile on the pipe failed with
    // ERROR_PIPE_BUSY. Anything reconnecting per command hit it on almost
    // every attempt: 298 of 300 bare reconnects failed, measured.
    //
    // Creating the next instance earlier in the handoff is NOT enough, and
    // measuring said so -- it still leaves nothing listening between a client
    // closing and this thread next being scheduled. The fix is a small pool,
    // each member with a ConnectNamedPipe already pending, so a client finds a
    // free instance wherever this thread happens to be.
    //
    // A listener whose ConnectNamedPipe returns ERROR_PIPE_CONNECTED was
    // claimed between CreateNamedPipeW and ConnectNamedPipe. That is a normal
    // race, not an error: the connection is good, and there is no overlapped
    // result to collect afterwards -- hence the immediate flag.
    struct Listener {
        HANDLE     hPipe = INVALID_HANDLE_VALUE;
        HANDLE     hEvent = NULL;
        OVERLAPPED ov = {};
        bool       immediate = false;  // connected before we asked
    };

    auto createInstance = [&]() -> HANDLE {
        HANDLE h = CreateNamedPipeW(
            m_szPipeName,
            PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            65536,   // out buffer
            65536,   // in buffer
            0,       // default timeout
            pSD ? &sa : NULL);
        if (h == INVALID_HANDLE_VALUE)
            PipeLog("PipeServer: CreateNamedPipe failed, err=%u\n", GetLastError());
        return h;
    };

    // Create an instance and put a connect request on it.
    auto arm = [&](Listener& L) -> bool {
        L.hPipe = createInstance();
        L.immediate = false;
        if (L.hPipe == INVALID_HANDLE_VALUE)
            return false;

        ResetEvent(L.hEvent);
        ZeroMemory(&L.ov, sizeof(L.ov));
        L.ov.hEvent = L.hEvent;

        if (ConnectNamedPipe(L.hPipe, &L.ov)) {
            L.immediate = true;
            SetEvent(L.hEvent);
            return true;
        }

        const DWORD err = GetLastError();
        if (err == ERROR_IO_PENDING)
            return true;
        if (err == ERROR_PIPE_CONNECTED) {
            L.immediate = true;
            SetEvent(L.hEvent);
            return true;
        }

        PipeLog("PipeServer: ConnectNamedPipe failed, err=%u\n", err);
        CloseHandle(L.hPipe);
        L.hPipe = INVALID_HANDLE_VALUE;
        return false;
    };

    // Sixteen. The pool has to absorb two different bursts: a client
    // reconnecting in a tight loop while this thread is still handing the
    // previous one off, and several clients connecting at once -- Remote, the
    // MCP server and a test harness together already exceeded eight. The
    // buffer sizes above are quotas rather than upfront allocations, so an
    // idle instance costs a handle.
    const int kListeners = 16;
    Listener listeners[kListeners];
    for (int i = 0; i < kListeners; i++) {
        listeners[i].hEvent = CreateEventW(NULL, TRUE, FALSE, NULL);  // manual-reset
        arm(listeners[i]);
    }

    while (!m_bShutdown.load()) {
        // Clean up finished handler threads
        SweepFinished();

        // Wait on every armed listener at once, plus shutdown.
        HANDLE waits[kListeners + 1];
        int    which[kListeners];
        DWORD  n = 0;
        for (int i = 0; i < kListeners; i++) {
            if (listeners[i].hPipe == INVALID_HANDLE_VALUE)
                continue;
            which[n] = i;
            waits[n++] = listeners[i].hEvent;
        }

        if (n == 0) {
            // Nothing armed at all -- out of handles, or the name is unusable.
            // Back off rather than spin.
            Sleep(1000);
            for (int i = 0; i < kListeners; i++)
                arm(listeners[i]);
            continue;
        }

        waits[n] = m_hShutdownEvent;
        const DWORD w = WaitForMultipleObjects(n + 1, waits, FALSE, INFINITE);

        if (w == WAIT_OBJECT_0 + n || m_bShutdown.load())
            break;
        if (w < WAIT_OBJECT_0 || w >= WAIT_OBJECT_0 + n) {
            PipeLog("PipeServer: wait failed, err=%u\n", GetLastError());
            Sleep(100);
            continue;
        }

        Listener& L = listeners[which[w - WAIT_OBJECT_0]];

        if (!L.immediate) {
            DWORD bytesTransferred = 0;
            if (!GetOverlappedResult(L.hPipe, &L.ov, &bytesTransferred, FALSE)) {
                CloseHandle(L.hPipe);
                L.hPipe = INVALID_HANDLE_VALUE;
                arm(L);
                continue;
            }
        }

        // Take the connection and re-arm the slot at once, so the pool is back
        // to strength before any of the handoff work below runs.
        HANDLE hConnected = L.hPipe;
        L.hPipe = INVALID_HANDLE_VALUE;
        arm(L);

        // Client connected -- create context and spawn handler thread
        PipeClientContext* ctx = new PipeClientContext();
        ctx->hPipe = hConnected;
        ctx->pServer = this;
        ctx->hOutEvent = CreateEventW(NULL, FALSE, FALSE, NULL);  // auto-reset

        {
            std::lock_guard<std::mutex> lock(m_clientsMutex);
            ctx->nClientId = m_nNextClientId++;
            m_clients.push_back(ctx);
            m_bClientConnected.store(true);
        }

        // Capture client exe path
        ULONG clientPid = 0;
        if (GetNamedPipeClientProcessId(hConnected, &clientPid) && clientPid != 0) {
            HANDLE hProc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, clientPid);
            if (hProc) {
                DWORD pathLen = MAX_PATH;
                if (QueryFullProcessImageNameW(hProc, 0, ctx->szClientExePath, &pathLen)) {
                    // Update shared last-client path (most recent wins)
                    wcscpy_s(m_szLastClientExePath, ctx->szClientExePath);
                    PipeLog("PipeServer: client #%d exe: %ls\n", ctx->nClientId, ctx->szClientExePath);
                }
                CloseHandle(hProc);
            }
        }

        // Start handler thread
        ctx->hThread = (HANDLE)_beginthreadex(
            nullptr, 0, &PipeServer::ClientThread, ctx, 0, nullptr);

        if (!ctx->hThread) {
            PipeLog("PipeServer: failed to start handler for client #%d\n", ctx->nClientId);
            DisconnectNamedPipe(hConnected);
            CloseHandle(hConnected);
            CloseHandle(ctx->hOutEvent);
            {
                std::lock_guard<std::mutex> lock(m_clientsMutex);
                auto it = std::find(m_clients.begin(), m_clients.end(), ctx);
                if (it != m_clients.end()) m_clients.erase(it);
                m_bClientConnected.store(!m_clients.empty());
            }
            delete ctx;
            continue;
        }

        PipeLog("PipeServer: client #%d connected (total: %d)\n",
                ctx->nClientId, GetClientCount());
    }

    // Listeners still waiting when the loop ended belong to nobody.
    for (int i = 0; i < kListeners; i++) {
        if (listeners[i].hPipe != INVALID_HANDLE_VALUE) {
            CancelIoEx(listeners[i].hPipe, &listeners[i].ov);
            CloseHandle(listeners[i].hPipe);
        }
        if (listeners[i].hEvent)
            CloseHandle(listeners[i].hEvent);
    }

    if (pSD)
        LocalFree(pSD);
}

// ─── Per-client read/write loop ─────────────────────────────────────────────────

void PipeServer::ClientHandler(PipeClientContext* ctx) {
    OVERLAPPED ovRead = {};
    ovRead.hEvent = CreateEventW(NULL, TRUE, FALSE, NULL);

    wchar_t readBuf[32768];  // 64KB in wchars
    bool readPending = false;

    // A message longer than readBuf arrives as several reads, each but the last
    // reporting ERROR_MORE_DATA. Under PIPE_READMODE_MESSAGE the unread tail
    // stays queued, so dispatching the first chunk and discarding the rest does
    // not merely truncate this message -- the leftovers surface as the front of
    // the NEXT one. Assemble the whole thing before dispatching it.
    std::vector<wchar_t> assembled;
    bool overlong = false;

    // Every real command is a few dozen characters. This cap exists only so a
    // malformed sender cannot make the server allocate without bound; the
    // message is dropped and the connection stays usable.
    const size_t kMaxMessageChars = 1u << 22;  // 4M wchars

    auto absorb = [&](DWORD bytesRead, bool more) {
        const size_t n = bytesRead / sizeof(wchar_t);
        if (!overlong) {
            if (assembled.size() + n > kMaxMessageChars) {
                PipeLog("PipeServer: client #%d message over %zu chars, dropped\n",
                        ctx->nClientId, kMaxMessageChars);
                assembled.clear();
                overlong = true;
            } else {
                assembled.insert(assembled.end(), readBuf, readBuf + n);
            }
        }
        if (more)
            return;  // the rest of this message is still queued on the pipe
        if (!overlong && !assembled.empty()) {
            assembled.push_back(L'\0');
            DispatchMessage(assembled.data(), assembled.size() - 1);
        }
        assembled.clear();
        overlong = false;
    };

    while (!m_bShutdown.load()) {
        // Start an async read if not already pending
        if (!readPending) {
            ResetEvent(ovRead.hEvent);
            DWORD bytesRead = 0;
            BOOL ok = ReadFile(ctx->hPipe, readBuf, sizeof(readBuf), &bytesRead, &ovRead);
            if (ok) {
                absorb(bytesRead, false);  // completed immediately
            } else {
                DWORD err = GetLastError();
                if (err == ERROR_IO_PENDING) {
                    readPending = true;
                } else if (err == ERROR_MORE_DATA) {
                    // Complete, but only the head of the message fit. The count
                    // is not set on a failing return, so go and collect it.
                    if (!GetOverlappedResult(ctx->hPipe, &ovRead, &bytesRead, FALSE) &&
                        GetLastError() != ERROR_MORE_DATA)
                        break;
                    absorb(bytesRead, true);
                } else {
                    break;  // broken pipe, no data, or anything unexpected
                }
            }
        }

        // Wait on: read completion, outgoing data, or shutdown
        HANDLE waitHandles[] = { ovRead.hEvent, ctx->hOutEvent, m_hShutdownEvent };
        DWORD nHandles = readPending ? 3 : 2;
        DWORD waitIdx = WaitForMultipleObjects(
            nHandles, readPending ? waitHandles : waitHandles + 1,
            FALSE, 50);  // 50ms timeout for responsiveness

        if (m_bShutdown.load())
            break;

        // Check read completion
        if (readPending && waitIdx == WAIT_OBJECT_0) {
            DWORD bytesRead = 0;
            const BOOL got = GetOverlappedResult(ctx->hPipe, &ovRead, &bytesRead, FALSE);
            const DWORD err = got ? ERROR_SUCCESS : GetLastError();
            if (!got && err != ERROR_MORE_DATA) {
                // Broken pipe, cancelled I/O, or anything unexpected: end the
                // client. Leaving readPending set and looping is worse than a
                // lost message -- ovRead.hEvent is manual-reset and nothing
                // above resets it until the next read is armed, so the wait
                // returns instantly and this loop spins on a full core.
                break;
            }
            readPending = false;
            absorb(bytesRead, err == ERROR_MORE_DATA);
        }

        // Drain outgoing queue
        {
            std::lock_guard<std::mutex> lock(ctx->outMutex);
            while (!ctx->outQueue.empty() && !m_bShutdown.load()) {
                std::wstring& msg = ctx->outQueue.front();
                DWORD bytesWritten = 0;
                DWORD cbWrite = (DWORD)((msg.size() + 1) * sizeof(wchar_t));
                OVERLAPPED ovWrite = {};
                ovWrite.hEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
                BOOL ok = WriteFile(ctx->hPipe, msg.c_str(), cbWrite, &bytesWritten, &ovWrite);
                if (!ok) {
                    DWORD err = GetLastError();
                    if (err == ERROR_IO_PENDING) {
                        DWORD wr = WaitForSingleObject(ovWrite.hEvent, 1000);
                        if (wr == WAIT_OBJECT_0)
                            GetOverlappedResult(ctx->hPipe, &ovWrite, &bytesWritten, FALSE);
                    } else if (err == ERROR_BROKEN_PIPE || err == ERROR_NO_DATA) {
                        CloseHandle(ovWrite.hEvent);
                        ctx->outQueue.pop();
                        break;  // client disconnected during write
                    }
                }
                CloseHandle(ovWrite.hEvent);
                ctx->outQueue.pop();
            }
        }
    }

    // Cancel pending read
    if (readPending)
        CancelIoEx(ctx->hPipe, &ovRead);
    CloseHandle(ovRead.hEvent);

    // RemoveClient retires the pipe and publishes ctx to m_finishedClients,
    // where SweepFinished on the accept thread -- or Stop -- may delete it
    // before the next line runs. Nothing after it may dereference ctx, so take
    // what the log needs while the context is still ours.
    const int nClientId = ctx->nClientId;
    RemoveClient(ctx);
    PipeLog("PipeServer: client #%d disconnected\n", nClientId);
}

// ─── Message dispatch ───────────────────────────────────────────────────────────

void PipeServer::DispatchMessage(const wchar_t* message, size_t len) {
    if (!message || len == 0 || !m_hTargetWindow) {
        PipeLog("PipeServer::DispatchMessage: null/empty (msg=%p len=%zu hwnd=%p)\n",
                message, len, m_hTargetWindow);
        return;
    }

    // Length first: PipeLog formats into 512 bytes, so a long message used to
    // push the one field worth reading off the end of its own log line.
    PipeLog("PipeServer: received len=%zu [%ls]\n", len, message);

    // Check for SIGNAL| prefix — these map to PostMessage calls.
    //
    // A SIGNAL| the table does not carry is finished here. Falling through used
    // to post the raw "SIGNAL|..." string on as a generic IPC message, and
    // Engine::LaunchMessage matches the same prefix and calls DispatchSignal
    // again -- the identical failure, one heap copy and one window message
    // later. This is NOT the script-engine fallback that unrecognised keywords
    // depend on: LaunchMessage claims the SIGNAL| prefix and returns, so a bad
    // signal never reached ExecuteScriptLine in the first place.
    if (wcsncmp(message, L"SIGNAL|", 7) == 0) {
        if (!DispatchSignal(message + 7))
            PipeLog("PipeServer: unknown signal [%ls]\n", message + 7);
        return;
    }

    // Check for SPOUT_SENDER= prefix — maps to WM_MW_IPC_MESSAGE with dwData=WM_MW_SETSPOUTSENDER
    if (wcsncmp(message, L"SPOUT_SENDER=", 13) == 0) {
        const wchar_t* name = message + 13;
        size_t nameLen = wcslen(name);
        wchar_t* copy = (wchar_t*)malloc((nameLen + 1) * sizeof(wchar_t));
        if (copy) {
            wcscpy_s(copy, nameLen + 1, name);
            // Use the SETSPOUTSENDER constant as dwData (base + 108)
            // The engine handler checks for this in the IPC message
            UINT wmSetSpoutSender = m_wmSignalBase + 108;
            if (!PostMessageW(m_hTargetWindow, m_wmIPCMessage,
                              (WPARAM)wmSetSpoutSender, (LPARAM)copy)) {
                free(copy);
            }
        }
        return;
    }

    // All other messages: heap-copy and post as WM_MW_IPC_MESSAGE with dwData=1
    size_t msgLen = wcslen(message);
    wchar_t* copy = (wchar_t*)malloc((msgLen + 1) * sizeof(wchar_t));
    if (copy) {
        wcscpy_s(copy, msgLen + 1, message);
        if (!PostMessageW(m_hTargetWindow, m_wmIPCMessage, (WPARAM)1, (LPARAM)copy)) {
            PipeLog("PipeServer: PostMessage FAILED err=%u\n", GetLastError());
            free(copy);
        } else {
            PipeLog("PipeServer: posted IPC message to hwnd=%p\n", m_hTargetWindow);
        }
    }
}

// ─── Signal dispatch table ─────────────────────────────────────────────────────
struct SignalEntry {
    const wchar_t* name;
    int offset;       // added to m_wmSignalBase
    bool hasValue;    // true = uses KEY=VALUE format (wcsncmp + send parsed value)
    WPARAM wparam;    // wParam for a simple signal; ignored when hasValue
};

static const SignalEntry s_signalTable[] = {
    // Simple signals (exact match, no value)
    { L"NEXT_PRESET",          100, false, 0 },
    { L"PREV_PRESET",          101, false, 0 },
    { L"COVER_CHANGED",        102, false, 0 },
    { L"SPRITE_MODE",          103, false, 0 },
    { L"MESSAGE_MODE",         104, false, 0 },
    { L"CAPTURE",              105, false, 0 },
    { L"FULLSCREEN",           160, false, MW_FS_TOGGLE },
    { L"WATERMARK",            161, false, 0 },
    { L"BORDERLESS_FS",        162, false, 0 },
    // MDropDX12-only display signals (SHOW_COVER 110, STRETCH 163, MIRROR 164,
    // MIRROR_WM 165, MIRROR_INDEPENDENT 167, ALWAYS_ON_TOP 168) are omitted:
    // Milkwave's WndProc has no handler for those offsets.
    // KEY=VALUE signals (prefix match, value parsed as int into WPARAM)
    { L"SETVIDEODEVICE",       106, true , 0 },
    { L"ENABLEVIDEOMIX",       107, true , 0 },
    { L"ENABLESPOUTMIX",       109, true , 0 },
    { L"SET_INPUTMIX_OPACITY", 150, true , 0 },
    { L"SET_INPUTMIX_ONTOP",   152, true , 0 },
};

bool PipeServer::DispatchSignal(const wchar_t* signal) {
    if (!signal || !m_hTargetWindow)
        return false;

    // Parse: NEXT_PRESET, PREV_PRESET, etc.
    // Uses m_wmSignalBase (WM_APP for MDropDX12, WM_USER for Milkwave)
    for (const auto& entry : s_signalTable) {
        if (!entry.hasValue) {
            // Exact match for simple signals
            if (wcscmp(signal, entry.name) == 0) {
                PostMessageW(m_hTargetWindow, m_wmSignalBase + entry.offset,
                             entry.wparam, 0);
                return true;
            }
        } else {
            // Prefix match for KEY=VALUE signals
            size_t nameLen = wcslen(entry.name);
            if (wcsncmp(signal, entry.name, nameLen) == 0 && signal[nameLen] == L'=') {
                const wchar_t* value = signal + nameLen + 1;
                PostMessageW(m_hTargetWindow, m_wmSignalBase + entry.offset, (WPARAM)_wtoi(value), 0);
                return true;
            }
        }
    }

    // Special case: SET_INPUTMIX_LUMAKEY parses threshold|softness into WPARAM and LPARAM
    const wchar_t kLumaKey[] = L"SET_INPUTMIX_LUMAKEY";
    const size_t kLumaKeyLen = _countof(kLumaKey) - 1;
    if (wcsncmp(signal, kLumaKey, kLumaKeyLen) == 0 && signal[kLumaKeyLen] == L'=') {
        const wchar_t* value = signal + kLumaKeyLen + 1;
        int threshold = _wtoi(value);
        int softness = 0;
        const wchar_t* pipe = wcschr(value, L'|');
        if (pipe)
            softness = _wtoi(pipe + 1);
        PostMessageW(m_hTargetWindow, m_wmSignalBase + 151, (WPARAM)threshold, (LPARAM)softness);
        return true;
    }

    return false;  // unknown signal
}

// ─── Second-instance forwarding ────────────────────────────────────────────────

bool PipeSendToExistingInstance(const wchar_t* message) {
    if (!message || !*message)
        return false;

    // Enumerate processes to find other instances of the current exe
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE)
        return false;

    DWORD myPid = GetCurrentProcessId();

    // Get our exe filename (just the name, no path)
    wchar_t myExePath[MAX_PATH];
    GetModuleFileNameW(NULL, myExePath, MAX_PATH);
    const wchar_t* myExeName = wcsrchr(myExePath, L'\\');
    myExeName = myExeName ? myExeName + 1 : myExePath;

    PROCESSENTRY32W pe = {};
    pe.dwSize = sizeof(pe);
    bool sent = false;

    if (Process32FirstW(hSnap, &pe)) {
        do {
            if (pe.th32ProcessID == myPid)
                continue;
            if (_wcsicmp(pe.szExeFile, myExeName) != 0)
                continue;

            // Same FILENAME is not the same install. Distribution is a portable
            // zip, and running one copy of the folder per display is the
            // supported way to get a different preset on each screen -- so
            // several unrelated MDropDX12.exe processes is the normal case, not
            // an exotic one. Matching on the name alone forwarded a
            // double-clicked preset to whichever the process snapshot happened
            // to yield first, and then exited silently, so the preset opened on
            // a window the user was not looking at.
            //
            // Compare the full image path instead. If it cannot be read, skip:
            // forwarding to an instance that cannot be proven to be this same
            // install is the behaviour being fixed.
            {
                HANDLE hProc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,
                                           FALSE, pe.th32ProcessID);
                if (!hProc)
                    continue;
                wchar_t theirPath[MAX_PATH] = {};
                DWORD len = MAX_PATH;
                const BOOL got = QueryFullProcessImageNameW(hProc, 0, theirPath, &len);
                CloseHandle(hProc);
                if (!got || _wcsicmp(theirPath, myExePath) != 0)
                    continue;
            }

            // Found another instance — try to connect to its pipe
            wchar_t pipeName[64];
            swprintf_s(pipeName, L"\\\\.\\pipe\\Milkwave_%u", pe.th32ProcessID);

            HANDLE hPipe = CreateFileW(
                pipeName,
                GENERIC_READ | GENERIC_WRITE,
                0, NULL,
                OPEN_EXISTING,
                0, NULL);

            if (hPipe != INVALID_HANDLE_VALUE) {
                // Set message read mode
                DWORD mode = PIPE_READMODE_MESSAGE;
                SetNamedPipeHandleState(hPipe, &mode, NULL, NULL);

                DWORD cbWrite = (DWORD)((wcslen(message) + 1) * sizeof(wchar_t));
                DWORD written = 0;
                WriteFile(hPipe, message, cbWrite, &written, NULL);
                CloseHandle(hPipe);
                sent = true;
                break;  // sent to first found instance
            }
        } while (Process32NextW(hSnap, &pe));
    }

    CloseHandle(hSnap);
    return sent;
}
