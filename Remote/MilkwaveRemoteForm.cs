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

    // Custom window messages for next/previous preset
    // must match definitions in Milkwave Visualizer
    private const int WM_NEXT_PRESET = 0x0400 + 100;
    private const int WM_PREV_PRESET = 0x0400 + 101;

    private const uint WM_KEYDOWN = 0x0100;

    private DarkModeCS dm;

    private System.Windows.Forms.Timer autoplayTimer;
    private int currentAutoplayIndex = 0;
    private int lastLineIndex = 0;
    private int lastReceivedShaderErrorLineNumber = -1;
    private float autoplayRemainingBeats = 1;
    private bool updatingWaveParams = false;

#if DEBUG
    private string BaseDir = Path.GetFullPath(Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "..\\..\\..\\..\\Release"));
#else
    private string BaseDir = AppDomain.CurrentDomain.BaseDirectory;
#endif

    private string VisualizerPresetsFolder = "";
    private string ShaderFilesFolder = "";
    private string PresetsShaderConvFolder = "";

    private string lastScriptFileName = "script-default.txt";
    private string windowNotFound = "Milkwave Visualizer Window not found";
    private string foundWindowTitle = "";
    private string defaultFontName = "Segoe UI";

    private string milkwaveSettingsFile = "settings-remote.json";
    private string milkwaveTagsFile = "tags-remote.json";

    // please request your own appKey at https://www.shadertoy.com/howto if you build your own version
    private string shadertoyAppKey = "ftrlhm";
    private string shadertoyQueryType = "";
    private int shadertoyQueryPageSize = 500;
    private string ShaderinfoLinePrefix = "// Shaderinfo: ";

    private List<String> shadertoyQueryList = new List<String>();

    Random rnd = new Random();
    private Settings Settings = new Settings();
    private Tags Tags = new Tags();
    private Shader Shader = new Shader();
    private RemoteHelper helper;

    private OpenFileDialog ofd;
    private OpenFileDialog ofdShader;
    private OpenFileDialog ofdShaderHLSL;

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
      Raw,
      Message,
      PresetFilePath,
      PresetLink,
      Amplify,
      Wave,
      WaveClear,
      WaveQuickSave,
      AudioDevice,
      Opacity,
      GetState,
      Config,
      TestFonts,
      ClearSprites,
      ClearTexts,
      TimeFactor,
      FrameFactor,
      FpsFactor,
      VisIntensity,
      VisShift
    }

    private void SetAllControlFontSizes(Control parent, float fontSize) {
      foreach (Control ctrl in parent.Controls) {
        ctrl.Font = new Font(ctrl.Font.FontFamily, fontSize, ctrl.Font.Style);
        if (ctrl.HasChildren) {
          SetAllControlFontSizes(ctrl, fontSize);
        }
      }
    }

    public MilkwaveRemoteForm() {
      InitializeComponent();

      VisualizerPresetsFolder = Path.Combine(BaseDir, "resources\\presets\\");
      ShaderFilesFolder = Path.Combine(BaseDir, "resources\\shader\\");
      PresetsShaderConvFolder = Path.Combine(VisualizerPresetsFolder, "Milkwave\\Shader\\Conv\\");

      FixNumericUpDownMouseWheel(this);

      Assembly executingAssembly = Assembly.GetExecutingAssembly();
      var fieVersionInfo = FileVersionInfo.GetVersionInfo(executingAssembly.Location);
      var version = fieVersionInfo.FileVersion;
      toolStripMenuItemHomepage.Text = $"Milkwave {version}";

      try {
        string jsonString = File.ReadAllText(Path.Combine(BaseDir, milkwaveSettingsFile));
        Settings? loadedSettings = JsonSerializer.Deserialize<Settings>(jsonString, new JsonSerializerOptions {
          PropertyNameCaseInsensitive = true
        });
        if (loadedSettings != null) {
          Settings = loadedSettings;
        }
        string tagsFile = Path.Combine(BaseDir, milkwaveTagsFile);
        jsonString = File.ReadAllText(tagsFile);
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
        cboParameters.Text = "size=20|time=5.0|x=0.5|y=0.5|growth=2";
      }

      // Fill cboFonts with available system fonts and add a blank first entry  
      cboFonts.Items.Add(""); // Add a blank first entry  
      using (InstalledFontCollection fontsCollection = new InstalledFontCollection()) {
        foreach (FontFamily font in fontsCollection.Families) {
          cboFonts.Items.Add(font.Name);
          cboFont1.Items.Add(font.Name);
          cboFont2.Items.Add(font.Name);
          cboFont3.Items.Add(font.Name);
          cboFont4.Items.Add(font.Name);
          cboFont5.Items.Add(font.Name);
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

      cboShadertoyType.SelectedIndex = 0;
    }

    private void MilkwaveRemoteForm_Load(object sender, EventArgs e) {
      LoadAndSetSettings();
      SetPanelsVisibility();

#if DEBUG
      //cboShadertoyURL.Text = "w3KGRK";
#else
      StartVisualizerIfNotFound(true);
#endif

      ofd = new OpenFileDialog();
      ofd.Filter = "MilkDrop Presets|*.milk;*.milk2|All files (*.*)|*.*";
      ofd.RestoreDirectory = true;
      SetAllControlFontSizes(this, 9f); // Sets all controls to font size 9

      txtShaderHLSL.Font = new Font(txtShaderHLSL.Font.FontFamily, 10f, txtShaderHLSL.Font.Style);
      txtShaderGLSL.Font = txtShaderHLSL.Font;

      helper = new RemoteHelper(Path.Combine(BaseDir, "settings.ini"));
      helper.FillAudioDevices(cboAudioDevice);
    }

    private IntPtr StartVisualizerIfNotFound(bool onlyIfNotFound) {
      bool doOpen = false;

      IntPtr result = FindVisualizerWindow();
      if (result == IntPtr.Zero || !onlyIfNotFound) {
        // Try to run MilkwaveVisualizer.exe from the same directory as the assembly
        string visualizerPath = Path.Combine(BaseDir, "MilkwaveVisualizer.exe");
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

      ofd = new OpenFileDialog();
      if (Directory.Exists(VisualizerPresetsFolder)) {
        ofd.InitialDirectory = VisualizerPresetsFolder;
      } else {
        ofd.InitialDirectory = BaseDir;
      }

      ofdShader = new OpenFileDialog();
      ofdShader.Filter = "GLSL files|*.glsl|All files (*.*)|*.*";
      ofdShader.InitialDirectory = Path.Combine(BaseDir, ShaderFilesFolder);

      ofdShaderHLSL = new OpenFileDialog();
      ofdShaderHLSL.Filter = "Presets or HLSL files|*.milk;*.hlsl|All files (*.*)|*.*";

      string MilkwavePresetsFolder = Path.Combine(VisualizerPresetsFolder, "Milkwave");
      if (Directory.Exists(MilkwavePresetsFolder)) {
        LoadPresetsFromDirectory(MilkwavePresetsFolder, true);
      }

      if (Settings.LoadFilters?.Count > 0) {
        ReloadLoadFiltersList(false);
        cboTagsFilter.SelectedIndex = 0;
      }

      if (Settings.ShadertoyIDs?.Count > 0) {
        ReloadShadertoyIDsList(false);
        cboShadertoyID.SelectedIndex = 0;
      }

      picShaderError.Image = SystemIcons.GetStockIcon(StockIconId.Warning, 64).ToBitmap();
      picShaderError.Visible = false;
      LoadVisualizerSettings();

      numShadertoyQueryIndex.Maximum = shadertoyQueryPageSize;

      SendToMilkwaveVisualizer("", MessageType.GetState);
    }

    protected override void WndProc(ref Message m) {
      if (m.Msg == WM_NEXT_PRESET) {
        if (chkPresetRandom.Checked) {
          SelectRandomPreset();
        } else {
          SelectNextPreset();
        }
        btnPresetSend_Click(null, null);
      } else if (m.Msg == WM_PREV_PRESET) {
        SelectPreviousPreset();
        btnPresetSend_Click(null, null);
      } else if (m.Msg == WM_COPYDATA) {
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
                  } else if (key.Equals("SCALE", StringComparison.OrdinalIgnoreCase)) {
                    numWaveScale.Value = decimal.Parse(value, CultureInfo.InvariantCulture);
                  } else if (key.Equals("ECHO", StringComparison.OrdinalIgnoreCase)) {
                    numWaveEcho.Value = decimal.Parse(value, CultureInfo.InvariantCulture);
                  } else if (key.Equals("BRIGHTEN", StringComparison.OrdinalIgnoreCase)) {
                    chkWaveBrighten.Checked = value.Equals("1", StringComparison.OrdinalIgnoreCase);
                  } else if (key.Equals("DARKEN", StringComparison.OrdinalIgnoreCase)) {
                    chkWaveDarken.Checked = value.Equals("1", StringComparison.OrdinalIgnoreCase);
                  } else if (key.Equals("SOLARIZE", StringComparison.OrdinalIgnoreCase)) {
                    chkWaveSolarize.Checked = value.Equals("1", StringComparison.OrdinalIgnoreCase);
                  } else if (key.Equals("INVERT", StringComparison.OrdinalIgnoreCase)) {
                    chkWaveInvert.Checked = value.Equals("1", StringComparison.OrdinalIgnoreCase);
                  } else if (key.Equals("ADDITIVE", StringComparison.OrdinalIgnoreCase)) {
                    chkWaveAdditive.Checked = value.Equals("1", StringComparison.OrdinalIgnoreCase);
                  } else if (key.Equals("DOTTED", StringComparison.OrdinalIgnoreCase)) {
                    chkWaveDotted.Checked = value.Equals("1", StringComparison.OrdinalIgnoreCase);
                  } else if (key.Equals("THICK", StringComparison.OrdinalIgnoreCase)) {
                    chkWaveThick.Checked = value.Equals("1", StringComparison.OrdinalIgnoreCase);
                  } else if (key.Equals("VOLALPHA", StringComparison.OrdinalIgnoreCase)) {
                    chkWaveVolAlpha.Checked = value.Equals("1", StringComparison.OrdinalIgnoreCase);
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
              SetRunningPresetText(displayText);
              toolTip1.SetToolTip(txtVisRunning, presetFilePath);
              UpdateTagsDisplay(false, true);
            }
          } else if (receivedString.StartsWith("STATUS=")) {
            string status = receivedString.Substring(receivedString.IndexOf("=") + 1);
            if (status.Length > 0) {
              SetStatusText(status);
            }
            if (status.Contains(": error ")) {
              string errLine = status.Substring(1, status.IndexOf(")") - 1);
              if (int.TryParse(errLine, out int lineNumber)) {
                if (lineNumber > 0) {
                  lastReceivedShaderErrorLineNumber = lineNumber;
                  MarkRow(lineNumber - (int)numOffset.Value);
                }
              }
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
          }
        }
      }

      base.WndProc(ref m);
    }

    private void SetRunningPresetText(string displayText) {
      txtVisRunning.Text = displayText.Replace("PRESET=", "");
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
        SendToMilkwaveVisualizer(txtMessage.Text, MessageType.Raw);
      } else {
        SendToMilkwaveVisualizer(txtMessage.Text, MessageType.Message);
      }

      txtMessage.Focus();
      txtMessage.SelectAll();
    }

    bool SendingMessage = false;

    private void SendToMilkwaveVisualizer(string messageToSend, MessageType type) {
      SetStatusText("");
      string partialTitle = cboWindowTitle.Text;
      string statusMessage = "";

      try {
        if (!SendingMessage) {
          SendingMessage = true;
          IntPtr foundWindow = FindVisualizerWindow();
          if (foundWindow != IntPtr.Zero) {
            string message = "";
            if (type == MessageType.Wave) {
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
                "|DECAY=" + numWaveDecay.Value.ToString(CultureInfo.InvariantCulture) +
                "|SCALE=" + numWaveScale.Value.ToString(CultureInfo.InvariantCulture) +
                "|ECHO=" + numWaveEcho.Value.ToString(CultureInfo.InvariantCulture) +
                "|BRIGHTEN=" + (chkWaveBrighten.Checked ? "1" : "0") +
                "|DARKEN=" + (chkWaveDarken.Checked ? "1" : "0") +
                "|SOLARIZE=" + (chkWaveSolarize.Checked ? "1" : "0") +
                "|INVERT=" + (chkWaveInvert.Checked ? "1" : "0") +
                "|ADDITIVE=" + (chkWaveAdditive.Checked ? "1" : "0") +
                "|DOTTED=" + (chkWaveDotted.Checked ? "1" : "0") +
                "|THICK=" + (chkWaveThick.Checked ? "1" : "0") +
                "|VOLALPHA=" + (chkWaveVolAlpha.Checked ? "1" : "0");
              statusMessage = $"Changed Wave in";
            } else if (type == MessageType.PresetFilePath) {
              message = "PRESET=" + messageToSend;
              string fileName = Path.GetFileNameWithoutExtension(messageToSend);
              statusMessage = $"Sent preset \"{fileName}\" to";
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
                  if (selectedItem.IsInputDevice) {
                    message = "DEVICE=IN|" + selectedItem.Device.FriendlyName;
                  } else {
                    message = "DEVICE=OUT|" + selectedItem.Device.FriendlyName;
                  }
                  //statusMessage = $"Set device '{cboAudioDevice.Text}' in";
                }
              }
            } else if (type == MessageType.Opacity) {
              decimal val = numOpacity.Value / 100;
              message = "OPACITY=" + val.ToString(CultureInfo.InvariantCulture);
            } else if (type == MessageType.GetState) {
              message = "STATE";
            } else if (type == MessageType.WaveClear) {
              message = "CLEARPRESET";
            } else if (type == MessageType.WaveQuickSave) {
              message = "QUICKSAVE";
            } else if (type == MessageType.Config) {
              message = "CONFIG";
            } else if (type == MessageType.TestFonts) {
              message = "TESTFONTS";
            } else if (type == MessageType.ClearSprites) {
              message = "CLEARSPRITES";
            } else if (type == MessageType.ClearTexts) {
              message = "CLEARTEXTS";
            } else if (type == MessageType.TimeFactor) {
              message = "VAR_TIME=" + numFactorTime.Value.ToString(CultureInfo.InvariantCulture);
            } else if (type == MessageType.FrameFactor) {
              message = "VAR_FRAME=" + numFactorFrame.Value.ToString(CultureInfo.InvariantCulture);
            } else if (type == MessageType.FpsFactor) {
              message = "VAR_FPS=" + numFactorFPS.Value.ToString(CultureInfo.InvariantCulture);
            } else if (type == MessageType.VisIntensity) {
              message = "VAR_INTENSITY=" + numVisIntensity.Value.ToString(CultureInfo.InvariantCulture);
            } else if (type == MessageType.VisShift) {
              message = "VAR_SHIFT=" + numVisShift.Value.ToString(CultureInfo.InvariantCulture);
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
              statusMessage = $"Sent '{messageToSend}' to";
            } else if (type == MessageType.Raw) {
              message = messageToSend;
              statusMessage = $"Sent '{messageToSend}' to";
            }

            // if line doesn't contain font face, size or color, use form-defined values
            if (type == MessageType.Message || type == MessageType.Raw) {
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
      } finally {
        SendingMessage = false;
      }
    }

    private void SetStatusText(string text) {
      text = text
        .Replace(" " + Environment.NewLine, Environment.NewLine)
        .Replace(Environment.NewLine + " ", Environment.NewLine)
        .Replace(Environment.NewLine, " // ").Replace("&", "&&").Trim();
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
            } else if (tokenUpper.StartsWith("SIZE=")) {
              string size = token.Substring(5);
              if (float.TryParse(size, out float parsedSize)) {
                numSize.Value = (decimal)parsedSize;
              }
            } else if (tokenUpper.StartsWith("COLOR=")) {
              string colorRGB = token.Substring(6);
              string[] valuesRGB = colorRGB.Split(",");
              if (valuesRGB.Length == 3 &&
                  int.TryParse(valuesRGB[0], out int r) &&
                  int.TryParse(valuesRGB[1], out int g) &&
                  int.TryParse(valuesRGB[2], out int b)) {
                pnlColorMessage.BackColor = Color.FromArgb(r, g, b);
              }
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
                presetFilePath = Path.Combine(BaseDir, presetFilePath);
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
            } else if (tokenUpper.StartsWith("MSG=")) {
              string sendString = "MSG|" + token.Substring(4).Replace(";", "|");
              SendToMilkwaveVisualizer(sendString, MessageType.Raw);
            } else if (tokenUpper.Equals("CLEARSPRITES")) {
              SendToMilkwaveVisualizer("", MessageType.ClearSprites);
            } else if (tokenUpper.Equals("CLEARTEXTS")) {
              SendToMilkwaveVisualizer("", MessageType.ClearTexts);
            } else if (tokenUpper.Equals("CLEARPARAMS")) {
              cboParameters.Text = "";
            } else if (tokenUpper.StartsWith("TIME=")) {
              string value = token.Substring(5);
              if (float.TryParse(value, NumberStyles.Number, CultureInfo.InvariantCulture, out float parsedValue)) {
                numFactorTime.Value = (decimal)parsedValue;
              }
            } else if (tokenUpper.StartsWith("FRAME=")) {
              string value = token.Substring(6);
              if (float.TryParse(value, NumberStyles.Number, CultureInfo.InvariantCulture, out float parsedValue)) {
                numFactorFrame.Value = (decimal)parsedValue;
              }
            } else if (tokenUpper.StartsWith("FPS=")) {
              string value = token.Substring(4);
              if (float.TryParse(value, NumberStyles.Number, CultureInfo.InvariantCulture, out float parsedValue)) {
                numFactorFPS.Value = (decimal)parsedValue;
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
        filePath = Path.Combine(BaseDir, fileName);
      }
      if (File.Exists(filePath)) {
        string[] strings = File.ReadAllLines(filePath);
        foreach (string line in strings) {
          if (!line.StartsWith("#")) {
            cboAutoplay.Items.Add(line);
          }
        }
        SetStatusText("Loaded " + cboAutoplay.Items.Count + " lines from " + fileName);
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
          chkAutoplay.Enabled = true;
        }
        toolTip1.SetToolTip(cboAutoplay, cboAutoplay.Text);
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

        // Supported combos:
        // Shift + Ctrl
        // Shift
        // Alt
        // Ctrl
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
        } else if (doCtrl) {
          inputs = new INPUT[4];

          inputs[0] = new INPUT {
            type = 1, // Keyboard input
            u = new InputUnion {
              ki = new KEYBDINPUT {
                wVk = VK_CTRL,
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
                wVk = VK_CTRL,
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
        } else if (e.KeyCode == Keys.B) {
          e.SuppressKeyPress = true;
          toolStripMenuItemButtonPanel_Click(null, null);
        } else if (e.KeyCode == Keys.D) {
          btnPresetLoadDirectory_Click(null, null);
        } else if (e.KeyCode == Keys.F) {
          if (tabControl.SelectedTab.Name.Equals("tabShader")) {
            e.SuppressKeyPress = true; // Prevent the beep sound on Enter key press
            if ((Control.ModifierKeys & Keys.Shift) == Keys.Shift) {
              txtShaderFind.SelectAll();
              txtShaderFind.Focus();
            } else if ((Control.ModifierKeys & Keys.Alt) == Keys.Alt) {
              txtShaderHLSL.Text = Shader.BasicFormatShaderCode(txtShaderHLSL.Text);
              SetStatusText("HLSL code formatted");
            } else {
              FindShaderString();
            }
          }
        } else if (e.KeyCode == Keys.L) {
          if (tabControl.SelectedTab.Name.Equals("tabShader")) {
            e.SuppressKeyPress = true; // Prevent the beep sound on Enter key press
            LoadShadertoyQuery();
          }
        } else if (e.KeyCode == Keys.N) {
          SelectNextPreset();
          btnPresetSend_Click(null, null);
        } else if (e.KeyCode == Keys.O) {
          e.SuppressKeyPress = true;
          StartVisualizerIfNotFound(false);
        } else if (e.KeyCode == Keys.P) {
          btnPresetSend_Click(null, null);
        } else if (e.KeyCode == Keys.S) {
          if (tabControl.SelectedTab.Name.Equals("tabShader")) {
            e.SuppressKeyPress = true; // Prevent the beep sound on Enter key press
            btnSendShader_Click(null, null);
          } else if (tabControl.SelectedTab.Name.Equals("tabMessage")) {
            SendToMilkwaveVisualizer(txtMessage.Text, MessageType.Message);
          }
        } else if (e.KeyCode == Keys.T) {
          btnTagsSave_Click(null, null);
        } else if (e.KeyCode == Keys.X) {
          if (tabControl.SelectedTab.Name.Equals("tabMessage")) {
            btnSendFile_Click(null, null);
          }
          if ((Control.ModifierKeys & Keys.Shift) == Keys.Shift) {
            SelectNextAutoplayEntry();
          }
        } else if (e.KeyCode == Keys.Y) {
          e.SuppressKeyPress = true; // Prevent the beep sound on Enter key press
          if (tabControl.SelectedTab.Name.Equals("tabMessage")) {
            chkAutoplay.Checked = !chkAutoplay.Checked;
          } else {
            chkShaderLeft.Checked = !chkShaderLeft.Checked;
          }
        } else if (e.KeyCode == Keys.Tab) {
          if ((Control.ModifierKeys & Keys.Shift) == Keys.Shift) {
            // Switch to the previous tab
            int previousIndex = (tabControl.SelectedIndex - 1 + tabControl.TabPages.Count) % tabControl.TabPages.Count; // Loop back to the last tab if at the first
            tabControl.SelectedIndex = previousIndex;
          } else {
            int nextIndex = (tabControl.SelectedIndex + 1) % tabControl.TabPages.Count; // Loop back to the first tab if at the last
            tabControl.SelectedIndex = nextIndex;
          }
        } else if (e.KeyCode == Keys.Space) {
          if ((Control.ModifierKeys & Keys.Shift) == Keys.Shift) {
            SendPostMessage(VK_BACKSPACE, "Backspace");
          } else {
            SendPostMessage(VK_SPACE, "Space");
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
      if (MessageBox.Show(this, "Really remove all saved styles?",
          "Please Confirm", MessageBoxButtons.YesNo, MessageBoxIcon.Question, MessageBoxDefaultButton.Button1)
          == DialogResult.Yes) {
        Settings.Styles.Clear();
        ReloadStylesList();
        SetStatusText($"Saved presets cleared");
      }

    }

    private void lblParameters_MouseDown(object sender, MouseEventArgs e) {
      if (e.Button == MouseButtons.Right) {
        string helpText = "font={fontname}  // The font shaderName to use" + Environment.NewLine +
          "size={int:0..100}  // The font size (0=tiny, 100=enormous, 20-60=normal range)" + Environment.NewLine +
          "growth={float:0.25..4.0}  // The factor to grow or shrink over time (0.5=shrink to half-size, 2.0=grow to double size)" + Environment.NewLine +
          "x={float:0..1}  // The x-position of the center of the text (0.0=left side, 1.0=right side)" + Environment.NewLine +
          "y={float:0..1}  // The y-position of the center of the text (0.0=top, 1.0=bottom)" + Environment.NewLine +
          "randx={float:0..1}  // X-randomization: x will be bumped within +/- this value" + Environment.NewLine +
          "randy={float:0..1}  // Y-randomization: y will be bumped within +/- this value" + Environment.NewLine +
          "time={float}  // The duration (in seconds) the text will display" + Environment.NewLine +
          "fade={float}  // The duration (in seconds) spent fading in the text" + Environment.NewLine +
          "fadeout={float}  // The duration (in seconds) spent fading out the text" + Environment.NewLine +
          "ital={0|1}  // Font italics override (0=off, 1=on)" + Environment.NewLine +
          "bold={0|1}  // Font bold override (0=off, 1=on)" + Environment.NewLine +
          "r={int:0..255}  // Red color component for the font" + Environment.NewLine +
          "g={int:0..255}  // Green color component for the font" + Environment.NewLine +
          "b={int:0..255}  // Blue color component for the font" + Environment.NewLine +
          "randr={int:0..255}  // Randomization for the red component (r will be bumped within +/- this value)" + Environment.NewLine +
          "randg={int:0..255}  // Randomization for the green component (g will be bumped within +/- this value)" + Environment.NewLine +
          "randb={int:0..255}  // Randomization for the blue component (b will be bumped within +/- this value)" + Environment.NewLine +
          "" + Environment.NewLine +
          "New in Milkwave:" + Environment.NewLine +
          "startx={float}  // The x-position for text moving animation (can be negative)" + Environment.NewLine +
          "starty={float}  // The y-position for text moving animation (can be negative)" + Environment.NewLine +
          "movetime={float}  // The duration (in seconds) the text will move from startx/starty to x/y" + Environment.NewLine +
          "easemode={int:0|1|2}  // Moving animation smoothing: 0=linear, 1=ease-in, 2=ease-out (default=2)" + Environment.NewLine +
          "easefactor={float:1..5}  // Smoothing strengh (default=2.0)" + Environment.NewLine +
          "shadowoffset={float}  // Text drop shadow offsetNum: 0=no shadow (default=2.0)" + Environment.NewLine +
          "burntime={float}  // The duration (in seconds) the text will \"burn in\" at the end (default=0.1)" + Environment.NewLine;

        new MilkwaveInfoForm(toolStripMenuItemDarkMode.Checked).ShowDialog("Parameters", helpText, 9, 800, 600);
      }
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

    private void ReloadLoadFiltersList(bool addCuurent) {
      if (addCuurent && cboTagsFilter.Text.Length > 0 && !Settings.LoadFilters.Contains(cboTagsFilter.Text)) {
        Settings.LoadFilters.Insert(0, cboTagsFilter.Text);
        if (Settings.LoadFilters.Count > 5) {
          Settings.LoadFilters.RemoveAt(5);
        }
      }
      cboTagsFilter.Items.Clear();
      cboTagsFilter.Items.AddRange(Settings.LoadFilters.ToArray());
      cboTagsFilter.Refresh();
    }

    private void ReloadShadertoyIDsList(bool addCuurent) {
      if (addCuurent && cboShadertoyID.Text.Length > 0 && !Settings.ShadertoyIDs.Contains(cboShadertoyID.Text)) {
        Settings.ShadertoyIDs.Insert(0, cboShadertoyID.Text);
        if (Settings.ShadertoyIDs.Count > 5) {
          Settings.ShadertoyIDs.RemoveAt(5);
        }
      }
      cboShadertoyID.Items.Clear();
      cboShadertoyID.Items.AddRange(Settings.ShadertoyIDs.ToArray());
      cboShadertoyID.Refresh();
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
      string settingsFile = Path.Combine(BaseDir, milkwaveSettingsFile);
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
      string tagsFile = Path.Combine(BaseDir, milkwaveTagsFile);
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
        ofdScriptFile.InitialDirectory = BaseDir;

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
      StartVisualizerIfNotFound(true);
    }

    private void toolStripMenuItemReleases_Click(object sender, EventArgs e) {
      OpenURL("https://github.com/IkeC/Milkwave/releases");
    }

    private void OpenURL(string url) {
      try {
        Process.Start(new ProcessStartInfo(url) { UseShellExecute = true });
      } catch (Exception ex) {
        MessageBox.Show($"Unable to open URL: {url}" + Environment.NewLine + Environment.NewLine + ex.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
      }
    }

    private void toolStripMenuItemHelp_Click(object sender, EventArgs e) {
      string dialogtext =
  "There are many tooltips explaining all features when you move your mouse over all the form elements." + Environment.NewLine +
   Environment.NewLine +
  "Manual: https://github.com/IkeC/Milkwave/blob/main/Build/Manual.md" + Environment.NewLine +
  "Readme: https://github.com/IkeC/Milkwave/blob/main/Build/README.txt" + Environment.NewLine +
  Environment.NewLine +
  "GitHub homepage: https://github.com/IkeC/Milkwave" + Environment.NewLine +
  "GitHub issues: https://github.com/IkeC/Milkwave/issues" + Environment.NewLine +
  "Discord: https://bit.ly/Ikes-Discord" + Environment.NewLine +
  Environment.NewLine +
  "More Presets: https://github.com/projectM-visualizer/projectm?tab=readme-ov-file#presets" + Environment.NewLine +
  Environment.NewLine +
  "To uninstall Milkwave, run Uninstall.exe from the Milkwave folder.";
      new MilkwaveInfoForm(toolStripMenuItemDarkMode.Checked).ShowDialog("Milkwave Help", dialogtext, 10, 800, 400);
    }

    private void toolStripMenuItemSupporters_Click(object sender, EventArgs e) {
      string dialogtext =
  "Milkwave Supporters — Thank you very much!  ❤️" + Environment.NewLine +
  Environment.NewLine +
  "• Shanev" + Environment.NewLine +
  "• Tures1955" + Environment.NewLine +
  Environment.NewLine +
  "Milkwave is and will always be free software, being the collaborative effort of many diffent authors. " +
  "If you like it and want to appreciate and support our share of the work, please consider donating." + Environment.NewLine +
  Environment.NewLine +
  "Ko-fi: https://ko-fi.com/ikeserver" + Environment.NewLine +
  "PayPal: https://www.paypal.com/ncp/payment/5XMP3S69PJLCU" + Environment.NewLine +
  "" + Environment.NewLine +
  "Any amount is valued! You'll be listed on this page unless you do not want to.";
      new MilkwaveInfoForm(toolStripMenuItemDarkMode.Checked).ShowDialog("Supporters", dialogtext);
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

    private void toolStripMenuItemOpenVisualizer_Click(object sender, EventArgs e) {
      StartVisualizerIfNotFound(false);
    }

    private void toolStripMenuItemTabsPanel_Click(object sender, EventArgs e) {
      toolStripMenuItemTabsPanel.Checked = !toolStripMenuItemTabsPanel.Checked;
      if (!toolStripMenuItemTabsPanel.Checked && !toolStripMenuItemButtonPanel.Checked) {
        toolStripMenuItemButtonPanel.Checked = true;
      }
      SetPanelsVisibility();
    }

    private void toolStripMenuItemButtonPanel_Click(object? sender, EventArgs? e) {
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
      ReloadLoadFiltersList(true);
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
      ReloadLoadFiltersList(true);
      using (var fbd = new FolderBrowserDialog()) {
        if (Directory.Exists(VisualizerPresetsFolder)) {
          fbd.InitialDirectory = VisualizerPresetsFolder;
        } else {
          fbd.InitialDirectory = BaseDir;
        }
        DialogResult result = fbd.ShowDialog();

        if (result == DialogResult.OK && !string.IsNullOrWhiteSpace(fbd.SelectedPath)) {
          LoadPresetsFromDirectory(fbd.SelectedPath, false);
        }
      }
    }

    private void LoadPresetsFromDirectory(string dirToLoad, bool forceIncludeSubdirs) {
      cboPresets.Items.Clear();
      cboPresets.Text = "";
      bool includeSubdirs = false;

      if (Directory.GetDirectories(dirToLoad).Length > 0) {
        if (forceIncludeSubdirs || MessageBox.Show(this, "Include subdirectories?",
          "Please Confirm", MessageBoxButtons.YesNo, MessageBoxIcon.Question, MessageBoxDefaultButton.Button1) == DialogResult.Yes) {
          includeSubdirs = true;
        }
      }
      FillCboPresetsFromDir(dirToLoad, includeSubdirs, "");
      SetStatusText($"Loaded {cboPresets.Items.Count} presets from '{dirToLoad}'");
      if (cboPresets.Items.Count > 0) {
        cboPresets.SelectedIndex = 0;
        UpdateTagsDisplay(false, false);
      }
    }

    private void FillCboPresetsFromDir(string dirToLoad, bool includeSubdirs, string displayDirPrefix) {
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
          Data.Preset newPreset = new Data.Preset {
            DisplayName = displayDirPrefix + fileNameOnlyNoExtension,
            MaybeRelativePath = fileNameMaybeRelativePath
          };
          if (txtFilter.Text.ToUpper().StartsWith("AGE=")) {
            if (Int32.TryParse(txtFilter.Text.Substring(4), out int age) && age > 0) {
              DateTime lastFileWriteTime = File.GetLastWriteTime(fileName);
              if ((DateTime.Now - lastFileWriteTime).TotalDays < age) {
                // Check if the preset already exists in the combo box
                if (!cboPresets.Items.Contains(newPreset)) {
                  cboPresets.Items.Add(newPreset);
                }
              }
            }
          } else if (String.IsNullOrEmpty(txtFilter.Text) || fileNameOnlyNoExtension.Contains(txtFilter.Text, StringComparison.InvariantCultureIgnoreCase)) {
            // Check if the preset already exists in the combo box
            if (!cboPresets.Items.Contains(newPreset)) {
              cboPresets.Items.Add(newPreset);
            }
          }
        }
      }
      if (includeSubdirs) {
        foreach (string subDir in Directory.GetDirectories(dirToLoad)) {
          string? prefix = Path.GetFileName(subDir) + "\\";
          FillCboPresetsFromDir(subDir, true, prefix);
        }
      }
    }

    private void cboPresets_KeyDown(object sender, KeyEventArgs e) {
      if (e.KeyCode == Keys.Enter) {
        e.SuppressKeyPress = true; // Prevent the beep sound on Enter key press
        btnPresetSend.PerformClick();
      }
    }

    private void lblPreset_Click(object sender, EventArgs e) {
      String fullPath = "";
      if (cboPresets.SelectedItem != null) {
        Data.Preset preset = (Data.Preset)cboPresets.SelectedItem;
        if (!string.IsNullOrEmpty(preset.MaybeRelativePath)) {
          fullPath = preset.MaybeRelativePath;
          if (!Path.IsPathRooted(fullPath)) {
            fullPath = Path.Combine(VisualizerPresetsFolder, preset.MaybeRelativePath);
          }
        }
      }

      if (fullPath.Length > 0) {
        if ((Control.ModifierKeys & Keys.Control) == Keys.Control) {
          OpenFile(fullPath);
        } else {
          Clipboard.SetText(fullPath);
          SetStatusText($"Copied '{fullPath}' to clipboard");
        }
      } else {
        // Handle the case where the selected item is not a Preset
        SetStatusText("No valid preset selected");
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
      SetExpIncrements(numAmpLeft);
      SendToMilkwaveVisualizer("", MessageType.Amplify);
    }

    private void numAmpRight_ValueChanged(object sender, EventArgs e) {
      if (chkAmpLinked.Checked) {
        numAmpLeft.Value = numAmpRight.Value;
      }
      SetExpIncrements(numAmpRight);
      SendToMilkwaveVisualizer("", MessageType.Amplify);
    }

    private void SetExpIncrements(NumericUpDown nud) {
      // Ensure the Tag property is cast to decimal before comparison
      decimal previousValue = nud.Tag is decimal tagValue ? tagValue : 0;
      bool up = previousValue < nud.Value;

      if (nud.Value < 0.1M || (nud.Value == 0.1M && !up)) {
        nud.Increment = 0.01M;
      } else if (nud.Value < 2M || (nud.Value == 2M && !up)) {
        nud.Increment = 0.1M;
      } else if (nud.Value < 10M || (nud.Value == 10M && !up)) {
        nud.Increment = 1M;
      } else if (nud.Value < 100M || (nud.Value == 100M && !up)) {
        nud.Increment = 5M;
      } else {
        nud.Increment = 10M;
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

    private void lblCurrentPreset_Click(object sender, EventArgs e) {
      string? text = toolTip1.GetToolTip(txtVisRunning);
      if (!string.IsNullOrEmpty(text)) {
        if ((Control.ModifierKeys & Keys.Control) == Keys.Control) {
          OpenFile(text);
        } else {
          Clipboard.SetText(text);
          SetStatusText($"Copied '{text}' to clipboard");
        }
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

    private void btnWatermark_MouseDown(object sender, MouseEventArgs e) {
      if (e.Button == MouseButtons.Right) {
        SendInput(VK_F9, "F9", false, false, true);
      }
    }

    private void btnTagsSave_Click(object? sender, EventArgs? e) {
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
      List<(string Tag, int Count)> sortedTags = GetTagsListSortedByCount();

      if (sortedTags.Count > 0) {
        SetButtonTagInfo(btnTag1, sortedTags[0]);
      } else {
        btnTag1.Tag = null;
        btnTag1.Text = "";
      }

      if (sortedTags.Count > 1) {
        SetButtonTagInfo(btnTag2, sortedTags[1]);
      } else {
        btnTag2.Tag = null;
        btnTag2.Text = "";
      }

      if (sortedTags.Count > 2) {
        SetButtonTagInfo(btnTag3, sortedTags[2]);
      } else {
        btnTag3.Tag = null;
        btnTag3.Text = "";
      }

      if (sortedTags.Count > 3) {
        SetButtonTagInfo(btnTag4, sortedTags[3]);
      } else {
        btnTag4.Tag = null;
        btnTag4.Text = "";
      }

      if (sortedTags.Count > 4) {
        SetButtonTagInfo(btnTag5, sortedTags[4]);
      } else {
        btnTag5.Tag = null;
        btnTag5.Text = "";
      }

      if (sortedTags.Count > 5) {
        SetButtonTagInfo(btnTag6, sortedTags[5]);
      } else {
        btnTag6.Tag = null;
        btnTag6.Text = "";
      }

      if (sortedTags.Count > 6) {
        SetButtonTagInfo(btnTag7, sortedTags[6]);
      } else {
        btnTag7.Tag = null;
        btnTag7.Text = "";
      }

      if (sortedTags.Count > 7) {
        SetButtonTagInfo(btnTag8, sortedTags[7]);
      } else {
        btnTag8.Tag = null;
        btnTag8.Text = "";
      }

      if (sortedTags.Count > 8) {
        SetButtonTagInfo(btnTag9, sortedTags[8]);
      } else {
        btnTag9.Tag = null;
        btnTag9.Text = "";
      }

      if (sortedTags.Count > 9) {
        SetButtonTagInfo(btnTag10, sortedTags[9]);
      } else {
        btnTag10.Tag = null;
        btnTag10.Text = "";
      }
    }

    private List<(string Tag, int Count)> GetTagsListSortedByCount() {
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
      return sortedTags;
    }

    private void SetButtonTagInfo(Button button, (string Tag, int Count) tagInfo) {
      string text = "Add/remove " + tagInfo.Tag.ToUpper() +
        Environment.NewLine + "Used " + tagInfo.Count + " times" +
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

    private void btnTag_Click(object sender, EventArgs e) {
      AddOrRemoveTopTag(sender);
    }

    private void AddOrRemoveTopTag(object sender) {
      Control srcCombobox = txtTags;
      Char tokenChar = ',';
      string joinSep = ", ";
      if ((Control.ModifierKeys & Keys.Control) == Keys.Control) {
        srcCombobox = cboTagsFilter;
      }
      if ((Control.ModifierKeys & Keys.Shift) == Keys.Shift) {
        srcCombobox = cboTagsFilter;
        tokenChar = '&';
        joinSep = " & ";
      }
      if (sender is Button button && button.Tag is string tag && tag.Length > 0) {

        // Split txtTags.Text into tokens, trimming whitespace
        var tokens = srcCombobox.Text
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
        srcCombobox.Text = string.Join(joinSep, tokens);
      }
    }

    private void lblAudioDevice_MouseClick(object sender, MouseEventArgs e) {
      if (e.Button == MouseButtons.Right) {
        helper.SelectDefaultDevice(cboAudioDevice);
        btnSetAudioDevice_Click(null, null);
        SetStatusText($"Default audio device selected and set");
      }
    }

    private void btnPresetLoadTags_Click(object? sender, EventArgs? e) {
      ReloadLoadFiltersList(true);
      cboPresets.Items.Clear();
      var presetList = new List<Data.Preset>();
      if (cboTagsFilter.Text.Length > 0) {
        var filteredEntries = FilterTagEntries();
        foreach (var entry in filteredEntries) {
          if (String.IsNullOrEmpty(txtFilter.Text) || entry.Key.Contains(txtFilter.Text, StringComparison.InvariantCultureIgnoreCase)) {
            Data.Preset newPreset = new Data.Preset {
              DisplayName = entry.Key,
              MaybeRelativePath = entry.Value.PresetPath
            };
            presetList.Add(newPreset);
          }
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
      var filterText = cboTagsFilter.Text.Trim();

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
      cboTagsFilter.Text = "";
    }

    private void lblTags_DoubleClick(object sender, EventArgs e) {
      txtTags.Text = "";
    }

    private void LoadAndSetSettings() {
      Location = Settings.RemoteWindowLocation;
      Size = Settings.RemoteWindowSize;
      toolStripMenuItemTabsPanel.Checked = Settings.ShowTabsPanel;
      toolStripMenuItemButtonPanel.Checked = Settings.ShowButtonPanel;
      try {
        splitContainer1.SplitterDistance = Settings.SplitterDistance1;
      } catch (Exception) {
        // igonre
      }
      chkShaderFile.Checked = Settings.ShaderFileChecked;
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
      Settings.ShaderFileChecked = chkShaderFile.Checked;
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

    private void ctrlWave_ValueChanged(object sender, EventArgs e) {
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

    private void lblEcho_DoubleClick(object sender, EventArgs e) {
      numWaveEcho.Value = 2;
    }

    private void lblScale_DoubleClick(object sender, EventArgs e) {
      numWaveScale.Value = 1;
    }

    private void lblLoad_MouseDown(object sender, MouseEventArgs e) {
      if (e.Button == MouseButtons.Right) {
        Settings.LoadFilters.Clear();
        ReloadLoadFiltersList(false);
      }
    }

    private void cboDirOrTagsFilter_KeyDown(object sender, KeyEventArgs e) {
      if (e.KeyCode == Keys.Enter) {
        e.SuppressKeyPress = true; // Prevent the beep sound on Enter key press
        btnPresetLoadTags_Click(null, null);
      }
    }

    private void txtTags_Enter(object sender, EventArgs e) {
      if (txtTags.Text.Length > 0 && !txtTags.Text.Trim().EndsWith(",")) {
        txtTags.Text = txtTags.Text.Trim() + ", ";
      }
    }

    private void txtTags_Leave(object sender, EventArgs e) {
      if (txtTags.Text.Length > 0 && txtTags.Text.Trim().EndsWith(",")) {
        txtTags.Text = txtTags.Text.Trim().Substring(0, txtTags.Text.Trim().Length - 1);
      }
    }

    private void lblMostUsed_DoubleClick(object sender, EventArgs e) {
      string dialogtext = "";
      List<(string Tag, int Count)> sortedTags = GetTagsListSortedByCount();
      foreach ((string Tag, int Count) tagInfo in sortedTags) {
        dialogtext += tagInfo.Tag.ToUpper() +
          " used " + tagInfo.Count +
          " time" + (tagInfo.Count > 1 ? "s" : "") + Environment.NewLine;
      }
      if (dialogtext.EndsWith(Environment.NewLine)) {
        dialogtext = dialogtext.Substring(0, dialogtext.Length - Environment.NewLine.Length);
      }
      new MilkwaveInfoForm(toolStripMenuItemDarkMode.Checked).ShowDialog("Tag Statistics", dialogtext);
    }

    private void LoadVisualizerSettings() {
      try {
        // Notify
        string fontFace1 = helper.GetIniValueFonts("FontFace1", "Bahnschrift");
        cboFont1.SelectedItem = fontFace1;
        string fontSize1 = helper.GetIniValueFonts("FontSize1", "20");
        numFont1.Value = int.Parse(fontSize1);

        string fontBold1 = helper.GetIniValueFonts("FontBold1", "0");
        chkFontBold1.Checked = !fontBold1.Equals("0");
        string fontItalic1 = helper.GetIniValueFonts("FontItalic1", "0");
        chkFontItalic1.Checked = !fontItalic1.Equals("0");
        string fontAA1 = helper.GetIniValueFonts("FontAA1", "1");
        chkFontAA1.Checked = !fontAA1.Equals("0");

        string fontColorR1 = helper.GetIniValueFonts("FontColorR1", "255");
        int fontColorR1Val = int.Parse(fontColorR1);
        string fontColorG1 = helper.GetIniValueFonts("FontColorG1", "255");
        int fontColorG1Val = int.Parse(fontColorG1);
        string fontColorB1 = helper.GetIniValueFonts("FontColorB1", "0");
        int fontColorB1Val = int.Parse(fontColorB1);
        pnlColorFont1.BackColor = Color.FromArgb(fontColorR1Val, fontColorG1Val, fontColorB1Val);

        // Preset
        string fontFace2 = helper.GetIniValueFonts("FontFace2", "Bahnschrift");
        cboFont2.SelectedItem = fontFace2;
        string fontSize2 = helper.GetIniValueFonts("FontSize2", "25");
        numFont2.Value = int.Parse(fontSize2);

        string fontBold2 = helper.GetIniValueFonts("FontBold2", "0");
        chkFontBold2.Checked = !fontBold2.Equals("0");
        string fontItalic2 = helper.GetIniValueFonts("FontItalic2", "0");
        chkFontItalic2.Checked = !fontItalic2.Equals("0");
        string fontAA2 = helper.GetIniValueFonts("FontAA2", "1");
        chkFontAA2.Checked = !fontAA2.Equals("0");

        string fontColorR2 = helper.GetIniValueFonts("FontColorR2", "255");
        int fontColorR2Val = int.Parse(fontColorR2);
        string fontColorG2 = helper.GetIniValueFonts("FontColorG2", "86");
        int fontColorG2Val = int.Parse(fontColorG2);
        string fontColorB2 = helper.GetIniValueFonts("FontColorB2", "0");
        int fontColorB2Val = int.Parse(fontColorB2);
        pnlColorFont2.BackColor = Color.FromArgb(fontColorR2Val, fontColorG2Val, fontColorB2Val);

        // Artist: Ini-Index is 5!
        string fontFace3 = helper.GetIniValueFonts("FontFace5", "Bahnschrift");
        cboFont3.SelectedItem = fontFace3;
        string fontSize3 = helper.GetIniValueFonts("FontSize5", "30");
        numFont3.Value = int.Parse(fontSize3);

        string fontBold3 = helper.GetIniValueFonts("FontBold5", "0");
        chkFontBold3.Checked = !fontBold3.Equals("0");
        string fontItalic3 = helper.GetIniValueFonts("FontItalic5", "0");
        chkFontItalic3.Checked = !fontItalic3.Equals("0");
        string fontAA3 = helper.GetIniValueFonts("FontAA5", "1");
        chkFontAA3.Checked = !fontAA3.Equals("0");

        string fontColorR3 = helper.GetIniValueFonts("FontColorR5", "211");
        int fontColorR3Val = int.Parse(fontColorR3);
        string fontColorG3 = helper.GetIniValueFonts("FontColorG5", "0");
        int fontColorG3Val = int.Parse(fontColorG3);
        string fontColorB3 = helper.GetIniValueFonts("FontColorB5", "9");
        int fontColorB3Val = int.Parse(fontColorB3);
        pnlColorFont3.BackColor = Color.FromArgb(fontColorR3Val, fontColorG3Val, fontColorB3Val);

        // Title: Ini-Index is 6!
        string fontFace4 = helper.GetIniValueFonts("FontFace6", "Bahnschrift");
        cboFont4.SelectedItem = fontFace4;
        string fontSize4 = helper.GetIniValueFonts("FontSize6", "40");
        numFont4.Value = int.Parse(fontSize4);

        string fontBold4 = helper.GetIniValueFonts("FontBold6", "1");
        chkFontBold4.Checked = !fontBold4.Equals("0");
        string fontItalic4 = helper.GetIniValueFonts("FontItalic6", "0");
        chkFontItalic4.Checked = !fontItalic4.Equals("0");
        string fontAA4 = helper.GetIniValueFonts("FontAA6", "1");
        chkFontAA4.Checked = !fontAA4.Equals("0");

        string fontColorR4 = helper.GetIniValueFonts("FontColorR6", "255");
        int fontColorR4Val = int.Parse(fontColorR4);
        string fontColorG4 = helper.GetIniValueFonts("FontColorG6", "86");
        int fontColorG4Val = int.Parse(fontColorG4);
        string fontColorB4 = helper.GetIniValueFonts("FontColorB6", "0");
        int fontColorB4Val = int.Parse(fontColorB4);
        pnlColorFont4.BackColor = Color.FromArgb(fontColorR4Val, fontColorG4Val, fontColorB4Val);

        // Album: Ini-Index is 7!
        string fontFace5 = helper.GetIniValueFonts("FontFace7", "Bahnschrift");
        cboFont5.SelectedItem = fontFace5;
        string fontSize5 = helper.GetIniValueFonts("FontSize7", "25");
        numFont5.Value = int.Parse(fontSize5);

        string fontBold5 = helper.GetIniValueFonts("FontBold7", "0");
        chkFontBold5.Checked = !fontBold5.Equals("0");
        string fontItalic5 = helper.GetIniValueFonts("FontItalic7", "0");
        chkFontItalic5.Checked = !fontItalic5.Equals("0");
        string fontAA5 = helper.GetIniValueFonts("FontAA7", "1");
        chkFontAA5.Checked = !fontAA5.Equals("0");

        string fontColorR5 = helper.GetIniValueFonts("FontColorR7", "211");
        int fontColorR5Val = int.Parse(fontColorR5);
        string fontColorG5 = helper.GetIniValueFonts("FontColorG7", "0");
        int fontColorG5Val = int.Parse(fontColorG5);
        string fontColorB5 = helper.GetIniValueFonts("FontColorB7", "9");
        int fontColorB5Val = int.Parse(fontColorB5);
        pnlColorFont5.BackColor = Color.FromArgb(fontColorR5Val, fontColorG5Val, fontColorB5Val);

      } catch (Exception) {
        // ignore
      }
    }

    private void btnSettingsSave_Click(object? sender, EventArgs? e) {

      // Notify
      helper.SetIniValueFonts("FontFace1", cboFont1.Text);
      helper.SetIniValueFonts("FontSize1", numFont1.Value.ToString());
      helper.SetIniValueFonts("FontBold1", chkFontBold1.Checked ? "1" : "0");
      helper.SetIniValueFonts("FontItalic1", chkFontItalic1.Checked ? "1" : "0");
      helper.SetIniValueFonts("FontAA1", chkFontAA1.Checked ? "1" : "0");
      helper.SetIniValueFonts("FontColorR1", pnlColorFont1.BackColor.R.ToString());
      helper.SetIniValueFonts("FontColorG1", pnlColorFont1.BackColor.G.ToString());
      helper.SetIniValueFonts("FontColorB1", pnlColorFont1.BackColor.B.ToString());

      // Preset
      helper.SetIniValueFonts("FontFace2", cboFont2.Text);
      helper.SetIniValueFonts("FontSize2", numFont2.Value.ToString());
      helper.SetIniValueFonts("FontBold2", chkFontBold2.Checked ? "1" : "0");
      helper.SetIniValueFonts("FontItalic2", chkFontItalic2.Checked ? "1" : "0");
      helper.SetIniValueFonts("FontAA2", chkFontAA2.Checked ? "1" : "0");
      helper.SetIniValueFonts("FontColorR2", pnlColorFont2.BackColor.R.ToString());
      helper.SetIniValueFonts("FontColorG2", pnlColorFont2.BackColor.G.ToString());
      helper.SetIniValueFonts("FontColorB2", pnlColorFont2.BackColor.B.ToString());

      // Artist: Ini-Index is 5!
      helper.SetIniValueFonts("FontFace5", cboFont3.Text);
      helper.SetIniValueFonts("FontSize5", numFont3.Value.ToString());
      helper.SetIniValueFonts("FontBold5", chkFontBold3.Checked ? "1" : "0");
      helper.SetIniValueFonts("FontItalic5", chkFontItalic3.Checked ? "1" : "0");
      helper.SetIniValueFonts("FontAA5", chkFontAA3.Checked ? "1" : "0");
      helper.SetIniValueFonts("FontColorR5", pnlColorFont3.BackColor.R.ToString());
      helper.SetIniValueFonts("FontColorG5", pnlColorFont3.BackColor.G.ToString());
      helper.SetIniValueFonts("FontColorB5", pnlColorFont3.BackColor.B.ToString());

      // Title: Ini-Index is 6!
      helper.SetIniValueFonts("FontFace6", cboFont4.Text);
      helper.SetIniValueFonts("FontSize6", numFont4.Value.ToString());
      helper.SetIniValueFonts("FontBold6", chkFontBold4.Checked ? "1" : "0");
      helper.SetIniValueFonts("FontItalic6", chkFontItalic4.Checked ? "1" : "0");
      helper.SetIniValueFonts("FontAA6", chkFontAA4.Checked ? "1" : "0");
      helper.SetIniValueFonts("FontColorR6", pnlColorFont4.BackColor.R.ToString());
      helper.SetIniValueFonts("FontColorG6", pnlColorFont4.BackColor.G.ToString());
      helper.SetIniValueFonts("FontColorB6", pnlColorFont4.BackColor.B.ToString());

      // Album: Ini-Index is 7!
      helper.SetIniValueFonts("FontFace7", cboFont5.Text);
      helper.SetIniValueFonts("FontSize7", numFont5.Value.ToString());
      helper.SetIniValueFonts("FontBold7", chkFontBold5.Checked ? "1" : "0");
      helper.SetIniValueFonts("FontItalic7", chkFontItalic5.Checked ? "1" : "0");
      helper.SetIniValueFonts("FontAA7", chkFontAA5.Checked ? "1" : "0");
      helper.SetIniValueFonts("FontColorR7", pnlColorFont5.BackColor.R.ToString());
      helper.SetIniValueFonts("FontColorG7", pnlColorFont5.BackColor.G.ToString());
      helper.SetIniValueFonts("FontColorB7", pnlColorFont5.BackColor.B.ToString());

      SendToMilkwaveVisualizer("", MessageType.Config);
      SendToMilkwaveVisualizer("", MessageType.TestFonts);
    }

    private void btnSettingsLoad_Click(object sender, EventArgs e) {
      LoadVisualizerSettings();
    }

    private void pnlColorFont_Click(object sender, EventArgs e) {
      ColorDialog dlg = new ColorDialog();
      dlg.FullOpen = true;
      dlg.CustomColors = new int[] {
        ColorTranslator.ToOle(pnlColorFont1.BackColor),
        ColorTranslator.ToOle(pnlColorFont2.BackColor),
        ColorTranslator.ToOle(pnlColorFont3.BackColor),
        ColorTranslator.ToOle(pnlColorFont4.BackColor),
        ColorTranslator.ToOle(pnlColorFont5.BackColor)
      };
      Panel pnlColorFont = (Panel)sender;
      dlg.Color = pnlColorFont.BackColor;
      if (dlg.ShowDialog(this) == DialogResult.OK) {
        pnlColorFont.BackColor = dlg.Color;
      }
    }

    private void btnTestFonts_Click(object sender, EventArgs e) {
      SendToMilkwaveVisualizer("", MessageType.TestFonts);
    }

    private void lblFont1_DoubleClick(object sender, EventArgs e) {
      cboFont1.SelectedItem = "Bahnschrift";
      numFont1.Value = 20;
      chkFontBold1.Checked = false;
      chkFontItalic1.Checked = false;
      chkFontAA1.Checked = false;
      pnlColorFont1.BackColor = Color.FromArgb(255, 255, 0);
    }

    private void lblFont2_DoubleClick(object sender, EventArgs e) {
      cboFont2.SelectedItem = "Bahnschrift";
      numFont2.Value = 25;
      chkFontBold2.Checked = false;
      chkFontItalic2.Checked = false;
      chkFontAA2.Checked = true;
      pnlColorFont2.BackColor = Color.FromArgb(255, 86, 0);
    }

    private void lblFont3_DoubleClick(object sender, EventArgs e) {
      cboFont3.SelectedItem = "Bahnschrift";
      numFont3.Value = 30;
      chkFontBold3.Checked = false;
      chkFontItalic3.Checked = false;
      chkFontAA3.Checked = true;
      pnlColorFont3.BackColor = Color.FromArgb(211, 0, 9);
    }

    private void lblFont4_DoubleClick(object sender, EventArgs e) {
      cboFont4.SelectedItem = "Bahnschrift";
      numFont4.Value = 40;
      chkFontBold4.Checked = true;
      chkFontItalic4.Checked = false;
      chkFontAA4.Checked = true;
      pnlColorFont4.BackColor = Color.FromArgb(255, 86, 0);
    }

    private void lblFont5_DoubleClick(object sender, EventArgs e) {
      cboFont5.SelectedItem = "Bahnschrift";
      numFont5.Value = 25;
      chkFontBold5.Checked = false;
      chkFontItalic5.Checked = false;
      chkFontAA5.Checked = true;
      pnlColorFont5.BackColor = Color.FromArgb(211, 0, 9);
    }

    private void cboFont1_SelectedIndexChanged(object sender, EventArgs e) {
      if ((Control.ModifierKeys & Keys.Alt) == Keys.Alt) {
        helper.SetIniValueFonts("FontFace1", cboFont1.Text);
        SendToMilkwaveVisualizer("", MessageType.Config);
        SendToMilkwaveVisualizer("", MessageType.TestFonts);
      }
    }

    private void cboFont2_SelectedIndexChanged(object sender, EventArgs e) {
      if ((Control.ModifierKeys & Keys.Alt) == Keys.Alt) {
        helper.SetIniValueFonts("FontFace2", cboFont2.Text);
        SendToMilkwaveVisualizer("", MessageType.Config);
        SendToMilkwaveVisualizer("", MessageType.TestFonts);
      }
    }

    private void cboFont3_SelectedIndexChanged(object sender, EventArgs e) {
      if ((Control.ModifierKeys & Keys.Alt) == Keys.Alt) {
        helper.SetIniValueFonts("FontFace5", cboFont3.Text);
        SendToMilkwaveVisualizer("", MessageType.Config);
        SendToMilkwaveVisualizer("", MessageType.TestFonts);
      }
    }

    private void cboFont4_SelectedIndexChanged(object sender, EventArgs e) {
      if ((Control.ModifierKeys & Keys.Alt) == Keys.Alt) {
        helper.SetIniValueFonts("FontFace6", cboFont4.Text);
        SendToMilkwaveVisualizer("", MessageType.Config);
        SendToMilkwaveVisualizer("", MessageType.TestFonts);
      }
    }

    private void cboFont5_SelectedIndexChanged(object sender, EventArgs e) {
      if ((Control.ModifierKeys & Keys.Alt) == Keys.Alt) {
        helper.SetIniValueFonts("FontFace7", cboFont5.Text);
        SendToMilkwaveVisualizer("", MessageType.Config);
        SendToMilkwaveVisualizer("", MessageType.TestFonts);
      }
    }

    private void numFont1_ValueChanged(object sender, EventArgs e) {
      if ((Control.ModifierKeys & Keys.Alt) == Keys.Alt) {
        helper.SetIniValueFonts("FontSize1", numFont1.Value.ToString());
        SendToMilkwaveVisualizer("", MessageType.Config);
        SendToMilkwaveVisualizer("", MessageType.TestFonts);
      }
    }

    private void numFont2_ValueChanged(object sender, EventArgs e) {
      if ((Control.ModifierKeys & Keys.Alt) == Keys.Alt) {
        helper.SetIniValueFonts("FontSize2", numFont2.Value.ToString());
        SendToMilkwaveVisualizer("", MessageType.Config);
        SendToMilkwaveVisualizer("", MessageType.TestFonts);
      }
    }

    private void numFont3_ValueChanged(object sender, EventArgs e) {
      if ((Control.ModifierKeys & Keys.Alt) == Keys.Alt) {
        helper.SetIniValueFonts("FontSize5", numFont3.Value.ToString());
        SendToMilkwaveVisualizer("", MessageType.Config);
        SendToMilkwaveVisualizer("", MessageType.TestFonts);
      }
    }

    private void numFont4_ValueChanged(object sender, EventArgs e) {
      if ((Control.ModifierKeys & Keys.Alt) == Keys.Alt) {
        helper.SetIniValueFonts("FontSize6", numFont4.Value.ToString());
        SendToMilkwaveVisualizer("", MessageType.Config);
        SendToMilkwaveVisualizer("", MessageType.TestFonts);
      }
    }

    private void numFont5_ValueChanged(object sender, EventArgs e) {
      if ((Control.ModifierKeys & Keys.Alt) == Keys.Alt) {
        helper.SetIniValueFonts("FontSize7", numFont5.Value.ToString());
        SendToMilkwaveVisualizer("", MessageType.Config);
        SendToMilkwaveVisualizer("", MessageType.TestFonts);
      }
    }

    private void cboAutoplay_SelectedIndexChanged(object sender, EventArgs e) {
      toolTip1.SetToolTip(cboAutoplay, cboAutoplay.Text);
    }

    private void numFactorTime_ValueChanged(object sender, EventArgs e) {
      SetExpIncrements(numFactorTime);
      SendToMilkwaveVisualizer("", MessageType.TimeFactor);
    }

    private void munFactorFrame_ValueChanged(object sender, EventArgs e) {
      SendToMilkwaveVisualizer("", MessageType.FrameFactor);
    }

    private void numFactorFPS_ValueChanged(object sender, EventArgs e) {
      SetExpIncrements(numFactorFPS);
      SendToMilkwaveVisualizer("", MessageType.FpsFactor);
    }

    private void numVisIntensity_ValueChanged(object sender, EventArgs e) {
      if ((Control.ModifierKeys & Keys.Alt) == Keys.Alt) {
        numVisIntensity.Increment = 0.05M;
      } else {
        numVisIntensity.Increment = 0.01M;
      }
      SendToMilkwaveVisualizer("", MessageType.VisIntensity);
    }

    private void numVisShift_ValueChanged(object sender, EventArgs e) {
      if ((Control.ModifierKeys & Keys.Alt) == Keys.Alt) {
        numVisShift.Increment = 0.05M;
      } else {
        numVisShift.Increment = 0.01M;
      }
      SendToMilkwaveVisualizer("", MessageType.VisShift);
    }

    private void lblFactorTime_Click(object sender, EventArgs e) {
      numFactorTime.Value = 1;
    }

    private void lblFactorFrame_Click(object sender, EventArgs e) {
      numFactorFrame.Value = 1;
    }

    private void lblFactorFPS_Click(object sender, EventArgs e) {
      numFactorFPS.Value = 1;
    }

    private void lblVisIntensity_Click(object sender, EventArgs e) {
      numVisIntensity.Value = 1;
    }

    private void lblVisShift_Click(object sender, EventArgs e) {
      numVisShift.Value = 0;
    }
    private void OpenFile(string filePath) {
      filePath = Path.Combine(BaseDir, filePath);
      if (File.Exists(filePath)) {
        try {
          Process.Start(new ProcessStartInfo {
            FileName = filePath,
            UseShellExecute = true
          });
        } catch (Exception ex) {
          SetStatusText($"Error opening file: {ex.Message}");
        }
      } else {
        SetStatusText($"File not found: {filePath}");
      }
    }

    private void btnOpenSettingsIni_Click(object sender, EventArgs e) {
      OpenFile("settings.ini");
    }

    private void btnOpenSpritesIni_Click(object sender, EventArgs e) {
      OpenFile("sprites.ini");
    }

    private void btnOpenMessagesIni_Click(object sender, EventArgs e) {
      OpenFile("messages.ini");
    }

    private void btnOpenScriptDefault_Click(object sender, EventArgs e) {
      OpenFile("script-default.txt");
    }

    private void btnOpenSettingsRemote_Click(object sender, EventArgs e) {
      OpenFile("settings-remote.json");
    }

    private void btnOpenTagsRemote_Click(object sender, EventArgs e) {
      OpenFile("tags-remote.json");
    }

    private void txtFilter_KeyDown(object sender, KeyEventArgs e) {
      if (e.KeyCode == Keys.Enter) {
        e.SuppressKeyPress = true; // Prevent the beep sound on Enter key press
        LoadPresetsFromDirectory(VisualizerPresetsFolder, true);
      }
    }

    private void btnSendShader_Click(object? sender, EventArgs? e) {
      try {
        // Ensure the shader directory exists
        Directory.CreateDirectory(PresetsShaderConvFolder);

        string presetName = txtShaderinfo.Text.Split(Environment.NewLine)[0].Trim();
        if (!chkShaderFile.Checked || string.IsNullOrEmpty(presetName)) {
          presetName = "Shader";
        }
        string fileName = StripInvalidFileNameChars(presetName + ".milk");

        // Build the file path
        string shaderFile = Path.Combine(PresetsShaderConvFolder, fileName);

        // Prepare the header and shader content
        var sb = new StringBuilder();
        sb.AppendLine("MILKDROP_PRESET_VERSION=201");
        sb.AppendLine("PSVERSION=" + numPSVersion.Text);
        sb.AppendLine("PSVERSION_WARP=" + numPSVersion.Text);
        sb.AppendLine("PSVERSION_COMP=" + numPSVersion.Text);

        // Write shader info as comment into preset file
        string shaderinfo = "// " + txtShaderinfo.Text.Trim().Replace(Environment.NewLine, "\n").Replace('\r', '\n').Replace("\n", " / ");
        shaderinfo += Environment.NewLine + "// Transpiled to HLSL using Milkwave" + Environment.NewLine + Environment.NewLine;
        string shaderText = shaderinfo + txtShaderHLSL.Text.Trim();

        // Prefix each line in txtShader.Text with comp_X=
        var lines = shaderText.Replace("\r\n", "\n").Replace('\r', '\n').Split('\n');
        int lineNum = 1;
        foreach (var line in lines) {
          sb.AppendLine($"comp_{lineNum}={line}");
          lineNum++;
        }

        // Write to file (overwrite if exists)
        File.WriteAllText(shaderFile, sb.ToString());
        SetStatusText($"Shader saved to {shaderFile}");

        SendToMilkwaveVisualizer(shaderFile, MessageType.PresetFilePath);
      } catch (Exception ex) {
        SetStatusText($"Error saving shader: {ex.Message}");
      }
    }

    private void btnLoadShaderInput_Click(object sender, EventArgs e) {

      if (ofdShader.ShowDialog() == DialogResult.OK) {
        ofdShader.InitialDirectory = Path.GetDirectoryName(ofdShader.FileName);
        String shaderInput = ofdShader.FileName;
        if (File.Exists(shaderInput)) {
          try {
            // Read the shader content from the input file
            string shaderContent = File.ReadAllText(shaderInput);
            txtShaderGLSL.Text = shaderContent;
            // Notify the user
            SetStatusText($"Shader loaded from {shaderInput}");
          } catch (Exception ex) {
            SetStatusText($"Error loading shader: {ex.Message}");
          }
        } else {
          SetStatusText($"Shader input file not found: {shaderInput}");
        }
      }
    }

    private void statusBar_Click(object sender, EventArgs e) {
      try {
        if (statusBar.Text != null && !statusBar.Text.StartsWith("Copied ")) {
          Clipboard.SetText(statusBar.Text);
          SetStatusText($"Copied '{statusBar.Text}' to clipboard");
        }
      } catch (Exception) {
        // ignore
      }
    }

    private void SetCurrentShaderLineNumber() {
      string sub = txtShaderHLSL.Text.Substring(0, txtShaderHLSL.SelectionStart);
      int lineNumber = sub.Count(f => f == '\n') + 1;

      // offsetNum lines are inserted as header by Visualizer
      txtLineNumberError.Text = (lineNumber + numOffset.Value).ToString();
    }

    private void txtShaderSetLineNumber(object sender, EventArgs e) {
      SetCurrentShaderLineNumber();
    }

    private void btnShaderConvert_Click(object sender, EventArgs e) {
      ConvertShader();
    }

    private void btnLoadShadertoyID_Click(object? sender, EventArgs? e) {
      string id = cboShadertoyID.Text.Trim();
      int index = id.LastIndexOf("/");
      if (index > 0) {
        id = id.Substring(index + 1);
        cboShadertoyID.Text = id; // Set the ID back to the combobox
      }
      ReloadShadertoyIDsList(true);

      if (string.IsNullOrEmpty(id)) {
        SetStatusText("Please enter a Shadertoy.com URL or ID");
        return;
      }

      SetStatusText($"Loading code for Shadertoy ID {id}");

      string url = $"https://www.shadertoy.com/api/v1/shaders/{id}?key={shadertoyAppKey}";
      using var httpClient = new HttpClient();
      try {
        var jsonString = httpClient.GetStringAsync(url).Result;
        JsonDocument doc = JsonDocument.Parse(jsonString);
        if (doc.RootElement.TryGetProperty("Error", out JsonElement elError)) {
          // If the error property exists, it means the shader was not found
          SetStatusText($"Shadertoy.com says: {elError.GetString()}");
          return;
        }

        JsonElement elShader = doc.RootElement.GetProperty("Shader");
        JsonElement elInfo = elShader.GetProperty("info");
        string? shaderId = elInfo.GetProperty("id").GetString();
        string? shaderName = elInfo.GetProperty("name").GetString();
        string? shaderUsername = elInfo.GetProperty("username").GetString();
        string? date = elInfo.GetProperty("date").GetString();
        if (long.TryParse(date, out long unixTimestamp)) {
          DateTimeOffset dateTimeOffset = DateTimeOffset.FromUnixTimeSeconds(unixTimestamp);
          string formattedDate = dateTimeOffset.ToString("yyyy-MM-dd HH:mm:ss");
          toolTip1.SetToolTip(txtShaderinfo, formattedDate);
        }

        JsonElement firstRenderpassElement = elShader.GetProperty("renderpass").EnumerateArray().First();
        string? shaderCode = firstRenderpassElement.GetProperty("code").GetString();
        if (shaderCode == null) {
          SetStatusText("Shader code not found in the response");
        } else {
          txtShaderinfo.Text = $"{shaderUsername} - {shaderName}" + Environment.NewLine + $"https://www.shadertoy.com/view/{shaderId}";
          string? formattedShaderCode = shaderCode?.Replace("\n", Environment.NewLine);
          txtShaderGLSL.Text = formattedShaderCode;

          if (!String.IsNullOrEmpty(txtShaderGLSL.Text)) {
            ConvertShader();
            btnSendShader_Click(null, null);
          }
          SetStatusText($"Shader code loaded and converted");
        }

      } catch (Exception ex) {
        SetStatusText($"Loading failed: {ex.Message}");
      }
    }

    private string StripInvalidFileNameChars(string fileName) {
      var invalidChars = Path.GetInvalidFileNameChars();
      var sb = new StringBuilder(fileName.Length);
      foreach (char c in fileName) {
        if (!invalidChars.Contains(c))
          sb.Append(c);
      }
      return sb.ToString();
    }

    private void ConvertShader() {
      txtShaderHLSL.Text = Shader.ConvertGLSLtoHLSL(txtShaderGLSL.Text);
      if (Shader.ConversionErrors.Length > 0) {
        picShaderError.Visible = true;
        toolTip1.SetToolTip(picShaderError, Shader.ConversionErrors.ToString());
      } else {
        picShaderError.Visible = false;
      }
    }

    private void btnShaderHelp_Click(object sender, EventArgs e) {
      OpenURL("https://github.com/IkeC/Milkwave/blob/main/Build/Manual.md#tab-shader");
    }

    private void txtShader_MouseWheel(object sender, MouseEventArgs e) {
      TextBox ctrl = (TextBox)sender;
      if ((Control.ModifierKeys & Keys.Control) == Keys.Control) {
        // Ctrl+MouseWheel detected
        if (e.Delta > 0) {
          // Scrolled up
          int fontSize = (int)ctrl.Font.Size + 1;
          ctrl.Font = new Font(ctrl.Font.FontFamily, fontSize, ctrl.Font.Style);
        } else {
          int fontSize = (int)ctrl.Font.Size - 1;
          if (fontSize > 0) {
            ctrl.Font = new Font(ctrl.Font.FontFamily, fontSize, ctrl.Font.Style);
          }
        }
        // Optionally, mark the event as handled if needed
        if (e is HandledMouseEventArgs hme)
          hme.Handled = true;
      }
    }

    private void cboShadertoyURL_KeyDown(object sender, KeyEventArgs e) {
      if (e.KeyCode == Keys.Enter) {
        e.SuppressKeyPress = true; // Prevent the beep sound on Enter key press
        btnLoadShadertoyID_Click(null, null);
      }
    }

    public int GetNthIndex(string s, char t, int n) {
      int count = 0;
      for (int i = 0; i < s.Length; i++) {
        if (s[i] == t) {
          count++;
          if (count == n) {
            return i;
          }
        }
      }
      return -1;
    }

    private void MarkRow(int row) {
      try {
        txtShaderHLSL.SelectionStart = GetNthIndex(txtShaderHLSL.Text, '\n', row - 1) + 1;
        txtShaderHLSL.SelectionLength = GetNthIndex(txtShaderHLSL.Text, '\n', row) - txtShaderHLSL.SelectionStart;
        txtShaderHLSL.Focus();
        txtShaderHLSL.ScrollToCaret();
      } catch (Exception ex) {
        // ignore
      }
    }

    private void numOffset_ValueChanged(object sender, EventArgs e) {
      if (lastReceivedShaderErrorLineNumber > 0) {
        MarkRow(lastReceivedShaderErrorLineNumber - (int)numOffset.Value);
      }
    }

    private void btnLoadShadertoyQuery_Click(object? sender, EventArgs? e) {
      LoadShadertoyQuery();
    }

    private bool LoadShadertoyQuery() {
      try {
        bool altPressed = ((Control.ModifierKeys & Keys.Alt) == Keys.Alt);
        int selectIndex = (int)numShadertoyQueryIndex.Value - 1;
        int page = (int)Math.Floor((decimal)selectIndex / shadertoyQueryPageSize);

        if (altPressed || selectIndex >= shadertoyQueryList.Count || !cboShadertoyType.Text.Equals(shadertoyQueryType)) {
          if (altPressed || !cboShadertoyType.Text.Equals(shadertoyQueryType)) {
            page = 0;
            shadertoyQueryList.Clear();
          }
          string url = "https://www.shadertoy.com/api/v1/shaders/query?" +
            $"key={shadertoyAppKey}&" +
            $"sort={cboShadertoyType.Text}&" +
            $"from={page * shadertoyQueryPageSize}&" +
            $"num={shadertoyQueryPageSize}";
          using var httpClient = new HttpClient();
          SetStatusText($"Trying to load {cboShadertoyType.Text} entries...");
          var jsonString = httpClient.GetStringAsync(url).Result;
          JsonDocument doc = JsonDocument.Parse(jsonString);
          if (doc.RootElement.TryGetProperty("Error", out JsonElement elError)) {
            SetStatusText($"Shadertoy.com says: {elError.GetString()}");
            return false;
          }

          // JsonElement elShaders = doc.RootElement.GetProperty("Shaders");
          JsonElement elResults = doc.RootElement.GetProperty("Results");

          // using null-forgiving operator
          shadertoyQueryList.AddRange(elResults.EnumerateArray().Select(e => e.GetString()!));

          if (shadertoyQueryList.Count > 0) {
            // seems like a proper query result, let's cache it
            shadertoyQueryType = cboShadertoyType.Text;
            numShadertoyQueryIndex.Maximum = shadertoyQueryPageSize * (page + 2);
          }
        }

        if (shadertoyQueryList.Count > 0) {
          if (selectIndex > shadertoyQueryList.Count - 1) {
            numShadertoyQueryIndex.Value = 1;
            selectIndex = 0;
          }

          string id = shadertoyQueryList[selectIndex];
          if (!string.IsNullOrEmpty(id)) {
            cboShadertoyID.Text = id;
            btnLoadShadertoyID_Click(null, null);
            numShadertoyQueryIndex.Value = (int)numShadertoyQueryIndex.Value + 1;
          }
        }
      } catch (Exception ex) {
        SetStatusText($"Loading failed: {ex.Message}");
      }

      return true;
    }

    private void cboShadertoyID_Click(object sender, EventArgs e) {
      if ((Control.ModifierKeys & Keys.Control) == Keys.Control) {
        OpenURL($"https://www.shadertoy.com/view/{cboShadertoyID.Text}");
      }
    }

    private void chkShaderLeft_CheckedChanged(object sender, EventArgs e) {
      splitContainerShader.Panel1Collapsed = chkShaderLeft.Checked;
    }

    private void btnHLSLSave_Click(object sender, EventArgs e) {

      StringBuilder sb = new StringBuilder();
      string[] txtShaderinfoLines = txtShaderinfo.Text.Split(Environment.NewLine);
      string hlslFilename = "Shader";
      foreach (string line in txtShaderinfoLines) {
        sb.AppendLine(ShaderinfoLinePrefix + line.Trim());
      }
      if (txtShaderinfoLines.Length > 0 && txtShaderinfoLines[0].Length > 0) {
        hlslFilename = StripInvalidFileNameChars(txtShaderinfoLines[0]);
      }
      hlslFilename = Path.Combine(ShaderFilesFolder, hlslFilename + ".hlsl");
      sb.Append(txtShaderHLSL.Text);

      try {
        File.WriteAllText(hlslFilename, sb.ToString());
        SetStatusText($"Saved {hlslFilename}");
      } catch (UnauthorizedAccessException ex) {
        SetStatusText($"Error saving HLSL file: {ex.Message}");
      }
    }

    private void btnHLSLLoad_Click(object sender, EventArgs e) {

      if (string.IsNullOrEmpty(ofdShaderHLSL.InitialDirectory)) {
        ofdShaderHLSL.InitialDirectory = Path.Combine(ShaderFilesFolder);
      }

      if (ofdShaderHLSL.ShowDialog() == DialogResult.OK) {
        ofdShaderHLSL.InitialDirectory = Path.GetDirectoryName(ofdShaderHLSL.FileName);
        if (File.Exists(ofdShaderHLSL.FileName)) {
          string[] content = File.ReadAllLines(ofdShaderHLSL.FileName);
          txtShaderinfo.Clear();

          StringBuilder sb = new StringBuilder();
          if (ofdShaderHLSL.FileName.EndsWith(".milk", StringComparison.InvariantCultureIgnoreCase)) {
            // If it's a preset file, extract the comp shader info
            foreach (string line in content) {
              if (line.StartsWith("comp_", StringComparison.InvariantCultureIgnoreCase)) {
                int index = line.IndexOf('=');
                if (index > 0) {
                  string shaderLine = line.Substring(index + 1).Trim();
                  if (shaderLine.Contains("// Transpiled")) {
                    continue;
                  }
                  if (line.StartsWith("comp_1=//")) {
                    // treat as shader info line(s)
                    shaderLine = line.Substring(9).Trim();
                    shaderLine = shaderLine.Replace(" / ", Environment.NewLine).Trim();
                    txtShaderinfo.Text = shaderLine;
                  } else {
                    sb.AppendLine(shaderLine);
                  }
                }
              }
            }
            content = sb.ToString().Split(new[] { Environment.NewLine }, StringSplitOptions.None);
            sb = new StringBuilder();
          }

          foreach (string line in content) {
            if (line.StartsWith(ShaderinfoLinePrefix)) {
              if (txtShaderinfo.Text.Length > 0) {
                txtShaderinfo.AppendText(Environment.NewLine);
              }
              txtShaderinfo.AppendText(line.Substring(ShaderinfoLinePrefix.Length).Trim());
            } else {
              sb.AppendLine(line);
            }
          }
          txtShaderHLSL.Text = Shader.BasicFormatShaderCode(sb.ToString());
        }
        txtShaderinfo.SelectionStart = 0;
        txtShaderinfo.ScrollToCaret();
        txtShaderHLSL.SelectionStart = 0;
        txtShaderHLSL.ScrollToCaret();

        SetStatusText($"Loaded HLSL from {ofdShaderHLSL.FileName}");
      }
    }

    private void txtShaderFind_KeyDown(object sender, KeyEventArgs e) {
      if (e.KeyCode == Keys.Enter) {
        e.SuppressKeyPress = true; // Prevent the beep sound on Enter key press
        FindShaderString();
      }
    }

    private void FindShaderString() {
      string searchText = txtShaderFind.Text;
      if (searchText.Length > 0) {
        try {
          txtShaderHLSL.Focus();
          int pos = txtShaderHLSL.SelectionStart + 1;
          string remainingText = txtShaderHLSL.Text.Substring(pos);
          int indexInRemaining = remainingText.IndexOf(searchText, StringComparison.InvariantCultureIgnoreCase);
          if (indexInRemaining >= 0) {
            txtShaderHLSL.SelectionStart = pos + indexInRemaining;
            txtShaderHLSL.SelectionLength = searchText.Length;
            txtShaderHLSL.ScrollToCaret();
          } else {
            // If not found, start from the beginning
            pos = 0;
            indexInRemaining = txtShaderHLSL.Text.IndexOf(searchText, StringComparison.InvariantCultureIgnoreCase);
            if (indexInRemaining >= 0) {
              txtShaderHLSL.SelectionStart = indexInRemaining;
              txtShaderHLSL.SelectionLength = searchText.Length;
              txtShaderHLSL.ScrollToCaret();
            }
          }
        } catch { }
      }
    }

    private void txtShaderHLSL_KeyDown(object sender, KeyEventArgs e) {
      if (e.KeyCode == Keys.Enter) {
        TextBox? tb = sender as TextBox;
        int caretIndex = tb.SelectionStart;

        // Get all text before the caret
        string textBeforeCaret = tb.Text.Substring(0, caretIndex);
        int lastLineBreak = textBeforeCaret.LastIndexOf('\n');
        string currentLine = lastLineBreak >= 0
            ? textBeforeCaret.Substring(lastLineBreak + 1)
            : textBeforeCaret;

        // Extract leading whitespace
        string indentation = new string(currentLine.TakeWhile(char.IsWhiteSpace).ToArray());

        // Insert newline and indentation using SelectedText
        tb.SelectedText = "\r\n" + indentation;

        e.SuppressKeyPress = true;
      }
    }

  } // end class
} // end namespace