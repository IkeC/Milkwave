using MilkwaveRemote.Data;
using MilkwaveRemote.Helper;
using System.Diagnostics;
using System.Drawing.Text;
using System.Globalization;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.Json;
using static MilkwaveRemote.Helper.DarkModeCS;
using static MilkwaveRemote.Helper.RemoteHelper;

namespace MilkwaveRemote {
  public partial class MilkwaveRemoteForm : Form {
    [DllImport("user32.dll", SetLastError = true)]
    private static extern bool EnumWindows(EnumWindowsProc lpEnumFunc, IntPtr lParam);

    [DllImport("user32.dll", SetLastError = true)]
    private static extern int GetWindowText(IntPtr hWnd, StringBuilder lpString, int nMaxCount);

    [DllImport("user32.dll", SetLastError = true)]
    private static extern int GetWindowTextLength(IntPtr hWnd);

    [DllImport("user32.dll", SetLastError = true, CharSet = CharSet.Unicode)]
    private static extern IntPtr SendMessageW(IntPtr hWnd, uint Msg, IntPtr wParam, ref COPYDATASTRUCT lParam);

    [DllImport("user32.dll", SetLastError = true)]
    private static extern IntPtr PostMessage(IntPtr hWnd, uint Msg, IntPtr wParam, IntPtr lParam);

    [DllImport("user32.dll", SetLastError = true)]
    private static extern uint SendInput(uint nInputs, INPUT[] pInputs, int cbSize);

    [DllImport("user32.dll")]
    private static extern bool SetForegroundWindow(IntPtr hWnd);

    [DllImport("user32.dll")]
    private static extern IntPtr GetForegroundWindow();

    [DllImport("user32.dll", SetLastError = true)]
    private static extern bool GetWindowRect(IntPtr hWnd, out RECT lpRect);

    // Add the missing P/Invoke declaration for SetWindowPos
    [DllImport("user32.dll", SetLastError = true)]
    private static extern bool SetWindowPos(IntPtr hWnd, IntPtr hWndInsertAfter, int X, int Y, int cx, int cy, uint uFlags);

    // Add the missing constants for SWP_NOZORDER and SWP_NOACTIVATE
    private const uint SWP_NOZORDER = 0x0004;
    private const uint SWP_NOACTIVATE = 0x0010;

    private delegate bool EnumWindowsProc(IntPtr hWnd, IntPtr lParam);

    private const uint WM_COPYDATA = 0x004A;
    private const uint WM_KEYDOWN = 0x0100;

    private DarkModeCS dm;

    private System.Windows.Forms.Timer autoplayTimer;
    private int currentAutoplayIndex = 0;
    private int lastLineIndex = 0;
    private float autoplayRemainingBeats = 1;
    private long timerStart;
    private bool updatingWaveParams = false;

    private string VisualizerPresetsFolder = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "resources\\presets\\");

    private string lastScriptFileName = "script-default.txt";
    private string windowNotFound = "Milkwave Visualizer Window not found";
    private string foundWindowTitle = "";
    private string defaultFontName = "Segoe UI";

    private string milkwaveSettingsFile = "settings-remote.json";
    private string milkwaveTagsFile = "tags-remote.json";

    Random rnd = new Random();
    private Settings Settings = new Settings();
    private Tags Tags = new Tags();
    private RemoteHelper helper = new RemoteHelper();

    private OpenFileDialog ofd;
    private static readonly IntPtr HWND_TOPMOST = new IntPtr(-1);

    private const int VK_F1 = 0x70;
    private const int VK_F2 = 0x71;
    private const int VK_F3 = 0x72;
    private const int VK_F4 = 0x73;
    private const int VK_F5 = 0x74;
    private const int VK_F6 = 0x75;
    private const int VK_F7 = 0x76;
    private const int VK_F8 = 0x77;
    private const int VK_F9 = 0x78;
    private const int VK_F10 = 0x79;
    private const int VK_F11 = 0x7A;
    private const int VK_F12 = 0x7B;

    private const int VK_0 = 0x30;
    private const int VK_1 = 0x31;
    private const int VK_2 = 0x32;
    private const int VK_3 = 0x33;
    private const int VK_4 = 0x34;
    private const int VK_5 = 0x35;
    private const int VK_6 = 0x36;
    private const int VK_7 = 0x37;
    private const int VK_8 = 0x38;
    private const int VK_9 = 0x39;

    private const int VK_B = 0x42;
    private const int VK_K = 0x4B;
    private const int VK_N = 0x4E;

    private const int VK_SHIFT = 0x10;
    private const int VK_CTRL = 0x11;
    private const int VK_ALT = 0x12;

    private const int VK_SPACE = 0x20;
    private const int VK_DELETE = 0x2E;

    private const int VK_ENTER = 0x0D;
    private const int VK_BACKSPACE = 0x08;

    [StructLayout(LayoutKind.Sequential)]
    private struct COPYDATASTRUCT {
      public IntPtr dwData;
      public int cbData;
      public IntPtr lpData;
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct INPUT {
      public uint type;
      public InputUnion u;
    }

    [StructLayout(LayoutKind.Explicit)]
    private struct InputUnion {
      [FieldOffset(0)]
      public MOUSEINPUT mi;
      [FieldOffset(0)]
      public KEYBDINPUT ki;
      [FieldOffset(0)]
      public HARDWAREINPUT hi;
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct MOUSEINPUT {
      public int dx;
      public int dy;
      public uint mouseData;
      public uint dwFlags;
      public uint time;
      public IntPtr dwExtraInfo;
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct KEYBDINPUT {
      public ushort wVk;
      public ushort wScan;
      public uint dwFlags;
      public uint time;
      public IntPtr dwExtraInfo;
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct HARDWAREINPUT {
      public uint uMsg;
      public ushort wParamL;
      public ushort wParamH;
    }

    private enum MessageType {
      Direct,
      Message,
      PresetFilePath,
      PresetLink,
      Amplify,
      Wave,
      WaveClear,
      WaveQuickSave,
      AudioDevice,
      Opacity,
      GetState
    }
    private void SetAllControlFontSizes(Control parent, float fontSize)
    {
        foreach (Control ctrl in parent.Controls)
        {
            ctrl.Font = new Font(ctrl.Font.FontFamily, fontSize, ctrl.Font.Style);
            if (ctrl.HasChildren)
            {
                SetAllControlFontSizes(ctrl, fontSize);
            }
        }
    }
    public MilkwaveRemoteForm() {
      InitializeComponent();
      FixNumericUpDownMouseWheel(this);

      Assembly executingAssembly = Assembly.GetExecutingAssembly();
      var fieVersionInfo = FileVersionInfo.GetVersionInfo(executingAssembly.Location);
      var version = fieVersionInfo.FileVersion;
      toolStripMenuItemHomepage.Text = $"Milkwave {version}";

      ofd = new OpenFileDialog();

      try {
        string jsonString = File.ReadAllText(milkwaveSettingsFile);
        Settings? loadedSettings = JsonSerializer.Deserialize<Settings>(jsonString, new JsonSerializerOptions {
          PropertyNameCaseInsensitive = true
        });
        if (loadedSettings != null) {
          Settings = loadedSettings;
        }
        jsonString = File.ReadAllText(milkwaveTagsFile);
        Tags? loadedTags = JsonSerializer.Deserialize<Tags>(jsonString, new JsonSerializerOptions {
          PropertyNameCaseInsensitive = true
        });
        if (loadedTags != null) {
          Tags = loadedTags;
          SetTopTags();
        }
      } catch (Exception ex) {
        Settings = new Settings();
        Tags = new Tags();
      }

      dm = new DarkModeCS(this) {
        ColorMode = Settings.DarkMode ? DarkModeCS.DisplayMode.DarkMode : DarkModeCS.DisplayMode.ClearMode,
      };

      toolStripMenuItemDarkMode.Checked = Settings.DarkMode;
      SetBarIcon(Settings.DarkMode);

      if (Settings.Styles?.Count > 0) {
        ReloadStylesList();
      } else {
        cboParameters.Text = "size=25|time=5.0|x=0.5|y=0.5|growth=2";
      }

      // Fill cboFonts with available system fonts and add a blank first entry  
      cboFonts.Items.Add(""); // Add a blank first entry  
      using (InstalledFontCollection fontsCollection = new InstalledFontCollection()) {
        foreach (FontFamily font in fontsCollection.Families) {
          cboFonts.Items.Add(font.Name);
        }
        if (cboFonts.Items.Contains(defaultFontName)) {
          cboFonts.SelectedItem = defaultFontName;
        }
      }

      LoadMessages(lastScriptFileName);

      autoplayTimer = new System.Windows.Forms.Timer();
      autoplayTimer.Tick += AutoplayTimer_Tick;

      tabControl.SelectedIndex = Settings.SelectedTabIndex;
      cboWindowTitle.SelectedIndex = 0;

    }

    private void MilkwaveRemoteForm_Load(object sender, EventArgs e) {
      LoadAndSetSettings();
      SetPanelsVisibility();

      StartVisualizerIfNotFound();

      ofd = new OpenFileDialog();
      ofd.Filter = "MilkDrop Presets|*.milk;*.milk2|All files (*.*)|*.*";
      ofd.RestoreDirectory = true;
      SetAllControlFontSizes(this, 9f); // Sets all controls to font size 9
      helper.FillAudioDevices(cboAudioDevice);
    }

    private IntPtr StartVisualizerIfNotFound() {
      IntPtr result = FindVisualizerWindow();
      if (FindVisualizerWindow() == IntPtr.Zero) {
        // Try to run MilkwaveVisualizer.exe from the same directory as the assembly
        string visualizerPath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "MilkwaveVisualizer.exe");
        if (File.Exists(visualizerPath)) {
          Process.Start(new ProcessStartInfo(visualizerPath) { UseShellExecute = true });
        }
        int maxWait = 30; // 3 seconds
        while (result == IntPtr.Zero && maxWait > 0) {
          // Wait for the visualizer window to be found
          Thread.Sleep(100);
          result = FindVisualizerWindow();
          maxWait--;
        }
      }
      return result;
    }

    private void MainForm_Shown(object sender, EventArgs e) {
      btnSend.Focus();

      pnlColorMessage.BackColor = Color.FromArgb(200, 0, 200); // purple
      colorDialogMessage.Color = pnlColorMessage.BackColor;

      pnlColorWave.BackColor = Color.FromArgb(0, 200, 0); // dark green
      colorDialogWave.Color = pnlColorWave.BackColor;

      if (cboParameters.Items.Count > 0) {
        cboParameters.SelectedIndex = 0;
      }
      SetFormattedMessage();

      if (Directory.Exists(VisualizerPresetsFolder)) {
        ofd.InitialDirectory = VisualizerPresetsFolder;
      } else {
        ofd.InitialDirectory = AppDomain.CurrentDomain.BaseDirectory;
      }

      string MilkwavePresetsFolder = Path.Combine(VisualizerPresetsFolder, "Milkwave");
      if (Directory.Exists(MilkwavePresetsFolder)) {
        LoadPresetsFromDirectory(MilkwavePresetsFolder);
      }

      SendToMilkwaveVisualizer("", MessageType.GetState);
    }

    protected override void WndProc(ref Message m) {
      const int WM_COPYDATA = 0x004A;

      if (m.Msg == WM_COPYDATA) {
        // Extract the COPYDATASTRUCT from the message
        COPYDATASTRUCT cds = (COPYDATASTRUCT)Marshal.PtrToStructure(m.LParam, typeof(COPYDATASTRUCT))!;
        if (cds.lpData != IntPtr.Zero) {
          // Convert the received data to a string
          string receivedString = Marshal.PtrToStringUni(cds.lpData, cds.cbData / 2)?.TrimEnd('\0') ?? "";
          if (receivedString.StartsWith("WAVE|")) {

            string waveInfo = receivedString.Substring(receivedString.IndexOf("|") + 1);
            string[] waveParams = waveInfo.Split('|');
            updatingWaveParams = true;
            foreach (string param in waveParams) {
              string[] keyValue = param.Split('=');
              if (keyValue.Length == 2) {
                string key = keyValue[0].Trim();
                string value = keyValue[1].Trim();
                try {
                  if (key.Equals("MODE", StringComparison.OrdinalIgnoreCase)) {
                    numWaveMode.Value = int.Parse(value);
                  } else if (key.Equals("ALPHA", StringComparison.OrdinalIgnoreCase)) {
                    numWaveAlpha.Value = decimal.Parse(value, CultureInfo.InvariantCulture);
                  } else if (key.Equals("COLORR", StringComparison.OrdinalIgnoreCase)) {
                    numWaveR.Value = int.Parse(value);
                  } else if (key.Equals("COLORG", StringComparison.OrdinalIgnoreCase)) {
                    numWaveG.Value = int.Parse(value);
                  } else if (key.Equals("COLORB", StringComparison.OrdinalIgnoreCase)) {
                    numWaveB.Value = int.Parse(value);
                  } else if (key.Equals("PUSHX", StringComparison.OrdinalIgnoreCase)) {
                    numWavePushX.Value = decimal.Parse(value, CultureInfo.InvariantCulture);
                  } else if (key.Equals("PUSHY", StringComparison.OrdinalIgnoreCase)) {
                    numWavePushY.Value = decimal.Parse(value, CultureInfo.InvariantCulture);
                  } else if (key.Equals("ZOOM", StringComparison.OrdinalIgnoreCase)) {
                    numWaveZoom.Value = decimal.Parse(value, CultureInfo.InvariantCulture);
                  } else if (key.Equals("WARP", StringComparison.OrdinalIgnoreCase)) {
                    numWaveWarp.Value = decimal.Parse(value, CultureInfo.InvariantCulture);
                  } else if (key.Equals("ROTATION", StringComparison.OrdinalIgnoreCase)) {
                    numWaveRotation.Value = decimal.Parse(value, CultureInfo.InvariantCulture);
                  } else if (key.Equals("DECAY", StringComparison.OrdinalIgnoreCase)) {
                    numWaveDecay.Value = decimal.Parse(value, CultureInfo.InvariantCulture);
                  }
                } catch (Exception ex) {
                  // ignore
                }
              }
            }
            updatingWaveParams = false;
          } else if (receivedString.StartsWith("PRESET=")) {
            string presetFilePath = receivedString.Substring(receivedString.IndexOf("=") + 1);
            if (receivedString.Length > 0) {
              string findString = "RESOURCES\\PRESETS\\";
              int index = receivedString.IndexOf(findString, StringComparison.CurrentCultureIgnoreCase);
              string displayText = receivedString;
              if (index > -1) {
                displayText = receivedString.Substring(index + findString.Length);
                displayText = Path.ChangeExtension(displayText, null);
              }

              // Process the received string
              txtVisRunning.Text = displayText;
              toolTip1.SetToolTip(txtVisRunning, presetFilePath);
              UpdateTagsDisplay(false, true);
            }
          } else if (receivedString.StartsWith("STATUS=")) {
            string status = receivedString.Substring(receivedString.IndexOf("=") + 1);
            if (status.Length > 0) {
              SetStatusText(status);
            }
          } else if (receivedString.StartsWith("OPACITY=")) {
            string opacity = receivedString.Substring(receivedString.IndexOf("=") + 1);
            if (int.TryParse(opacity, out int parsedOpacity) && parsedOpacity >= 0 && parsedOpacity <= 100) {
              if (numOpacity.Value != parsedOpacity) {
                // Temporarily detach the event handler
                numOpacity.ValueChanged -= numOpacity_ValueChanged;
                numOpacity.Value = parsedOpacity;
                numOpacity.ValueChanged += numOpacity_ValueChanged;
              }
            }
          } else if (receivedString.StartsWith("DEVICE=")) {
            string device = receivedString.Substring(receivedString.IndexOf("=") + 1);
            helper.SelectDeviceByName(cboAudioDevice, device);
          } else if (receivedString.StartsWith("LINKCMD=")) {
            string cmd = receivedString.Substring(receivedString.IndexOf("=") + 1);
            if (cmd.ToUpper().Equals("NEXT")) {
              if (chkPresetRandom.Checked) {
                SelectRandomPreset();
              } else {
                SelectNextPreset();
              }
              btnPresetSend_Click(null, null);
            } else if (cmd.ToUpper().Equals("PREV")) {
              SelectPreviousPreset();
              btnPresetSend_Click(null, null);
            }
          }
        }
      }

      base.WndProc(ref m);
    }

    private nint FindVisualizerWindow() {
      IntPtr foundWindow = IntPtr.Zero;
      EnumWindows((hWnd, lParam) => {
        int length = GetWindowTextLength(hWnd);
        if (length == 0) return true;

        StringBuilder windowTitle = new StringBuilder(length + 1);
        GetWindowText(hWnd, windowTitle, windowTitle.Capacity);

        if (windowTitle.ToString().Equals(cboWindowTitle.Text, StringComparison.InvariantCultureIgnoreCase)) {
          foundWindow = hWnd;
          foundWindowTitle = windowTitle.ToString();
          return false; // Stop enumeration
        }

        return true; // Continue enumeration
      }, IntPtr.Zero);
      return foundWindow;
    }

    private void btnSend_Click(object sender, EventArgs e) {

      if ((Control.ModifierKeys & Keys.Control) == Keys.Control) {
        SendToMilkwaveVisualizer(txtMessage.Text, MessageType.Direct);
      } else {
        SendToMilkwaveVisualizer(txtMessage.Text, MessageType.Message);
      }

      txtMessage.Focus();
      txtMessage.SelectAll();
    }

    private void SendToMilkwaveVisualizer(string messageToSend, MessageType type) {
      SetStatusText("");
      string partialTitle = cboWindowTitle.Text;
      string statusMessage = "";
      IntPtr foundWindow = FindVisualizerWindow();
      if (foundWindow != IntPtr.Zero) {
        string message = "";
        if (type == MessageType.Direct) {
          message = messageToSend;
          statusMessage = $"Sent '{messageToSend}' to";
        } else if (type == MessageType.Wave) {
          message = "WAVE" +
            "|MODE=" + numWaveMode.Value +
            "|ALPHA=" + numWaveAlpha.Value.ToString(CultureInfo.InvariantCulture) +
            "|COLORR=" + pnlColorWave.BackColor.R +
            "|COLORG=" + pnlColorWave.BackColor.G +
            "|COLORB=" + pnlColorWave.BackColor.B +
            "|PUSHX=" + numWavePushX.Value.ToString(CultureInfo.InvariantCulture) +
            "|PUSHY=" + numWavePushY.Value.ToString(CultureInfo.InvariantCulture) +
            "|ZOOM=" + numWaveZoom.Value.ToString(CultureInfo.InvariantCulture) +
            "|WARP=" + numWaveWarp.Value.ToString(CultureInfo.InvariantCulture) +
            "|ROTATION=" + numWaveRotation.Value.ToString(CultureInfo.InvariantCulture) +
            "|DECAY=" + numWaveDecay.Value.ToString(CultureInfo.InvariantCulture);
          statusMessage = $"Changed Wave in";
        } else if (type == MessageType.PresetFilePath) {
          message = "PRESET=" + messageToSend;
          string fileName = Path.GetFileNameWithoutExtension(messageToSend);
          statusMessage = $"Sent preset {fileName} to";
        } else if (type == MessageType.Amplify) {
          message = "AMP" +
            "|l=" + numAmpLeft.Value.ToString(CultureInfo.InvariantCulture) +
            "|r=" + numAmpRight.Value.ToString(CultureInfo.InvariantCulture);
          statusMessage = $"Sent amplification {numAmpLeft.Value.ToString(CultureInfo.InvariantCulture)}" +
            $"/{numAmpRight.Value.ToString(CultureInfo.InvariantCulture)} to";
        } else if (type == MessageType.AudioDevice) {
          if (cboAudioDevice.Text.Length > 0) {
            ComboBoxItemDevice? selectedItem = (ComboBoxItemDevice?)cboAudioDevice.SelectedItem;
            if (selectedItem != null) {
              message = "DEVICE=" + selectedItem.Device.FriendlyName;
              statusMessage = $"Set device '{cboAudioDevice.Text}' in";
            }
          }
        } else if (type == MessageType.Opacity) {
          decimal val = numOpacity.Value / 100;
          message = "OPACITY=" + val.ToString(CultureInfo.InvariantCulture);
        } else if (type == MessageType.GetState) {
          message = "STATE";
        } else if (type == MessageType.WaveClear) {
          message = "CLEAR";
        } else if (type == MessageType.WaveQuickSave) {
          message = "QUICKSAVE";
        } else if (type == MessageType.PresetLink) {
          message = "LINK=" + messageToSend;
        } else if (type == MessageType.Message) {
          if (chkWrap.Checked && messageToSend.Length >= numWrap.Value && !messageToSend.Contains("//") && !messageToSend.Contains(Environment.NewLine)) {
            // try auto-wrap
            if (chkWrap.Checked && !message.Contains("//") && !message.Contains(Environment.NewLine)) {
              // Find the whitespace character closest to the middle of messageToSend
              int middleIndex = messageToSend.Length / 2;
              int closestWhitespaceIndex = messageToSend.LastIndexOf(' ', middleIndex);
              if (closestWhitespaceIndex == -1) {
                closestWhitespaceIndex = messageToSend.IndexOf(' ', middleIndex);
              }
              // Replace the closest whitespace with a newline placeholder
              if (closestWhitespaceIndex != -1) {
                messageToSend = messageToSend.Remove(closestWhitespaceIndex, 1).Insert(closestWhitespaceIndex, "//");
              }
            }
          }

          // hard limit is 507 characters
          if (messageToSend.Length > 500) {
            messageToSend = messageToSend.Substring(0, 500);
          }

          message = "MSG" +
            "|text=" + messageToSend;
          if (cboParameters.Text.Length > 0) {
            message += "|" + cboParameters.Text;
          }
          if (!message.Contains("font=")) {
            message += "|font=" + cboFonts.Text;
          }
          if (!message.Contains("r=") && !message.Contains("g=") && !message.Contains("b=")) {
            message += "|r=" + pnlColorMessage.BackColor.R;
            message += "|g=" + pnlColorMessage.BackColor.G;
            message += "|b=" + pnlColorMessage.BackColor.B;
          }
          if (!message.Contains("size=")) {
            message += "|size=" + numSize.Value;
          }

          message = message
            .Replace(" //", "//")
            .Replace("// ", "//")
            .Replace("//", " " + Environment.NewLine + " ");

          if (message.Contains(Environment.NewLine)) {
            string size = GetParam("size", message);
            if (size.Length > 0) {
              int newSize = (int)(int.Parse(size) * 1.8);
              message = message.Replace("size=" + size, "size=" + newSize);
            }
          }

          statusMessage = $"Sent '{messageToSend}' to";
        }

        byte[] messageBytes = Encoding.Unicode.GetBytes(message);
        IntPtr messagePtr = Marshal.AllocHGlobal(messageBytes.Length);
        Marshal.Copy(messageBytes, 0, messagePtr, messageBytes.Length);

        COPYDATASTRUCT cds = new COPYDATASTRUCT {
          dwData = 1,
          cbData = messageBytes.Length,
          lpData = messagePtr
        };

        SendMessageW(foundWindow, WM_COPYDATA, IntPtr.Zero, ref cds);
        if (statusMessage.Length > 0) {
          SetStatusText($"{statusMessage} {foundWindowTitle}");
        }

        Marshal.FreeHGlobal(messagePtr);

      } else {
        SetStatusText(windowNotFound);
      }
    }

    private void SetStatusText(string text) {
      text = text
        .Replace(" " + Environment.NewLine, Environment.NewLine)
        .Replace(Environment.NewLine + " ", Environment.NewLine)
        .Replace(Environment.NewLine, " // ").Trim();
      if (!text.Equals(statusBar.Text)) {
        statusBar.Text = text;
      }
    }

    private void btnSaveParam_Click(object sender, EventArgs e) {
      if (txtStyle.Text.Length == 0) {
        txtStyle.Text = "Style A";
      }

      var newPreset = new Style {
        Name = txtStyle.Text,
        Value = cboParameters.Text
      };

      int index = Settings.Styles.FindIndex(item => item.Name == newPreset.Name);
      if (index >= 0) {
        Settings.Styles[index] = newPreset;
      } else {
        Settings.Styles.Add(newPreset);
      }

      ReloadStylesList();

      SetStatusText($"Saved preset '{txtStyle.Text}'");
    }

    private void chkAutoplay_CheckedChanged(object sender, EventArgs e) {
      if (chkAutoplay.Checked) {
        ResetAndStartTimer(true);
      } else {
        autoplayTimer.Stop();
        SetStatusText("");
        autoplayRemainingBeats = 0;
      }
    }

    private void ResetAndStartTimer(bool startInstant) {
      if (float.TryParse(numBeats.Text, out float interval)) {
        autoplayRemainingBeats = 0;
        setTimerInterval();
        if (startInstant) {
          AutoplayTimer_Tick(null, null);
        }
        autoplayTimer.Start();
        timerStart = DateTime.Now.Ticks;
      } else {
        SetStatusText("Invalid wait value");
      }
    }

    private void setTimerInterval() {
      if (autoplayTimer != null) {
        float bpm = 120;
        try {
          bpm = float.Parse(numBPM.Text);
        } catch (Exception) {
          bpm = 120;
        }
        autoplayTimer.Interval = (int)((float)60 / bpm * 1000) - 15; // Timer inaccuracy compensation
      }
    }

    private void AutoplayTimer_Tick(object? sender, EventArgs? e) {
      SendAutoplayLine(false);
    }

    private void SendAutoplayLine(bool manualSend) {

      if (cboAutoplay.Items?.Count > 0) {
        if (autoplayRemainingBeats == 0 || manualSend) {
          string[] strings = cboAutoplay.Text.Split('|');
          foreach (string s in strings) {
            string token = s.Trim();
            string tokenUpper = token.ToUpper();
            if (tokenUpper.Equals("NEXT")) {
              btnSpace.PerformClick();
              Thread.Sleep(100);
            } else if (tokenUpper.Equals("PREV")) {
              btnBackspace.PerformClick();
            } else if (tokenUpper.Equals("STOP")) {
              chkAutoplay.CheckState = CheckState.Unchecked;
            } else if (tokenUpper.Equals("RESET")) {
              ResetAndStartTimer(false);
            } else if (tokenUpper.StartsWith("BPM=")) {
              string BPM = tokenUpper.Substring(4);
              if (int.TryParse(BPM, out int bpm)) {
                numBPM.Text = BPM;
              }
            } else if (tokenUpper.StartsWith("BEATS=")) {
              string beats = tokenUpper.Substring(6);
              if (int.TryParse(beats, out int b)) {
                numBeats.Text = beats;
              }
            } else if (tokenUpper.StartsWith("FONT=")) {
              string font = token.Substring(5);
              cboFonts.Text = font;
            } else if (tokenUpper.StartsWith("STYLE=")) {
              string preset = tokenUpper.Substring(6);
              var foundItem = from item in cboParameters.Items.Cast<Style>()
                              where item.Name.ToUpper() == preset
                              select item;
              if (foundItem != null && foundItem.Any()) {
                cboParameters.SelectedItem = foundItem.First();
              } else {
                SetStatusText($"Style '{preset}' not found");
              }
            } else if (tokenUpper.StartsWith("PRESET=")) {
              string presetFilePath = token.Substring(7);
              if (!File.Exists(presetFilePath)) {
                presetFilePath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, presetFilePath);
              }
              if (File.Exists(presetFilePath)) {
                SendToMilkwaveVisualizer(presetFilePath, MessageType.PresetFilePath);
              }
            } else if (tokenUpper.StartsWith("FILE=")) {
              string fileName = token.Substring(5);
              if (fileName.Length > 0) {
                LoadMessages(fileName);
                lastScriptFileName = fileName;
                if (!manualSend) {
                  SetStatusText($"Next line in {autoplayRemainingBeats} beats");
                }
              }
            } else if (tokenUpper.StartsWith("SEND=")) {
              string sendString = token.Substring(5);
              if (sendString.Length > 0) {
                SendUnicodeChars(sendString);
              }
            } else { // no known command, send as message
              SendToMilkwaveVisualizer(token, MessageType.Message);
            }
          }

          if (!manualSend) {
            try {
              autoplayRemainingBeats = int.Parse(numBeats.Text) - 1;
            } catch (Exception) {
              autoplayRemainingBeats = 1;
            }

            if (autoplayRemainingBeats < 1) {
              autoplayRemainingBeats = 1;
            }
            lastLineIndex = currentAutoplayIndex;

            if (chkFileRandom.Checked && cboAutoplay.Items?.Count > 1) {
              while (currentAutoplayIndex == lastLineIndex) {
                currentAutoplayIndex = rnd.Next(0, cboAutoplay.Items.Count);
                cboAutoplay.SelectedIndex = currentAutoplayIndex;
              }
            } else {
              if (cboAutoplay.Items?.Count > 0) {
                currentAutoplayIndex = (int)((currentAutoplayIndex + 1) % cboAutoplay.Items.Count);
                cboAutoplay.SelectedIndex = currentAutoplayIndex;
              }
            }
          }

        } else if (!manualSend) {
          // SelectNextAutoplayEntry();
          SetStatusText($"Next line in {autoplayRemainingBeats} beats");
          autoplayRemainingBeats--;
        }
      }
    }

    private void SelectNextAutoplayEntry() {
      if (cboAutoplay.Items.Count > 0) {
        // Move to the next item or loop back to the first
        if (cboAutoplay.SelectedIndex < cboAutoplay.Items.Count - 1) {
          cboAutoplay.SelectedIndex++;
        } else {
          cboAutoplay.SelectedIndex = 0;
        }
      }
    }

    private void btnFontAppend_Click(object sender, EventArgs e) {
      RemoveParam("font");
      AppendParam("font=" + cboFonts.Text);
    }

    private void pnlColorMessage_Click(object sender, EventArgs e) {
      if (colorDialogMessage.ShowDialog() == DialogResult.OK) {
        pnlColorMessage.BackColor = colorDialogMessage.Color;
        SetFormattedMessage();
      }
    }

    private void pnlColorWave_Click(object sender, EventArgs e) {
      if (colorDialogWave.ShowDialog() == DialogResult.OK) {
        pnlColorWave.BackColor = colorDialogWave.Color;
        // detach event handlers
        numWaveR.ValueChanged -= numWaveR_ValueChanged;
        numWaveG.ValueChanged -= numWaveG_ValueChanged;
        numWaveB.ValueChanged -= numWaveB_ValueChanged;
        numWaveR.Value = colorDialogWave.Color.R;
        numWaveG.Value = colorDialogWave.Color.G;
        numWaveB.Value = colorDialogWave.Color.B;
        numWaveR.ValueChanged += numWaveR_ValueChanged;
        numWaveG.ValueChanged += numWaveG_ValueChanged;
        numWaveB.ValueChanged += numWaveB_ValueChanged;
        SendWaveInfoIfLinked();
      }
    }

    private void btnAppendColor_Click(object sender, EventArgs e) {
      RemoveParam("r");
      RemoveParam("g");
      RemoveParam("b");
      AppendParam("r=" + pnlColorMessage.BackColor.R);
      AppendParam("g=" + pnlColorMessage.BackColor.G);
      AppendParam("b=" + pnlColorMessage.BackColor.B);
    }

    private void LoadMessages(string fileName) {
      currentAutoplayIndex = 0;
      cboAutoplay.Items.Clear();
      string filePath = fileName;
      if (!fileName.Contains("\\")) {
        filePath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, fileName);
      }
      if (File.Exists(filePath)) {
        string[] strings = File.ReadAllLines(filePath);
        foreach (string line in strings) {
          if (!line.StartsWith("#")) {
            cboAutoplay.Items.Add(line);
          }
        }
      }

      if (cboAutoplay.Items.Count > 0) {
        cboAutoplay.SelectedIndex = 0;
        if (cboAutoplay.Items.Count > 1 && chkFileRandom.Checked) {
          currentAutoplayIndex = rnd.Next(0, cboAutoplay.Items.Count);
          try {
            cboAutoplay.SelectedIndex = currentAutoplayIndex;
          } catch (Exception) {
            // ignore
          }
          toolTip1.SetToolTip(cboAutoplay, "Line from file " + fileName);
          chkAutoplay.Enabled = true;
        }
      } else {
        if (txtAutoplay != null) {
          txtAutoplay.Text = "No messages in " + fileName;
          chkAutoplay.Enabled = false;
        }
      }
    }

    private void SendPostMessage(int VKKey, string keyName) {
      IntPtr foundWindow = FindVisualizerWindow();

      if (foundWindow != IntPtr.Zero) {
        PostMessage(foundWindow, WM_KEYDOWN, (IntPtr)VKKey, IntPtr.Zero);
        SetStatusText($"Pressed {keyName} in '{foundWindowTitle}'");
      } else {
        SetStatusText(windowNotFound);
      }
    }

    private void SendInputTwoKeys(int VKKey, int VKKey2, string keyName) {
      IntPtr currentWindow = GetForegroundWindow();
      IntPtr foundWindow = FindVisualizerWindow();
      if (foundWindow != IntPtr.Zero) {
        SetForegroundWindow(foundWindow);

        INPUT[] inputs;
        inputs = new INPUT[4];

        inputs[0] = new INPUT {
          type = 1, // Keyboard input
          u = new InputUnion {
            ki = new KEYBDINPUT {
              wVk = (ushort)VKKey,
              dwFlags = 0 // Key down
            }
          }
        };

        inputs[1] = new INPUT {
          type = 1, // Keyboard input
          u = new InputUnion {
            ki = new KEYBDINPUT {
              wVk = (ushort)VKKey,
              dwFlags = 2 // Key up
            }
          }
        };

        inputs[2] = new INPUT {
          type = 1, // Keyboard input
          u = new InputUnion {
            ki = new KEYBDINPUT {
              wVk = (ushort)VKKey2,
              dwFlags = 0 // Key down
            }
          }
        };

        inputs[3] = new INPUT {
          type = 1, // Keyboard input
          u = new InputUnion {
            ki = new KEYBDINPUT {
              wVk = (ushort)VKKey2,
              dwFlags = 2 // Key up
            }
          }
        };

        SendInput((uint)inputs.Length, inputs, Marshal.SizeOf(typeof(INPUT)));
        SetStatusText($"Pressed {keyName} in '{foundWindowTitle}'");

        SetForegroundWindow(currentWindow);
      } else {
        SetStatusText(windowNotFound);
      }
    }

    private void SendInput(int VKKey, string keyName, bool doShift, bool doAlt, bool doCtrl) {
      IntPtr currentWindow = GetForegroundWindow();
      IntPtr foundWindow = FindVisualizerWindow();
      if (foundWindow != IntPtr.Zero) {
        SetForegroundWindow(foundWindow);

        INPUT[] inputs;

        if (doShift && doCtrl) {
          inputs = new INPUT[6];

          inputs[0] = new INPUT {
            type = 1, // Keyboard input
            u = new InputUnion {
              ki = new KEYBDINPUT {
                wVk = VK_SHIFT,
                dwFlags = 0 // Key down
              }
            }
          };

          inputs[1] = new INPUT {
            type = 1, // Keyboard input
            u = new InputUnion {
              ki = new KEYBDINPUT {
                wVk = VK_CTRL,
                dwFlags = 0 // Key down
              }
            }
          };

          inputs[2] = new INPUT {
            type = 1, // Keyboard input
            u = new InputUnion {
              ki = new KEYBDINPUT {
                wVk = (ushort)VKKey,
                dwFlags = 0 // Key down
              }
            }
          };

          inputs[3] = new INPUT {
            type = 1, // Keyboard input
            u = new InputUnion {
              ki = new KEYBDINPUT {
                wVk = (ushort)VKKey,
                dwFlags = 2 // Key up
              }
            }
          };

          inputs[4] = new INPUT {
            type = 1, // Keyboard input
            u = new InputUnion {
              ki = new KEYBDINPUT {
                wVk = VK_SHIFT,
                dwFlags = 2 // Key up
              }
            }
          };

          inputs[5] = new INPUT {
            type = 1, // Keyboard input
            u = new InputUnion {
              ki = new KEYBDINPUT {
                wVk = VK_CTRL,
                dwFlags = 2 // Key up
              }
            }
          };
        } else if (doShift) {
          inputs = new INPUT[4];

          inputs[0] = new INPUT {
            type = 1, // Keyboard input
            u = new InputUnion {
              ki = new KEYBDINPUT {
                wVk = VK_SHIFT,
                dwFlags = 0 // Key down
              }
            }
          };

          inputs[1] = new INPUT {
            type = 1, // Keyboard input
            u = new InputUnion {
              ki = new KEYBDINPUT {
                wVk = (ushort)VKKey,
                dwFlags = 0 // Key down
              }
            }
          };

          inputs[2] = new INPUT {
            type = 1, // Keyboard input
            u = new InputUnion {
              ki = new KEYBDINPUT {
                wVk = (ushort)VKKey,
                dwFlags = 2 // Key up
              }
            }
          };

          inputs[3] = new INPUT {
            type = 1, // Keyboard input
            u = new InputUnion {
              ki = new KEYBDINPUT {
                wVk = VK_SHIFT,
                dwFlags = 2 // Key up
              }
            }
          };
        } else if (doAlt) {
          inputs = new INPUT[4];

          inputs[0] = new INPUT {
            type = 1, // Keyboard input
            u = new InputUnion {
              ki = new KEYBDINPUT {
                wVk = VK_ALT,
                dwFlags = 0 // Key down
              }
            }
          };

          inputs[1] = new INPUT {
            type = 1, // Keyboard input
            u = new InputUnion {
              ki = new KEYBDINPUT {
                wVk = (ushort)VKKey,
                dwFlags = 0 // Key down
              }
            }
          };

          inputs[2] = new INPUT {
            type = 1, // Keyboard input
            u = new InputUnion {
              ki = new KEYBDINPUT {
                wVk = (ushort)VKKey,
                dwFlags = 2 // Key up
              }
            }
          };

          inputs[3] = new INPUT {
            type = 1, // Keyboard input
            u = new InputUnion {
              ki = new KEYBDINPUT {
                wVk = VK_ALT,
                dwFlags = 2 // Key up
              }
            }
          };
        } else {
          inputs = new INPUT[2];

          inputs[0] = new INPUT {
            type = 1, // Keyboard input
            u = new InputUnion {
              ki = new KEYBDINPUT {
                wVk = (ushort)VKKey,
                dwFlags = 0 // Key down
              }
            }
          };

          inputs[1] = new INPUT {
            type = 1, // Keyboard input
            u = new InputUnion {
              ki = new KEYBDINPUT {
                wVk = (ushort)VKKey,
                dwFlags = 2 // Key up
              }
            }
          };
        }

        SendInput((uint)inputs.Length, inputs, Marshal.SizeOf(typeof(INPUT)));
        SetStatusText($"Pressed {keyName} in '{foundWindowTitle}'");

        SetForegroundWindow(currentWindow);
      } else {
        SetStatusText(windowNotFound);
      }
    }

    private void btnF3_Click(object sender, EventArgs e) {
      SendPostMessage(VK_F3, "F3");
    }

    private void btnF4_Click(object sender, EventArgs e) {
      SendPostMessage(VK_F4, "F4");
    }

    private void btnF7_Click(object sender, EventArgs e) {
      SendPostMessage(VK_F7, "F7");
    }

    private void btnSpace_Click(object sender, EventArgs e) {
      SendPostMessage(VK_SPACE, "Space");
    }

    private void btnBackspace_Click(object sender, EventArgs e) {
      SendPostMessage(VK_BACKSPACE, "Backspace");
    }

    private void SendUnicodeChars(string inputString) {
      IntPtr currentWindow = GetForegroundWindow();
      IntPtr foundWindow = FindVisualizerWindow();

      if (foundWindow != IntPtr.Zero) {
        SetForegroundWindow(foundWindow);

        for (int i = 0; i < inputString.Length; i++) {
          INPUT[] inputs = new INPUT[1];
          inputs[0] = new INPUT {
            type = 1, // Keyboard input
            u = new InputUnion {
              ki = new KEYBDINPUT {
                wVk = 0,
                wScan = (ushort)inputString[i],
                dwFlags = 4, // KEYEVENTF_UNICODE
                time = 0,
                dwExtraInfo = IntPtr.Zero
              }
            }
          };
          SendInput((uint)inputs.Length, inputs, Marshal.SizeOf(typeof(INPUT)));
          Thread.Sleep(50);
        }

        SetStatusText($"Pressed {inputString.ToUpper()} in '{foundWindowTitle}'");

        SetForegroundWindow(currentWindow);

      } else {
        SetStatusText(windowNotFound);
      }
    }

    private void btnTilde_Click(object sender, EventArgs e) {
      SendUnicodeChars("~");
    }

    private void btnDelete_Click(object sender, EventArgs e) {
      SendPostMessage(VK_DELETE, "Delete");
    }

    private void btnAltEnter_Click(object sender, EventArgs e) {
      SendInput(VK_ENTER, "Alt+Enter", false, true, false);
    }

    private void btnN_Click(object sender, EventArgs e) {
      SendPostMessage(VK_N, "N");
    }

    private void btnF2_Click(object sender, EventArgs e) {
      SendPostMessage(VK_F2, "F2");
    }

    private void btnK_Click(object sender, EventArgs e) {
      SendPostMessage(VK_K, "K");
    }

    private void btnF10_Click(object sender, EventArgs e) {
      SendPostMessage(VK_F10, "F10");
    }

    private void btn00_Click(object sender, EventArgs e) {
      SendInputTwoKeys(VK_0, VK_0, "00");
      // SendUnicodeChars("00");
    }

    private void btn11_Click(object sender, EventArgs e) {
      SendInputTwoKeys(VK_1, VK_1, "11");
      // SendUnicodeChars("00");
    }

    private void btn22_Click(object sender, EventArgs e) {
      SendInputTwoKeys(VK_2, VK_2, "22");
      // SendUnicodeChars("22");
    }

    private void btn33_Click(object sender, EventArgs e) {
      SendInputTwoKeys(VK_3, VK_3, "33");
      // SendUnicodeChars("33");
    }

    private void btn44_Click(object sender, EventArgs e) {
      SendInputTwoKeys(VK_4, VK_4, "44");
      // SendUnicodeChars("44");
    }

    private void btn55_Click(object sender, EventArgs e) {
      SendInputTwoKeys(VK_5, VK_5, "55");
      // SendUnicodeChars("55");
    }

    private void btn66_Click(object sender, EventArgs e) {
      SendInputTwoKeys(VK_6, VK_6, "66");
      // SendUnicodeChars("66");
    }

    private void btn77_Click(object sender, EventArgs e) {
      SendInputTwoKeys(VK_7, VK_7, "77");
      // SendUnicodeChars("77");
    }

    private void btn88_Click(object sender, EventArgs e) {
      SendInputTwoKeys(VK_8, VK_8, "88");
      // SendUnicodeChars("88");
    }

    private void btn99_Click(object sender, EventArgs e) {
      SendInputTwoKeys(VK_9, VK_9, "99");
      // SendUnicodeChars("99");
    }

    private void lblFromFile_DoubleClick(object sender, EventArgs e) {
      LoadMessages(lastScriptFileName);
    }

    private void MainForm_KeyDown(object sender, KeyEventArgs e) {
      if ((Control.ModifierKeys & Keys.Control) == Keys.Control) {
        if (e.KeyCode == Keys.A) {
          txtMessage.Focus();
          txtMessage.SelectAll();
        } else if (e.KeyCode == Keys.D) {
          btnPresetLoadDirectory_Click(null, null);
        } else if (e.KeyCode == Keys.P) {
          btnPresetSend_Click(null, null);
        } else if (e.KeyCode == Keys.N) {
          SelectNextPreset();
          btnPresetSend_Click(null, null);
        } else if (e.KeyCode == Keys.S) {
          SendToMilkwaveVisualizer(txtMessage.Text, MessageType.Message);
        } else if (e.KeyCode == Keys.X) {
          btnSendFile_Click(null, null);
          if ((Control.ModifierKeys & Keys.Shift) == Keys.Shift) {
            SelectNextAutoplayEntry();
          }
        } else if (e.KeyCode == Keys.Y) {
          chkAutoplay.Checked = !chkAutoplay.Checked;
        } else if (e.KeyCode == Keys.Tab) {
          if ((Control.ModifierKeys & Keys.Shift) == Keys.Shift) {
            // Switch to the previous tab
            int previousIndex = (tabControl.SelectedIndex - 1 + tabControl.TabPages.Count) % tabControl.TabPages.Count; // Loop back to the last tab if at the first
            tabControl.SelectedIndex = previousIndex;
          } else {
            int nextIndex = (tabControl.SelectedIndex + 1) % tabControl.TabPages.Count; // Loop back to the first tab if at the last
            tabControl.SelectedIndex = nextIndex;
          }
        }
      }

      if (e.KeyCode == Keys.F1) {
        SendPostMessage(VK_F1, "F1");
      } else if (e.KeyCode == Keys.F2) {
        SendPostMessage(VK_F2, "F2");
      } else if (e.KeyCode == Keys.F3) {
        SendPostMessage(VK_F3, "F3");
      } else if (e.KeyCode == Keys.F4) {
        SendPostMessage(VK_F4, "F4");
      } else if (e.KeyCode == Keys.F5) {
        SendPostMessage(VK_F5, "F5");
      } else if (e.KeyCode == Keys.F6) {
        SendPostMessage(VK_F6, "F6");
      } else if (e.KeyCode == Keys.F7) {
        SendPostMessage(VK_F7, "F7");
      } else if (e.KeyCode == Keys.F8) {
        SendPostMessage(VK_F8, "F8");
      } else if (e.KeyCode == Keys.F9) {
        SendPostMessage(VK_F9, "F9");
      } else if (e.KeyCode == Keys.F10) {
        SendPostMessage(VK_F10, "F10");
      } else if (e.KeyCode == Keys.F11) {
        SendPostMessage(VK_F11, "F11");
      } else if (e.KeyCode == Keys.F12) {
        SendPostMessage(VK_F12, "F12");
      }
    }

    private void txtBPM_TextChanged(object sender, EventArgs e) {
      setTimerInterval();
    }

    private void lblBPM_Click(object sender, EventArgs e) {
      if (!chkAutoplay.Checked) {
        chkAutoplay.Checked = true;
      } else {
        ResetAndStartTimer(true);
      }
    }

    private void cboParameters_SelectedIndexChanged(object sender, EventArgs e) {
      if (cboParameters.SelectedItem is Style selectedPreset) {
        txtStyle.Text = selectedPreset.Name;
        BeginInvoke(new Action(() => cboParameters.Text = selectedPreset.Value));
      }
    }

    private void lblParameters_DoubleClick(object sender, EventArgs e) {
      Settings.Styles.Clear();
      ReloadStylesList();
      SetStatusText($"Saved presets cleared");
    }

    private void lblStyle_DoubleClick(object sender, EventArgs e) {
      var foundItem = from item in Settings.Styles
                      where item.Name == txtStyle.Text
                      select item;
      if (foundItem != null && foundItem.Any()) {
        Settings.Styles.Remove(foundItem.First());
        ReloadStylesList();
      }
    }

    private void ReloadStylesList() {
      cboParameters.Items.Clear();
      cboParameters.Items.AddRange(Settings.Styles.OrderBy(x => x.Name).ToArray());
      cboParameters.Refresh();
    }

    private void txtStyle_KeyDown(object sender, KeyEventArgs e) {
      if (e.KeyCode == Keys.Enter) {
        e.SuppressKeyPress = true; // Prevent the beep sound on Enter key press
        btnSaveParam.PerformClick();
      }
    }

    private void MainForm_FormClosing(object sender, FormClosingEventArgs e) {
      try {

        SetAndSaveSettings();

        IntPtr foundWindow = FindVisualizerWindow();
        if (foundWindow != IntPtr.Zero) {
          // Close the Visualizer window if CloseVisualizerWithRemote=true or Alt ot Ctrl key are pressed
          if (Settings.CloseVisualizerWithRemote || (Control.ModifierKeys & Keys.Alt) == Keys.Alt || (Control.ModifierKeys & Keys.Control) == Keys.Control) {
            PostMessage(foundWindow, 0x0010, IntPtr.Zero, IntPtr.Zero); // WM_CLOSE message
          }
        }

      } catch (Exception ex) {
        Program.SaveErrorToFile(ex, "Error");
      }
    }

    private void SaveSettingsToFile() {
      string jsonString = JsonSerializer.Serialize(Settings, new JsonSerializerOptions { WriteIndented = true });
      string settingsFile = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, milkwaveSettingsFile);
      try {
        File.WriteAllText(settingsFile, jsonString);
      } catch (UnauthorizedAccessException ex) {
        MessageBox.Show($"Unable to save settings to {settingsFile}." +
          Environment.NewLine + Environment.NewLine +
          "Please make sure that Milkwave is installed to a directory with full write access (eg. not 'Program Files').",
          "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
      } catch (Exception ex) {
        Program.SaveErrorToFile(ex, "Error");
      }
    }

    private void SaveTagsToFile() {

      var sortedTags = new Tags {
        TagEntries = Tags.TagEntries
            .OrderBy(kvp => kvp.Key, StringComparer.OrdinalIgnoreCase)
            .ToDictionary(kvp => kvp.Key, kvp => kvp.Value)
      };

      string jsonString = JsonSerializer.Serialize(sortedTags, new JsonSerializerOptions { WriteIndented = true });
      string tagsFile = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, milkwaveTagsFile);
      try {
        File.WriteAllText(tagsFile, jsonString);
        SetStatusText($"Tags saved");
      } catch (UnauthorizedAccessException ex) {
        MessageBox.Show($"Unable to save Tags to {tagsFile}." +
          Environment.NewLine + Environment.NewLine +
          "Please make sure that Milkwave is installed to a directory with full write access (eg. not 'Program Files').",
          "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
      } catch (Exception ex) {
        Program.SaveErrorToFile(ex, "Error");
      }
      SetTopTags();
    }

    private void cboParameters_KeyDown(object sender, KeyEventArgs e) {
      if (e.KeyCode == Keys.Enter) {
        e.SuppressKeyPress = true; // Prevent the beep sound on Enter key press
        btnSaveParam.PerformClick();
      }
    }

    private void txtMessage_KeyDown(object sender, KeyEventArgs e) {
      if (e.KeyCode == Keys.Enter && (Control.ModifierKeys & Keys.Shift) != Keys.Shift) {
        e.SuppressKeyPress = true; // Prevent the beep sound on Enter key press
        btnSend.PerformClick();
      }
    }

    private void lblFont_DoubleClick(object sender, EventArgs e) {
      RemoveParam("font");
    }

    private void RemoveParam(string param) {
      try {
        int rIndex = cboParameters.Text.IndexOf("|" + param + "=", StringComparison.CurrentCultureIgnoreCase);
        if (rIndex == -1) {
          rIndex = cboParameters.Text.IndexOf(param + "=", StringComparison.CurrentCultureIgnoreCase);
        }
        if (rIndex > -1) {
          int rIndex2 = cboParameters.Text.IndexOf("|", rIndex + 1);
          if (rIndex2 > -1) {
            cboParameters.Text = cboParameters.Text.Remove(rIndex, rIndex2 - rIndex + 1);
          } else {
            cboParameters.Text = cboParameters.Text.Remove(rIndex);
          }
        }
      } catch (Exception ex) {
        SetStatusText("Error: " + ex.Message);
      }
    }

    private string GetParam(string paramame, string haystack) {
      string result = "";
      try {
        int rIndex = haystack.IndexOf("|" + paramame + "=", StringComparison.CurrentCultureIgnoreCase);
        if (rIndex == -1) {
          rIndex = haystack.IndexOf(paramame + "=", StringComparison.CurrentCultureIgnoreCase);
        }
        if (rIndex > -1) {
          int rIndex2 = haystack.IndexOf("|", rIndex + 1);
          if (rIndex2 > -1) {
            result = haystack.Substring(rIndex, rIndex2 - rIndex);
          } else {
            result = haystack.Substring(rIndex);
          }
        }
        if (result.Length > 0) {
          result = result.Substring(result.IndexOf("=") + 1);
        }
      } catch (Exception ex) {
        SetStatusText("Error: " + ex.Message);
      }
      return result;
    }

    private void lblColor_DoubleClick(object sender, EventArgs e) {
      RemoveParam("r");
      RemoveParam("g");
      RemoveParam("b");
    }

    private void lblSize_DoubleClick(object sender, EventArgs e) {
      RemoveParam("size");
    }

    private void btnAppendSize_Click(object sender, EventArgs e) {
      RemoveParam("size");
      AppendParam("size=" + numSize.Text);
    }

    private void AppendParam(string param) {
      if (cboParameters.Text.Length > 0) {
        cboParameters.Text += "|";
      }
      cboParameters.Text += param;
    }

    private void txtSize_KeyDown(object sender, KeyEventArgs e) {
      if (e.KeyCode == Keys.Enter) {
        e.SuppressKeyPress = true; // Prevent the beep sound on Enter key press
        btnAppendSize.PerformClick();
      }
    }

    private void cboFonts_KeyDown(object sender, KeyEventArgs e) {
      if (e.KeyCode == Keys.Enter) {
        e.SuppressKeyPress = true; // Prevent the beep sound on Enter key press
        btnFontAppend.PerformClick();
      }
    }

    private void SetFormattedMessage() {
      if (!chkPreview.Checked) {
        return;
      }
      try {
        string fontName = GetParam("font", cboParameters.Text);
        if (fontName.Length == 0) {
          fontName = cboFonts.Text;
        }

        Color fontColor;
        string colorR = GetParam("r", cboParameters.Text);
        string colorG = GetParam("g", cboParameters.Text);
        string colorB = GetParam("b", cboParameters.Text);
        if (colorR.Length == 0 || colorG.Length == 0 || colorB.Length == 0) {
          fontColor = pnlColorMessage.BackColor;
        } else {
          fontColor = Color.FromArgb(int.Parse(colorR), int.Parse(colorG), int.Parse(colorB));
        }

        int fontSize;
        string size = GetParam("size", cboParameters.Text);
        if (size.Length == 0 || !int.TryParse(size, out fontSize)) {
          fontSize = int.Parse(numSize.Text);
        }

        FontStyle style = cboParameters.Text.ToUpper().Contains("bold=1") ? FontStyle.Bold : FontStyle.Regular;
        txtMessage.Font = new Font(fontName, fontSize, style);
        txtMessage.ForeColor = fontColor;

        txtMessage.Refresh();
      } catch (Exception e) {
        // ignore
      }
    }

    private void txtSize_TextChanged(object sender, EventArgs e) {
      SetFormattedMessage();
    }

    private void cboFonts_SelectedIndexChanged(object sender, EventArgs e) {
      SetFormattedMessage();
    }

    private void cboParameters_TextChanged(object sender, EventArgs e) {
      SetFormattedMessage();
    }

    private void chkPreview_CheckedChanged(object sender, EventArgs e) {
      if (chkPreview.Checked) {
        SetFormattedMessage();
      } else {
        txtMessage.Font = cboAutoplay.Font;
        txtMessage.ForeColor = cboAutoplay.ForeColor;
      }
    }

    static void FixNumericUpDownMouseWheel(Control c) {
      foreach (var num in c.Controls.OfType<NumericUpDown>())
        num.MouseWheel += FixNumericUpDownMouseWheelHandler;

      foreach (var child in c.Controls.OfType<Control>())
        FixNumericUpDownMouseWheel(child);
    }

    static private void FixNumericUpDownMouseWheelHandler(object? sender, MouseEventArgs e) {
      ((HandledMouseEventArgs)e).Handled = true;
      var self = ((NumericUpDown)sender!);
      self.Value = Math.Max(Math.Min(
          self.Value + ((e.Delta > 0) ? self.Increment : -self.Increment), self.Maximum), self.Minimum);
    }

    private void lblFromFile_MouseClick(object sender, MouseEventArgs e) {
      if (e.Button == MouseButtons.Right) {
        OpenFileDialog ofdScriptFile = new OpenFileDialog();
        ofdScriptFile.Filter = "Milkwave script files|*.txt|All files (*.*)|*.*";
        ofdScriptFile.RestoreDirectory = true;
        ofdScriptFile.InitialDirectory = AppDomain.CurrentDomain.BaseDirectory;

        if (ofdScriptFile.ShowDialog() == DialogResult.OK) {
          lastScriptFileName = ofdScriptFile.FileName;
          LoadMessages(lastScriptFileName);
        }
      }
    }

    private void lblStyle_MouseClick(object sender, MouseEventArgs e) {
      if (e.Button == MouseButtons.Right) {
        string fontName = GetParam("font", cboParameters.Text);
        if (fontName.Length > 0) {
          cboFonts.Text = fontName;
        }

        string colorR = GetParam("r", cboParameters.Text);
        string colorG = GetParam("g", cboParameters.Text);
        string colorB = GetParam("b", cboParameters.Text);
        if (colorR.Length > 0 && colorG.Length > 0 && colorB.Length > 0) {
          pnlColorMessage.BackColor = Color.FromArgb(int.Parse(colorR), int.Parse(colorG), int.Parse(colorB));
        }

        int fontSize;
        string size = GetParam("size", cboParameters.Text);
        if (size.Length > 0 && int.TryParse(size, out fontSize)) {
          numSize.Value = fontSize;
        }
      }
    }

    private void lblWindow_DoubleClick(object sender, EventArgs e) {
      StartVisualizerIfNotFound();
    }

    private void toolStripMenuItemReleases_Click(object sender, EventArgs e) {
      string url = "https://github.com/IkeC/Milkwave/releases";
      Process.Start(new ProcessStartInfo(url) { UseShellExecute = true });
    }

    private void toolStripMenuItemHelp_Click(object sender, EventArgs e) {
      string dialogtext =
  "There are many tooltips explaining all features when you move your mouse over all the form elements." + Environment.NewLine +
  "" + Environment.NewLine +
  "More help resources:" + Environment.NewLine +
  "" + Environment.NewLine +
  "GitHub homepage: https://github.com/IkeC/Milkwave" + Environment.NewLine +
  "GitHub issues: https://github.com/IkeC/Milkwave/issues" + Environment.NewLine +
  "Ikes Discord: https://bit.ly/Ikes-Discord" + Environment.NewLine +
  "" + Environment.NewLine +
  "and the README.txt in the program folder.";
      new MilkwaveInfoForm(toolStripMenuItemDarkMode.Checked).ShowDialog("Milkwave Help", dialogtext);
    }

    private void toolStripMenuItemSupporters_Click(object sender, EventArgs e) {
      string dialogtext =
  "Supporters:" + Environment.NewLine +
  Environment.NewLine +
  "Shanev — Thank you very much!" +
  Environment.NewLine +
  Environment.NewLine +
  "Milkwave is and will always be free software, being the collaborative effort of many diffent authors. " +
  "If you like it and want to appreciate and support my share of the work, please consider donating." +
  "" + Environment.NewLine +
  "https://www.paypal.com/ncp/payment/5XMP3S69PJLCU" + Environment.NewLine +
  "" + Environment.NewLine +
  "Any amount is valued. You'll be listed on this page unless you do not want to.";
      new MilkwaveInfoForm(toolStripMenuItemDarkMode.Checked).ShowDialog("Milkwave Supporters", dialogtext);
    }

    private void SetBarIcon(bool isDarkMode) {
      if (isDarkMode) {
        using (var ms = new MemoryStream(Properties.Resources.MilkwaveOutlineInverted)) {
          toolStripDropDownButton.Image = Image.FromStream(ms);
        }
      } else {
        using (var ms = new MemoryStream(Properties.Resources.MilkwaveOutline)) {
          toolStripDropDownButton.Image = Image.FromStream(ms);
        }
      }
    }

    private void toolStripMenuItemDarkMode_Click(object sender, EventArgs e) {
      toolStripMenuItemDarkMode.Checked = !toolStripMenuItemDarkMode.Checked;
      Settings.DarkMode = toolStripMenuItemDarkMode.Checked;
      var tmpColorMessage = pnlColorMessage.BackColor;
      var tmpColorWave = pnlColorWave.BackColor;
      dm.ColorMode = Settings.DarkMode ? DisplayMode.DarkMode : DisplayMode.ClearMode;
      dm.ApplyTheme(Settings.DarkMode);
      SetBarIcon(Settings.DarkMode);
      pnlColorMessage.BackColor = tmpColorMessage;
      pnlColorWave.BackColor = tmpColorWave;
      SetFormattedMessage();
    }

    private void toolStripMenuItemTabsPanel_Click(object sender, EventArgs e) {
      toolStripMenuItemTabsPanel.Checked = !toolStripMenuItemTabsPanel.Checked;
      if (!toolStripMenuItemTabsPanel.Checked && !toolStripMenuItemButtonPanel.Checked) {
        toolStripMenuItemButtonPanel.Checked = true;
      }
      SetPanelsVisibility();
    }

    private void toolStripMenuItemButtonPanel_Click(object sender, EventArgs e) {
      toolStripMenuItemButtonPanel.Checked = !toolStripMenuItemButtonPanel.Checked;
      if (!toolStripMenuItemTabsPanel.Checked && !toolStripMenuItemButtonPanel.Checked) {
        toolStripMenuItemTabsPanel.Checked = true;
      }
      SetPanelsVisibility();
    }

    private void SetPanelsVisibility() {
      Settings.ShowTabsPanel = toolStripMenuItemTabsPanel.Checked;
      Settings.ShowButtonPanel = toolStripMenuItemButtonPanel.Checked;
      splitContainer1.Panel1Collapsed = !Settings.ShowTabsPanel;
      splitContainer1.Panel2Collapsed = !Settings.ShowButtonPanel;
    }

    private void btnPresetLoadFile_Click(object sender, EventArgs e) {
      if (ofd.ShowDialog() == DialogResult.OK) {
        string fileName = ofd.FileName;
        if (fileName.EndsWith(".milk", StringComparison.CurrentCultureIgnoreCase) || ofd.FileName.EndsWith(".milk2", StringComparison.CurrentCultureIgnoreCase)) {
          int index = fileName.IndexOf(VisualizerPresetsFolder, StringComparison.CurrentCultureIgnoreCase);
          string maybeRelativePath = fileName;
          if (index > -1) {
            maybeRelativePath = fileName.Substring(index + VisualizerPresetsFolder.Length);
          }
          Data.Preset newPreset = new Data.Preset {
            DisplayName = Path.GetFileNameWithoutExtension(fileName),
            MaybeRelativePath = maybeRelativePath
          };
          if (!cboPresets.Items.Contains(newPreset)) {
            cboPresets.Items.Insert(0, newPreset);
          }
          cboPresets.SelectedItem = newPreset;
          cboPresets.Text = newPreset.DisplayName;
        }
      }
    }

    private void btnPresetSend_MouseDown(object sender, MouseEventArgs e) {
      if (e.Button == MouseButtons.Middle) {
        SelectPreviousPreset();
        btnPresetSend_Click(null, null);
      } else if (e.Button == MouseButtons.Right) {
        SelectNextPreset();
        btnPresetSend_Click(null, null);
      }
    }

    private void btnPresetSend_Click(object? sender, EventArgs? e) {

      if (cboPresets.Text.Length > 0) {
        Data.Preset? preset = null; // Use nullable type to handle potential null values
        preset = cboPresets.SelectedItem as Data.Preset; // Use 'as' operator to safely cast
        if (preset != null) { // Check for null before accessing properties
          if (preset.MaybeRelativePath.Length > 0) {
            string fullPath = preset.MaybeRelativePath;
            if (!Path.IsPathRooted(fullPath)) {
              fullPath = Path.Combine(VisualizerPresetsFolder, preset.MaybeRelativePath);
            }
            if (File.Exists(fullPath)) {
              SendToMilkwaveVisualizer(fullPath, MessageType.PresetFilePath);
            } else {
              SetStatusText($"Preset file '{fullPath}' not found");
            }
          } else {
            SetStatusText($"Preset file '{preset.MaybeRelativePath}' not found");
          }
        } else {
          SetStatusText("No valid preset selected");
        }
      } else {
        SetStatusText("No valid preset selected");
      }
    }

    private void SelectNextPreset() {
      // Move to the next item in cboPresets if possible
      if (cboPresets.SelectedIndex < cboPresets.Items.Count - 1) {
        cboPresets.SelectedIndex++;
      } else {
        // Optionally, loop back to the first item
        cboPresets.SelectedIndex = 0;
      }
    }

    private void SelectPreviousPreset() {
      // Move to the previous item in cboPresets if possible
      if (cboPresets.SelectedIndex > 0) {
        cboPresets.SelectedIndex--;
      } else {
        // Optionally, loop back to the last item
        cboPresets.SelectedIndex = cboPresets.Items.Count - 1;
      }
    }

    private void SelectRandomPreset() {
      // Select a random item from cboPresets
      Random random = new Random();
      int randomIndex = random.Next(cboPresets.Items.Count);
      cboPresets.SelectedIndex = randomIndex;
    }

    private void lblPreset_DoubleClick(object sender, EventArgs e) {
      cboPresets.Items.Clear();
      cboPresets.Text = "";
    }

    private void btnPresetLoadDirectory_Click(object? sender, EventArgs? e) {
      using (var fbd = new FolderBrowserDialog()) {
        if (Directory.Exists(VisualizerPresetsFolder)) {
          fbd.InitialDirectory = VisualizerPresetsFolder;
        } else {
          fbd.InitialDirectory = AppDomain.CurrentDomain.BaseDirectory;
        }
        DialogResult result = fbd.ShowDialog();

        if (result == DialogResult.OK && !string.IsNullOrWhiteSpace(fbd.SelectedPath)) {
          LoadPresetsFromDirectory(fbd.SelectedPath);
        }
      }
    }

    private void LoadPresetsFromDirectory(string dirToLoad) {
      cboPresets.Items.Clear();
      cboPresets.Text = "";
      int relIndex = -1;
      foreach (string fileName in Directory.GetFiles(dirToLoad)) {
        if (relIndex == -1) {
          relIndex = fileName.IndexOf(VisualizerPresetsFolder, StringComparison.CurrentCultureIgnoreCase);
        }
        string fileNameMaybeRelativePath = fileName;
        if (relIndex > -1) {
          fileNameMaybeRelativePath = fileName.Substring(relIndex + VisualizerPresetsFolder.Length);
        }
        if (fileNameMaybeRelativePath.EndsWith(".milk") || fileNameMaybeRelativePath.EndsWith(".milk2")) {
          string fileNameOnlyNoExtension = Path.GetFileNameWithoutExtension(fileNameMaybeRelativePath);
          if (txtDirOrTagsFilter.Text.Length == 0 || fileNameOnlyNoExtension.Contains(txtDirOrTagsFilter.Text, StringComparison.InvariantCultureIgnoreCase)) {
            Data.Preset newPreset = new Data.Preset {
              DisplayName = fileNameOnlyNoExtension,
              MaybeRelativePath = fileNameMaybeRelativePath
            };
            cboPresets.Items.Add(newPreset);
          }
        }
      }
      SetStatusText($"Loaded {cboPresets.Items.Count} presets from '{dirToLoad}'");
      if (cboPresets.Items.Count > 0) {
        cboPresets.SelectedIndex = 0;
        UpdateTagsDisplay(false, false);
      }
    }

    private void cboPresets_KeyDown(object sender, KeyEventArgs e) {
      if (e.KeyCode == Keys.Enter) {
        e.SuppressKeyPress = true; // Prevent the beep sound on Enter key press
        btnPresetSend.PerformClick();
      }
    }

    private void lblPreset_Click(object sender, EventArgs e) {
      if (cboPresets.SelectedItem != null) {
        try {
          Data.Preset preset = (Data.Preset)cboPresets.SelectedItem;
          if (!string.IsNullOrEmpty(preset.MaybeRelativePath)) {
            Clipboard.SetText(preset.MaybeRelativePath);
            SetStatusText($"Copied '{preset.MaybeRelativePath}' to clipboard");
          }
        } catch {
          // Handle the case where the selected item is not a Preset
          SetStatusText("No valid preset selected");
        }
      }
    }

    private void cboPresets_SelectedIndexChanged(object sender, EventArgs e) {
      // Check if the Alt key is pressed
      if ((Control.ModifierKeys & Keys.Alt) == Keys.Alt) {
        // Trigger the Click event of btnPresetSend
        btnPresetSend.PerformClick();
      }
      UpdateTagsDisplay(false, false);
    }

    private void UpdateTagsDisplay(bool force, bool runningPresetChanged) {
      string key = "";
      if (!chkTagsFromRunning.Checked) {
        if (force || !runningPresetChanged) {
          Data.Preset? preset = cboPresets.SelectedItem as Data.Preset;
          if (preset != null) {
            key = preset.DisplayName.ToLower();
          }
        }
      } else if (force || runningPresetChanged) {
        // may contain path info (without file extension), so use only the file name
        key = Path.GetFileName(txtVisRunning.Text).ToLower();
      }
      if (key.Length > 0) {
        if (Tags.TagEntries.ContainsKey(key)) {
          txtTags.Text = string.Join(", ", Tags.TagEntries[key].Tags);
        } else {
          txtTags.Text = string.Empty; // Clear the text if the key doesn't exist
        }
      }
    }

    private void lblAmpLeft_Click(object sender, EventArgs e) {
      numAmpLeft.Value = 1.0M;
      numAmpRight.Value = 1.0M;
    }

    private void lblAmpRight_Click(object sender, EventArgs e) {
      numAmpRight.Value = 1.0M;
      if (chkAmpLinked.Checked) {
        numAmpLeft.Value = numAmpRight.Value;
      }
    }

    private void numAmpLeft_ValueChanged(object sender, EventArgs e) {
      if (chkAmpLinked.Checked) {
        numAmpRight.Value = numAmpLeft.Value;
      }
      SetAmpIncrements(numAmpLeft);
      SendToMilkwaveVisualizer("", MessageType.Amplify);
    }

    private void numAmpRight_ValueChanged(object sender, EventArgs e) {
      if (chkAmpLinked.Checked) {
        numAmpLeft.Value = numAmpRight.Value;
      }
      SetAmpIncrements(numAmpRight);
      SendToMilkwaveVisualizer("", MessageType.Amplify);
    }

    private void SetAmpIncrements(NumericUpDown nud) {
      // Ensure the Tag property is cast to decimal before comparison
      decimal previousValue = nud.Tag is decimal tagValue ? tagValue : 0;
      bool up = previousValue < nud.Value;

      if (nud.Value < 0.1M || (nud.Value == 0.1M && !up)) {
        nud.Increment = 0.01M;
      } else if (nud.Value < 2M || (nud.Value == 2M && !up)) {
        nud.Increment = 0.1M;
      } else if (nud.Value < 10M || (nud.Value == 10M && !up)) {
        nud.Increment = 1M;
      } else {
        nud.Increment = 5M;
      }
      nud.Tag = nud.Value;
    }

    private void numWavemode_ValueChanged(object sender, EventArgs e) {
      SendWaveInfoIfLinked();
    }

    private void btnSendWave_Click(object sender, EventArgs e) {
      SendWaveInfo();
    }

    private void SendWaveInfoIfLinked() {
      if (chkWaveLink.Checked) SendWaveInfo();
    }

    private void SendWaveInfo() {
      if (!updatingWaveParams) {
        SendToMilkwaveVisualizer("", MessageType.Wave);
      }
    }

    private void lblWaveColor_DoubleClick(object sender, EventArgs e) {
      string copyText = colorDialogWave.Color.R + "," + colorDialogWave.Color.G + "," + colorDialogWave.Color.B;
      string displayText = copyText;

      if ((Control.ModifierKeys & Keys.Alt) == Keys.Alt) {
        float redValue = colorDialogWave.Color.R / 255f;
        string formattedRedValue = redValue.ToString("F3", CultureInfo.InvariantCulture);
        float greenValue = colorDialogWave.Color.G / 255f;
        string formattedGreenValue = greenValue.ToString("F3", CultureInfo.InvariantCulture);
        float blueValue = colorDialogWave.Color.B / 255f;
        string formattedBlueValue = blueValue.ToString("F3", CultureInfo.InvariantCulture);
        copyText = "r=" + formattedRedValue + Environment.NewLine + "g=" + formattedGreenValue + Environment.NewLine + "b=" + formattedBlueValue;
        displayText = "r=" + formattedRedValue + ", g=" + formattedGreenValue + ", b=" + formattedBlueValue;
      }
      Clipboard.SetText(copyText);
      SetStatusText($"Copied '{displayText}' to clipboard");
    }

    private void lblCurrentPreset_DoubleClick(object sender, EventArgs e) {
      string? text = toolTip1.GetToolTip(txtVisRunning);
      if (!string.IsNullOrEmpty(text)) {
        Clipboard.SetText(text);
        SetStatusText($"Copied '{text}' to clipboard");
      }
    }

    private void btnSetAudioDevice_Click(object? sender, EventArgs? e) {
      SendToMilkwaveVisualizer("", MessageType.AudioDevice);
    }

    private void cboAudioDevice_SelectedIndexChanged(object sender, EventArgs e) {
      // Check if the Alt key is pressed
      if ((Control.ModifierKeys & Keys.Alt) == Keys.Alt) {
        SendToMilkwaveVisualizer("", MessageType.AudioDevice);
      }
    }

    private void numOpacity_ValueChanged(object sender, EventArgs e) {
      SendToMilkwaveVisualizer("", MessageType.Opacity);
    }

    private void lblPercent_Click(object sender, EventArgs e) {
      numOpacity.Value = 100;
    }

    private void lblAudioDevice_DoubleClick(object sender, EventArgs e) {
      helper.ReloadAudioDevices(cboAudioDevice);
      SetStatusText($"Audio device list reloaded");
    }

    private void btnSendFile_Click(object? sender, EventArgs e) {
      SendAutoplayLine(true);
    }

    private void btnSendFile_MouseDown(object sender, MouseEventArgs e) {
      if (e.Button == MouseButtons.Right) {
        SendAutoplayLine(true);
        SelectNextAutoplayEntry();
      }
    }

    private void btnB_Click(object sender, EventArgs e) {
      SendPostMessage(VK_B, "B");
    }

    private void btnTransparency_Click(object sender, EventArgs e) {
      SendPostMessage(VK_F12, "F12");
    }

    private void btnWatermark_Click(object sender, EventArgs e) {
      SendInput(VK_F9, "F9", true, false, true);
    }

    private void btnTagsSave_Click(object sender, EventArgs e) {
      SaveTags();
    }

    private void SaveTags() {
      string key = "";
      string presetPath = "";

      if (chkTagsFromRunning.Checked) {
        key = Path.GetFileName(txtVisRunning.Text);
        string? fullPath = toolTip1.GetToolTip(txtVisRunning);
        if (fullPath != null) {
          presetPath = fullPath;
        }
      } else {
        Data.Preset? preset = cboPresets.SelectedItem as Data.Preset;
        if (preset != null) {
          key = preset.DisplayName;
          presetPath = preset.MaybeRelativePath;
        }
      }

      if (key.Length > 0 && presetPath.Length > 0) {

        // save relative path if preset is somewhere within the default presets folder
        int index = presetPath.IndexOf(VisualizerPresetsFolder, StringComparison.CurrentCultureIgnoreCase);
        if (index > -1) {
          presetPath = presetPath.Substring(index + VisualizerPresetsFolder.Length);
        }

        key = key.ToLower();
        TagEntry? tagEntry = null;
        if (Tags.TagEntries.ContainsKey(key)) {
          tagEntry = Tags.TagEntries.GetValueOrDefault(key);
        }
        if (tagEntry == null) {
          tagEntry = new TagEntry();
          Tags.TagEntries.Add(key, tagEntry);
        }
        tagEntry.PresetPath = presetPath;
        tagEntry.Tags.Clear();

        // Process txtTags.Text
        if (!string.IsNullOrWhiteSpace(txtTags.Text)) {
          var uniqueTags = new HashSet<string>(
              txtTags.Text
                  .Split(new[] { ',' }, StringSplitOptions.RemoveEmptyEntries) // Split by commas
                  .Select(tag => tag.Trim().ToLower()) // Trim and lowercase each token
                  .Where(tag => !string.IsNullOrWhiteSpace(tag)) // Remove empty or whitespace tokens
          );

          tagEntry.Tags = uniqueTags.OrderBy(tag => tag).ToList(); // Convert to a sorted list
        }
        SaveTagsToFile();
      }
    }

    private void txtTags_KeyDown(object sender, KeyEventArgs e) {
      if (e.KeyCode == Keys.Enter) {
        e.SuppressKeyPress = true; // Prevent the beep sound on Enter key press
        SaveTags();
        if ((Control.ModifierKeys & Keys.Control) == Keys.Control) {
          SendPostMessage(VK_SPACE, "Space");
        }
      }
    }

    private void chkTagsFromRunning_CheckedChanged(object sender, EventArgs e) {
      UpdateTagsDisplay(true, false);
    }

    private void SetTopTags() {
      // Dictionary to store tag counts
      var tagCounts = new Dictionary<string, int>();

      // Iterate over all TagEntries
      foreach (var tagEntry in Tags.TagEntries.Values) {
        foreach (var tag in tagEntry.Tags) {
          if (tagCounts.ContainsKey(tag)) {
            tagCounts[tag]++;
          } else {
            tagCounts[tag] = 1;
          }
        }
      }

      // Create a sorted list of tags by count in descending order
      var sortedTags = tagCounts
          .OrderByDescending(kvp => kvp.Value) // Sort by count (descending)
          .ThenBy(kvp => kvp.Key) // Optional: Sort alphabetically for ties
          .Select(kvp => (Tag: kvp.Key, Count: kvp.Value)) // Convert to tuple
          .ToList();

      if (sortedTags.Count > 0) {
        SetButtonTagInfo(btnTagA, sortedTags[0]);
      } else {
        // Clear button if no tags
        btnTagA.Tag = null;
        btnTagA.Text = "";
      }

      if (sortedTags.Count > 1) {
        SetButtonTagInfo(btnTagB, sortedTags[1]);
      } else {
        // Clear button if no tags
        btnTagB.Tag = null;
        btnTagB.Text = "";
      }

      if (sortedTags.Count > 2) {
        SetButtonTagInfo(btnTagC, sortedTags[2]);
      } else {
        // Clear button if no tags
        btnTagC.Tag = null;
        btnTagC.Text = "";
      }

      if (sortedTags.Count > 3) {
        SetButtonTagInfo(btnTagD, sortedTags[3]);
      } else {
        // Clear button if no tags
        btnTagD.Tag = null;
        btnTagD.Text = "";
      }

      if (sortedTags.Count > 4) {
        SetButtonTagInfo(btnTagE, sortedTags[4]);
      } else {
        // Clear button if no tags
        btnTagE.Tag = null;
        btnTagE.Text = "";
      }
    }

    private void SetButtonTagInfo(Button button, (string Tag, int Count) tagInfo) {
      string text = "Add/remove '" + tagInfo.Tag + "' in tags (used " + tagInfo.Count + " times)" +
        Environment.NewLine + "Ctrl+Click: Add/remove in load filter (OR)" +
        Environment.NewLine + "Shift+Click: Add/remove in load filter (AND)";
      toolTip1.SetToolTip(button, text);
      button.Tag = tagInfo.Tag;
      button.Text = GetTagButtonCaption(tagInfo.Tag);
    }

    private string GetTagButtonCaption(string tag) {
      if (tag.Length > 2) {
        return tag.Substring(0, 3).ToUpper();
      } else {
        return tag.ToUpper();
      }
    }

    private void btnTagA_Click(object sender, EventArgs e) {
      AddOrRemoveTopTag(sender);
    }

    private void btnTagB_Click(object sender, EventArgs e) {
      AddOrRemoveTopTag(sender);
    }

    private void btnTagC_Click(object sender, EventArgs e) {
      AddOrRemoveTopTag(sender);
    }

    private void btnTagD_Click(object sender, EventArgs e) {
      AddOrRemoveTopTag(sender);
    }

    private void btnTagE_Click(object sender, EventArgs e) {
      AddOrRemoveTopTag(sender);
    }

    private void AddOrRemoveTopTag(object sender) {
      TextBox srcTextbox = txtTags;
      Char tokenChar = ',';
      string joinSep = ", ";
      if ((Control.ModifierKeys & Keys.Control) == Keys.Control) {
        srcTextbox = txtDirOrTagsFilter;
      }
      if ((Control.ModifierKeys & Keys.Shift) == Keys.Shift) {
        srcTextbox = txtDirOrTagsFilter;
        tokenChar = '&';
        joinSep = " & ";
      }
      if (sender is Button button && button.Tag is string tag && tag.Length > 0) {

        // Split txtTags.Text into tokens, trimming whitespace
        var tokens = srcTextbox.Text
            .Split(new[] { tokenChar }, StringSplitOptions.RemoveEmptyEntries)
            .Select(t => t.Trim())
            .ToList();

        if (tokens.Contains(tag)) {
          // Remove the tag if it exists
          tokens.Remove(tag);
        } else {
          // Add the tag if it does not exist
          tokens.Add(tag);
        }

        tokens = tokens.OrderBy(tag => tag).ToList(); // Convert to a sorted list

        // Update txtTags.Text with the updated tokens, joined by ", "
        srcTextbox.Text = string.Join(joinSep, tokens);
      }
    }

    private void lblAudioDevice_MouseClick(object sender, MouseEventArgs e) {
      if (e.Button == MouseButtons.Right) {
        helper.SelectDefaultDevice(cboAudioDevice);
        btnSetAudioDevice_Click(null, null);
        SetStatusText($"Default audio device selected and set");
      }
    }

    private void btnPresetLoadTags_Click(object sender, EventArgs e) {
      cboPresets.Items.Clear();
      var presetList = new List<Data.Preset>();
      if (txtDirOrTagsFilter.Text.Length > 0) {
        var filteredEntries = FilterTagEntries();
        foreach (var entry in filteredEntries) {
          Data.Preset newPreset = new Data.Preset {
            DisplayName = entry.Key,
            MaybeRelativePath = entry.Value.PresetPath
          };
          presetList.Add(newPreset);
        }
        presetList.Sort((x, y) => string.Compare(x.DisplayName, y.DisplayName, StringComparison.OrdinalIgnoreCase));

        // Fix: Convert the Preset list to an object array before adding to Items
        cboPresets.Items.AddRange(presetList.Cast<object>().ToArray());
      }

      SetStatusText($"Loaded {cboPresets.Items.Count} filtered presets");
      if (cboPresets.Items.Count > 0) {
        cboPresets.SelectedIndex = 0;
        UpdateTagsDisplay(false, false);
      }

    }

    private List<KeyValuePair<string, TagEntry>> FilterTagEntries() {
      var filterText = txtDirOrTagsFilter.Text.Trim();

      // Split the filter text into tokens based on ',' or '&'
      var filters = filterText.Split(new[] { ',' }, StringSplitOptions.RemoveEmptyEntries)
                              .Select(f => f.Trim())
                              .ToList();

      var result = new List<KeyValuePair<string, TagEntry>>();

      foreach (var entry in Tags.TagEntries) {
        var tags = entry.Value.Tags;

        // Check if the entry matches any of the filters
        bool matches = filters.Any(filter => {
          if (filter.Contains("&")) {
            // Handle '&' (AND) condition
            var andTokens = filter.Split(new[] { '&' }, StringSplitOptions.RemoveEmptyEntries)
                                  .Select(t => t.Trim());
            return andTokens.All(token => {
              if (token.StartsWith("!")) {
                // Negation: must not match
                var negatedToken = token.Substring(1);
                return !tags.Contains(negatedToken, StringComparer.OrdinalIgnoreCase);
              } else {
                // Must match
                return tags.Contains(token, StringComparer.OrdinalIgnoreCase);
              }
            });
          } else {
            // Handle ',' (OR) condition
            if (filter.StartsWith("!")) {
              // Negation: must not match
              var negatedToken = filter.Substring(1);
              return !tags.Contains(negatedToken, StringComparer.OrdinalIgnoreCase);
            } else {
              // Must match
              return tags.Contains(filter, StringComparer.OrdinalIgnoreCase);
            }
          }
        });

        if (matches) {
          result.Add(entry);
        }
      }
      return result;
    }

    private void chkPresetLink_CheckedChanged(object sender, EventArgs e) {
      SendToMilkwaveVisualizer(chkPresetLink.Checked ? "1" : "0", MessageType.PresetLink);
    }

    private void lblLoad_DoubleClick(object sender, EventArgs e) {
      txtDirOrTagsFilter.Text = "";
    }

    private void lblTags_DoubleClick(object sender, EventArgs e) {
      txtTags.Text = "";
    }

    private void LoadAndSetSettings() {
      Location = Settings.RemoteWindowLocation;
      Size = Settings.RemoteWindowSize;
      toolStripMenuItemTabsPanel.Checked = Settings.ShowTabsPanel;
      toolStripMenuItemButtonPanel.Checked = Settings.ShowButtonPanel;
      txtDirOrTagsFilter.Text = Settings.DirOrTagsFilter;
      try {
        splitContainer1.SplitterDistance = Settings.SplitterDistance1;
      } catch (Exception) {
        // igonre
      }
    }

    private void SetAndSaveSettings() {
      if (WindowState == FormWindowState.Normal) {
        Settings.RemoteWindowLocation = Location;
        Settings.RemoteWindowSize = Size;
      } else {
        Settings.RemoteWindowLocation = RestoreBounds.Location;
        Settings.RemoteWindowSize = RestoreBounds.Size;
      }
      Settings.SplitterDistance1 = splitContainer1.SplitterDistance;
      Settings.SelectedTabIndex = tabControl.SelectedIndex;
      Settings.DirOrTagsFilter = txtDirOrTagsFilter.Text;

      SaveSettingsToFile();
    }

    private void numWaveR_ValueChanged(object? sender, EventArgs e) {
      UpdateWaveColorPicker();
      SendWaveInfoIfLinked();
    }

    private void numWaveG_ValueChanged(object? sender, EventArgs e) {
      UpdateWaveColorPicker();
      SendWaveInfoIfLinked();
    }

    private void numWaveB_ValueChanged(object? sender, EventArgs e) {
      UpdateWaveColorPicker();
      SendWaveInfoIfLinked();
    }

    private void UpdateWaveColorPicker() {
      int r = (int)numWaveR.Value;
      int g = (int)numWaveG.Value;
      int b = (int)numWaveB.Value;
      colorDialogWave.Color = Color.FromArgb(r, g, b);
      pnlColorWave.BackColor = colorDialogWave.Color;
    }

    private void btnWaveClear_Click(object sender, EventArgs e) {
      SendToMilkwaveVisualizer("", MessageType.WaveClear);
    }

    private void lblPushX_DoubleClick(object sender, EventArgs e) {
      numWavePushX.Value = 0;
    }

    private void lblPushY_DoubleClick(object sender, EventArgs e) {
      numWavePushY.Value = 0;
    }

    private void lblZoom_DoubleClick(object sender, EventArgs e) {
      numWaveZoom.Value = 1;
    }

    private void lblWarp_DoubleClick(object sender, EventArgs e) {
      numWaveWarp.Value = 0;
    }

    private void lblRotation_DoubleClick(object sender, EventArgs e) {
      numWaveRotation.Value = 0;
    }

    private void lblDecay_DoubleClick(object sender, EventArgs e) {
      numWaveDecay.Value = 0;
    }

    private void numWave_ValueChanged(object sender, EventArgs e) {
      SendWaveInfoIfLinked();
    }

    private void numWave_KeyDown(object sender, KeyEventArgs e) {
      if (e.KeyCode == Keys.Enter) {
        e.SuppressKeyPress = true; // Prevent the beep sound on Enter key press
        SendWaveInfoIfLinked();
      }
    }

    private void btnWaveQuicksave_Click(object sender, EventArgs e) {
      SendToMilkwaveVisualizer("", MessageType.WaveQuickSave);
    }

    private void lblAudioDevice_Click(object sender, EventArgs e) {

    }
  } // end class
} // end namespace