// pipe_server.cpp — Named Pipe IPC server for Milkwave visualizers
// See pipe_server.h for overview.

#include "pipe_server.h"
#include <process.h>   // _beginthreadex
#include <sddl.h>      // ConvertStringSecurityDescriptorToSecurityDescriptor
#include <tlhelp32.h>  // CreateToolhelp32Snapshot
#include <cstdlib>     // _wtoi, wcstof
#include <cstring>     // wcsncmp, wcsstr

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

    // Create events
    m_hShutdownEvent = CreateEventW(NULL, TRUE, FALSE, NULL);  // manual-reset
    m_hOutEvent = CreateEventW(NULL, FALSE, FALSE, NULL);       // auto-reset

    // Start server thread
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
    if (m_hOutEvent)
        SetEvent(m_hOutEvent);

    // Cancel any pending I/O on the pipe
    if (m_hPipe != INVALID_HANDLE_VALUE)
        CancelIoEx(m_hPipe, NULL);

    if (m_hServerThread) {
        WaitForSingleObject(m_hServerThread, 5000);
        CloseHandle(m_hServerThread);
        m_hServerThread = nullptr;
    }

    if (m_hPipe != INVALID_HANDLE_VALUE) {
        DisconnectNamedPipe(m_hPipe);
        CloseHandle(m_hPipe);
        m_hPipe = INVALID_HANDLE_VALUE;
    }

    if (m_hShutdownEvent) { CloseHandle(m_hShutdownEvent); m_hShutdownEvent = nullptr; }
    if (m_hOutEvent) { CloseHandle(m_hOutEvent); m_hOutEvent = nullptr; }

    m_bRunning.store(false);
    m_bClientConnected.store(false);
    PipeLog("PipeServer: stopped\n");
}

void PipeServer::Send(const wchar_t* message) {
    if (!message || !*message || m_bShutdown.load())
        return;
    {
        std::lock_guard<std::mutex> lock(m_outMutex);
        m_outQueue.emplace(message);
    }
    if (m_hOutEvent)
        SetEvent(m_hOutEvent);
}

void PipeServer::Send(const std::wstring& message) {
    Send(message.c_str());
}

unsigned __stdcall PipeServer::ServerThread(void* pParam) {
    PipeServer* self = static_cast<PipeServer*>(pParam);
    self->ServerLoop();
    return 0;
}

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

    while (!m_bShutdown.load()) {
        // Create the pipe instance
        m_hPipe = CreateNamedPipeW(
            m_szPipeName,
            PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            1,       // max instances (single client)
            65536,   // out buffer
            65536,   // in buffer
            0,       // default timeout
            pSD ? &sa : NULL);

        if (m_hPipe == INVALID_HANDLE_VALUE) {
            PipeLog("PipeServer: CreateNamedPipe failed, err=%u\n", GetLastError());
            Sleep(1000);
            continue;
        }

        // Wait for client connection (overlapped so we can cancel on shutdown)
        OVERLAPPED ovConnect = {};
        ovConnect.hEvent = CreateEventW(NULL, TRUE, FALSE, NULL);

        BOOL connected = ConnectNamedPipe(m_hPipe, &ovConnect);
        if (!connected) {
            DWORD err = GetLastError();
            if (err == ERROR_IO_PENDING) {
                // Wait for either connection or shutdown
                HANDLE waitHandles[] = { ovConnect.hEvent, m_hShutdownEvent };
                DWORD waitResult = WaitForMultipleObjects(2, waitHandles, FALSE, INFINITE);

                if (waitResult == WAIT_OBJECT_0 + 1 || m_bShutdown.load()) {
                    // Shutdown requested
                    CancelIoEx(m_hPipe, &ovConnect);
                    CloseHandle(ovConnect.hEvent);
                    CloseHandle(m_hPipe);
                    m_hPipe = INVALID_HANDLE_VALUE;
                    break;
                }

                // Check if ConnectNamedPipe completed successfully
                DWORD bytesTransferred;
                if (!GetOverlappedResult(m_hPipe, &ovConnect, &bytesTransferred, FALSE)) {
                    CloseHandle(ovConnect.hEvent);
                    CloseHandle(m_hPipe);
                    m_hPipe = INVALID_HANDLE_VALUE;
                    continue;
                }
            } else if (err == ERROR_PIPE_CONNECTED) {
                // Client already connected before ConnectNamedPipe was called
            } else {
                // Real error
                CloseHandle(ovConnect.hEvent);
                CloseHandle(m_hPipe);
                m_hPipe = INVALID_HANDLE_VALUE;
                Sleep(100);
                continue;
            }
        }
        CloseHandle(ovConnect.hEvent);

        // Client is connected
        m_bClientConnected.store(true);
        PipeLog("PipeServer: client connected\n");

        // ─── Read/Write loop ───────────────────────────────────────────────
        // Single thread: alternate between checking for incoming and draining outgoing.
        // Use overlapped reads with short waits so we can also send.

        OVERLAPPED ovRead = {};
        ovRead.hEvent = CreateEventW(NULL, TRUE, FALSE, NULL);

        wchar_t readBuf[32768];  // 64KB in wchars
        bool readPending = false;

        while (!m_bShutdown.load()) {
            // Start an async read if not already pending
            if (!readPending) {
                ResetEvent(ovRead.hEvent);
                DWORD bytesRead = 0;
                BOOL ok = ReadFile(m_hPipe, readBuf, sizeof(readBuf) - sizeof(wchar_t),
                                   &bytesRead, &ovRead);
                if (ok) {
                    // Completed immediately
                    readBuf[bytesRead / sizeof(wchar_t)] = L'\0';
                    DispatchMessage(readBuf, bytesRead / sizeof(wchar_t));
                } else {
                    DWORD err = GetLastError();
                    if (err == ERROR_IO_PENDING) {
                        readPending = true;
                    } else if (err == ERROR_BROKEN_PIPE || err == ERROR_NO_DATA) {
                        break;  // client disconnected
                    } else {
                        break;  // unexpected error
                    }
                }
            }

            // Wait on: read completion, outgoing data, or shutdown
            HANDLE waitHandles[] = { ovRead.hEvent, m_hOutEvent, m_hShutdownEvent };
            DWORD nHandles = readPending ? 3 : 2;
            DWORD waitIdx = WaitForMultipleObjects(
                nHandles, readPending ? waitHandles : waitHandles + 1,
                FALSE, 50);  // 50ms timeout for responsiveness

            if (m_bShutdown.load())
                break;

            // Check read completion
            if (readPending) {
                if (waitIdx == WAIT_OBJECT_0) {
                    DWORD bytesRead = 0;
                    if (GetOverlappedResult(m_hPipe, &ovRead, &bytesRead, FALSE)) {
                        readBuf[bytesRead / sizeof(wchar_t)] = L'\0';
                        DispatchMessage(readBuf, bytesRead / sizeof(wchar_t));
                        readPending = false;
                    } else {
                        DWORD err = GetLastError();
                        if (err == ERROR_BROKEN_PIPE || err == ERROR_NO_DATA) {
                            break;
                        } else if (err == ERROR_MORE_DATA) {
                            // Message too large for buffer — dispatch what we have, discard rest
                            readBuf[bytesRead / sizeof(wchar_t)] = L'\0';
                            DispatchMessage(readBuf, bytesRead / sizeof(wchar_t));
                            readPending = false;
                        }
                    }
                }
            }

            // Drain outgoing queue
            {
                std::lock_guard<std::mutex> lock(m_outMutex);
                while (!m_outQueue.empty() && !m_bShutdown.load()) {
                    std::wstring& msg = m_outQueue.front();
                    DWORD bytesWritten = 0;
                    DWORD cbWrite = (DWORD)((msg.size() + 1) * sizeof(wchar_t));
                    OVERLAPPED ovWrite = {};
                    ovWrite.hEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
                    BOOL ok = WriteFile(m_hPipe, msg.c_str(), cbWrite, &bytesWritten, &ovWrite);
                    if (!ok && GetLastError() == ERROR_IO_PENDING) {
                        // Wait for write (with timeout)
                        DWORD wr = WaitForSingleObject(ovWrite.hEvent, 1000);
                        if (wr == WAIT_OBJECT_0)
                            GetOverlappedResult(m_hPipe, &ovWrite, &bytesWritten, FALSE);
                    }
                    CloseHandle(ovWrite.hEvent);
                    m_outQueue.pop();
                }
            }
        }

        // Cancel pending read
        if (readPending)
            CancelIoEx(m_hPipe, &ovRead);
        CloseHandle(ovRead.hEvent);

        // Disconnect and loop to accept next client
        m_bClientConnected.store(false);
        DisconnectNamedPipe(m_hPipe);
        CloseHandle(m_hPipe);
        m_hPipe = INVALID_HANDLE_VALUE;
        PipeLog("PipeServer: client disconnected\n");

        // Clear pending outgoing messages (stale for next client)
        {
            std::lock_guard<std::mutex> lock(m_outMutex);
            std::queue<std::wstring> empty;
            m_outQueue.swap(empty);
        }
    }

    if (pSD)
        LocalFree(pSD);
}

void PipeServer::DispatchMessage(const wchar_t* message, size_t len) {
    if (!message || len == 0 || !m_hTargetWindow)
        return;

    // Check for SIGNAL| prefix — these map to PostMessage calls
    if (wcsncmp(message, L"SIGNAL|", 7) == 0) {
        if (DispatchSignal(message + 7))
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
            free(copy);
        }
    }
}

bool PipeServer::DispatchSignal(const wchar_t* signal) {
    if (!signal || !m_hTargetWindow)
        return false;

    // Parse: NEXT_PRESET, PREV_PRESET, etc.
    // Uses m_wmSignalBase (WM_APP for MDropDX12, WM_USER for Milkwave)
    if (wcscmp(signal, L"NEXT_PRESET") == 0) {
        PostMessageW(m_hTargetWindow, m_wmSignalBase + 100, 0, 0);
        return true;
    }
    if (wcscmp(signal, L"PREV_PRESET") == 0) {
        PostMessageW(m_hTargetWindow, m_wmSignalBase + 101, 0, 0);
        return true;
    }
    if (wcscmp(signal, L"COVER_CHANGED") == 0) {
        PostMessageW(m_hTargetWindow, m_wmSignalBase + 102, 0, 0);
        return true;
    }
    if (wcscmp(signal, L"SPRITE_MODE") == 0) {
        PostMessageW(m_hTargetWindow, m_wmSignalBase + 103, 0, 0);
        return true;
    }
    if (wcscmp(signal, L"MESSAGE_MODE") == 0) {
        PostMessageW(m_hTargetWindow, m_wmSignalBase + 104, 0, 0);
        return true;
    }
    if (wcscmp(signal, L"CAPTURE") == 0) {
        PostMessageW(m_hTargetWindow, m_wmSignalBase + 105, 0, 0);
        return true;
    }

    // Signals with values: KEY=VALUE
    const wchar_t* eq = wcschr(signal, L'=');
    if (!eq)
        return false;

    // Extract key and value
    size_t keyLen = eq - signal;
    const wchar_t* value = eq + 1;

    if (wcsncmp(signal, L"ENABLESPOUTMIX", keyLen) == 0 && keyLen == 14) {
        PostMessageW(m_hTargetWindow, m_wmSignalBase + 109, (WPARAM)_wtoi(value), 0);
        return true;
    }
    if (wcsncmp(signal, L"SET_INPUTMIX_OPACITY", keyLen) == 0 && keyLen == 20) {
        PostMessageW(m_hTargetWindow, m_wmSignalBase + 150, (WPARAM)_wtoi(value), 0);
        return true;
    }
    if (wcsncmp(signal, L"SET_INPUTMIX_ONTOP", keyLen) == 0 && keyLen == 18) {
        PostMessageW(m_hTargetWindow, m_wmSignalBase + 152, (WPARAM)_wtoi(value), 0);
        return true;
    }
    if (wcsncmp(signal, L"SET_INPUTMIX_LUMAKEY", keyLen) == 0 && keyLen == 20) {
        // Value format: threshold|softness
        int threshold = _wtoi(value);
        int softness = 0;
        const wchar_t* pipe = wcschr(value, L'|');
        if (pipe)
            softness = _wtoi(pipe + 1);
        PostMessageW(m_hTargetWindow, m_wmSignalBase + 151, (WPARAM)threshold, (LPARAM)softness);
        return true;
    }
    if (wcsncmp(signal, L"ENABLEVIDEOMIX", keyLen) == 0 && keyLen == 14) {
        PostMessageW(m_hTargetWindow, m_wmSignalBase + 107, (WPARAM)_wtoi(value), 0);
        return true;
    }
    if (wcsncmp(signal, L"SETVIDEODEVICE", keyLen) == 0 && keyLen == 14) {
        PostMessageW(m_hTargetWindow, m_wmSignalBase + 106, (WPARAM)_wtoi(value), 0);
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
