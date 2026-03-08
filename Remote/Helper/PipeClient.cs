using System.Diagnostics;
using System.IO.Pipes;
using System.Runtime.InteropServices;
using System.Text;

namespace MilkwaveRemote.Helper {

  /// <summary>
  /// Named pipe client for communicating with Milkwave visualizers.
  /// Replaces WM_COPYDATA / EnumWindows / FindWindow IPC.
  /// Pipe name convention: \\.\pipe\Milkwave_{PID}
  /// </summary>
  public class PipeClient : IDisposable {
    [DllImport("kernel32.dll", SetLastError = true)]
    private static extern IntPtr OpenProcess(uint dwDesiredAccess, bool bInheritHandle, int dwProcessId);

    [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Unicode)]
    private static extern bool QueryFullProcessImageName(IntPtr hProcess, uint dwFlags, StringBuilder lpExeName, ref uint lpdwSize);

    [DllImport("kernel32.dll", SetLastError = true)]
    private static extern bool CloseHandle(IntPtr hObject);

    private const uint PROCESS_QUERY_LIMITED_INFORMATION = 0x1000;

    private NamedPipeClientStream? _pipe;
    private CancellationTokenSource? _cts;
    private Task? _readTask;
    private readonly object _writeLock = new();
    private int _connectedPid;

    /// <summary>Fires on the thread pool when a message is received from the visualizer.</summary>
    public event Action<string>? MessageReceived;

    /// <summary>Fires when the pipe disconnects.</summary>
    public event Action? Disconnected;

    public bool IsConnected => _pipe?.IsConnected == true;
    public int ConnectedPid => _connectedPid;

    /// <summary>
    /// Get the full exe path for a process by PID using QueryFullProcessImageName.
    /// Works cross-architecture (64-bit .NET querying 32-bit processes).
    /// </summary>
    private static string GetProcessExePath(int pid) {
      IntPtr hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, false, pid);
      if (hProcess == IntPtr.Zero)
        return "";
      try {
        var sb = new StringBuilder(1024);
        uint size = (uint)sb.Capacity;
        if (QueryFullProcessImageName(hProcess, 0, sb, ref size))
          return sb.ToString();
        return "";
      } finally {
        CloseHandle(hProcess);
      }
    }

    /// <summary>
    /// Discover all running visualizer instances that have a pipe server.
    /// Checks for existing Milkwave_* pipes without connecting (preserves the connection slot).
    /// Returns list of (PID, processName, exePath) tuples.
    /// </summary>
    public static List<(int pid, string name, string exePath)> DiscoverVisualizers() {
      var result = new List<(int, string, string)>();

      try {
        // Enumerate all active Milkwave_* pipes
        var pipeFiles = Directory.GetFiles(@"\\.\pipe\", "Milkwave_*");
        foreach (var pipePath in pipeFiles) {
          // Extract PID from pipe name (e.g. "\\.\pipe\Milkwave_12345" → 12345)
          string fileName = Path.GetFileName(pipePath);
          if (fileName.StartsWith("Milkwave_") &&
              int.TryParse(fileName.Substring("Milkwave_".Length), out int pid)) {
            try {
              var proc = Process.GetProcessById(pid);
              string exePath = GetProcessExePath(pid);
              result.Add((pid, proc.ProcessName, exePath));
            } catch {
              // Process no longer exists — stale pipe, skip
            }
          }
        }
      } catch {
        // Pipe enumeration failed — fall back to process scan
        var names = new[] { "MDropDX12", "MilkwaveVisualizer", "Milkdrop2PcmVisualizer" };
        foreach (var name in names) {
          try {
            foreach (var proc in Process.GetProcessesByName(name)) {
              string exePath = GetProcessExePath(proc.Id);
              result.Add((proc.Id, proc.ProcessName, exePath));
            }
          } catch { }
        }
      }

      return result;
    }

    /// <summary>
    /// Connect to a visualizer by PID.
    /// </summary>
    public bool Connect(int pid) {
      Disconnect();

      try {
        string pipeName = $"Milkwave_{pid}";
        _pipe = new NamedPipeClientStream(".", pipeName, PipeDirection.InOut, PipeOptions.Asynchronous);
        _pipe.Connect(2000); // 2 second timeout
        _pipe.ReadMode = PipeTransmissionMode.Message;
        _connectedPid = pid;

        // Start async read loop
        _cts = new CancellationTokenSource();
        _readTask = Task.Run(() => ReadLoop(_cts.Token));

        return true;
      } catch {
        _pipe?.Dispose();
        _pipe = null;
        _connectedPid = 0;
        return false;
      }
    }

    /// <summary>
    /// Send a text message to the visualizer via the pipe.
    /// Thread-safe.
    /// </summary>
    public bool Send(string message) {
      if (_pipe == null || !_pipe.IsConnected)
        return false;

      try {
        // UTF-16 encoding (wchar_t) with null terminator — matches C++ ReadFile expectations
        byte[] bytes = Encoding.Unicode.GetBytes(message + "\0");
        lock (_writeLock) {
          _pipe.Write(bytes, 0, bytes.Length);
          _pipe.Flush();
        }
        return true;
      } catch {
        return false;
      }
    }

    /// <summary>
    /// Send a SIGNAL message (replaces PostMessage for WM_USER+N signals).
    /// </summary>
    public bool SendSignal(string signalName) {
      return Send($"SIGNAL|{signalName}");
    }

    /// <summary>
    /// Send a Spout sender name (replaces WM_COPYDATA with dwData=WM_SETSPOUTSENDER).
    /// </summary>
    public bool SendSpoutSender(string senderName) {
      return Send($"SPOUT_SENDER={senderName}");
    }

    public void Disconnect() {
      _cts?.Cancel();

      try {
        _pipe?.Dispose();
      } catch { }

      _pipe = null;
      _connectedPid = 0;

      try {
        _readTask?.Wait(1000);
      } catch { }

      _readTask = null;
      _cts?.Dispose();
      _cts = null;
    }

    public void Dispose() {
      Disconnect();
      GC.SuppressFinalize(this);
    }

    private void ReadLoop(CancellationToken ct) {
      byte[] buffer = new byte[65536]; // 64KB buffer

      try {
        while (!ct.IsCancellationRequested && _pipe != null && _pipe.IsConnected) {
          int bytesRead = 0;
          try {
            bytesRead = _pipe.Read(buffer, 0, buffer.Length);
          } catch (IOException) {
            break; // pipe broken
          } catch (OperationCanceledException) {
            break;
          }

          if (bytesRead == 0)
            break; // pipe closed

          // Decode UTF-16 message (strip null terminator if present)
          string message = Encoding.Unicode.GetString(buffer, 0, bytesRead).TrimEnd('\0');
          if (!string.IsNullOrEmpty(message)) {
            try {
              MessageReceived?.Invoke(message);
            } catch {
              // Don't let handler exceptions kill the read loop
            }
          }
        }
      } catch {
        // Connection lost
      }

      Disconnected?.Invoke();
    }
  }
}
