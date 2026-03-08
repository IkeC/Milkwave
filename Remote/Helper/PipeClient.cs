using System.Diagnostics;
using System.IO.Pipes;
using System.Text;

namespace MilkwaveRemote.Helper;

/// <summary>
/// Named pipe client for communicating with MDropDX12 visualizer.
/// Pipe name format: \\.\pipe\Milkwave_{PID}
/// Protocol: duplex message-mode, wide-char (UTF-16) null-terminated strings.
/// </summary>
internal class PipeClient : IDisposable {
  private NamedPipeClientStream? _pipe;
  private CancellationTokenSource? _cts;
  private Thread? _readThread;
  private readonly object _lock = new();
  private bool _disposed;

  /// <summary>Fired on the thread-pool when a message arrives from the visualizer.</summary>
  public event Action<string>? MessageReceived;

  /// <summary>Fired when the pipe disconnects unexpectedly.</summary>
  public event Action? Disconnected;

  public bool IsConnected => _pipe is { IsConnected: true };

  /// <summary>
  /// Discover running visualizer instances by scanning for Milkwave_* named pipes.
  /// Falls back to process name search if pipe enumeration fails.
  /// </summary>
  public static List<(int pid, string processName)> DiscoverVisualizers() {
    var results = new List<(int pid, string processName)>();
    try {
      foreach (string path in Directory.GetFiles(@"\\.\pipe\", "Milkwave_*")) {
        string name = Path.GetFileName(path);
        if (name.StartsWith("Milkwave_") && int.TryParse(name.Substring(9), out int pid)) {
          try {
            var proc = Process.GetProcessById(pid);
            results.Add((pid, proc.ProcessName));
          } catch {
            // Process no longer exists — stale pipe
          }
        }
      }
    } catch {
      // Pipe enumeration not available — fall back below
    }
    return results;
  }

  /// <summary>
  /// Discover a running visualizer and connect to its named pipe.
  /// Tries pipe enumeration first, then falls back to process name search.
  /// </summary>
  /// <param name="exeName">Executable name to look for (e.g. "MDropDX12.exe").</param>
  /// <returns>True if connected successfully.</returns>
  public bool Connect(string exeName) {
    Disconnect();

    string baseName = Path.GetFileNameWithoutExtension(exeName);

    // Try pipe enumeration first — works even if exe is renamed
    var visualizers = DiscoverVisualizers();
    foreach (var (pid, processName) in visualizers) {
      if (!processName.Equals(baseName, StringComparison.OrdinalIgnoreCase))
        continue;
      if (TryConnectToPipe($"Milkwave_{pid}"))
        return true;
    }

    // Fall back to process name search
    Process[] procs = Process.GetProcessesByName(baseName);
    foreach (var proc in procs) {
      if (TryConnectToPipe($"Milkwave_{proc.Id}"))
        return true;
    }
    return false;
  }

  private bool TryConnectToPipe(string pipeName) {
    try {
      var pipe = new NamedPipeClientStream(".", pipeName, PipeDirection.InOut, PipeOptions.Asynchronous);
      pipe.Connect(1000); // 1-second timeout
      pipe.ReadMode = PipeTransmissionMode.Message;

      lock (_lock) {
        _pipe = pipe;
      }

      _cts = new CancellationTokenSource();
      _readThread = new Thread(ReadLoop) { IsBackground = true, Name = "PipeClientReader" };
      _readThread.Start();

      return true;
    } catch {
      return false;
    }
  }

  /// <summary>Send a UTF-16 message to the visualizer.</summary>
  public bool Send(string message) {
    lock (_lock) {
      if (_pipe == null || !_pipe.IsConnected)
        return false;
      try {
        byte[] data = Encoding.Unicode.GetBytes(message + "\0");
        _pipe.Write(data, 0, data.Length);
        _pipe.Flush();
        return true;
      } catch {
        HandleDisconnect();
        return false;
      }
    }
  }

  public void Disconnect() {
    _cts?.Cancel();
    lock (_lock) {
      if (_pipe != null) {
        try { _pipe.Close(); } catch { }
        _pipe.Dispose();
        _pipe = null;
      }
    }
    _readThread = null;
    _cts?.Dispose();
    _cts = null;
  }

  private void ReadLoop() {
    byte[] buffer = new byte[65536];
    try {
      while (_cts != null && !_cts.IsCancellationRequested) {
        NamedPipeClientStream? pipe;
        lock (_lock) { pipe = _pipe; }
        if (pipe == null || !pipe.IsConnected)
          break;

        int bytesRead;
        try {
          bytesRead = pipe.Read(buffer, 0, buffer.Length);
        } catch {
          break;
        }
        if (bytesRead == 0)
          break;

        string msg = Encoding.Unicode.GetString(buffer, 0, bytesRead).TrimEnd('\0');
        if (msg.Length > 0)
          MessageReceived?.Invoke(msg);
      }
    } catch {
      // Shutting down
    }
    HandleDisconnect();
  }

  private void HandleDisconnect() {
    lock (_lock) {
      if (_pipe != null) {
        try { _pipe.Close(); } catch { }
        _pipe.Dispose();
        _pipe = null;
      }
    }
    Disconnected?.Invoke();
  }

  public void Dispose() {
    if (_disposed) return;
    _disposed = true;
    Disconnect();
  }
}
