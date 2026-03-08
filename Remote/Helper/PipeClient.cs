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
  private int _disconnected;

  /// <summary>Fired on the thread-pool when a message arrives from the visualizer.</summary>
  public event Action<string>? MessageReceived;

  /// <summary>Fired when the pipe disconnects unexpectedly.</summary>
  public event Action? Disconnected;

  public bool IsConnected => _pipe is { IsConnected: true };

  /// <summary>
  /// Discover a running MDropDX12 process and connect to its named pipe.
  /// </summary>
  /// <param name="exeName">Executable name to look for (e.g. "MDropDX12.exe").</param>
  /// <returns>True if connected successfully.</returns>
  public bool Connect(string exeName) {
    Disconnect();

    string baseName = Path.GetFileNameWithoutExtension(exeName);
    Process[] procs = Process.GetProcessesByName(baseName);
    if (procs.Length == 0)
      return false;

    foreach (var proc in procs) {
      string pipeName = $"Milkwave_{proc.Id}";
      try {
        var pipe = new NamedPipeClientStream(".", pipeName, PipeDirection.InOut, PipeOptions.Asynchronous);
        pipe.Connect(1000); // 1-second timeout
        pipe.ReadMode = PipeTransmissionMode.Message;

        lock (_lock) {
          _pipe = pipe;
        }

        Interlocked.Exchange(ref _disconnected, 0);
        _cts = new CancellationTokenSource();
        _readThread = new Thread(ReadLoop) { IsBackground = true, Name = "PipeClientReader" };
        _readThread.Start();

        return true;
      } catch {
        // Try next process instance
      }
    }
    return false;
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
    _readThread?.Join(2000);
    _readThread = null;
    _cts?.Dispose();
    _cts = null;
  }

  private void ReadLoop() {
    byte[] buffer = new byte[65536];
    CancellationToken ct = _cts?.Token ?? CancellationToken.None;
    try {
      while (!ct.IsCancellationRequested) {
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
    if (Interlocked.Exchange(ref _disconnected, 1) == 0)
      Disconnected?.Invoke();
  }

  public void Dispose() {
    if (_disposed) return;
    _disposed = true;
    Disconnect();
  }
}
