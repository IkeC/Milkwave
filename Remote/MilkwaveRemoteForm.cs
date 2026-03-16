using MilkwaveRemote.Data;
using MilkwaveRemote.Helper;
using System.Diagnostics;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.Drawing.Text;
using System.Globalization;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.Json;
using static MilkwaveRemote.Data.MidiRow;
using static MilkwaveRemote.Helper.DarkModeCS;
using static MilkwaveRemote.Helper.RemoteHelper;

namespace MilkwaveRemote {
  public partial class MilkwaveRemoteForm : Form {
    private PipeClient? _pipeClient;

    private DarkModeCS dm;

    private System.Windows.Forms.Timer autoplayTimer;
    private System.Windows.Forms.Timer monitorTimer;
    private System.Windows.Forms.Timer spoutRefreshTimer;
    private System.Windows.Forms.Timer controllerTimer;

    private int currentAutoplayIndex = 0;
    private int lastLineIndex = 0;
    private int lastReceivedShaderErrorLineNumber = -1;
    private float autoplayRemainingBeats = 1;

    private bool updatingWaveParams = false;
    private bool updatingSettingsParams = false;
    private uint lastControllerButtons = 0;
    private Dictionary<int, string> controllerConfig = new();

#if DEBUG
    private string BaseDir = Path.GetFullPath(Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "..\\..\\..\\..\\Release"));
#else
    private string BaseDir = AppDomain.CurrentDomain.BaseDirectory;
#endif

    private string GetVisualizerDir(bool isDX12) {
      string subPath = isDX12 ? Settings.VisualizerDX12Path : Settings.VisualizerPath;
      if (string.IsNullOrEmpty(subPath)) return BaseDir;
      return Path.IsPathRooted(subPath) ? subPath : Path.Combine(BaseDir, subPath);
    }

    /// <summary>
    /// Determines if the currently selected visualizer is DX12 based on the discovered instance name.
    /// </summary>
    private bool IsSelectedVisualizerDX12() {
      int idx = cboVisualizerInstance.SelectedIndex;
      if (idx >= 0 && idx < _discoveredInstances.Count) {
        return _discoveredInstances[idx].name.Contains("DX12", StringComparison.OrdinalIgnoreCase);
      }
      return false;
    }

    private string ConnectedVisualizerName {
      get {
        int idx = cboVisualizerInstance.SelectedIndex;
        return (idx >= 0 && idx < _discoveredInstances.Count) ? _discoveredInstances[idx].name : "Visualizer";
      }
    }

    private string VisualizerPresetsFolder = "";
    private string ShaderFilesFolder = "";
    private string PresetsShaderConvFolder = "";

    private string lastScriptFileName = "script-default.txt";
    private string midiDefaultFileName = "midi-default.txt";

    private string windowNotFound = "Not connected to a visualizer";
    private string defaultFontName = "Segoe UI";
    private string lastSpoutSenderName = "";

    private string milkwaveSettingsFile = "settings-remote.json";
    private string milkwaveTagsFile = "tags-remote.json";
    private string milkwaveMidiFile = "midi-remote.json";
    private string milkwavePresetDeckFile = "presetdeck-remote.json";
    private string milkwaveControllerFile = "controller-remote.json";
    private string milkwaveMessagesEditorFile = "messages-editor.html";

    private string visualizerSpritesFile = "sprites.ini";
    private string visualizerMessagesFile = "messages.ini";
    private string visualizerSettingsFile = "settings.ini";

    private readonly Dictionary<Button, string> spriteButtonSectionMap = new();
    private readonly Dictionary<Button, string> spriteButtonLabelMap = new();
    private readonly Dictionary<Button, Image?> buttonImageCache = new();
    private readonly Dictionary<string, string> spriteSectionImageMap = new(StringComparer.OrdinalIgnoreCase);
    private readonly Dictionary<string, string> messageCodeTextMap = new(StringComparer.OrdinalIgnoreCase);

    private string ShaderinfoLinePrefix = "// Shaderinfo: ";
    private bool AllowMidiRowDataUpdate = true;

    private List<Data.Preset> PresetsMasterList = new List<Data.Preset>();
    private List<String> shadertoyFilesList = new List<String>();

    private Random rnd = new Random();
    private bool visualizerSpriteModeActive = true;
    private Settings Settings = new Settings();
    private Tags Tags = new Tags();

    private PresetDeck presetDeck = new PresetDeck();
    private Button[] presetDeckButtons = Array.Empty<Button>();
    private readonly Dictionary<int, PendingThumbnail> pendingThumbnailAssignments = new();

    private ShaderHelper ShaderHelper = new ShaderHelper();
    private MidiHelper MidiHelper;
    private RemoteHelper RemoteHelper;
    private OpenFileDialog ofd;
    private OpenFileDialog ofdShader;
    private OpenFileDialog ofdShaderHLSL;

    private readonly Dictionary<int, CancellationTokenSource> KnobActionDelays = new();

    private CancellationTokenSource CancellationTokenFilterPresetList;

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

    private const int VK_B = 0x42;
    private const int VK_K = 0x4B;
    private const int VK_N = 0x4E;
    private const int VK_R = 0x52;

    private const int VK_SPACE = 0x20;
    private const int VK_DELETE = 0x2E;

    private const int VK_BACKSPACE = 0x08;

    private const int VK_CURSOR_UP = 0x26;
    private const int VK_CURSOR_DOWN = 0x28;

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
      Settings,
      TestFonts,
      ClearSprites,
      ClearTexts,
      TimeFactor,
      FrameFactor,
      FpsFactor,
      VisIntensity,
      VisShift,
      VisVersion,
      SpoutActive,
      SpoutFixedSize,
      SpoutResolution,
      RenderQuality,
      QualityAuto,
      ColHue,
      ColSaturation,
      ColBrightness,
      HueAuto,
      HueAutoSeconds,
      CaptureScreenshot,
      VideoInput,
      SpoutInput,
      InputMixOnTop,
      InputMixOpacity,
      InputMixLuma,
      PrecompileCache,
      FFTAttack,
      FFTDecay
    }

    private class PendingThumbnail {
      public Preset Preset { get; set; } = new Preset();
      public DateTime RequestTime { get; set; }
    }

    private void SetAllControlFontSizes(Control parent, float fontSize) {
      foreach (Control ctrl in parent.Controls) {
        ctrl.Font = new Font(ctrl.Font.FontFamily, fontSize, ctrl.Font.Style);
        if (ctrl.HasChildren) {
          SetAllControlFontSizes(ctrl, fontSize);
        }
      }
    }

    private void InitializeSpriteButtonSupport() {
      Button[] buttons = new[] { btn00, btn11, btn22, btn33, btn44, btn55, btn66, btn77, btn88 };
      spriteButtonSectionMap.Clear();
      spriteButtonLabelMap.Clear();

      foreach (Button button in buttons) {
        if (button == null) {
          continue;
        }

        string buttonLabel = button.Text.Trim();
        string section = "img" + buttonLabel;
        spriteButtonSectionMap[button] = section;
        spriteButtonLabelMap[button] = buttonLabel;
        button.MouseUp += SpriteButton_MouseUp;
        button.Resize += SpriteButton_Resize;
      }

      RefreshSpriteButtonImages();
    }

    private void InitializePresetDeckButtons() {
      presetDeckButtons = new[] {
        btnSpace, btnBackspace, btnN, btnB, btnTilde,
        btnF2, btnF3, btnF4, btnF7, btnF10,
        btnAltEnter, btnK, btnDelete, btnWatermark, btnTransparency
      };

      for (int i = 0; i < presetDeckButtons.Length; i++) {
        Button button = presetDeckButtons[i];
        if (button != null) {
          button.Tag = i + 1;
          button.MouseDown += PresetDeckButton_MouseDown;
          button.Resize += PresetDeckButton_Resize;
        }
      }

      btnSwitchMode.Resize += BtnSwitchMode_Resize;

      // btn88 is the bank switcher in preset mode
      btn88.MouseDown += Btn88_MouseDown;
      btn88.Resize += Btn88_Resize;
    }

    private void BtnSwitchMode_Resize(object? sender, EventArgs e) {
      UpdateModeToggleButton();
    }

    private void RefreshSpriteButtonImages(bool reloadConfig = true) {
      if (reloadConfig) {
        LoadSpriteDefinitions();
      }
      LoadMessageDefinitions();

      foreach (Button button in spriteButtonSectionMap.Keys) {
        // Skip btn88 in preset mode - it shows bank icon instead
        if (Settings.IsPresetMode && button == btn88) {
          continue;
        }
        UpdateSpriteButtonAppearance(button);
      }
    }

    private void LoadSpriteDefinitions() {
      spriteSectionImageMap.Clear();
      string spritesPath = Path.Combine(GetVisualizerDir(IsSelectedVisualizerDX12()), visualizerSpritesFile);

      if (!File.Exists(spritesPath)) {
        return;
      }

      try {
        string? currentSection = null;
        foreach (string rawLine in File.ReadAllLines(spritesPath)) {
          string line = rawLine.Trim();
          if (line.Length == 0 || line.StartsWith("//", StringComparison.Ordinal) || line.StartsWith(";", StringComparison.Ordinal)) {
            continue;
          }

          if (line.StartsWith("[", StringComparison.Ordinal) && line.EndsWith("]", StringComparison.Ordinal)) {
            currentSection = line.Substring(1, line.Length - 2).Trim();
            continue;
          }

          if (string.IsNullOrEmpty(currentSection) || !currentSection.StartsWith("img", StringComparison.OrdinalIgnoreCase)) {
            continue;
          }

          int equalsIndex = line.IndexOf('=');
          if (equalsIndex <= 0) {
            continue;
          }

          string key = line.Substring(0, equalsIndex).Trim();
          if (!key.Equals("img", StringComparison.OrdinalIgnoreCase)) {
            continue;
          }

          string value = StripInlineComment(line.Substring(equalsIndex + 1));
          value = value.Replace('/', '\\');
          if (value.Length == 0) {
            continue;
          }

          spriteSectionImageMap[currentSection.ToLowerInvariant()] = value;
        }
      } catch (Exception ex) {
        Program.SaveErrorToFile(ex, "Read sprites.ini");
      }
    }

    private void LoadMessageDefinitions() {
      messageCodeTextMap.Clear();
      string messagesPath = Path.Combine(GetVisualizerDir(IsSelectedVisualizerDX12()), visualizerMessagesFile);

      if (!File.Exists(messagesPath)) {
        return;
      }

      try {
        string? currentSection = null;
        string[] lines;
        using (var fs = new FileStream(messagesPath, FileMode.Open, FileAccess.Read, FileShare.ReadWrite))
        using (var sr = new StreamReader(fs))
          lines = sr.ReadToEnd().Split(['\r', '\n']);
        foreach (string rawLine in lines) {
          string trimmed = rawLine.Trim();
          if (trimmed.Length == 0 || trimmed.StartsWith("//", StringComparison.Ordinal) || trimmed.StartsWith(";", StringComparison.Ordinal)) {
            continue;
          }

          if (trimmed.StartsWith("[", StringComparison.Ordinal) && trimmed.EndsWith("]", StringComparison.Ordinal)) {
            currentSection = trimmed.Substring(1, trimmed.Length - 2).Trim();
            continue;
          }

          if (string.IsNullOrEmpty(currentSection) || !currentSection.StartsWith("message", StringComparison.OrdinalIgnoreCase)) {
            continue;
          }

          string cleanLine = StripInlineComment(trimmed);
          int equalsIndex = cleanLine.IndexOf('=');
          if (equalsIndex <= 0) {
            continue;
          }

          string key = cleanLine.Substring(0, equalsIndex).Trim();
          if (!key.Equals("text", StringComparison.OrdinalIgnoreCase)) {
            continue;
          }

          string value = cleanLine.Substring(equalsIndex + 1).Trim();
          if (value.Length == 0) {
            continue;
          }

          if (value.StartsWith("{", StringComparison.Ordinal) && value.EndsWith("}", StringComparison.Ordinal) && value.Length > 1) {
            value = value.Substring(1, value.Length - 2).Trim();
          }

          value = value.Replace("\\n", Environment.NewLine);

          string messageCode = currentSection.Substring("message".Length).Trim();
          if (messageCode.Length == 0) {
            continue;
          }

          messageCodeTextMap[messageCode] = value;
          messageCodeTextMap[currentSection] = value;
        }
      } catch (Exception ex) {
        Program.SaveErrorToFile(ex, "Read messages.ini");
      }
    }

    private bool TryGetMessageText(string code, out string messageText) {
      messageText = string.Empty;
      if (string.IsNullOrWhiteSpace(code)) {
        return false;
      }

      if (messageCodeTextMap.TryGetValue(code, out string? mapped) && !string.IsNullOrWhiteSpace(mapped)) {
        messageText = mapped;
        return true;
      }

      string sectionKey = "message" + code;
      if (messageCodeTextMap.TryGetValue(sectionKey, out string? sectionValue) && !string.IsNullOrWhiteSpace(sectionValue)) {
        messageText = sectionValue;
        return true;
      }

      return false;
    }

    private void ApplyVisualizerMode(bool isSpriteMode) {
      if (visualizerSpriteModeActive == isSpriteMode) {
        return;
      }

      visualizerSpriteModeActive = isSpriteMode;
      RefreshSpriteButtonImages(false);
    }

    private void UpdateSpriteButtonAppearance(Button button) {
      if (!spriteButtonSectionMap.TryGetValue(button, out string? section)) {
        return;
      }

      string sectionKey = section.ToLowerInvariant();
      spriteSectionImageMap.TryGetValue(sectionKey, out string? configuredPath);
      UpdateSpriteButtonTooltip(button, configuredPath);

      if (!visualizerSpriteModeActive || !Settings.EnableSpriteButtonImage) {
        ShowSpriteLabel(button);
        return;
      }

      if (string.IsNullOrWhiteSpace(configuredPath)) {
        ShowSpriteLabel(button);
        return;
      }

      string resolvedPath = ResolveSpriteImagePath(configuredPath);
      if (string.IsNullOrEmpty(resolvedPath) || !File.Exists(resolvedPath)) {
        ShowSpriteLabel(button);
        return;
      }

      if (button.ClientSize.Width <= 0 || button.ClientSize.Height <= 0) {
        return;
      }

      try {
        Image? preview = CreateMonochromePreview(resolvedPath, button.ClientSize);
        if (preview != null) {
          ApplyButtonImage(button, preview);
        } else {
          ShowSpriteLabel(button);
        }
      } catch (Exception ex) {
        ShowSpriteLabel(button);
        Program.SaveErrorToFile(ex, "Sprite preview");
      }
    }

    private void ApplyButtonImage(Button button, Image preview) {
      DisposeButtonImage(button);
      button.Image = preview;
      buttonImageCache[button] = preview;
      button.Text = string.Empty;
      button.Padding = new Padding(2);
      button.ImageAlign = ContentAlignment.MiddleCenter;
      button.TextImageRelation = TextImageRelation.Overlay;
    }

    private void DisposeButtonImage(Button button) {
      if (buttonImageCache.TryGetValue(button, out Image? cached) && cached != null) {
        if (button.Image == cached) {
          button.Image = null;
        }
        cached.Dispose();
        buttonImageCache.Remove(button);
      } else {
        button.Image = null;
      }
    }

    private void ShowSpriteLabel(Button button) {
      DisposeButtonImage(button);
      if (spriteButtonLabelMap.TryGetValue(button, out string? label)) {
        button.Text = label;
      }
      button.Padding = new Padding(0);
      button.TextAlign = ContentAlignment.MiddleCenter;
      button.ImageAlign = ContentAlignment.MiddleCenter;
      button.TextImageRelation = TextImageRelation.Overlay;
    }

    private void UpdateSpriteButtonTooltip(Button button, string? configuredPath) {
      if (toolTip1 == null) {
        return;
      }

      if (!visualizerSpriteModeActive) {
        string label = spriteButtonLabelMap.TryGetValue(button, out string? mappedLabel) ? mappedLabel : string.Empty;
        if (!string.IsNullOrWhiteSpace(label) && TryGetMessageText(label, out string messageText)) {
          toolTip1.SetToolTip(button, $"Message {label}: {messageText}");
        } else if (!string.IsNullOrWhiteSpace(label)) {
          toolTip1.SetToolTip(button, $"Message {label}: (not defined in {visualizerMessagesFile})");
        } else {
          toolTip1.SetToolTip(button, "Message slot");
        }
        return;
      }

      if (button == btn00) {
        toolTip1.SetToolTip(button, "Cover slot");
        return;
      }

      string header = string.IsNullOrWhiteSpace(configuredPath) ? "(no image assigned)" : configuredPath;
      string text = header + Environment.NewLine + "Right-click to change image";
      toolTip1.SetToolTip(button, text);
    }

    private string ResolveSpriteImagePath(string configuredPath) {
      string normalized = configuredPath.Replace('/', Path.DirectorySeparatorChar).Trim();
      if (string.IsNullOrWhiteSpace(normalized)) {
        return string.Empty;
      }

      try {
        if (Path.IsPathRooted(normalized)) {
          return Path.GetFullPath(normalized);
        }

        string candidate = Path.GetFullPath(Path.Combine(BaseDir, normalized));
        if (File.Exists(candidate)) {
          return candidate;
        }

        // Try resources fallback for: simple filenames, or paths starting with "sprites\"
        bool isSimpleFilename = !normalized.Contains(Path.DirectorySeparatorChar) && !normalized.Contains(Path.AltDirectorySeparatorChar);
        bool isSpritesPath = normalized.StartsWith("sprites" + Path.DirectorySeparatorChar, StringComparison.OrdinalIgnoreCase) ||
                             normalized.StartsWith("sprites/", StringComparison.OrdinalIgnoreCase);

        if (isSimpleFilename || isSpritesPath) {
          string resourcesFallback = Path.GetFullPath(Path.Combine(BaseDir, "resources", normalized));
          if (File.Exists(resourcesFallback)) {
            return resourcesFallback;
          }
        }

        return candidate;
      } catch (Exception) {
        return string.Empty;
      }
    }

    private Image? CreateMonochromePreview(string imagePath, Size targetSize) {
      if (targetSize.Width <= 0 || targetSize.Height <= 0) {
        return null;
      }

      using FileStream stream = new FileStream(imagePath, FileMode.Open, FileAccess.Read, FileShare.ReadWrite);
      using Image temp = Image.FromStream(stream, useEmbeddedColorManagement: false, validateImageData: false);
      using Bitmap source = new Bitmap(temp);

      Rectangle destRect = CalculatePreviewBounds(source.Size, targetSize);
      if (destRect.Width <= 0 || destRect.Height <= 0) {
        return null;
      }

      Bitmap preview = new Bitmap(targetSize.Width, targetSize.Height, PixelFormat.Format32bppArgb);
      using (Graphics g = Graphics.FromImage(preview)) {
        g.Clear(Color.Transparent);
        g.CompositingQuality = CompositingQuality.HighQuality;
        g.InterpolationMode = InterpolationMode.HighQualityBicubic;
        g.PixelOffsetMode = PixelOffsetMode.HighQuality;
        g.SmoothingMode = SmoothingMode.HighQuality;

        if (Settings.EnableColorButtonImages) {
          g.DrawImage(source, destRect, 0, 0, source.Width, source.Height, GraphicsUnit.Pixel);
        } else {
          using ImageAttributes attributes = new ImageAttributes();
          ColorMatrix grayscale = new ColorMatrix(new float[][] {
            new float[] { 0.299f, 0.299f, 0.299f, 0, 0 },
            new float[] { 0.587f, 0.587f, 0.587f, 0, 0 },
            new float[] { 0.114f, 0.114f, 0.114f, 0, 0 },
            new float[] { 0, 0, 0, 1, 0 },
            new float[] { 0, 0, 0, 0, 1 }
          });
          attributes.SetColorMatrix(grayscale);
          g.DrawImage(source, destRect, 0, 0, source.Width, source.Height, GraphicsUnit.Pixel, attributes);
        }
      }

      return preview;
    }

    private Image? CreatePresetThumbnail(string imagePath, Size targetSize) {
      if (targetSize.Width <= 0 || targetSize.Height <= 0) {
        return null;
      }

      using FileStream stream = new FileStream(imagePath, FileMode.Open, FileAccess.Read, FileShare.ReadWrite);
      using Image temp = Image.FromStream(stream, useEmbeddedColorManagement: false, validateImageData: false);
      using Bitmap source = new Bitmap(temp);

      const int margin = 4;
      int contentWidth = Math.Max(1, targetSize.Width - margin * 2);
      int contentHeight = Math.Max(1, targetSize.Height - margin * 2);

      float scaleX = (float)contentWidth / source.Width;
      float scaleY = (float)contentHeight / source.Height;
      float scale = Math.Max(scaleX, scaleY);

      int scaledWidth = (int)Math.Round(source.Width * scale);
      int scaledHeight = (int)Math.Round(source.Height * scale);

      int sourceX = 0;
      int sourceY = 0;
      int sourceWidth = source.Width;
      int sourceHeight = source.Height;

      if (scaledWidth > contentWidth) {
        sourceWidth = (int)Math.Round(contentWidth / scale);
        sourceX = (source.Width - sourceWidth) / 2;
      }

      if (scaledHeight > contentHeight) {
        sourceHeight = (int)Math.Round(contentHeight / scale);
        sourceY = (source.Height - sourceHeight) / 2;
      }

      Bitmap preview = new Bitmap(targetSize.Width, targetSize.Height, PixelFormat.Format32bppArgb);
      using (Graphics g = Graphics.FromImage(preview)) {
        g.Clear(Color.Transparent);
        g.CompositingQuality = CompositingQuality.HighQuality;
        g.InterpolationMode = InterpolationMode.HighQualityBicubic;
        g.PixelOffsetMode = PixelOffsetMode.HighQuality;
        g.SmoothingMode = SmoothingMode.HighQuality;

        Rectangle destRect = new Rectangle(margin, margin, contentWidth, contentHeight);

        if (Settings.EnableColorButtonImages) {
          g.DrawImage(source, destRect, sourceX, sourceY, sourceWidth, sourceHeight, GraphicsUnit.Pixel);
        } else {
          using ImageAttributes attributes = new ImageAttributes();
          ColorMatrix grayscale = new ColorMatrix(new float[][] {
            new float[] { 0.299f, 0.299f, 0.299f, 0, 0 },
            new float[] { 0.587f, 0.587f, 0.587f, 0, 0 },
            new float[] { 0.114f, 0.114f, 0.114f, 0, 0 },
            new float[] { 0, 0, 0, 1, 0 },
            new float[] { 0, 0, 0, 0, 1 }
          });
          attributes.SetColorMatrix(grayscale);
          g.DrawImage(source, destRect, sourceX, sourceY, sourceWidth, sourceHeight, GraphicsUnit.Pixel, attributes);
        }
      }

      return preview;
    }

    private Rectangle CalculatePreviewBounds(Size original, Size target) {
      if (original.Width <= 0 || original.Height <= 0 || target.Width <= 0 || target.Height <= 0) {
        return Rectangle.Empty;
      }

      const int margin = 9;
      int availableWidth = Math.Max(1, target.Width - margin * 2);
      int availableHeight = Math.Max(1, target.Height - margin * 2);

      float scaleX = (float)availableWidth / original.Width;
      float scaleY = (float)availableHeight / original.Height;
      float scale = Math.Min(scaleX, scaleY);

      if (scale <= 0) {
        scale = Math.Min((float)target.Width / original.Width, (float)target.Height / original.Height);
      }

      int width = Math.Max(1, (int)Math.Round(original.Width * scale));
      int height = Math.Max(1, (int)Math.Round(original.Height * scale));
      int x = margin + (availableWidth - width) / 2;
      int y = margin + (availableHeight - height) / 2;
      return new Rectangle(x, y, width, height);
    }

    private void SpriteButton_MouseUp(object? sender, MouseEventArgs e) {
      if (e.Button == MouseButtons.Right && visualizerSpriteModeActive && sender is Button button && spriteButtonSectionMap.ContainsKey(button) && button != btn00) {
        PromptSpriteImageSelection(button);
      }
    }

    private void SpriteButton_Resize(object? sender, EventArgs e) {
      if (sender is Button button && spriteButtonSectionMap.ContainsKey(button)) {
        UpdateSpriteButtonAppearance(button);
      }
    }

    private void PromptSpriteImageSelection(Button button) {
      if (button == btn00) {
        return;
      }

      if (!spriteButtonSectionMap.TryGetValue(button, out string? section)) {
        return;
      }

      string? configuredPath = GetConfiguredSpritePath(section);
      string initialDirectory = BaseDir;
      if (!string.IsNullOrWhiteSpace(configuredPath)) {
        string resolved = ResolveSpriteImagePath(configuredPath);
        if (!string.IsNullOrEmpty(resolved) && File.Exists(resolved)) {
          string? directory = Path.GetDirectoryName(resolved);
          if (!string.IsNullOrEmpty(directory) && Directory.Exists(directory)) {
            initialDirectory = directory;
          }
        }
      }

      using OpenFileDialog dialog = new OpenFileDialog {
        Title = $"Select image for {section.ToUpperInvariant()}",
        Filter = "Images|*.png;*.jpg;*.jpeg;*.bmp;*.gif;*.dds;*.tga;*.tif;*.tiff|All files|*.*",
        RestoreDirectory = true
      };

      if (Directory.Exists(initialDirectory)) {
        dialog.InitialDirectory = initialDirectory;
      }

      if (dialog.ShowDialog(this) == DialogResult.OK) {
        string storedPath = GetSpriteConfigPath(dialog.FileName);
        if (UpdateSpriteDefinition(section, storedPath)) {
          spriteSectionImageMap[section.ToLowerInvariant()] = storedPath;
          RefreshSpriteButtonImages();
          SetStatusText($"Sprite {section.ToUpperInvariant()} image updated");
        } else {
          SetStatusText($"Unable to update sprite {section.ToUpperInvariant()}");
        }
      }
    }

    private string? GetConfiguredSpritePath(string section) {
      string key = section.ToLowerInvariant();
      if (spriteSectionImageMap.TryGetValue(key, out string? value)) {
        return value;
      }
      return null;
    }

    private string GetSpriteConfigPath(string absolutePath) {
      try {
        string fullPath = Path.GetFullPath(absolutePath);
        string spritesRoot = Path.GetFullPath(Path.Combine(BaseDir, "sprites"));
        string resourcesSpritesRoot = Path.GetFullPath(Path.Combine(BaseDir, Path.Combine("resources", "sprites")));

        // Try to make relative if inside sprites folders
        string? spriteRelative = TryBuildSpriteRelative(fullPath, spritesRoot);
        if (spriteRelative != null) {
          return spriteRelative;
        }

        spriteRelative = TryBuildSpriteRelative(fullPath, resourcesSpritesRoot);
        if (spriteRelative != null) {
          return spriteRelative;
        }

        // For files outside sprites folders (e.g., capture\), save absolute path
        // This handles Debug/Release BaseDir differences and makes it clear where the file is
        return fullPath.Replace('/', '\\');
      } catch (Exception) {
        // ignore
      }
      return absolutePath.Replace('/', '\\');
    }

    private static string? TryBuildSpriteRelative(string fullPath, string root) {
      if (!fullPath.StartsWith(root, StringComparison.OrdinalIgnoreCase)) {
        return null;
      }

      if (fullPath.Length == root.Length) {
        return "sprites";
      }

      char separator = Path.DirectorySeparatorChar;
      if (fullPath.Length > root.Length && fullPath[root.Length] != separator && fullPath[root.Length] != Path.AltDirectorySeparatorChar) {
        return null;
      }

      string remainder = fullPath.Substring(root.Length).TrimStart(separator, Path.AltDirectorySeparatorChar);
      return string.IsNullOrEmpty(remainder)
        ? "sprites"
        : ("sprites" + '\\' + remainder.Replace('/', '\\'));
    }

    private bool UpdateSpriteDefinition(string section, string newValue) {
      string spritesPath = Path.Combine(GetVisualizerDir(IsSelectedVisualizerDX12()), visualizerSpritesFile);
      if (!File.Exists(spritesPath)) {
        return false;
      }

      try {
        List<string> lines = File.ReadAllLines(spritesPath).ToList();
        string header = $"[{section}]";
        bool inSection = false;
        bool updated = false;

        for (int i = 0; i < lines.Count; i++) {
          string trimmed = lines[i].Trim();
          if (trimmed.StartsWith("[", StringComparison.Ordinal) && trimmed.EndsWith("]", StringComparison.Ordinal)) {
            if (string.Equals(trimmed, header, StringComparison.OrdinalIgnoreCase)) {
              inSection = true;
              continue;
            }

            if (inSection) {
              break;
            }

            continue;
          }

          if (!inSection) {
            continue;
          }

          string withoutComment = StripInlineComment(trimmed);
          if (withoutComment.StartsWith("img=", StringComparison.OrdinalIgnoreCase)) {
            int imgIndex = lines[i].IndexOf("img=", StringComparison.OrdinalIgnoreCase);
            int commentIndex = FindInlineCommentIndex(lines[i], imgIndex + 4);
            string commentSuffix = commentIndex >= 0 ? lines[i].Substring(commentIndex) : string.Empty;
            string leading = lines[i].Substring(0, imgIndex);
            lines[i] = $"{leading}img={newValue}{commentSuffix}";
            updated = true;
            break;
          }
        }

        if (!updated) {
          for (int i = 0; i < lines.Count; i++) {
            if (string.Equals(lines[i].Trim(), header, StringComparison.OrdinalIgnoreCase)) {
              lines.Insert(i + 1, $"img={newValue}");
              updated = true;
              break;
            }
          }
        }

        if (updated) {
          File.WriteAllLines(spritesPath, lines);
        }

        return updated;
      } catch (Exception ex) {
        Program.SaveErrorToFile(ex, "Update sprites.ini");
        return false;
      }
    }

    private static string StripInlineComment(string value) {
      string trimmed = value.Trim();
      int commentIndex = trimmed.IndexOf("//", StringComparison.Ordinal);
      if (commentIndex >= 0) {
        trimmed = trimmed.Substring(0, commentIndex);
      }
      commentIndex = trimmed.IndexOf(';');
      if (commentIndex >= 0) {
        trimmed = trimmed.Substring(0, commentIndex);
      }
      return trimmed.Trim();
    }

    private static int FindInlineCommentIndex(string line, int startIndex) {
      int slashIndex = line.IndexOf("//", startIndex, StringComparison.Ordinal);
      int semicolonIndex = line.IndexOf(';', startIndex);
      if (slashIndex >= 0 && semicolonIndex >= 0) {
        return Math.Min(slashIndex, semicolonIndex);
      }

      if (slashIndex >= 0) {
        return slashIndex;
      }

      return semicolonIndex;
    }

    private void DisposeSpriteButtonImages() {
      foreach (KeyValuePair<Button, Image?> kvp in buttonImageCache.ToList()) {
        if (kvp.Value != null) {
          kvp.Value.Dispose();
        }
        if (kvp.Key.Image == kvp.Value) {
          kvp.Key.Image = null;
        }
      }
      buttonImageCache.Clear();
    }

    public MilkwaveRemoteForm() {
      Program.LogToFile("### Milkwave Remote Starting ###");
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
      } catch (Exception) {
        Settings = new Settings();
      }

      try {
        string tagsFile = Path.Combine(BaseDir, milkwaveTagsFile);
        string tagsJson = File.ReadAllText(tagsFile);
        Tags? loadedTags = JsonSerializer.Deserialize<Tags>(tagsJson, new JsonSerializerOptions {
          PropertyNameCaseInsensitive = true
        });
        if (loadedTags != null) {
          Tags = loadedTags;
          SetTopTags();
        }
      } catch (Exception) {
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
          cboFontMenu.Items.Add(font.Name);
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

      monitorTimer = new System.Windows.Forms.Timer();
      monitorTimer.Tick += MonitorTimer_Tick;
      monitorTimer.Interval = Settings.MonitorPollingInterval;

      controllerTimer = new System.Windows.Forms.Timer();
      controllerTimer.Tick += ControllerTimer_Tick;
      controllerTimer.Interval = 50; // 20Hz polling

      toolStripStatusLabelMonitorCPU.Text = "";
      toolStripStatusLabelMonitorGPU.Text = "";

      tabControl.SelectedIndex = Settings.SelectedTabIndex;
#if !DEBUG
      ConnectToVisualizer();
#else
      ScanAndPopulateVisualizers(); // DEBUG: scan only, no auto-launch
#endif
      cboSettingsOpenFile.SelectedIndex = 0;

      InitializeSpriteButtonSupport();
      InitializePresetDeckButtons();
      LoadPresetDeck();
      ApplyPanelMode();
    }

    private void MilkwaveRemoteForm_Load(object sender, EventArgs e) {
      LoadAndSetSettings();
      RefreshSpriteButtonImages();
      ApplyPanelMode(); // This will call UpdateModeToggleButton
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

      RemoteHelper = new RemoteHelper(Path.Combine(GetVisualizerDir(IsSelectedVisualizerDX12()), visualizerSettingsFile));

      string fTimeBetweenPresets = RemoteHelper.GetIniValue("Settings", "fTimeBetweenPresets", "60");
      if (!decimal.TryParse(fTimeBetweenPresets, NumberStyles.Float, CultureInfo.InvariantCulture, out var timeBetweenPresets)) {
        timeBetweenPresets = 60m;
      }
      numPresetChange.Value = Math.Clamp(timeBetweenPresets, numPresetChange.Minimum, numPresetChange.Maximum);
    }

    private void StartVisualizerIfNotFound(bool onlyIfNotFound) {
      if (IsPipeConnected && onlyIfNotFound)
        return;

      // Launch the visualizer and connect via pipe
      LaunchAndConnectVisualizer();
    }

    /// <summary>
    /// Initialize all device lists using OBS-style enumeration pattern
    /// </summary>
    private void InitializeDeviceLists() {
      try {
        // Audio devices (existing NAudio-based enumeration)
        RemoteHelper.FillAudioDevices(cboAudioDevice);

        // Video devices (OBS-style DirectShow enumeration)
        if (cboVideoInput != null) {
          string savedVideoDevice = RemoteHelper.GetIniValue("Milkwave", "VideoDevice", "");
          DeviceManager.PopulateVideoDevices(cboVideoInput, savedVideoDevice);
          chkVideoMix.Enabled = cboVideoInput.Enabled;
          if (!chkVideoMix.Enabled) chkVideoMix.Checked = false;
        }

        // Spout senders (OBS-style pattern with registry access)
        if (cboSputInput != null) {
          string savedSpoutSender = RemoteHelper.GetIniValue("Milkwave", "SpoutSender", "");
          DeviceManager.PopulateSpoutSenders(cboSputInput, savedSpoutSender);
          chkSpoutMix.Enabled = cboSputInput.Enabled;
          if (!chkSpoutMix.Enabled) chkSpoutMix.Checked = false;
          StartSpoutRefreshTimer();
        }

        // Game controllers (winmm.dll enumeration)
        if (cboInputController != null) {
          string savedController = RemoteHelper.GetIniValue("Milkwave", "Controller", "");
          DeviceManager.PopulateGameControllers(cboInputController, savedController);
          chkControllerActive.Enabled = cboInputController.Enabled;
          if (!chkControllerActive.Enabled) chkControllerActive.Checked = false;
        }
      } catch (Exception ex) {
        Debug.WriteLine($"Error initializing device lists: {ex.Message}");
      }
    }

    /// <summary>
    /// Start periodic Spout sender refresh timer
    /// Allows detection of new Spout senders appearing at runtime
    /// </summary>
    private void StartSpoutRefreshTimer() {
      try {
        if (spoutRefreshTimer == null) {
          spoutRefreshTimer = new System.Windows.Forms.Timer();
          spoutRefreshTimer.Interval = 2000; // Refresh every 2 seconds
          spoutRefreshTimer.Tick += (s, e) => {
            try {
              DeviceManager.RefreshSpoutSenders(cboSputInput);
              chkSpoutMix.Enabled = cboSputInput.Enabled;
              if (!chkSpoutMix.Enabled) chkSpoutMix.Checked = false;
            } catch (Exception ex) {
              Debug.WriteLine($"Error refreshing Spout senders: {ex.Message}");
            }
          };
          spoutRefreshTimer.Start();
        }
      } catch (Exception ex) {
        Debug.WriteLine($"Error starting Spout refresh timer: {ex.Message}");
      }
    }

    private bool IsPipeConnected => _pipeClient?.IsConnected == true;
    private List<(int pid, string name, string exePath)> _discoveredInstances = new();

    /// <summary>
    /// Unsubscribes pipe events and disposes the client so that intentional
    /// disconnects never trigger OnPipeDisconnected and cause a cascade loop.
    /// </summary>
    private void DetachAndDisposePipeClient() {
      if (_pipeClient == null) return;
      _pipeClient.Disconnected -= OnPipeDisconnected;
      _pipeClient.MessageReceived -= OnPipeMessageReceived;
      _pipeClient.Dispose();
      _pipeClient = null;
    }

    private void ScanAndPopulateVisualizers() {
      _discoveredInstances = PipeClient.DiscoverVisualizers();

      cboVisualizerInstance.SelectedIndexChanged -= cboWindowTitle_SelectedIndexChanged;
      cboVisualizerInstance.Items.Clear();

      bool found = _discoveredInstances.Count > 0;
      if (!found) {
        cboVisualizerInstance.Items.Add("No visualizers");
      } else {
        foreach (var (pid, name, _) in _discoveredInstances) {
          cboVisualizerInstance.Items.Add($"{name} (PID: {pid})");
        }
      }
      cboVisualizerInstance.Enabled = found;
      cboVisualizerInstance.SelectedIndex = 0;
      cboVisualizerInstance.SelectedIndexChanged += cboWindowTitle_SelectedIndexChanged;
    }

    private void btnVisualizerScan_Click(object? sender, EventArgs e) {
      DetachAndDisposePipeClient();
      ScanAndPopulateVisualizers();

      if (_discoveredInstances.Count > 0) {
        ConnectToInstance(_discoveredInstances[0]);
      } else {
        SetStatusText("No visualizer instances found");
      }
    }

    private void btnVisualizerDX9_Click(object sender, EventArgs e) {
      LaunchVisualizerExe(isDX12: false);
    }

    private void btnVisualizerDX12_Click(object sender, EventArgs e) {
      LaunchVisualizerExe(isDX12: true);
    }

    /// <summary>
    /// Launches a new visualizer window (DX9 or DX12), then polls for its pipe
    /// and selects it in cboVisualizerInstance once it appears.
    /// </summary>
    private void LaunchVisualizerExe(bool isDX12) {
      string exePath = isDX12 ? Settings.VisualizerExeDX12 : Settings.VisualizerExe;
      if (string.IsNullOrEmpty(exePath)) {
        exePath = isDX12 ? "MDropDX12.exe" : "MilkwaveVisualizer.exe";
      }

      if (!Path.IsPathRooted(exePath)) {
        exePath = Path.Combine(GetVisualizerDir(isDX12), exePath);
      }

      if (!File.Exists(exePath)) {
        SetStatusText($"Visualizer not found: {exePath}");
        return;
      }

      // Snapshot existing PIDs so we can identify the newly spawned instance
      var knownPids = new HashSet<int>(PipeClient.DiscoverVisualizers().Select(v => v.pid));

      // First-time DX12 run (no cache folder) takes much longer to start due to shader compilation
      bool isFirstDX12Run = isDX12 && !Directory.Exists(Path.Combine(GetVisualizerDir(isDX12), "cache"));
      int pollCount = isFirstDX12Run ? 150 : 50;
      int pollIntervalMs = isFirstDX12Run ? 200 : 100;

      try {
        Process.Start(new ProcessStartInfo(exePath) { UseShellExecute = true });
        SetStatusText($"Launched {Path.GetFileName(exePath)}");
      } catch (Exception ex) {
        SetStatusText($"Failed to launch: {ex.Message}");
        return;
      }

      // Poll off the UI thread so the form stays responsive
      Task.Run(() => {
        for (int i = 0; i < pollCount; i++) {
          Thread.Sleep(pollIntervalMs);
          var instances = PipeClient.DiscoverVisualizers();
          var newInstance = instances.FirstOrDefault(v => !knownPids.Contains(v.pid));
          if (newInstance.pid != 0) {
            BeginInvoke(() => {
              ScanAndPopulateVisualizers();
              // Select and connect to the newly opened instance
              int idx = _discoveredInstances.FindIndex(v => v.pid == newInstance.pid);
              if (idx >= 0) {
                cboVisualizerInstance.SelectedIndexChanged -= cboWindowTitle_SelectedIndexChanged;
                cboVisualizerInstance.SelectedIndex = idx;
                cboVisualizerInstance.SelectedIndexChanged += cboWindowTitle_SelectedIndexChanged;
                ConnectToInstance(_discoveredInstances[idx]);
              }
            });
            return;
          }
        }
        // Timed out — still rescan so the combo reflects current state
        BeginInvoke(() => {
          ScanAndPopulateVisualizers();
          SetStatusText("Visualizer launched but pipe not ready — try Scan");
        });
      });
      SetStatusText("");
    }

    private void ConnectToVisualizer() {
     DetachAndDisposePipeClient();

      ScanAndPopulateVisualizers();

      if (_discoveredInstances.Count == 0) {
        // No visualizers running — try to launch the last known one
        if (!string.IsNullOrEmpty(Settings.VisualizerExe)) {
          LaunchAndConnectVisualizer();
        } else {
          SetStatusText("No visualizer instances found");
        }
        return;
      }

      ConnectToInstance(_discoveredInstances[0]);
    }

    private void ConnectToInstance((int pid, string name, string exePath) target, bool autoSwitch = true) {
     DetachAndDisposePipeClient();
     _pipeClient = new PipeClient();
     _pipeClient.MessageReceived += OnPipeMessageReceived;
     _pipeClient.Disconnected += OnPipeDisconnected;

      if (_pipeClient.Connect(target.pid)) {
        SetStatusText($"Connected to {target.name} (PID: {target.pid})");

        // Remember the full exe path in the correct setting for future auto-launch
        string resolvedExe = target.exePath;
        if (!string.IsNullOrEmpty(resolvedExe) && File.Exists(resolvedExe)) {
          bool targetIsDX12 = target.name.Contains("DX12", StringComparison.OrdinalIgnoreCase);
          if (targetIsDX12) {
            Settings.VisualizerExeDX12 = resolvedExe;
          } else {
            Settings.VisualizerExe = resolvedExe;
          }
          SaveSettingsToFile();
        }

        // Request current state (running preset, wave params, settings, etc.)
        SendToMilkwaveVisualizer("", MessageType.GetState);
      } else {
        string failedMsg = $"Failed to connect";
        DetachAndDisposePipeClient();
        ScanAndPopulateVisualizers();

        if (autoSwitch && _discoveredInstances.Count > 0) {
          int highestIdx = _discoveredInstances
            .Select((inst, idx) => (inst.pid, idx))
            .OrderByDescending(x => x.pid)
            .First().idx;
          cboVisualizerInstance.SelectedIndexChanged -= cboWindowTitle_SelectedIndexChanged;
          cboVisualizerInstance.SelectedIndex = highestIdx;
          cboVisualizerInstance.SelectedIndexChanged += cboWindowTitle_SelectedIndexChanged;
          var sw = _discoveredInstances[highestIdx];
          ConnectToInstance(sw, autoSwitch: false); // prevent recursive auto-switch
          SetStatusText($"{failedMsg} — switched to {sw.name} (PID: {sw.pid})");
        } else {
          SetStatusText(failedMsg);
        }
      }
    }

    private void LaunchAndConnectVisualizer() {
      bool isDX12 = IsSelectedVisualizerDX12();
      string exePath = isDX12 ? Settings.VisualizerExeDX12 : Settings.VisualizerExe;
      if (string.IsNullOrEmpty(exePath)) {
        // Fall back to default exe name
        exePath = isDX12 ? "MDropDX12.exe" : "MilkwaveVisualizer.exe";
      }

      // If it's just a filename, resolve relative to the appropriate visualizer dir
      if (!Path.IsPathRooted(exePath)) {
        exePath = Path.Combine(GetVisualizerDir(isDX12), exePath);
      }

      if (!File.Exists(exePath)) {
        SetStatusText($"Visualizer not found: {exePath}");
        return;
      }

      SetStatusText($"Launching {Path.GetFileName(exePath)}...");
      try {
        Process.Start(new ProcessStartInfo(exePath) { UseShellExecute = true });
      } catch (Exception ex) {
        SetStatusText($"Failed to launch: {ex.Message}");
        return;
      }

      // Poll for pipe to appear (up to 5 seconds)
      for (int i = 0; i < 50; i++) {
        Thread.Sleep(100);
        var instances = PipeClient.DiscoverVisualizers();
        if (instances.Count > 0) {
          ScanAndPopulateVisualizers();
          ConnectToInstance(instances[0]);
          return;
        }
      }

      ScanAndPopulateVisualizers();
      SetStatusText("Visualizer launched but pipe not ready — try Scan");
    }

    private void cboWindowTitle_SelectedIndexChanged(object? sender, EventArgs e) {
      int idx = cboVisualizerInstance.SelectedIndex;
      if (idx >= 0 && idx < _discoveredInstances.Count) {
        ConnectToInstance(_discoveredInstances[idx]);
      }
    }

    private void OnPipeDisconnected() {
      if (InvokeRequired) {
        BeginInvoke(new Action(OnPipeDisconnected));
        return;
      }

      int closedPid = _pipeClient?.ConnectedPid ?? 0;
      bool wasSelected = closedPid != 0 &&
                         cboVisualizerInstance.SelectedIndex >= 0 &&
                         cboVisualizerInstance.SelectedIndex < _discoveredInstances.Count &&
                         _discoveredInstances[cboVisualizerInstance.SelectedIndex].pid == closedPid;

     // The pipe already disconnected naturally — just clean up without triggering the event again
     if (_pipeClient != null) {
       _pipeClient.Disconnected -= OnPipeDisconnected;
       _pipeClient.MessageReceived -= OnPipeMessageReceived;
       _pipeClient.Dispose();
       _pipeClient = null;
     }

      ScanAndPopulateVisualizers();

      if (_discoveredInstances.Count == 0) {
        SetStatusText("Visualizer closed — no instances remaining");
        return;
      }

      if (wasSelected) {
        // Select and connect to the instance with the highest PID
        int highestIdx = _discoveredInstances
          .Select((inst, idx) => (inst.pid, idx))
          .OrderByDescending(x => x.pid)
          .First().idx;
        cboVisualizerInstance.SelectedIndexChanged -= cboWindowTitle_SelectedIndexChanged;
        cboVisualizerInstance.SelectedIndex = highestIdx;
        cboVisualizerInstance.SelectedIndexChanged += cboWindowTitle_SelectedIndexChanged;
        ConnectToInstance(_discoveredInstances[highestIdx]);
        SetStatusText($"Visualizer closed — switched to {_discoveredInstances[highestIdx].name} (PID: {_discoveredInstances[highestIdx].pid})");
      } else {
        SetStatusText("Visualizer closed");
      }
    }

    private void OnPipeMessageReceived(string message) {
      if (InvokeRequired) {
        BeginInvoke(new Action<string>(OnPipeMessageReceived), message);
        return;
      }

      try {
        if (message.StartsWith("SIGNAL|")) {
          string signal = message.Substring(7);
          if (signal == "NEXT_PRESET") {
            if (chkPresetRandom.Checked) SelectRandomPreset(); else SelectNextPreset();
            btnPresetSend_Click(null, null);
          } else if (signal == "PREV_PRESET") {
            SelectPreviousPreset();
            btnPresetSend_Click(null, null);
          } else if (signal == "COVER_CHANGED") {
            RefreshSpriteButtonImages(false);
          } else if (signal == "SPRITE_MODE") {
            ApplyVisualizerMode(true);
          } else if (signal == "MESSAGE_MODE") {
            ApplyVisualizerMode(false);
          }
        } else if (message.StartsWith("WAVE|")) {
          string waveInfo = message.Substring(message.IndexOf("|") + 1);
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
              } catch { }
            }
          }
          updatingWaveParams = false;
        } else if (message.StartsWith("PRESET=")) {
          string presetFilePath = message.Substring(message.IndexOf("=") + 1);
          if (message.Length > 0) {
            string findString = "RESOURCES\\PRESETS\\";
            int index = message.IndexOf(findString, StringComparison.CurrentCultureIgnoreCase);
            string displayText = message;
            if (index > -1) {
              displayText = message.Substring(index + findString.Length);
              displayText = Path.ChangeExtension(displayText, null);
            }
            SetRunningPresetText(displayText);
            toolTip1.SetToolTip(txtVisRunning, presetFilePath);
            UpdateTagsDisplay(false, true);
          }
        } else if (message.StartsWith("STATUS=")) {
          string status = message.Substring(message.IndexOf("=") + 1);
          if (status.Length > 0) {
            SetStatusText(status);
          }
          if (status.Equals("Sprite Mode set", StringComparison.OrdinalIgnoreCase)) {
            ApplyVisualizerMode(true);
          } else if (status.Equals("Message Mode set", StringComparison.OrdinalIgnoreCase)) {
            ApplyVisualizerMode(false);
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
        } else if (message.StartsWith("OPACITY=")) {
          string opacity = message.Substring(message.IndexOf("=") + 1);
          if (int.TryParse(opacity, out int parsedOpacity) && parsedOpacity >= 0 && parsedOpacity <= 100) {
            if (numOpacity.Value != parsedOpacity) {
              numOpacity.ValueChanged -= numOpacity_ValueChanged;
              numOpacity.Value = parsedOpacity;
              numOpacity.ValueChanged += numOpacity_ValueChanged;
            }
          }
        } else if (message.StartsWith("DEVICE=")) {
          string device = message.Substring(message.IndexOf("=") + 1);
          RemoteHelper.SelectDeviceByName(cboAudioDevice, device);
        } else if (message.StartsWith("SETTINGS|")) {
          string settingsInfo = message.Substring(message.IndexOf("|") + 1);
          string[] settingsParams = settingsInfo.Split('|');
          updatingSettingsParams = true;
          foreach (string param in settingsParams) {
            string[] keyValue = param.Split('=');
            if (keyValue.Length == 2) {
              string key = keyValue[0].Trim();
              string value = keyValue[1].Trim();
              try {
                if (key.Equals("ACTIVE", StringComparison.OrdinalIgnoreCase)) {
                  chkSpoutActive.Checked = value.Equals("1", StringComparison.OrdinalIgnoreCase);
                } else if (key.Equals("FIXEDSIZE", StringComparison.OrdinalIgnoreCase)) {
                  chkSpoutFixedSize.Checked = value.Equals("1", StringComparison.OrdinalIgnoreCase);
                } else if (key.Equals("FIXEDWIDTH", StringComparison.OrdinalIgnoreCase)) {
                  cboSpoutWidth.Text = value;
                } else if (key.Equals("FIXEDHEIGHT", StringComparison.OrdinalIgnoreCase)) {
                  cboSpoutHeight.Text = value;
                } else if (key.Equals("QUALITY", StringComparison.OrdinalIgnoreCase)) {
                  numQuality.Value = decimal.Parse(value, CultureInfo.InvariantCulture);
                } else if (key.Equals("AUTO", StringComparison.OrdinalIgnoreCase)) {
                  chkQualityAuto.Checked = value.Equals("1", StringComparison.OrdinalIgnoreCase);
                } else if (key.Equals("HUE", StringComparison.OrdinalIgnoreCase)) {
                  if (decimal.TryParse(value, NumberStyles.Float, CultureInfo.InvariantCulture, out decimal parsedHue)) {
                    numSettingsHue.Value = Math.Clamp(parsedHue, numSettingsHue.Minimum, numSettingsHue.Maximum);
                  }
                } else if (key.Equals("LOCKED", StringComparison.OrdinalIgnoreCase)) {
                  chkPresetLocked.Checked = value.Equals("1", StringComparison.OrdinalIgnoreCase);
                } else if (key.Equals("RANDOM", StringComparison.OrdinalIgnoreCase)) {
                  chkSettingsPresetRandom.Checked = value.Equals("1", StringComparison.OrdinalIgnoreCase);
                } else if (key.Equals("INPUTTOP", StringComparison.OrdinalIgnoreCase)) {
                  chkInputTop.Checked = value.Equals("1", StringComparison.OrdinalIgnoreCase);
                } else if (key.Equals("LUMAACTIVE", StringComparison.OrdinalIgnoreCase)) {
                  chkMixLumaActive.Checked = value.Equals("1", StringComparison.OrdinalIgnoreCase);
                } else if (key.Equals("LUMATHR", StringComparison.OrdinalIgnoreCase)) {
                  if (decimal.TryParse(value, NumberStyles.Integer, CultureInfo.InvariantCulture, out decimal thr)) {
                    numLumaThreshold.Value = Math.Clamp(thr, numLumaThreshold.Minimum, numLumaThreshold.Maximum);
                  }
                } else if (key.Equals("LUMASOFT", StringComparison.OrdinalIgnoreCase)) {
                  if (decimal.TryParse(value, NumberStyles.Integer, CultureInfo.InvariantCulture, out decimal soft)) {
                    numLumaSoftness.Value = Math.Clamp(soft, numLumaSoftness.Minimum, numLumaSoftness.Maximum);
                  }
                }
              } catch { }
            }
          }
          updatingSettingsParams = false;
        }
      } catch (Exception ex) {
        Program.LogToFile($"OnPipeMessageReceived: {ex.Message}");
      }
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
      ofdShader.Filter = "GLSL files|*.glsl|Shadertoy files|*.json|All files (*.*)|*.*";
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

      picShaderError.Image = SystemIcons.GetStockIcon(StockIconId.Warning, 64).ToBitmap();
      picShaderError.Visible = false;
      LoadVisualizerSettings();

      SendToMilkwaveVisualizer("", MessageType.GetState);

      if (Settings.MidiEnabled) {
        MidiHelper = new MidiHelper();
        LoadMIDISettings();
        PopulateMidiDevicesList();
        MidiHelper.MidiMessageReceived += MidiMessageReceived();
      } else {
        tabControl.TabPages.Remove(tabMidi);
      }

      // count the lines in data\include.fx
      int lines = 0;
      string includeFile = Path.Combine(BaseDir, "resources\\data\\include.fx");
      try {
        using (StreamReader sr = new StreamReader(includeFile)) {
          while (sr.ReadLine() != null) {
            lines++;
          }
        }
        numOffset.Value = lines + 8;
      } catch (Exception) {
        // ignore
      }

      if (!string.IsNullOrEmpty(Settings.ShadertoyFilesDirectory)) {
        setShadertoyFilesFromDir(Settings.ShadertoyFilesDirectory);
        numShadertoyFileIndex.Value = Math.Clamp(Settings.ShadertoyFileIndex, 1, shadertoyFilesList.Count);
        setShadertoyFileText();
      }

      // Initialize devices using OBS-style enumeration pattern
      InitializeDeviceLists();

      // Wire up game controller events
      if (btnControllerInputScan != null) btnControllerInputScan.Click += btnControllerInputScan_Click;
      if (cboInputController != null) cboInputController.SelectedIndexChanged += cboInputController_SelectedIndexChanged;
      if (chkControllerActive != null) chkControllerActive.CheckedChanged += chkControllerActive_CheckedChanged;

      if (Settings.ControllerActive && cboInputController.Enabled && cboInputController.Items.Count > 0) {
        chkControllerActive.Checked = true;
      }
    }

    private void PopulateMidiDevicesList() {
      try {
        var devices = MidiHelper.GetInputDevices();
        cboMidiDevice.DataSource = devices;
        cboMidiDevice.DisplayMember = nameof(MidiDeviceEntry.DeviceName);
        cboMidiDevice.ValueMember = nameof(MidiDeviceEntry.DeviceIndex);

        // Optionally select the first device by default
        if (devices.Count > 0) {
          cboMidiDevice.SelectedIndex = 0;
        }
      } catch (Exception ex) {
        SetStatusText(ex.Message);
      }
    }

    protected override void WndProc(ref Message m) {
      try {
        // All IPC now handled via named pipe (see OnPipeMessageReceived)
        base.WndProc(ref m);
      } catch (Exception ex) {
        Program.LogToFile($"WndProc: {ex.Message}");
      }
    }

    private void SetRunningPresetText(string displayText) {
      txtVisRunning.Text = displayText.Replace("PRESET=", "");
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
      string statusMessage = "";

      try {
        if (!SendingMessage) {
          SendingMessage = true;
          if (IsPipeConnected) {
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
              statusMessage = $"Changed Wave in {ConnectedVisualizerName}";
            } else if (type == MessageType.PresetFilePath) {
              message = "PRESET=" + messageToSend;
              string fileName = Path.GetFileNameWithoutExtension(messageToSend);
              statusMessage = $"Sent preset \"{fileName}\" to {ConnectedVisualizerName}";
            } else if (type == MessageType.Amplify) {
              message = "AMP" +
                "|l=" + numAmpLeft.Value.ToString(CultureInfo.InvariantCulture) +
                "|r=" + numAmpRight.Value.ToString(CultureInfo.InvariantCulture);
              statusMessage = $"Sent amplification {numAmpLeft.Value.ToString(CultureInfo.InvariantCulture)}" +
                $"/{numAmpRight.Value.ToString(CultureInfo.InvariantCulture)} to {ConnectedVisualizerName}";
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
            } else if (type == MessageType.Settings) {
              message = "SETTINGS";
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
            } else if (type == MessageType.VisVersion) {
              message = "VAR_VERSION=" + numVisVersion.Value.ToString(CultureInfo.InvariantCulture);
            } else if (type == MessageType.ColHue) {
              message = "COL_HUE=" + numSettingsHue.Value.ToString(CultureInfo.InvariantCulture);
            } else if (type == MessageType.HueAuto) {
              message = "HUE_AUTO=" + (chkHueAuto.Checked ? "1" : "0");
            } else if (type == MessageType.HueAutoSeconds) {
              message = "HUE_AUTO_SECONDS=" + numSettingsHueAuto.Value.ToString(CultureInfo.InvariantCulture);
            } else if (type == MessageType.ColSaturation) {
              message = "COL_SATURATION=" + numSettingsSaturation.Value.ToString(CultureInfo.InvariantCulture);
            } else if (type == MessageType.ColBrightness) {
              message = "COL_BRIGHTNESS=" + numSettingsBrightness.Value.ToString(CultureInfo.InvariantCulture);
            } else if (type == MessageType.FFTAttack) {
              message = "FFT_ATTACK=" + numFFTAttack.Value.ToString(CultureInfo.InvariantCulture);
            } else if (type == MessageType.FFTDecay) {
              message = "FFT_DECAY=" + numFFTDecay.Value.ToString(CultureInfo.InvariantCulture);
            } else if (type == MessageType.PresetLink) {
              message = "LINK=" + messageToSend;
            } else if (type == MessageType.SpoutActive) {
              message = "SPOUT_ACTIVE=" + (chkSpoutActive.Checked ? "1" : "0");
            } else if (type == MessageType.SpoutFixedSize) {
              message = "SPOUT_FIXEDSIZE=" + (chkSpoutFixedSize.Checked ? "1" : "0");
            } else if (type == MessageType.SpoutResolution) {
              if (int.TryParse(cboSpoutWidth.Text, out int spoutWidth) && int.TryParse(cboSpoutHeight.Text, out int spoutHeight)) {
                if (spoutHeight > 9 && spoutWidth > 9) {
                  message = "SPOUT_RESOLUTION=" + spoutWidth + "x" + spoutHeight;
                }
              }
            } else if (type == MessageType.RenderQuality) {
              message = "VAR_QUALITY=" + numQuality.Value.ToString(CultureInfo.InvariantCulture);
            } else if (type == MessageType.QualityAuto) {
              message = "VAR_AUTO=" + (chkQualityAuto.Checked ? "1" : "0");
            } else if (type == MessageType.CaptureScreenshot) {
              message = messageToSend;
            } else if (type == MessageType.VideoInput) {
              string deviceName = cboVideoInput.Text;
              bool mixEnabled = chkVideoMix.Checked;
              message = "VIDEOINPUT=" + (mixEnabled ? "1" : "0") + "|" + deviceName;
              statusMessage = $"Video input mixing {(mixEnabled ? "enabled" : "disabled")}: {deviceName}";
            } else if (type == MessageType.SpoutInput) {
              string senderName = cboSputInput.Text;
              bool mixEnabled = chkSpoutMix.Checked;
              statusMessage = $"Spout mixing {(mixEnabled ? "enabled" : "disabled")}: {senderName}";

              if (mixEnabled && senderName.Length > 0) {
                _pipeClient!.SendSpoutSender(senderName);
                System.Threading.Thread.Sleep(50);
              }
              _pipeClient!.SendSignal($"ENABLESPOUTMIX={(mixEnabled ? 1 : 0)}");
              if (statusMessage.Length > 0) {
                SetStatusText($"{statusMessage}");
              }
              SendingMessage = false;
              return;
            } else if (type == MessageType.InputMixOnTop) {
              try {
                if (updatingSettingsParams) return;
                bool onTop = chkInputTop.Checked;
                _pipeClient!.SendSignal($"SET_INPUTMIX_ONTOP={(onTop ? 1 : 0)}");
                SetStatusText($"Input layer position set to {(onTop ? "Top (Overlay)" : "Background")}");
              } catch (Exception ex) {
                SetStatusText($"Error setting input layer position: {ex.Message}");
              }
              SendingMessage = false;
              return;
            } else if (type == MessageType.InputMixOpacity) {
              try {
                if (updatingSettingsParams) return;
                int opacityInt = (int)numInputMixOpacity.Value;
                _pipeClient!.SendSignal($"SET_INPUTMIX_OPACITY={opacityInt}");
                SetStatusText($"Input mix opacity set to {opacityInt}%");
              } catch (Exception ex) {
                SetStatusText($"Error setting input mix opacity: {ex.Message}");
              }
              SendingMessage = false;
              return;
            } else if (type == MessageType.InputMixLuma) {
              try {
                if (updatingSettingsParams) return;
                bool active = chkMixLumaActive.Checked;
                int threshold = active ? (int)numLumaThreshold.Value : -1;
                int softness = (int)numLumaSoftness.Value;
                _pipeClient!.SendSignal($"SET_INPUTMIX_LUMAKEY={threshold}|{softness}");
                if (active)
                  SetStatusText($"Luma Key set to {threshold}% (softness {softness}%)");
                else
                  SetStatusText($"Luma Key disabled");
              } catch (Exception ex) {
                SetStatusText($"Error setting input luma key: {ex.Message}");
              }
              SendingMessage = false;
              return;
            } else if (type == MessageType.PrecompileCache) {
              message = "PRECOMPILE_CACHE";
              statusMessage = "Requested shader cache precompilation";
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
              statusMessage = $"Sent '{messageToSend}' to {ConnectedVisualizerName}";
            } else if (type == MessageType.Raw) {
              message = messageToSend;
              statusMessage = $"Sent '{messageToSend}' to {ConnectedVisualizerName}";
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

            _pipeClient!.Send(message);
            if (statusMessage.Length > 0) {
              SetStatusText($"{statusMessage}");
            }

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
      if ((Control.ModifierKeys & Keys.Alt) == Keys.Alt) {
        if (chkAutoplay.Checked) {
          LoadMessages(lastScriptFileName);
        }
      }

      if (chkAutoplay.Checked) {
        ResetAndStartTimer(true);
      } else {
        autoplayTimer.Stop();
        SetStatusText("");
        autoplayRemainingBeats = 0;
      }
    }

    private void PressMediaKeyPlayPause() {
      SendPostMessage(VK_CURSOR_DOWN, "Cursor down");
    }

    private void PressMediaKeyStop() {
      SendPostMessage(VK_CURSOR_UP, "Cursor up");
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
          string line = cboAutoplay.Text;

          HandleScriptLine(manualSend, line);

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

    private void HandleScriptLine(bool manualSend, string line) {
      string[] strings = line.Split('|');
      foreach (string s in strings) {
        string token = s.Trim();
        string tokenUpper = token.ToUpper();
        if (tokenUpper.Equals("NEXT")) {
          SendPostMessage(VK_SPACE, "Space");
        } else if (tokenUpper.Equals("PREV")) {
          SendPostMessage(VK_BACKSPACE, "Backspace");
        } else if (tokenUpper.Equals("STOP")) {
          chkAutoplay.CheckState = CheckState.Unchecked;
        } else if (tokenUpper.Equals("RESET")) {
          ResetAndStartTimer(false);
        } else if (tokenUpper.StartsWith("BPM=")) {
          string value = tokenUpper.Substring(4);
          if (float.TryParse(value, NumberStyles.Number, CultureInfo.InvariantCulture, out float parsedValue)) {
            numBPM.Value = (decimal)parsedValue;
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
            colorDialogMessage.Color = pnlColorMessage.BackColor;
            SetFormattedMessage();
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
            if (sendString.StartsWith("0x")) {
              if (int.TryParse(sendString.Substring(2), NumberStyles.HexNumber, CultureInfo.InvariantCulture, out int charCode)) {
                SendPostMessage(charCode, sendString);
              }
            } else {
              SendUnicodeChars(sendString);
            }
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
        } else if (tokenUpper.StartsWith("INTENSITY=")) {
          string value = token.Substring(10);
          if (float.TryParse(value, NumberStyles.Number, CultureInfo.InvariantCulture, out float parsedValue)) {
            numVisIntensity.Value = (decimal)parsedValue;
          }
        } else if (tokenUpper.StartsWith("SHIFT=")) {
          string value = token.Substring(6);
          if (float.TryParse(value, NumberStyles.Number, CultureInfo.InvariantCulture, out float parsedValue)) {
            numVisShift.Value = (decimal)parsedValue;
          }
        } else if (tokenUpper.StartsWith("VERSION=")) {
          string value = token.Substring(8);
          if (int.TryParse(value, NumberStyles.Number, CultureInfo.InvariantCulture, out int parsedValue)) {
            numVisVersion.Value = (int)parsedValue;
          }
        } else if (tokenUpper.Equals("MEDIA_PLAY")) {
          PressMediaKeyPlayPause();
        } else if (tokenUpper.Equals("MEDIA_STOP")) {
          PressMediaKeyStop();
        } else if (tokenUpper.StartsWith("LINE=")) {
          try {
            string value = token.Substring(tokenUpper.IndexOf("=") + 1);
            if (value.Equals("CURR")) {
              SendAutoplayLine(true);
            } else if (value.Equals("PREV")) {
              SelectPreviousAutoplayEntry();
              SendAutoplayLine(true);
            } else if (value.Equals("NEXT")) {
              SelectNextAutoplayEntry();
              SendAutoplayLine(true);
            } else if (int.TryParse(value, NumberStyles.Number, CultureInfo.InvariantCulture, out int parsedValue)) {
              string? cmd = cboAutoplay.Items[parsedValue - 1]?.ToString();
              if (cmd != null) {
                HandleScriptLine(true, cmd);
              }
            }
          } catch (Exception) {
            // ignore
          }
        } else if (tokenUpper.StartsWith("EXEC=")) {
          string value = token.Substring(tokenUpper.IndexOf("=") + 1);
          if (!string.IsNullOrEmpty(value)) {
            try {
              Process.Start(new ProcessStartInfo(value) { UseShellExecute = true });
            } catch (Exception ex) {
              SetStatusText($"Unable to execute '{value}': {ex.Message}");
            }
          }
        } else if (tokenUpper.StartsWith("BTN=")) {
          string value = token.Substring(tokenUpper.IndexOf("=") + 1);
          if (!string.IsNullOrEmpty(value) && int.TryParse(value, NumberStyles.Number, CultureInfo.InvariantCulture, out int buttonIndex)) {
            if (buttonIndex >= 1 && buttonIndex <= 45) {
              TriggerPresetFromDeck(buttonIndex);
            } else {
              SetStatusText($"Button index {buttonIndex} out of range (1-45)");
            }
          }
        } else if (tokenUpper.StartsWith("QUALITY=")) {
          string value = token.Substring(token.IndexOf("=") + 1);
          if (float.TryParse(value, NumberStyles.Number, CultureInfo.InvariantCulture, out float parsedValue)) {
            numQuality.Value = Math.Clamp((decimal)parsedValue, numQuality.Minimum, numQuality.Maximum);
          }
        } else if (tokenUpper.StartsWith("HUE=")) {
          string value = token.Substring(token.IndexOf("=") + 1);
          if (float.TryParse(value, NumberStyles.Number, CultureInfo.InvariantCulture, out float parsedValue)) {
            numSettingsHue.Value = Math.Clamp((decimal)parsedValue, numSettingsHue.Minimum, numSettingsHue.Maximum);
          }
        } else if (tokenUpper.StartsWith("SATURATION=")) {
          string value = token.Substring(token.IndexOf("=") + 1);
          if (float.TryParse(value, NumberStyles.Number, CultureInfo.InvariantCulture, out float parsedValue)) {
            numSettingsSaturation.Value = Math.Clamp((decimal)parsedValue, numSettingsSaturation.Minimum, numSettingsSaturation.Maximum);
          }
        } else if (tokenUpper.StartsWith("BRIGHTNESS=")) {
          string value = token.Substring(token.IndexOf("=") + 1);
          if (float.TryParse(value, NumberStyles.Number, CultureInfo.InvariantCulture, out float parsedValue)) {
            numSettingsBrightness.Value = Math.Clamp((decimal)parsedValue, numSettingsBrightness.Minimum, numSettingsBrightness.Maximum);
          }
        } else if (tokenUpper.StartsWith("FFTATTACK=")) {
          string value = token.Substring(token.IndexOf("=") + 1);
          if (float.TryParse(value, NumberStyles.Number, CultureInfo.InvariantCulture, out float parsedValue)) {
            numFFTAttack.Value = Math.Clamp((decimal)parsedValue, numFFTAttack.Minimum, numFFTAttack.Maximum);
          }
        } else if (tokenUpper.StartsWith("FFTDECAY=")) {
          string value = token.Substring(token.IndexOf("=") + 1);
          if (float.TryParse(value, NumberStyles.Number, CultureInfo.InvariantCulture, out float parsedValue)) {
            numFFTDecay.Value = Math.Clamp((decimal)parsedValue, numFFTDecay.Minimum, numFFTDecay.Maximum);
          }
        } else if (tokenUpper.Equals("RAND")) {
          SendPostMessage(VK_R, "R");
        } else if (tokenUpper.Equals("LOCK")) {
          SendUnicodeChars("~");
        } else if (tokenUpper.Equals("PRESETINFO")) {
          SendPostMessage(VK_F4, "F4");
        } else if (tokenUpper.Equals("SONGINFO")) {
          SendPostMessage(VK_B, "B");
        } else if (tokenUpper.Equals("SOUNDINFO")) {
          SendPostMessage(VK_N, "N");
        } else if (tokenUpper.Equals("FULLSCREEN")) {
          if (IsPipeConnected) _pipeClient!.Send("SIGNAL|FULLSCREEN");
        } else if (!string.IsNullOrEmpty(token)) { // no known command, send as message
          SendToMilkwaveVisualizer(token, MessageType.Message);
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

    private void SelectPreviousAutoplayEntry() {
      if (cboAutoplay.Items.Count > 0) {
        // Move to the previous item or loop back to the last
        if (cboAutoplay.SelectedIndex > 0) {
          cboAutoplay.SelectedIndex--;
        } else {
          cboAutoplay.SelectedIndex = cboAutoplay.Items.Count;
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
        cboAutoplay.Text = "No messages in " + fileName;
        chkAutoplay.Enabled = false;
      }
    }

    private void SendPostMessage(int VKKey, string keyName) {
      if (!IsPipeConnected) {
        SetStatusText(windowNotFound);
        return;
      }

      _pipeClient!.Send($"SEND=0x{VKKey:X2}");
      SetStatusText($"Pressed {keyName}");
    }

    private void btnF3_Click(object sender, EventArgs e) {
      if (Settings.IsPresetMode) return;
      SendPostMessage(VK_F3, "F3");
    }

    private void btnF4_Click(object sender, EventArgs e) {
      if (Settings.IsPresetMode) return;
      SendPostMessage(VK_F4, "F4");
    }

    private void btnF7_Click(object sender, EventArgs e) {
      if (Settings.IsPresetMode) return;
      SendPostMessage(VK_F7, "F7");
    }

    private void btnSpace_Click(object sender, EventArgs e) {
      if (Settings.IsPresetMode) return;
      SendPostMessage(VK_SPACE, "Space");
    }

    private void btnBackspace_Click(object sender, EventArgs e) {
      if (Settings.IsPresetMode) return;
      SendPostMessage(VK_BACKSPACE, "Backspace");
    }

    private void SendUnicodeChars(string inputString) {
      if (!IsPipeConnected) {
        SetStatusText(windowNotFound);
        return;
      }

      _pipeClient!.Send($"SEND={inputString}");
      SetStatusText($"Pressed {inputString.ToUpper()}");
    }

    private void btnTilde_Click(object sender, EventArgs e) {
      if (Settings.IsPresetMode) return;
      SendUnicodeChars("~");
    }

    private void btnDelete_Click(object sender, EventArgs e) {
      if (Settings.IsPresetMode) return;
      SendPostMessage(VK_DELETE, "Delete");
    }

    private void btnAltEnter_Click(object sender, EventArgs e) {
      if (Settings.IsPresetMode) return;
      if (!IsPipeConnected) { SetStatusText(windowNotFound); return; }
      _pipeClient!.Send("SIGNAL|FULLSCREEN");
      SetStatusText("Pressed Fullscreen");
    }

    private void btnN_Click(object sender, EventArgs e) {
      if (Settings.IsPresetMode) return;
      SendPostMessage(VK_N, "N");
    }

    private void btnF2_Click(object sender, EventArgs e) {
      if (Settings.IsPresetMode) return;
      SendPostMessage(VK_F2, "F2");
    }

    private void btnK_Click(object sender, EventArgs e) {
      if (Settings.IsPresetMode) return;
      SendPostMessage(VK_K, "K");
    }

    private void btnF10_Click(object sender, EventArgs e) {
      if (Settings.IsPresetMode) return;
      SendPostMessage(VK_F10, "F10");
    }

    private void btn00_Click(object sender, EventArgs e) { SendUnicodeChars("00"); }
    private void btn11_Click(object sender, EventArgs e) { SendUnicodeChars("11"); }
    private void btn22_Click(object sender, EventArgs e) { SendUnicodeChars("22"); }
    private void btn33_Click(object sender, EventArgs e) { SendUnicodeChars("33"); }
    private void btn44_Click(object sender, EventArgs e) { SendUnicodeChars("44"); }
    private void btn55_Click(object sender, EventArgs e) { SendUnicodeChars("55"); }
    private void btn66_Click(object sender, EventArgs e) { SendUnicodeChars("66"); }
    private void btn77_Click(object sender, EventArgs e) { SendUnicodeChars("77"); }

    private void btn88_Click(object sender, EventArgs e) {
      if (Settings.IsPresetMode) return;
      SendUnicodeChars("88");
    }

    private void btnSwitchMode_Click(object sender, EventArgs e) {
      Settings.IsPresetMode = !Settings.IsPresetMode;
      ApplyPanelMode();
      SaveSettingsToFile();
      SetStatusText(Settings.IsPresetMode ? "Switched to Preset mode" : "Switched to Command mode");
    }

    private void lblFromFile_DoubleClick(object sender, EventArgs e) {
      LoadMessages(lastScriptFileName);
    }

    private void MainForm_KeyDown(object sender, KeyEventArgs e) {
      if ((Control.ModifierKeys & Keys.Control) == Keys.Control) {
        if (e.KeyCode == Keys.A) {
          e.SuppressKeyPress = true;
          txtMessage.Focus();
          txtMessage.SelectAll();
        } else if (e.KeyCode == Keys.B) {
          e.SuppressKeyPress = true;
          toolStripMenuItemButtonPanel_Click(null, null);
        } else if (e.KeyCode == Keys.D) {
          btnPresetLoadDirectory_Click(null, null);
        } else if (e.KeyCode == Keys.F) {
          e.SuppressKeyPress = true; // Prevent the beep sound on Enter key press
          if (tabControl.SelectedTab.Name.Equals("tabShader")) {
            if ((Control.ModifierKeys & Keys.Shift) == Keys.Shift) {
              txtShaderFind.SelectAll();
              txtShaderFind.Focus();
            } else if ((Control.ModifierKeys & Keys.Alt) == Keys.Alt) {
              txtShaderHLSL.Text = ShaderHelper.BasicFormatShaderCode(txtShaderHLSL.Text);
              SetStatusText("HLSL code formatted");
            } else {
              FindShaderString();
            }
          } else if (tabControl.SelectedTab.Name.Equals("tabPreset")) {
            txtFilterPresets.SelectAll();
            txtFilterPresets.Focus();
          }
        } else if (e.KeyCode == Keys.L) {
          if (tabControl.SelectedTab.Name.Equals("tabShader")) {
            e.SuppressKeyPress = true; // Prevent the beep sound on Enter key press
            btnShadertoyFilesLoad_Click(null, null);
          }
        } else if (e.KeyCode == Keys.N) {
          e.SuppressKeyPress = true;
          SelectNextPreset();
          btnPresetSend_Click(null, null);
        } else if (e.KeyCode == Keys.O) {
          e.SuppressKeyPress = true;
          StartVisualizerIfNotFound(false);
        } else if (e.KeyCode == Keys.P) {
          e.SuppressKeyPress = true;
          btnPresetSend_Click(null, null);
        } else if (e.KeyCode == Keys.S) {
          e.SuppressKeyPress = true;
          if (tabControl.SelectedTab.Name.Equals("tabShader")) {
            btnSendShader_Click(null, null);
          } else if (tabControl.SelectedTab.Name.Equals("tabMessage")) {
            SendToMilkwaveVisualizer(txtMessage.Text, MessageType.Message);
          }
        } else if (e.KeyCode == Keys.T) {
          e.SuppressKeyPress = true;
          btnTagsSave_Click(null, null);
        } else if (e.KeyCode == Keys.X) {
          if (tabControl.SelectedTab.Name.Equals("tabMessage")) {
            e.SuppressKeyPress = true;
            btnSendFile_Click(null, null);
          }
          if ((Control.ModifierKeys & Keys.Shift) == Keys.Shift) {
            e.SuppressKeyPress = true;
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

      if ((Control.ModifierKeys & Keys.Alt) == Keys.Alt && (Control.ModifierKeys & Keys.Control) == 0) {
        if (e.KeyCode == Keys.V) {
          e.SuppressKeyPress = true;
          if (cboVisualizerInstance.Items.Count > 1) {
            int next = (cboVisualizerInstance.SelectedIndex + 1) % cboVisualizerInstance.Items.Count;
            cboVisualizerInstance.SelectedIndex = next;
            SetStatusText($"Switched to {ConnectedVisualizerName}");
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
          "burntime={float}  // The duration (in seconds) the text will \"burn in\" at the end (default=0.1)" + Environment.NewLine +
          "box_alpha={float:0..1}  // Text background box alpha (0=transparent, 1=opaque, default=0)" + Environment.NewLine +
          "box_col={int:0..255,int:0..255,int:0..255}  // Text background RGB box color, (default=0,0,0)" + Environment.NewLine +
          "box_left={float}  // Text background box left side adjustment factor (default=1.0)" + Environment.NewLine +
          "box_right={float}  // Text background box right side adjustment factor (default=1.0)" + Environment.NewLine +
          "box_top={float}  // Text background box top side adjustment factor (default=1.0)" + Environment.NewLine +
          "box_bottom={float}  // Text background box bottom side adjustment factor (default=1.0)" + Environment.NewLine;

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

    private void txtStyle_KeyDown(object sender, KeyEventArgs e) {
      if (e.KeyCode == Keys.Enter) {
        e.SuppressKeyPress = true; // Prevent the beep sound on Enter key press
        btnSaveParam.PerformClick();
      }
    }

    private void MainForm_FormClosing(object sender, FormClosingEventArgs e) {
      try {

        DisposeSpriteButtonImages();
        SetAndSaveSettings();
        if (Settings.MidiEnabled) {
          SaveMIDISettings();
        }

        // Close the Visualizer window if CloseVisualizerWithRemote=true or Alt or Ctrl key are pressed
        if (Settings.CloseVisualizerWithRemote || (Control.ModifierKeys & Keys.Alt) == Keys.Alt || (Control.ModifierKeys & Keys.Control) == Keys.Control) {
          _pipeClient?.Send("SEND=0x1B"); // VK_ESCAPE — triggers close
          }

          DetachAndDisposePipeClient();

      } catch (Exception ex) {
        Program.SaveErrorToFile(ex, "Error");
      }
    }

    private void SaveSettingsToFile() {
      string jsonString = JsonSerializer.Serialize(Settings, new JsonSerializerOptions { WriteIndented = true });
      string settingsFile = Path.Combine(BaseDir, milkwaveSettingsFile);
      try {
        File.WriteAllText(settingsFile, jsonString);
      } catch (UnauthorizedAccessException) {
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
      } catch (UnauthorizedAccessException) {
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
      } catch (Exception) {
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
          colorDialogMessage.Color = pnlColorMessage.BackColor;
          SetFormattedMessage();
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
      var documentationLinks = new (string Label, string Url)[] {
        ("Milkwave Manual", "https://github.com/IkeC/Milkwave/blob/main/Manual.md"),
        ("Readme", "https://github.com/IkeC/Milkwave/blob/main/Build/README.txt"),
        ("More Presets", "https://github.com/projectM-visualizer/projectm?tab=readme-ov-file#presets"),
        ("Visualizer Help Menu (F1)", "https://github.com/IkeC/Milkwave/blob/main/Build/visualizer-keys.txt")
      };

      var communityLinks = new (string Label, string Url)[] {
        ("GitHub homepage", "https://github.com/IkeC/Milkwave"),
        ("GitHub issues", "https://github.com/IkeC/Milkwave/issues"),
        ("Discord", "https://bit.ly/Ikes-Discord")
      };

      var builder = RtfBuilder.Create(toolStripMenuItemDarkMode.Checked)
        .AppendText("There are many tooltips explaining all features when you move your mouse over all the form elements.")
        .AppendParagraphBreak();

      AppendLinkGroup(builder, documentationLinks);
      builder.AppendLine();

      AppendLinkGroup(builder, communityLinks);
      builder.AppendLine();

      string dialogtext = builder.Build();
      var helpLinks = documentationLinks.Concat(communityLinks).ToArray();

      new MilkwaveInfoForm(toolStripMenuItemDarkMode.Checked)
        .ShowDialog("Milkwave Help", dialogtext, 10, 800, 600, helpLinks);

      static void AppendLinkGroup(RtfBuilder builder, (string Label, string Url)[] links) {
        foreach (var (label, url) in links) {
          builder.AppendLink(label, url).AppendLine();
        }
      }
    }

    private void toolStripMenuItemSupporters_Click(object sender, EventArgs e) {
      var donationLinks = new (string Label, string Url)[] {
        ("Ko-fi", "https://ko-fi.com/ikeserver"),
        ("PayPal", "https://www.paypal.com/ncp/payment/5XMP3S69PJLCU")
      };

      var builder = RtfBuilder.Create(toolStripMenuItemDarkMode.Checked)
        .AppendText("Milkwave Supporters — Thank you very much!  ❤️")
        .AppendParagraphBreak()
        .AppendText("• Shane").AppendLine()
        .AppendText("• Tures1955").AppendLine()
        .AppendText("• hatecubed").AppendLine()
        .AppendText("• catchpen")
        .AppendParagraphBreak()
        .AppendText("Milkwave is and will always be free software, being the collaborative effort of many different authors. ")
        .AppendText("If you like it and want to appreciate and support our share of the work, please consider donating.")
        .AppendParagraphBreak();

      foreach (var (label, url) in donationLinks) {
        builder.AppendLink(label, url).AppendLine();
      }

      builder.AppendLine()
        .AppendText("Any amount is valued! You'll be listed on this page unless you do not want to.");

      string dialogtext = builder.Build();
      new MilkwaveInfoForm(toolStripMenuItemDarkMode.Checked)
        .ShowDialog("Supporters", dialogtext, links: donationLinks);
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
      colorDialogMessage.Color = pnlColorMessage.BackColor;
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

    private void toolStripMenuItemSpriteButtonImages_Click(object? sender, EventArgs? e) {
      toolStripMenuItemSpriteButtonImages.Checked = !toolStripMenuItemSpriteButtonImages.Checked;
      Settings.EnableSpriteButtonImage = toolStripMenuItemSpriteButtonImages.Checked;
      RefreshSpriteButtonImages(false);
    }

    private void toolStripMenuItemColorButtonImages_Click(object? sender, EventArgs? e) {
      toolStripMenuItemColorButtonImages.Checked = !toolStripMenuItemColorButtonImages.Checked;
      Settings.EnableColorButtonImages = toolStripMenuItemColorButtonImages.Checked;
      RefreshSpriteButtonImages(false);
      if (Settings.IsPresetMode) {
        ApplyPresetMode();
      }
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
          if (!PresetsMasterList.Contains(newPreset)) {
            PresetsMasterList.Insert(0, newPreset);
            FillAndFilterPresetList();
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
      PresetsMasterList.Clear();
      FillAndFilterPresetList();
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
      PresetsMasterList.Clear();
      bool includeSubdirs = false;

      if (Directory.GetDirectories(dirToLoad).Length > 0) {
        if (forceIncludeSubdirs || MessageBox.Show(this, "Include subdirectories?",
          "Please Confirm", MessageBoxButtons.YesNo, MessageBoxIcon.Question, MessageBoxDefaultButton.Button1) == DialogResult.Yes) {
          includeSubdirs = true;
        }
      }
      FillCboPresetsFromDir(dirToLoad, includeSubdirs, "");
      SetStatusText($"Loaded {cboPresets.Items.Count} presets from '{dirToLoad}'");
      FillAndFilterPresetList();
      if (cboPresets.Items.Count > 0) {
        SelectFirstPreset();
        UpdateTagsDisplay(false, false);
      }
    }

    private void SelectFirstPreset() {
      try {
        int index = -1;
        foreach (var item in cboPresets.Items) {
          index++;
          if (item is Data.Preset preset && !preset.DisplayName.Contains("\\")) {
            cboPresets.SelectedIndex = index;
            return;
          }
        }
        cboPresets.SelectedIndex = 0;
      } catch (Exception) {
        // ignore
      }
    }

    private void FillCboPresetsFromDir(string dirToLoad, bool includeSubdirs, string displayDirPrefix) {
      int relIndex = -1;
      if (includeSubdirs) {
        foreach (string subDir in Directory.GetDirectories(dirToLoad)) {
          string? prefix = Path.GetFileName(subDir) + "\\";
          FillCboPresetsFromDir(subDir, true, prefix);
        }
      }
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
          if (txtFilterTags.Text.ToUpper().StartsWith("AGE=")) {
            if (Int32.TryParse(txtFilterTags.Text.Substring(4), out int age) && age > 0) {
              DateTime lastFileWriteTime = File.GetLastWriteTime(fileName);
              if ((DateTime.Now - lastFileWriteTime).TotalDays < age) {
                // Check if the preset already exists in the list
                if (!PresetsMasterList.Contains(newPreset)) {
                  PresetsMasterList.Add(newPreset);
                }
              }
            }
          } else if (String.IsNullOrEmpty(txtFilterTags.Text) || fileNameOnlyNoExtension.Contains(txtFilterTags.Text, StringComparison.InvariantCultureIgnoreCase)) {
            // Check if the preset already exists in the list
            if (!PresetsMasterList.Contains(newPreset)) {
              PresetsMasterList.Add(newPreset);
            }
          }
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

    public void chkInputTop_CheckedChanged(object sender, EventArgs e) {
      if (!updatingSettingsParams) {
        SendToMilkwaveVisualizer("", MessageType.InputMixOnTop);
      }
    }

    public void numInputMixOpacity_ValueChanged(object sender, EventArgs e) {
      if (!updatingSettingsParams) {
        SendToMilkwaveVisualizer("", MessageType.InputMixOpacity);
      }
    }

    private void chkMixLumaActive_CheckedChanged(object sender, EventArgs e) {
      if (!updatingSettingsParams) {
        SendToMilkwaveVisualizer("", MessageType.InputMixLuma);
      }
    }

    private void numLumaThreshold_ValueChanged(object sender, EventArgs e) {
      if (!updatingSettingsParams) {
        SendToMilkwaveVisualizer("", MessageType.InputMixLuma);
      }
    }

    private void numLumaSoftness_ValueChanged(object sender, EventArgs e) {
      if (!updatingSettingsParams) {
        SendToMilkwaveVisualizer("", MessageType.InputMixLuma);
      }
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
      RemoteHelper.ReloadAudioDevices(cboAudioDevice);
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
      if (Settings.IsPresetMode) return;
      SendPostMessage(VK_B, "B");
    }

    private void btnTransparency_Click(object sender, EventArgs e) {
      if (Settings.IsPresetMode) return;
      SendPostMessage(VK_F12, "F12");
    }

    private void btnWatermark_Click(object sender, EventArgs e) {
      if (Settings.IsPresetMode) return;
      if (!IsPipeConnected) { SetStatusText(windowNotFound); return; }
      _pipeClient!.Send("SIGNAL|WATERMARK");
      SetStatusText("Pressed Watermark Mode");
    }

    private void btnWatermark_MouseDown(object sender, MouseEventArgs e) {
      if (Settings.IsPresetMode) return;
      if (e.Button == MouseButtons.Right) {
        if (!IsPipeConnected) { SetStatusText(windowNotFound); return; }
        _pipeClient!.Send("SIGNAL|BORDERLESS_FS");
        SetStatusText("Pressed Borderless Fullscreen");
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

    private void txtFilterTags_KeyDown(object sender, KeyEventArgs e) {
      if (e.KeyCode == Keys.Enter) {
        e.SuppressKeyPress = true; // Prevent the beep sound on Enter key press
        SaveTags();
        if ((Control.ModifierKeys & Keys.Control) == Keys.Control) {
          SendPostMessage(VK_SPACE, "Space");
        }
      }
    }

    private void txtFilterPresets_KeyDown(object sender, KeyEventArgs e) {
      if (e.KeyCode == Keys.Enter) {
        e.SuppressKeyPress = true;
        btnPresetSend_Click(null, null);
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
        RemoteHelper.SelectDefaultDevice(cboAudioDevice);
        btnSetAudioDevice_Click(null, null);
        SetStatusText($"Default audio device selected and set");
      }
    }

    private void btnPresetLoadTags_Click(object? sender, EventArgs? e) {
      ReloadLoadFiltersList(true);
      cboPresets.Items.Clear();
      cboPresets.Text = "";
      PresetsMasterList.Clear();
      if (cboTagsFilter.Text.Length > 0) {
        var filteredEntries = FilterTagEntries();
        foreach (var entry in filteredEntries) {
          if (String.IsNullOrEmpty(txtFilterTags.Text) || entry.Key.Contains(txtFilterTags.Text, StringComparison.InvariantCultureIgnoreCase)) {

            string filenameWithoutExt = Path.GetFileNameWithoutExtension(entry.Value.PresetPath);
            string displayName = filenameWithoutExt;
            string? directory = Path.GetDirectoryName(entry.Value.PresetPath);
            if (!string.IsNullOrEmpty(directory)) {
              string lastDirectory = new DirectoryInfo(directory).Name;
              if (!string.IsNullOrEmpty(lastDirectory)) {
                displayName = $"{lastDirectory}\\{filenameWithoutExt}";
              }
            }

            Data.Preset newPreset = new Data.Preset {
              DisplayName = displayName,
              MaybeRelativePath = entry.Value.PresetPath
            };
            PresetsMasterList.Add(newPreset);
          }
        }
        PresetsMasterList.Sort((x, y) => string.Compare(x.DisplayName, y.DisplayName, StringComparison.OrdinalIgnoreCase));
        FillAndFilterPresetList();
      }

      SetStatusText($"Loaded {cboPresets.Items.Count} filtered presets");
      if (cboPresets.Items.Count > 0) {
        SelectFirstPreset();
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
      updatingSettingsParams = true;
      Location = Settings.RemoteWindowLocation;
      Size optimalSize = GetCalculatedOptionalTopPanelSize();
      if (Settings.RemoteWindowSize.Width > 0 && Settings.RemoteWindowSize.Height > 0) {
        Size = Settings.RemoteWindowSize;
      } else {
        Height = optimalSize.Height + statusBar.Height + 500;
        Width = optimalSize.Width;
      }

      toolStripMenuItemTabsPanel.Checked = Settings.ShowTabsPanel;
      toolStripMenuItemButtonPanel.Checked = Settings.ShowButtonPanel;
      toolStripMenuItemSpriteButtonImages.Checked = Settings.EnableSpriteButtonImage;
      toolStripMenuItemColorButtonImages.Checked = Settings.EnableColorButtonImages;
      toolStripMenuItemMonitorCPU.Checked = Settings.EnableMonitorCPU;
      toolStripMenuItemMonitorGPU.Checked = Settings.EnableMonitorGPU;
      ToggleMonitors();

      try {
        if (Settings.SplitterDistance1 > 0) {
          splitContainer1.SplitterDistance = Settings.SplitterDistance1;
        } else {
          splitContainer1.SplitterDistance = optimalSize.Height + 50;
        }
      } catch (Exception) {
        // igonre
      }
      chkShaderFile.Checked = Settings.ShaderFileChecked;
      chkWrap.Checked = Settings.WrapChecked;

      string savedTitle;
      if (!string.IsNullOrEmpty(Settings.WindowTitle)) {
        savedTitle = Settings.WindowTitle;
      } else {
        savedTitle = cboVisualizerInstance.Items[0]?.ToString() ?? "Milkwave Visualizer";
      }
      cboVisualizerInstance.Text = savedTitle;

      numInputMixOpacity.Value = Math.Clamp(Settings.InputMixOpacity, numInputMixOpacity.Minimum, numInputMixOpacity.Maximum);
      chkInputTop.Checked = Settings.InputMixOnTop;
      chkMixLumaActive.Checked = Settings.InputMixLumaActive;
      numLumaThreshold.Value = Math.Clamp(Settings.InputMixLumaThreshold, numLumaThreshold.Minimum, numLumaThreshold.Maximum);
      numLumaSoftness.Value = Math.Clamp(Settings.InputMixLumaSoftness, numLumaSoftness.Minimum, numLumaSoftness.Maximum);

      numVisIntensity.Value = (decimal)Settings.VisIntensity;
      numVisShift.Value = (decimal)Settings.VisShift;
      numVisVersion.Value = Settings.VisVersion;

      RefreshSpriteButtonImages(false);
      updatingSettingsParams = false;
      UpdateInputMixControlsEnabled();
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
      Settings.WrapChecked = chkWrap.Checked;
      Settings.ControllerActive = chkControllerActive.Checked;
      Settings.EnableSpriteButtonImage = toolStripMenuItemSpriteButtonImages.Checked;

      Settings.InputMixOpacity = numInputMixOpacity.Value;
      Settings.InputMixOnTop = chkInputTop.Checked;
      Settings.InputMixLumaActive = chkMixLumaActive.Checked;
      Settings.InputMixLumaThreshold = numLumaThreshold.Value;
      Settings.InputMixLumaSoftness = numLumaSoftness.Value;

      Settings.VisIntensity = numVisIntensity.Value;
      Settings.VisShift = numVisShift.Value;
      Settings.VisVersion = (int)numVisVersion.Value;

      Settings.UseDX12 = IsSelectedVisualizerDX12();
      Settings.WindowTitle = cboVisualizerInstance.Text;

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
        string fontFace1 = RemoteHelper.GetIniValueFonts("FontFace1", "Bahnschrift");
        cboFont1.SelectedItem = fontFace1;
        string fontSize1 = RemoteHelper.GetIniValueFonts("FontSize1", "20");
        numFont1.Value = int.Parse(fontSize1);

        string fontBold1 = RemoteHelper.GetIniValueFonts("FontBold1", "0");
        chkFontBold1.Checked = !fontBold1.Equals("0");
        string fontItalic1 = RemoteHelper.GetIniValueFonts("FontItalic1", "0");
        chkFontItalic1.Checked = !fontItalic1.Equals("0");
        string fontAA1 = RemoteHelper.GetIniValueFonts("FontAA1", "1");
        chkFontAA1.Checked = !fontAA1.Equals("0");

        string fontColorR1 = RemoteHelper.GetIniValueFonts("FontColorR1", "255");
        int fontColorR1Val = int.Parse(fontColorR1);
        string fontColorG1 = RemoteHelper.GetIniValueFonts("FontColorG1", "255");
        int fontColorG1Val = int.Parse(fontColorG1);
        string fontColorB1 = RemoteHelper.GetIniValueFonts("FontColorB1", "0");
        int fontColorB1Val = int.Parse(fontColorB1);
        pnlColorFont1.BackColor = Color.FromArgb(fontColorR1Val, fontColorG1Val, fontColorB1Val);

        // Preset
        string fontFace2 = RemoteHelper.GetIniValueFonts("FontFace2", "Bahnschrift");
        cboFont2.SelectedItem = fontFace2;
        string fontSize2 = RemoteHelper.GetIniValueFonts("FontSize2", "25");
        numFont2.Value = int.Parse(fontSize2);

        string fontBold2 = RemoteHelper.GetIniValueFonts("FontBold2", "0");
        chkFontBold2.Checked = !fontBold2.Equals("0");
        string fontItalic2 = RemoteHelper.GetIniValueFonts("FontItalic2", "0");
        chkFontItalic2.Checked = !fontItalic2.Equals("0");
        string fontAA2 = RemoteHelper.GetIniValueFonts("FontAA2", "1");
        chkFontAA2.Checked = !fontAA2.Equals("0");

        string fontColorR2 = RemoteHelper.GetIniValueFonts("FontColorR2", "255");
        int fontColorR2Val = int.Parse(fontColorR2);
        string fontColorG2 = RemoteHelper.GetIniValueFonts("FontColorG2", "86");
        int fontColorG2Val = int.Parse(fontColorG2);
        string fontColorB2 = RemoteHelper.GetIniValueFonts("FontColorB2", "0");
        int fontColorB2Val = int.Parse(fontColorB2);
        pnlColorFont2.BackColor = Color.FromArgb(fontColorR2Val, fontColorG2Val, fontColorB2Val);

        // Menu: Ini-Index is 4
        string fontFaceMenu = RemoteHelper.GetIniValueFonts("FontFace4", "Bahnschrift");
        cboFontMenu.SelectedItem = fontFaceMenu;
        string fontSizeMenu = RemoteHelper.GetIniValueFonts("FontSize4", "25");
        numFontMenu.Value = int.Parse(fontSizeMenu);

        string fontBoldMenu = RemoteHelper.GetIniValueFonts("FontBold4", "0");
        chkMenuBold.Checked = !fontBoldMenu.Equals("0");
        string fontItalicMenu = RemoteHelper.GetIniValueFonts("FontItalic4", "0");
        chkMenuItalic.Checked = !fontItalicMenu.Equals("0");
        string fontAAMenu = RemoteHelper.GetIniValueFonts("FontAA4", "1");
        chkMenuAA.Checked = !fontAAMenu.Equals("0");

        string fontColorRMenu = RemoteHelper.GetIniValueFonts("FontColorR4", "255");
        int fontColorRMenuVal = int.Parse(fontColorRMenu);
        string fontColorGMenu = RemoteHelper.GetIniValueFonts("FontColorG4", "255");
        int fontColorGMenuVal = int.Parse(fontColorGMenu);
        string fontColorBMenu = RemoteHelper.GetIniValueFonts("FontColorB4", "255");
        int fontColorBMenuVal = int.Parse(fontColorBMenu);
        pnlColorMenu.BackColor = Color.FromArgb(fontColorRMenuVal, fontColorGMenuVal, fontColorBMenuVal);

        // Artist: Ini-Index is 5!
        string fontFace3 = RemoteHelper.GetIniValueFonts("FontFace5", "Bahnschrift");
        cboFont3.SelectedItem = fontFace3;
        string fontSize3 = RemoteHelper.GetIniValueFonts("FontSize5", "30");
        numFont3.Value = int.Parse(fontSize3);

        string fontBold3 = RemoteHelper.GetIniValueFonts("FontBold5", "0");
        chkFontBold3.Checked = !fontBold3.Equals("0");
        string fontItalic3 = RemoteHelper.GetIniValueFonts("FontItalic5", "0");
        chkFontItalic3.Checked = !fontItalic3.Equals("0");
        string fontAA3 = RemoteHelper.GetIniValueFonts("FontAA5", "1");
        chkFontAA3.Checked = !fontAA3.Equals("0");

        string fontColorR3 = RemoteHelper.GetIniValueFonts("FontColorR5", "211");
        int fontColorR3Val = int.Parse(fontColorR3);
        string fontColorG3 = RemoteHelper.GetIniValueFonts("FontColorG5", "0");
        int fontColorG3Val = int.Parse(fontColorG3);
        string fontColorB3 = RemoteHelper.GetIniValueFonts("FontColorB5", "9");
        int fontColorB3Val = int.Parse(fontColorB3);
        pnlColorFont3.BackColor = Color.FromArgb(fontColorR3Val, fontColorG3Val, fontColorB3Val);

        // Title: Ini-Index is 6!
        string fontFace4 = RemoteHelper.GetIniValueFonts("FontFace6", "Bahnschrift");
        cboFont4.SelectedItem = fontFace4;
        string fontSize4 = RemoteHelper.GetIniValueFonts("FontSize6", "40");
        numFont4.Value = int.Parse(fontSize4);

        string fontBold4 = RemoteHelper.GetIniValueFonts("FontBold6", "1");
        chkFontBold4.Checked = !fontBold4.Equals("0");
        string fontItalic4 = RemoteHelper.GetIniValueFonts("FontItalic6", "0");
        chkFontItalic4.Checked = !fontItalic4.Equals("0");
        string fontAA4 = RemoteHelper.GetIniValueFonts("FontAA6", "1");
        chkFontAA4.Checked = !fontAA4.Equals("0");

        string fontColorR4 = RemoteHelper.GetIniValueFonts("FontColorR6", "255");
        int fontColorR4Val = int.Parse(fontColorR4);
        string fontColorG4 = RemoteHelper.GetIniValueFonts("FontColorG6", "86");
        int fontColorG4Val = int.Parse(fontColorG4);
        string fontColorB4 = RemoteHelper.GetIniValueFonts("FontColorB6", "0");
        int fontColorB4Val = int.Parse(fontColorB4);
        pnlColorFont4.BackColor = Color.FromArgb(fontColorR4Val, fontColorG4Val, fontColorB4Val);

        // Album: Ini-Index is 7!
        string fontFace5 = RemoteHelper.GetIniValueFonts("FontFace7", "Bahnschrift");
        cboFont5.SelectedItem = fontFace5;
        string fontSize5 = RemoteHelper.GetIniValueFonts("FontSize7", "25");
        numFont5.Value = int.Parse(fontSize5);

        string fontBold5 = RemoteHelper.GetIniValueFonts("FontBold7", "0");
        chkFontBold5.Checked = !fontBold5.Equals("0");
        string fontItalic5 = RemoteHelper.GetIniValueFonts("FontItalic7", "0");
        chkFontItalic5.Checked = !fontItalic5.Equals("0");
        string fontAA5 = RemoteHelper.GetIniValueFonts("FontAA7", "1");
        chkFontAA5.Checked = !fontAA5.Equals("0");

        string fontColorR5 = RemoteHelper.GetIniValueFonts("FontColorR7", "211");
        int fontColorR5Val = int.Parse(fontColorR5);
        string fontColorG5 = RemoteHelper.GetIniValueFonts("FontColorG7", "0");
        int fontColorG5Val = int.Parse(fontColorG5);
        string fontColorB5 = RemoteHelper.GetIniValueFonts("FontColorB7", "9");
        int fontColorB5Val = int.Parse(fontColorB5);
        pnlColorFont5.BackColor = Color.FromArgb(fontColorR5Val, fontColorG5Val, fontColorB5Val);

      } catch (Exception) {
        // ignore
      }
    }

    private void btnSettingsSave_Click(object? sender, EventArgs? e) {

      // Notify
      RemoteHelper.SetIniValueFonts("FontFace1", cboFont1.Text);
      RemoteHelper.SetIniValueFonts("FontSize1", numFont1.Value.ToString());
      RemoteHelper.SetIniValueFonts("FontBold1", chkFontBold1.Checked ? "1" : "0");
      RemoteHelper.SetIniValueFonts("FontItalic1", chkFontItalic1.Checked ? "1" : "0");
      RemoteHelper.SetIniValueFonts("FontAA1", chkFontAA1.Checked ? "1" : "0");
      RemoteHelper.SetIniValueFonts("FontColorR1", pnlColorFont1.BackColor.R.ToString());
      RemoteHelper.SetIniValueFonts("FontColorG1", pnlColorFont1.BackColor.G.ToString());
      RemoteHelper.SetIniValueFonts("FontColorB1", pnlColorFont1.BackColor.B.ToString());

      // Preset
      RemoteHelper.SetIniValueFonts("FontFace2", cboFont2.Text);
      RemoteHelper.SetIniValueFonts("FontSize2", numFont2.Value.ToString());
      RemoteHelper.SetIniValueFonts("FontBold2", chkFontBold2.Checked ? "1" : "0");
      RemoteHelper.SetIniValueFonts("FontItalic2", chkFontItalic2.Checked ? "1" : "0");
      RemoteHelper.SetIniValueFonts("FontAA2", chkFontAA2.Checked ? "1" : "0");
      RemoteHelper.SetIniValueFonts("FontColorR2", pnlColorFont2.BackColor.R.ToString());
      RemoteHelper.SetIniValueFonts("FontColorG2", pnlColorFont2.BackColor.G.ToString());
      RemoteHelper.SetIniValueFonts("FontColorB2", pnlColorFont2.BackColor.B.ToString());

      // Menu: Ini-Index is 4
      RemoteHelper.SetIniValueFonts("FontFace4", cboFontMenu.Text);
      RemoteHelper.SetIniValueFonts("FontSize4", numFontMenu.Value.ToString());
      RemoteHelper.SetIniValueFonts("FontBold4", chkMenuBold.Checked ? "1" : "0");
      RemoteHelper.SetIniValueFonts("FontItalic4", chkMenuItalic.Checked ? "1" : "0");
      RemoteHelper.SetIniValueFonts("FontAA4", chkMenuAA.Checked ? "1" : "0");
      RemoteHelper.SetIniValueFonts("FontColorR4", pnlColorMenu.BackColor.R.ToString());
      RemoteHelper.SetIniValueFonts("FontColorG4", pnlColorMenu.BackColor.G.ToString());
      RemoteHelper.SetIniValueFonts("FontColorB4", pnlColorMenu.BackColor.B.ToString());

      // Artist: Ini-Index is 5!
      RemoteHelper.SetIniValueFonts("FontFace5", cboFont3.Text);
      RemoteHelper.SetIniValueFonts("FontSize5", numFont3.Value.ToString());
      RemoteHelper.SetIniValueFonts("FontBold5", chkFontBold3.Checked ? "1" : "0");
      RemoteHelper.SetIniValueFonts("FontItalic5", chkFontItalic3.Checked ? "1" : "0");
      RemoteHelper.SetIniValueFonts("FontAA5", chkFontAA3.Checked ? "1" : "0");
      RemoteHelper.SetIniValueFonts("FontColorR5", pnlColorFont3.BackColor.R.ToString());
      RemoteHelper.SetIniValueFonts("FontColorG5", pnlColorFont3.BackColor.G.ToString());
      RemoteHelper.SetIniValueFonts("FontColorB5", pnlColorFont3.BackColor.B.ToString());

      // Title: Ini-Index is 6!
      RemoteHelper.SetIniValueFonts("FontFace6", cboFont4.Text);
      RemoteHelper.SetIniValueFonts("FontSize6", numFont4.Value.ToString());
      RemoteHelper.SetIniValueFonts("FontBold6", chkFontBold4.Checked ? "1" : "0");
      RemoteHelper.SetIniValueFonts("FontItalic6", chkFontItalic4.Checked ? "1" : "0");
      RemoteHelper.SetIniValueFonts("FontAA6", chkFontAA4.Checked ? "1" : "0");
      RemoteHelper.SetIniValueFonts("FontColorR6", pnlColorFont4.BackColor.R.ToString());
      RemoteHelper.SetIniValueFonts("FontColorG6", pnlColorFont4.BackColor.G.ToString());
      RemoteHelper.SetIniValueFonts("FontColorB6", pnlColorFont4.BackColor.B.ToString());

      // Album: Ini-Index is 7!
      RemoteHelper.SetIniValueFonts("FontFace7", cboFont5.Text);
      RemoteHelper.SetIniValueFonts("FontSize7", numFont5.Value.ToString());
      RemoteHelper.SetIniValueFonts("FontBold7", chkFontBold5.Checked ? "1" : "0");
      RemoteHelper.SetIniValueFonts("FontItalic7", chkFontItalic5.Checked ? "1" : "0");
      RemoteHelper.SetIniValueFonts("FontAA7", chkFontAA5.Checked ? "1" : "0");
      RemoteHelper.SetIniValueFonts("FontColorR7", pnlColorFont5.BackColor.R.ToString());
      RemoteHelper.SetIniValueFonts("FontColorG7", pnlColorFont5.BackColor.G.ToString());
      RemoteHelper.SetIniValueFonts("FontColorB7", pnlColorFont5.BackColor.B.ToString());

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
        ColorTranslator.ToOle(pnlColorMenu.BackColor),
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

    private void btnTestFonts_Click(object? sender, EventArgs? e) {
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

    private void lblMenu_DoubleClick(object sender, EventArgs e) {
      cboFontMenu.SelectedItem = "Bahnschrift";
      numFontMenu.Value = 25;
      chkMenuBold.Checked = false;
      chkMenuItalic.Checked = false;
      chkMenuAA.Checked = true;
      pnlColorMenu.BackColor = Color.FromArgb(255, 255, 255);
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
        RemoteHelper.SetIniValueFonts("FontFace1", cboFont1.Text);
        SendToMilkwaveVisualizer("", MessageType.Config);
        SendToMilkwaveVisualizer("", MessageType.TestFonts);
      }
    }

    private void cboFont2_SelectedIndexChanged(object sender, EventArgs e) {
      if ((Control.ModifierKeys & Keys.Alt) == Keys.Alt) {
        RemoteHelper.SetIniValueFonts("FontFace2", cboFont2.Text);
        SendToMilkwaveVisualizer("", MessageType.Config);
        SendToMilkwaveVisualizer("", MessageType.TestFonts);
      }
    }

    private void cboFontMenu_SelectedIndexChanged(object sender, EventArgs e) {
      if ((Control.ModifierKeys & Keys.Alt) == Keys.Alt) {
        RemoteHelper.SetIniValueFonts("FontFace4", cboFontMenu.Text);
        SendToMilkwaveVisualizer("", MessageType.Config);
        SendToMilkwaveVisualizer("", MessageType.TestFonts);
      }
    }

    private void cboFont3_SelectedIndexChanged(object sender, EventArgs e) {
      if ((Control.ModifierKeys & Keys.Alt) == Keys.Alt) {
        RemoteHelper.SetIniValueFonts("FontFace5", cboFont3.Text);
        SendToMilkwaveVisualizer("", MessageType.Config);
        SendToMilkwaveVisualizer("", MessageType.TestFonts);
      }
    }

    private void cboFont4_SelectedIndexChanged(object sender, EventArgs e) {
      if ((Control.ModifierKeys & Keys.Alt) == Keys.Alt) {
        RemoteHelper.SetIniValueFonts("FontFace6", cboFont4.Text);
        SendToMilkwaveVisualizer("", MessageType.Config);
        SendToMilkwaveVisualizer("", MessageType.TestFonts);
      }
    }

    private void cboFont5_SelectedIndexChanged(object sender, EventArgs e) {
      if ((Control.ModifierKeys & Keys.Alt) == Keys.Alt) {
        RemoteHelper.SetIniValueFonts("FontFace7", cboFont5.Text);
        SendToMilkwaveVisualizer("", MessageType.Config);
        SendToMilkwaveVisualizer("", MessageType.TestFonts);
      }
    }

    private void numFont1_ValueChanged(object sender, EventArgs e) {
      if ((Control.ModifierKeys & Keys.Alt) == Keys.Alt) {
        RemoteHelper.SetIniValueFonts("FontSize1", numFont1.Value.ToString());
        SendToMilkwaveVisualizer("", MessageType.Config);
        SendToMilkwaveVisualizer("", MessageType.TestFonts);
      }
    }

    private void numFont2_ValueChanged(object sender, EventArgs e) {
      if ((Control.ModifierKeys & Keys.Alt) == Keys.Alt) {
        RemoteHelper.SetIniValueFonts("FontSize2", numFont2.Value.ToString());
        SendToMilkwaveVisualizer("", MessageType.Config);
        SendToMilkwaveVisualizer("", MessageType.TestFonts);
      }
    }

    private void numFontMenu_ValueChanged(object sender, EventArgs e) {
      if ((Control.ModifierKeys & Keys.Alt) == Keys.Alt) {
        RemoteHelper.SetIniValueFonts("FontSize4", numFontMenu.Value.ToString());
        SendToMilkwaveVisualizer("", MessageType.Config);
        SendToMilkwaveVisualizer("", MessageType.TestFonts);
      }
    }

    private void numFont3_ValueChanged(object sender, EventArgs e) {
      if ((Control.ModifierKeys & Keys.Alt) == Keys.Alt) {
        RemoteHelper.SetIniValueFonts("FontSize5", numFont3.Value.ToString());
        SendToMilkwaveVisualizer("", MessageType.Config);
        SendToMilkwaveVisualizer("", MessageType.TestFonts);
      }
    }

    private void numFont4_ValueChanged(object sender, EventArgs e) {
      if ((Control.ModifierKeys & Keys.Alt) == Keys.Alt) {
        RemoteHelper.SetIniValueFonts("FontSize6", numFont4.Value.ToString());
        SendToMilkwaveVisualizer("", MessageType.Config);
        SendToMilkwaveVisualizer("", MessageType.TestFonts);
      }
    }

    private void numFont5_ValueChanged(object sender, EventArgs e) {
      if ((Control.ModifierKeys & Keys.Alt) == Keys.Alt) {
        RemoteHelper.SetIniValueFonts("FontSize7", numFont5.Value.ToString());
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
        numVisIntensity.Increment = 0.02M;
      }
      SendToMilkwaveVisualizer("", MessageType.VisIntensity);
    }

    private void numVisShift_ValueChanged(object sender, EventArgs e) {
      if ((Control.ModifierKeys & Keys.Alt) == Keys.Alt) {
        numVisShift.Increment = 0.05M;
      } else {
        numVisShift.Increment = 0.02M;
      }
      SendToMilkwaveVisualizer("", MessageType.VisShift);
    }

    private void numVisVersion_ValueChanged(object sender, EventArgs e) {
      SendToMilkwaveVisualizer("", MessageType.VisVersion);
    }

    private void numSettingsHue_ValueChanged(object sender, EventArgs e) {
      if (updatingSettingsParams) return;
      if ((Control.ModifierKeys & Keys.Alt) == Keys.Alt) {
        numSettingsHue.Increment = 0.05M;
      } else {
        numSettingsHue.Increment = 0.02M;
      }
      SendToMilkwaveVisualizer("", MessageType.ColHue);
    }

    private void numSettingsSaturation_ValueChanged(object sender, EventArgs e) {
      if ((Control.ModifierKeys & Keys.Alt) == Keys.Alt) {
        numSettingsSaturation.Increment = 0.05M;
      } else {
        numSettingsSaturation.Increment = 0.02M;
      }
      SendToMilkwaveVisualizer("", MessageType.ColSaturation);
    }

    private void numSettingsBrightness_ValueChanged(object sender, EventArgs e) {
      if ((Control.ModifierKeys & Keys.Alt) == Keys.Alt) {
        numSettingsBrightness.Increment = 0.05M;
      } else {
        numSettingsBrightness.Increment = 0.02M;
      }
      SendToMilkwaveVisualizer("", MessageType.ColBrightness);
    }

    private void numFFTAttack_ValueChanged(object sender, EventArgs e) {
      if (updatingSettingsParams) return;
      SendToMilkwaveVisualizer("", MessageType.FFTAttack);
    }

    private void numFFTDecay_ValueChanged(object sender, EventArgs e) {
      if (updatingSettingsParams) return;
      SendToMilkwaveVisualizer("", MessageType.FFTDecay);
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

    private void lblVisVersion_Click(object sender, EventArgs e) {
      numVisVersion.Value = 1;
    }

    private void lblQuality_Click(object sender, EventArgs e) {
      numQuality.Value = 1;
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
      OpenFile(cboSettingsOpenFile.Text);
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
        String shaderInputFile = ofdShader.FileName;
        if (File.Exists(shaderInputFile)) {
          string fileString = File.ReadAllText(shaderInputFile);
          try {
            if (shaderInputFile.EndsWith(".json", StringComparison.OrdinalIgnoreCase)) {
              loadShaderFromJson(fileString, false);
            } else {
              txtShaderGLSL.Text = fileString;
            }
            SetStatusText($"Shader loaded from {shaderInputFile}");
          } catch (Exception ex) {
            SetStatusText($"Error loading shader: {ex.Message}");
          }
        } else {
          SetStatusText($"Shader input file not found: {shaderInputFile}");
        }
      }
    }

    private void statusBar_MouseDown(object? sender, MouseEventArgs e) {
      try {
        if (e.Button == MouseButtons.Middle) {
          // MMB action
          if (Settings.RemoteWindowCompactSize.Width > 0) {
            Width = Settings.RemoteWindowCompactSize.Width;
          } else {
            Width = btnTag10.Left + btnTag10.Width + btnTagsSave.Width + 57;
          }
          if (Settings.RemoteWindowCompactSize.Height > 0) {
            Height = Settings.RemoteWindowCompactSize.Height;
          } else {
            Height = cboFont5.Top + cboFont5.Height + statusBar.Height + 137;
          }

          toolStripMenuItemButtonPanel.Checked = false;
          SetPanelsVisibility();
        } else if (e.Button == MouseButtons.Right) {
          if (toolStripMenuItemButtonPanel.Checked) {
            toolStripMenuItemTabsPanel.Checked = true;
            toolStripMenuItemButtonPanel.Checked = false;
          } else {
            toolStripMenuItemTabsPanel.Checked = false;
            toolStripMenuItemButtonPanel.Checked = true;
          }
          SetPanelsVisibility();
        } else if (!string.IsNullOrEmpty(statusBar.Text) && !statusBar.Text.StartsWith("Copied ")) {
          Clipboard.SetText(statusBar.Text);
          SetStatusText($"Copied '{statusBar.Text}' to clipboard");
        }
      } catch { /* ignore */ }
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
      ConvertShader(true);
    }

    private void loadShaderFromJson(string jsonString, bool autoSend) {
      JsonDocument doc = JsonDocument.Parse(jsonString);
      if (doc.RootElement.TryGetProperty("Error", out JsonElement elError)) {
        // If the error property exists, it means the shader was not found
        SetStatusText($"Error: {elError.GetString()}");
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

      var renderpassElements = elShader.GetProperty("renderpass").EnumerateArray();
      ShaderHelper.ConversionErrors.Clear();
      if (renderpassElements.Count() > 1) {
        ShaderHelper.ConversionErrors.AppendLine("Multipass shaders (" + renderpassElements.Count() + ") not supported");
      }

      JsonElement firstRenderpassElement = renderpassElements.First();
      string? shaderCode = firstRenderpassElement.GetProperty("code").GetString();
      if (shaderCode == null) {
        SetStatusText("Shader code not found in the response");
      } else {
        txtShaderinfo.Text = $"{shaderUsername} - {shaderName}" + Environment.NewLine + $"https://www.shadertoy.com/view/{shaderId}";
        string? formattedShaderCode = shaderCode?.Replace("\n", Environment.NewLine);
        txtShaderGLSL.Text = formattedShaderCode;

        if (!String.IsNullOrEmpty(txtShaderGLSL.Text)) {
          ConvertShader(renderpassElements.Count() <= 1);
          if (autoSend) {
            btnSendShader_Click(null, null);
          }
        }
        SetStatusText($"Shader code loaded and converted");
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

    private void ConvertShader(bool clearErrors) {
      txtShaderHLSL.Text = ShaderHelper.ConvertGLSLtoHLSL(txtShaderGLSL.Text, clearErrors);
      if (ShaderHelper.ConversionErrors.Length > 0) {
        picShaderError.Visible = true;
        toolTip1.SetToolTip(picShaderError, ShaderHelper.ConversionErrors.ToString());
      } else {
        picShaderError.Visible = false;
      }
    }

    private void btnShaderHelp_Click(object sender, EventArgs e) {
      OpenURL("https://github.com/IkeC/Milkwave/blob/main/Manual.md#tab-shader");
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
      } catch (Exception) {
        // ignore
      }
    }

    private void numOffset_ValueChanged(object sender, EventArgs e) {
      if (lastReceivedShaderErrorLineNumber > 0) {
        MarkRow(lastReceivedShaderErrorLineNumber - (int)numOffset.Value);
      }
    }
    private void openShadertoyURLForId(string id) {
      OpenURL($"https://www.shadertoy.com/view/{id}");
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
        if ((Control.ModifierKeys & Keys.Control) == Keys.Control) {
          ofdShaderHLSL.InitialDirectory = Path.Combine(ShaderFilesFolder);
        } else {
          ofdShaderHLSL.InitialDirectory = Path.Combine(VisualizerPresetsFolder);
        }
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
          txtShaderHLSL.Text = ShaderHelper.BasicFormatShaderCode(sb.ToString());
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

    private void chkMidiLearn_CheckedChanged(object sender, EventArgs e) {
      int index = GetIndexFromMidiControl((Control)sender);
      var chk = (CheckBox)sender;
      if (chk.Checked && index > 0) {
        if (index != 1) chkMidi1Learn.Checked = false;
        if (index != 2) chkMidi2Learn.Checked = false;
        if (index != 3) chkMidi3Learn.Checked = false;
        if (index != 4) chkMidi4Learn.Checked = false;
        if (index != 5) chkMidi5Learn.Checked = false;
      }
    }

    private Action<MidiEventInfo> MidiMessageReceived() {
      return note => {
        // Marshal all UI changes to the UI thread
        this.BeginInvoke((Action)(() => {
          bool isButton = note.Controller == 0;
          int rowIndex = GetLearningRowIndex(); // starts from 1

          if (rowIndex > 0) {
            TextBox txtMidiLabel = FindTextbox($"txtMidi{rowIndex}Label");
            if ((string.IsNullOrEmpty(txtMidiLabel.Text) || txtMidiLabel.Text.Equals("Button/Note") || txtMidiLabel.Text.Equals("Knob/Fader"))) {
              txtMidiLabel.Text = isButton ? "Button/Note" : "Knob/Fader";
            }

            TextBox txtMidiCh = FindTextbox($"txtMidi{rowIndex}Ch");
            txtMidiCh.Text = note.Channel.ToString();

            TextBox txtMidiVal = FindTextbox($"txtMidi{rowIndex}Val");
            txtMidiVal.Text = note.Value.ToString();

            TextBox txtMidiCon = FindTextbox($"txtMidi{rowIndex}Con");
            txtMidiCon.Text = note.Controller.ToString();

            // update row
            UpdateRowData(rowIndex);
            UpdateRowDataActionType(rowIndex, note.Controller == 0 ? MidiActionType.Button : MidiActionType.Knob);

            ComboBox cboMidiAction = FindCombobox($"cboMidi{rowIndex}Action");
            if (isButton) {
              PopulateMidiActionComboBoxForButton(cboMidiAction, false);
            } else {
              PopulateMidiActionComboBoxForKnob(cboMidiAction, false);
            }
          } else {
            // Not in learning mode, check if it matches any of the configured rows
            for (int i = 0; i < MidiHelper.MidiRows.Count; i++) {
              var row = MidiHelper.MidiRows[i];
              if (row.Active && row.Channel == note.Channel && row.Controller == note.Controller) {
                if (row.Controller == 0 && row.Value == note.Value) {
                  TriggerMidiButtonAction(row);
                } else if (row.Controller != 0) {
                  // Cancel any pending knob action for this controller
                  if (KnobActionDelays.TryGetValue((int)row.Controller, out var existingCts)) {
                    existingCts.Cancel();
                    existingCts.Dispose();
                  }

                  var cts = new CancellationTokenSource();
                  KnobActionDelays[(int)row.Controller] = cts;

                  Task.Delay(Settings.MidiBufferDelay, cts.Token).ContinueWith(t => {
                    if (!t.IsCanceled) {
                      TriggerMidiKnobAction(row, note.Value);
                    }
                  }, TaskScheduler.FromCurrentSynchronizationContext());
                }
              }
            }

          }
        }));
      };
    }

    private void PopulateMidiActionComboBoxForKnob(ComboBox cboMidiAction, bool force) {
      if (cboMidiAction.DropDownStyle != ComboBoxStyle.DropDownList || force) {
        cboMidiAction.DropDownStyle = ComboBoxStyle.DropDownList;
        DarkModeCS.RemoveControl(cboMidiAction);
        dm.ThemeControl(cboMidiAction);
        cboMidiAction.Items.Clear();
        cboMidiAction.DisplayMember = nameof(MidiActionEntry.ActionText);
        cboMidiAction.ValueMember = nameof(MidiActionEntry.ActionId);
        cboMidiAction.Items.Add(new MidiActionEntry("Preset: Amp (L)", MidiActionId.KnobPresetAmpL));
        cboMidiAction.Items.Add(new MidiActionEntry("Preset: Amp (R)", MidiActionId.KnobPresetAmpR));
        cboMidiAction.Items.Add(new MidiActionEntry("Message: BPM", MidiActionId.KnobMessageBPM));
        cboMidiAction.Items.Add(new MidiActionEntry("Message: Beats", MidiActionId.KnobMessageBeats));
        cboMidiAction.Items.Add(new MidiActionEntry("Settings: Time", MidiActionId.KnobSettingsTime));
        cboMidiAction.Items.Add(new MidiActionEntry("Settings: FPS", MidiActionId.KnobSettingsFPS));
        cboMidiAction.Items.Add(new MidiActionEntry("Settings: Intensity", MidiActionId.KnobSettingsIntensity));
        cboMidiAction.Items.Add(new MidiActionEntry("Settings: Shift", MidiActionId.KnobSettingsShift));
        cboMidiAction.Items.Add(new MidiActionEntry("Settings: Quality", MidiActionId.KnobSettingsQuality));
        cboMidiAction.Items.Add(new MidiActionEntry("Settings: Hue", MidiActionId.KnobSettingsHue));
        cboMidiAction.Items.Add(new MidiActionEntry("Settings: Saturation", MidiActionId.KnobSettingsSaturation));
        cboMidiAction.Items.Add(new MidiActionEntry("Settings: Brightness", MidiActionId.KnobSettingsBrightness));

        cboMidiAction.SelectedIndex = 0;
      }
    }

    private void PopulateMidiActionComboBoxForButton(ComboBox cboMidiAction, bool force) {
      if (cboMidiAction.DropDownStyle != ComboBoxStyle.DropDown || force) {
        cboMidiAction.DropDownStyle = ComboBoxStyle.DropDown;
        DarkModeCS.RemoveControl(cboMidiAction);
        dm.ThemeControl(cboMidiAction);
        cboMidiAction.Text = "";
      }
      cboMidiAction.Items.Clear();
      cboMidiAction.DisplayMember = nameof(MidiActionEntry.ActionText);
      cboMidiAction.ValueMember = nameof(MidiActionEntry.ActionId);
      string filePath = midiDefaultFileName;
      if (!midiDefaultFileName.Contains("\\")) {
        filePath = Path.Combine(BaseDir, midiDefaultFileName);
      }
      if (File.Exists(filePath)) {
        string[] strings = File.ReadAllLines(filePath);
        foreach (string line in strings) {
          if (!line.StartsWith("#")) {
            cboMidiAction.Items.Add(line);
          }
        }
      }
    }

    private void TriggerMidiKnobAction(MidiRow row, int value) {
      decimal inc = 0.02M;
      if (row.Increment.Length > 0) {
        decimal.TryParse(row.Increment, NumberStyles.Number, CultureInfo.InvariantCulture, out inc);
      }
      if (row.ActionId == MidiActionId.KnobPresetAmpL) {
        // base value is 1.0
        numAmpLeft.Value = Math.Clamp(1 + ((value - 64) * inc), numAmpLeft.Minimum, numAmpLeft.Maximum);
      } else if (row.ActionId == MidiActionId.KnobPresetAmpR) {
        // base value is 1.0
        numAmpRight.Value = Math.Clamp(1 + ((value - 64) * inc), numAmpRight.Minimum, numAmpRight.Maximum);
      } else if (row.ActionId == MidiActionId.KnobMessageBPM) {
        // base value is 120
        numBPM.Value = Math.Clamp(120 + ((value - 64) * inc), numBPM.Minimum, numBPM.Maximum);
      } else if (row.ActionId == MidiActionId.KnobMessageBeats) {
        // base value is 8
        numBeats.Value = Math.Clamp(8 + ((value - 64) * inc), numBeats.Minimum, numBeats.Maximum);
      } else if (row.ActionId == MidiActionId.KnobSettingsTime) {
        // base value is 1.0
        numFactorTime.Value = Math.Clamp(1 + ((value - 64) * inc), numFactorTime.Minimum, numFactorTime.Maximum);
      } else if (row.ActionId == MidiActionId.KnobSettingsFPS) {
        // base value is 1.0
        numFactorFPS.Value = Math.Clamp(1 + ((value - 64) * inc), numFactorFPS.Minimum, numFactorFPS.Maximum);
      } else if (row.ActionId == MidiActionId.KnobSettingsIntensity) {
        // base value is 1.0
        numVisIntensity.Value = Math.Clamp(1 + ((value - 64) * inc), numVisIntensity.Minimum, numVisIntensity.Maximum);
      } else if (row.ActionId == MidiActionId.KnobSettingsShift) {
        // base value is 0.0
        numVisShift.Value = Math.Clamp((value - 64) * inc, numVisShift.Minimum, numVisShift.Maximum);
      } else if (row.ActionId == MidiActionId.KnobSettingsQuality) {
        // 0..1
        numQuality.Value = Math.Clamp((decimal)value / 127, numQuality.Minimum, numQuality.Maximum);
      } else if (row.ActionId == MidiActionId.KnobSettingsHue) {
        // -1..1
        numSettingsHue.Value = Math.Clamp((value - 64) * inc, numSettingsHue.Minimum, numSettingsHue.Maximum);
      } else if (row.ActionId == MidiActionId.KnobSettingsSaturation) {
        // -1..1
        numSettingsSaturation.Value = Math.Clamp((value - 64) * inc, numSettingsSaturation.Minimum, numSettingsSaturation.Maximum);
      } else if (row.ActionId == MidiActionId.KnobSettingsBrightness) {
        // -1..1
        numSettingsBrightness.Value = Math.Clamp((value - 64) * inc, numSettingsBrightness.Minimum, numSettingsBrightness.Maximum);
      }
    }

    private void TriggerMidiButtonAction(MidiRow row) {
      HandleScriptLine(true, row.ActionText);
    }

    private int GetLearningRowIndex() {
      if (chkMidi1Learn.Checked) return 1;
      if (chkMidi2Learn.Checked) return 2;
      if (chkMidi3Learn.Checked) return 3;
      if (chkMidi4Learn.Checked) return 4;
      if (chkMidi5Learn.Checked) return 5;
      return 0;
    }

    private void cboMidiDevice_SelectedIndexChanged(object sender, EventArgs e) {
      try {
        MidiDeviceEntry? entry = (MidiDeviceEntry?)cboMidiDevice.SelectedItem;
        int deviceIndex = entry?.DeviceIndex ?? -1;
        if (deviceIndex >= 0) {
          MidiHelper.SelectDevice(deviceIndex);
        }
      } catch (Exception ex) {
        SetStatusText($"Error selecting MIDI device: {ex.Message}");
      }
    }

    private void btnMidiDeviceScan_Click(object sender, EventArgs e) {
      PopulateMidiDevicesList();
    }

    private void cboMidiAction_SelectedValueChanged(object sender, EventArgs e) {
      if (!AllowMidiRowDataUpdate) return;

      ComboBox cbo = (ComboBox)sender;
      int rowIndex = GetIndexFromMidiControl((Control)sender);
      var chk = FindCheckbox($"chkMidi{rowIndex}Active");
      if (chk != null && !chk.Checked) {
        chk.Checked = true;
      }

      var rowData = MidiHelper.MidiRows[rowIndex - 1];
      TextBox txtMidiInc = FindTextbox($"txtMidi{rowIndex}Inc");
      UpdateRowData(rowIndex);

      AllowMidiRowDataUpdate = false;

      if (rowData.ActionType == MidiActionType.Knob) {
        if (rowData.ActionId == MidiActionId.KnobPresetAmpL || rowData.ActionId == MidiActionId.KnobPresetAmpR) {
          // default increment is 0.1
          txtMidiInc.Text = "0.1";
        } else if (rowData.ActionId == MidiActionId.KnobMessageBPM || rowData.ActionId == MidiActionId.KnobMessageBeats) {
          // default increment is 1
          txtMidiInc.Text = "1.0";
        } else if (rowData.ActionId == MidiActionId.KnobSettingsTime || rowData.ActionId == MidiActionId.KnobSettingsFPS) {
          // default increment is 0.1
          txtMidiInc.Text = "0.1";
        } else if (rowData.ActionId == MidiActionId.KnobSettingsIntensity || rowData.ActionId == MidiActionId.KnobSettingsShift || rowData.ActionId == MidiActionId.KnobSettingsQuality) {
          // default increment is 0.02
          txtMidiInc.Text = "0.02";
        } else if (rowData.ActionId == MidiActionId.KnobSettingsHue || rowData.ActionId == MidiActionId.KnobSettingsSaturation || rowData.ActionId == MidiActionId.KnobSettingsBrightness) {
          // default increment is 0.02
          txtMidiInc.Text = "0.02";
        }
      } else {
        txtMidiInc.Text = "";
      }

      AllowMidiRowDataUpdate = true;
      UpdateRowData(rowIndex);
    }

    private static int GetIndexFromMidiControl(Control ctrl) {
      return int.Parse(ctrl.Name.Substring(ctrl.Name.IndexOf("Midi") + 4, 1));
    }

    private TextBox FindTextbox(string name) {
      return this.Controls.Find(name, true).FirstOrDefault() as TextBox
        ?? throw new InvalidOperationException($"Control {name} missing");
    }

    private ComboBox FindCombobox(string name) {
      return this.Controls.Find(name, true).FirstOrDefault() as ComboBox
        ?? throw new InvalidOperationException($"Control {name} missing");
    }

    private CheckBox FindCheckbox(string name) {
      return this.Controls.Find(name, true).FirstOrDefault() as CheckBox
        ?? throw new InvalidOperationException($"Control {name} missing");
    }

    private void txtMidiInc_TextChanged(object sender, EventArgs e) {
      int rowIndex = GetIndexFromMidiControl((Control)sender);
      UpdateRowData(rowIndex);
    }

    private void btnMIDIHelp_Click(object sender, EventArgs e) {
      OpenURL("https://github.com/IkeC/Milkwave/blob/main/Manual.md#tab-midi");
    }

    private void numMidiBank_ValueChanged(object sender, EventArgs e) {
      int baseVal = ((int)numMidiBank.Value - 1) * 5;
      lblMidi1Row.Text = (baseVal + 1).ToString();
      lblMidi2Row.Text = (baseVal + 2).ToString();
      lblMidi3Row.Text = (baseVal + 3).ToString();
      lblMidi4Row.Text = (baseVal + 4).ToString();
      lblMidi5Row.Text = (baseVal + 5).ToString();
      chkMidi1Learn.Checked = false;
      chkMidi2Learn.Checked = false;
      chkMidi3Learn.Checked = false;
      chkMidi4Learn.Checked = false;
      chkMidi5Learn.Checked = false;
      FillRowsFromData();
    }

    private void btnMIDISave_Click(object sender, EventArgs e) {
      SaveMIDISettings();
    }

    private void btnMIDILoad_Click(object sender, EventArgs e) {
      LoadMIDISettings();
    }

    private void SaveMIDISettings() {
      string jsonString = JsonSerializer.Serialize(MidiHelper.MidiRows, new JsonSerializerOptions { WriteIndented = true });
      string settingsFile = Path.Combine(BaseDir, milkwaveMidiFile);
      try {
        File.WriteAllText(settingsFile, jsonString);
      } catch (UnauthorizedAccessException) {
        MessageBox.Show($"Unable to save settings to {settingsFile}." +
          Environment.NewLine + Environment.NewLine +
          "Please make sure that Milkwave is installed to a directory with full write access (eg. not 'Program Files').",
          "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
      } catch (Exception ex) {
        Program.SaveErrorToFile(ex, "Error");
      }
    }

    private void LoadMIDISettings() {
      try {
        string jsonString = File.ReadAllText(Path.Combine(BaseDir, milkwaveMidiFile));
        List<MidiRow>? MidiRows = JsonSerializer.Deserialize<List<MidiRow>>(jsonString, new JsonSerializerOptions {
          PropertyNameCaseInsensitive = true
        });
        if (MidiRows != null) {
          MidiHelper.MidiRows = MidiRows;
        }
      } catch (Exception) {
        // ignore
      }
      FillRowsFromData();
    }

    private void txtMidiLabel_TextChanged(object sender, EventArgs e) {
      int rowIndex = GetIndexFromMidiControl((Control)sender);
      UpdateRowData(rowIndex);
    }

    private void UpdateRowData(int rowIndex) {
      if (!AllowMidiRowDataUpdate) return;

      var dataRowIndex = (rowIndex - 1) + ((int)numMidiBank.Value - 1) * 5;
      var row = MidiHelper.MidiRows[dataRowIndex];

      ComboBox cboAction = FindCombobox($"cboMidi{rowIndex}Action");
      MidiActionEntry? cboActionEntry = cboAction.SelectedItem as MidiActionEntry;
      if (cboActionEntry != null) {
        row.ActionId = cboActionEntry.ActionId;
      }

      row.Label = FindTextbox($"txtMidi{rowIndex}Label").Text;

      if (int.TryParse(FindTextbox($"txtMidi{rowIndex}Ch").Text, out var channel)) {
        row.Channel = channel;
      }

      if (row.ActionType == MidiActionType.Knob) {
        row.Value = 0; // value is not relevant for knobs
      } else if (int.TryParse(FindTextbox($"txtMidi{rowIndex}Val").Text, out var value)) {
        row.Value = value;
      }

      if (int.TryParse(FindTextbox($"txtMidi{rowIndex}Con").Text, out var controller)) {
        row.Controller = controller;
      }

      row.Active = FindCheckbox($"chkMidi{rowIndex}Active").Checked;
      row.Increment = FindTextbox($"txtMidi{rowIndex}Inc").Text;

      if (row.ActionType == MidiActionType.Button) {
        row.ActionText = cboAction.Text;
      } else {
        row.ActionText = "";
      }
    }

    private void UpdateRowDataActionType(int rowIndex, MidiActionType type) {
      if (!AllowMidiRowDataUpdate) return;

      var dataRowIndex = (rowIndex - 1) + ((int)numMidiBank.Value - 1) * 5;
      var row = MidiHelper.MidiRows[dataRowIndex];
      row.ActionType = type;
    }

    private void FillRowsFromData() {
      AllowMidiRowDataUpdate = false;
      try {
        int dataIndex = ((int)numMidiBank.Value - 1) * 5;
        for (int rowIndex = 1; rowIndex <= 5; rowIndex++, dataIndex++) {
          FindTextbox($"txtMidi{rowIndex}Label").Text = MidiHelper.MidiRows[dataIndex].Label;
          FindTextbox($"txtMidi{rowIndex}Ch").Text = MidiHelper.MidiRows[dataIndex].Channel.ToString();
          FindTextbox($"txtMidi{rowIndex}Val").Text = MidiHelper.MidiRows[dataIndex].Value.ToString();
          FindTextbox($"txtMidi{rowIndex}Con").Text = MidiHelper.MidiRows[dataIndex].Controller.ToString();
          FindCheckbox($"chkMidi{rowIndex}Active").Checked = MidiHelper.MidiRows[dataIndex].Active;
          FindTextbox($"txtMidi{rowIndex}Inc").Text = MidiHelper.MidiRows[dataIndex].Increment;

          ComboBox cboMidiAction = FindCombobox($"cboMidi{rowIndex}Action");
          if (cboMidiAction != null) {
            cboMidiAction.Text = "";
            cboMidiAction.Items.Clear();
            if (MidiHelper.MidiRows[dataIndex].ActionType == MidiActionType.Button) {
              PopulateMidiActionComboBoxForButton(cboMidiAction, true);
              cboMidiAction.Text = MidiHelper.MidiRows[dataIndex].ActionText;
            } else if (MidiHelper.MidiRows[dataIndex].ActionType == MidiActionType.Knob) {
              PopulateMidiActionComboBoxForKnob(cboMidiAction, true);
              // Try to select the existing action
              foreach (var item in cboMidiAction.Items) {
                if (item is MidiActionEntry entry && entry.ActionId == MidiHelper.MidiRows[dataIndex].ActionId) {
                  cboMidiAction.SelectedItem = item;
                  break;
                }
              }
            }
          }
        }
      } catch (Exception ex) {
        SetStatusText($"Error: {ex.Message}");
      } finally {
        AllowMidiRowDataUpdate = true;
      }
    }

    private void chkMidiActive_CheckedChanged(object sender, EventArgs e) {
      int rowIndex = GetIndexFromMidiControl((Control)sender);
      UpdateRowData(rowIndex);
    }

    private void cboSpoutRes_ValueChanged(object sender, EventArgs e) {
      ComboBox cbo = (ComboBox)sender;
      if (cbo.Text.Length > 0) {
        // remove any non-digit characters
        cbo.Text = new string(cbo.Text.Where(char.IsDigit).ToArray());
      }
      if (!updatingSettingsParams) {
        SendToMilkwaveVisualizer("", MessageType.SpoutResolution);
      }
    }

    private void chkSpoutActive_CheckedChanged(object sender, EventArgs e) {
      if (!updatingSettingsParams) {
        SendToMilkwaveVisualizer("", MessageType.SpoutActive);
      }
    }

    private void chkSpoutFixedSize_CheckedChanged(object sender, EventArgs e) {
      if (!updatingSettingsParams) {
        SendToMilkwaveVisualizer("", MessageType.SpoutFixedSize);
      }
    }

    private void numQuality_ValueChanged(object sender, EventArgs e) {
      if (updatingSettingsParams) return;

      if ((Control.ModifierKeys & Keys.Alt) == Keys.Alt) {
        numQuality.Increment = 0.05M;
      } else if (numQuality.Value > 0.1M) {
        numQuality.Increment = 0.02M;
      } else {
        numQuality.Increment = 0.01M;
      }
      SendToMilkwaveVisualizer("", MessageType.RenderQuality);
    }

    private async void MonitorTimer_Tick(object? sender, EventArgs? e) {
      float usageCPU = 0, usageGPU = 0;
      if (toolStripMenuItemMonitorCPU.Checked) {
        usageCPU = await Task.Run(() => MonitorHelper.GetCPUUsage());
      }
      if (toolStripMenuItemMonitorGPU.Checked) {
        usageGPU = await Task.Run(() => MonitorHelper.GetGPUUsage());
      }
      if (toolStripMenuItemMonitorCPU.Checked) {
        if (usageCPU >= 0) {
          toolStripStatusLabelMonitorCPU.Text = $"{usageCPU:F0}";
        } else {
          toolStripStatusLabelMonitorCPU.Text = "?";
        }
      }
      if (toolStripMenuItemMonitorGPU.Checked) {
        if (usageGPU >= 0) {
          toolStripStatusLabelMonitorGPU.Text = $"{usageGPU:F0}";
        } else {
          toolStripStatusLabelMonitorGPU.Text = "?";
        }
      }
    }

    private void toolStripMenuItemMonitorCPU_Click(object sender, EventArgs e) {
      toolStripMenuItemMonitorCPU.Checked = !toolStripMenuItemMonitorCPU.Checked;
      ToggleMonitors();
    }

    private void toolStripMenuItemMonitorGPU_Click(object sender, EventArgs e) {
      toolStripMenuItemMonitorGPU.Checked = !toolStripMenuItemMonitorGPU.Checked;
      ToggleMonitors();
    }

    private void ToggleMonitors() {
      toolStripStatusLabelMonitorCPU.Visible = toolStripMenuItemMonitorCPU.Checked;
      toolStripStatusLabelMonitorGPU.Visible = toolStripMenuItemMonitorGPU.Checked;

      if (toolStripMenuItemMonitorCPU.Checked || toolStripMenuItemMonitorGPU.Checked) {
        monitorTimer.Start();
      } else {
        monitorTimer.Stop();
      }
    }

    private void lblMidiRow_DoubleClick(object sender, EventArgs e) {
      int rowIndex = GetIndexFromMidiControl((Control)sender);
      ClearRow(rowIndex);
    }

    private void ClearRow(int rowIndex) {
      var chk = FindCheckbox($"chkMidi{rowIndex}Active");
      if (chk != null && chk.Checked) {
        chk.Checked = false;
      }
      chk = FindCheckbox($"chkMidi{rowIndex}Learn");
      if (chk != null && chk.Checked) {
        chk.Checked = false;
      }
      FindTextbox($"txtMidi{rowIndex}Label").Text = "";
      FindTextbox($"txtMidi{rowIndex}Ch").Text = "0";
      FindTextbox($"txtMidi{rowIndex}Val").Text = "0";
      FindTextbox($"txtMidi{rowIndex}Con").Text = "0";
      FindTextbox($"txtMidi{rowIndex}Inc").Text = "";
      ComboBox cboMidiAction = FindCombobox($"cboMidi{rowIndex}Action");
      if (cboMidiAction != null) {
        cboMidiAction.DropDownStyle = ComboBoxStyle.DropDown;
        DarkModeCS.RemoveControl(cboMidiAction);
        dm.ThemeControl(cboMidiAction);
        cboMidiAction.Text = "";
        cboMidiAction.Items.Clear();
      }
      UpdateRowData(rowIndex);
    }

    private void btnQualityHalf_Click(object sender, EventArgs e) {
      numQuality.Value = Math.Clamp(numQuality.Value / 2, numQuality.Minimum, numQuality.Maximum);
    }

    private void btnQualityDouble_Click(object sender, EventArgs e) {
      numQuality.Value = Math.Clamp(numQuality.Value * 2, numQuality.Minimum, numQuality.Maximum);
    }

    private void lblHue_Click(object sender, EventArgs e) {
      numSettingsHue.Value = 0;
    }

    private void lblSaturation_Click(object sender, EventArgs e) {
      numSettingsSaturation.Value = 0;
    }

    private void lblValue_Click(object sender, EventArgs e) {
      numSettingsBrightness.Value = 0;
    }

    private async void txtPreset_TextChanged(object sender, EventArgs e) {
      // Cancel any pending invocation
      CancellationTokenFilterPresetList?.Cancel();
      var cts = (CancellationTokenFilterPresetList = new CancellationTokenSource());

      try {
        // Wait delay or until cancelled
        await Task.Delay(150, cts.Token);
        FillAndFilterPresetList();
      } catch (TaskCanceledException) {
        // Swallow—new keystroke arrived
      }
    }

    private void FillAndFilterPresetList() {
      string filter = txtFilterPresets.Text.Trim().ToLowerInvariant();
      cboPresets.BeginUpdate();
      cboPresets.Items.Clear();
      foreach (var preset in PresetsMasterList) {
        if (string.IsNullOrEmpty(filter) || preset.DisplayName.ToLowerInvariant().Contains(filter)) {
          cboPresets.Items.Add(preset);
        }
      }
      if (cboPresets.Items.Count > 0) {
        cboPresets.SelectedIndex = 0;
      }
      if (cboPresets.Items.Count != PresetsMasterList.Count) {
        SetStatusText($"Showing {cboPresets.Items.Count} out of {PresetsMasterList.Count} presets");
      } else {
        SetStatusText($"{cboPresets.Items.Count} presets");
      }
      cboPresets.EndUpdate();
    }

    private void chkQualityAuto_CheckedChanged(object sender, EventArgs e) {
      if (!updatingSettingsParams) {
        SendToMilkwaveVisualizer("", MessageType.QualityAuto);
      }
    }

    private void btnShadertoyFilesLoad_Click(object? sender, EventArgs? e) {
      using (var fbd = new FolderBrowserDialog()) {
        if (Directory.Exists(VisualizerPresetsFolder)) {
          fbd.InitialDirectory = VisualizerPresetsFolder;
        } else {
          fbd.InitialDirectory = BaseDir;
        }
        if (fbd.ShowDialog() == DialogResult.OK && !string.IsNullOrWhiteSpace(fbd.SelectedPath)) {
          // add all .json files in the selected folder without file extension
          var files = Directory.GetFiles(fbd.SelectedPath, "*.json");
          // remove .json extension for all files
          Settings.ShadertoyFilesDirectory = fbd.SelectedPath;
          setShadertoyFilesFromDir(Settings.ShadertoyFilesDirectory);
          numShadertoyFileIndex.Value = 1;
          setShadertoyFileText();

          SetStatusText($"Found {files.Length} files");
        }
      }
    }

    private void setShadertoyFilesFromDir(string directory) {
      shadertoyFilesList.Clear();
      var files = Directory.GetFiles(directory, "*.json");
      foreach (var file in files) {
        shadertoyFilesList.Add(Path.GetFileNameWithoutExtension(file));
      }
      shadertoyFilesList.Sort(StringComparer.InvariantCultureIgnoreCase);
      numShadertoyFileIndex.Maximum = Math.Max(1, shadertoyFilesList.Count);
    }

    private void btnShadertoyFileLoadThis_Click(object? sender, EventArgs? e) {
      // get selected file from shadertoyFilesList based on numShadertoyFileIndex
      int index = (int)numShadertoyFileIndex.Value - 1;
      if (index >= 0 && index < shadertoyFilesList.Count) {
        string selectedFile = shadertoyFilesList[index];
        string filePath = Path.Combine(Settings.ShadertoyFilesDirectory, selectedFile + ".json");
        if (File.Exists(filePath)) {
          try {
            string jsonString = File.ReadAllText(filePath);
            loadShaderFromJson(jsonString, true);
          } catch (Exception ex) {
            SetStatusText($"Error loading file: {ex.Message}");
          }
        } else {
          SetStatusText($"Error: File {selectedFile}.json not found");
        }
      }
    }

    private void setShadertoyFileText() {
      if (shadertoyFilesList.Count < numShadertoyFileIndex.Value) {
        txtShadertoyFile.Text = "";
      } else {
        txtShadertoyFile.Text = shadertoyFilesList[(int)numShadertoyFileIndex.Value - 1];
      }
    }

    private void btnShadertoyFileLoadNext_Click(object sender, EventArgs e) {
      if ((Control.ModifierKeys & Keys.Control) == Keys.Control) {
        if (numShadertoyFileIndex.Value > numShadertoyFileIndex.Minimum) {
          numShadertoyFileIndex.Value = numShadertoyFileIndex.Value - 1;
        }
      } else {
        if (numShadertoyFileIndex.Value < numShadertoyFileIndex.Maximum) {
          numShadertoyFileIndex.Value = numShadertoyFileIndex.Value + 1;
        }
      }
      if (numShadertoyFileIndex.Value % 5 == 0) {
        // every 5 files save the settings to prevent loss
        SaveSettingsToFile();
      }
      chkShaderFile.Checked = false;
      btnShadertoyFileLoadThis_Click(null, null);
      if ((Control.ModifierKeys & Keys.Alt) == Keys.Alt) {
        openShadertoyURLForId(txtShadertoyFile.Text);
      }
    }

    private void numShadertoyFileIndex_ValueChanged(object sender, EventArgs e) {
      setShadertoyFileText();
      Settings.ShadertoyFileIndex = (int)numShadertoyFileIndex.Value;
    }

    private void txtShadertoyFile_MouseDown(object sender, MouseEventArgs e) {
      if (e.Button == MouseButtons.Middle) {
        openShadertoyURLForId(txtShadertoyFile.Text);
      }
    }

    private void lblTags_MouseDown(object sender, MouseEventArgs e) {
      if ((Control.ModifierKeys & Keys.Control) == Keys.Control) {
        if (MessageBox.Show("Remove tags for ALL presets?", "Please confirm", MessageBoxButtons.YesNo, MessageBoxIcon.Question) == DialogResult.Yes) {
          // remove tags-remote.json and reload tags to clear
          string tagsFile = Path.Combine(BaseDir, "tags-remote.json");
          if (File.Exists(tagsFile)) {
            try {
              File.Delete(tagsFile);
              txtTags.Text = "";
              Tags = new Tags();
              SetTopTags();
              SetStatusText("All tags cleared");
            } catch (Exception ex) {
              SetStatusText($"Error clearing tags: {ex.Message}");
            }
          } else {
            SetStatusText("No tags file found to clear.");
          }
        }
      }
    }

    private Size GetCalculatedOptionalTopPanelSize() {
      int width = btnTag10.Left + btnTag10.Width + btnTagsSave.Width + cboPresets.Top * 4;
      int height = cboFont5.Top + cboFont5.Height + cboPresets.Top;
      return new Size(width, height);
    }

    private void btnFontGlobalMinus_Click(object sender, EventArgs e) {
      numFont1.Value = Math.Clamp(numFont1.Value - 5, numFont1.Minimum, numFont1.Maximum);
      numFont2.Value = Math.Clamp(numFont2.Value - 5, numFont2.Minimum, numFont2.Maximum);
      numFontMenu.Value = Math.Clamp(numFontMenu.Value - 5, numFontMenu.Minimum, numFontMenu.Maximum);
      numFont3.Value = Math.Clamp(numFont3.Value - 5, numFont3.Minimum, numFont3.Maximum);
      numFont4.Value = Math.Clamp(numFont4.Value - 5, numFont4.Minimum, numFont4.Maximum);
      numFont5.Value = Math.Clamp(numFont5.Value - 5, numFont5.Minimum, numFont5.Maximum);
      btnSettingsSave_Click(null, null);
      btnTestFonts_Click(null, null);
    }

    private void btnFontGlobalPlus_Click(object sender, EventArgs e) {
      numFont1.Value = Math.Clamp(numFont1.Value + 5, numFont1.Minimum, numFont1.Maximum);
      numFont2.Value = Math.Clamp(numFont2.Value + 5, numFont2.Minimum, numFont2.Maximum);
      numFontMenu.Value = Math.Clamp(numFontMenu.Value + 5, numFontMenu.Minimum, numFontMenu.Maximum);
      numFont3.Value = Math.Clamp(numFont3.Value + 5, numFont3.Minimum, numFont3.Maximum);
      numFont4.Value = Math.Clamp(numFont4.Value + 5, numFont4.Minimum, numFont4.Maximum);
      numFont5.Value = Math.Clamp(numFont5.Value + 5, numFont5.Minimum, numFont5.Maximum);
      btnSettingsSave_Click(null, null);
      btnTestFonts_Click(null, null);
    }

    private void chkHueAuto_CheckedChanged(object sender, EventArgs e) {
      if (!updatingSettingsParams) {
        SendToMilkwaveVisualizer("", MessageType.HueAuto);
      }
    }

    private void numSettingsHueAuto_ValueChanged(object sender, EventArgs e) {
      if (updatingSettingsParams) return;
      SendToMilkwaveVisualizer("", MessageType.HueAutoSeconds);
    }

    private void numPresetChange_ValueChanged(object sender, EventArgs e) {
      if (updatingSettingsParams) return;
      string fTimeBetweenPresets = numPresetChange.Value.ToString("F6", CultureInfo.InvariantCulture);
      RemoteHelper.SetIniValue("Settings", "fTimeBetweenPresets", fTimeBetweenPresets);

      SendToMilkwaveVisualizer("", MessageType.Settings);
    }

    private void chkPresetLocked_CheckedChanged(object sender, EventArgs e) {
      if (updatingSettingsParams) return;
      SendUnicodeChars("~");
    }

    private void chkSettingsPresetRandom_CheckedChanged(object sender, EventArgs e) {
      if (updatingSettingsParams) return;
      SendUnicodeChars("r");
    }

    private void LoadPresetDeck() {
      try {
        string path = Path.Combine(BaseDir, milkwavePresetDeckFile);
        if (File.Exists(path)) {
          string json = File.ReadAllText(path);
          var loaded = JsonSerializer.Deserialize<PresetDeck>(json);
          if (loaded != null) {
            presetDeck = loaded;
          }
        }
      } catch (Exception ex) {
        Program.SaveErrorToFile(ex, "Load preset deck");
      }
    }

    private void SavePresetDeck() {
      try {
        string path = Path.Combine(BaseDir, milkwavePresetDeckFile);
        string json = JsonSerializer.Serialize(presetDeck, new JsonSerializerOptions { WriteIndented = true });
        File.WriteAllText(path, json);
      } catch (Exception ex) {
        Program.SaveErrorToFile(ex, "Save preset deck");
      }
    }

    private void ApplyPanelMode() {
      if (Settings.IsPresetMode) {
        ApplyPresetMode();
      } else {
        ApplyCommandMode();
      }
    }

    private void ApplyPresetMode() {
      UpdateModeToggleButton();
      UpdateBankSwitchButton();
      for (int i = 0; i < presetDeckButtons.Length; i++) {
        UpdatePresetDeckButtonAppearance(i + 1);
      }
    }

    private void ApplyCommandMode() {
      UpdateModeToggleButton();
      RestoreBankSwitchButton();
      for (int i = 0; i < presetDeckButtons.Length; i++) {
        RestoreCommandButtonAppearance(i);
      }
    }

    private void UpdateBankSwitchButton() {
      if (!Settings.IsPresetMode) {
        return;
      }

      string bankIconPath = Path.Combine(BaseDir, $"resources\\buttons\\btn-bank-{Settings.CurrentPresetBank}.png");

      if (File.Exists(bankIconPath) && btn88.ClientSize.Width > 0 && btn88.ClientSize.Height > 0) {
        try {
          using (Image original = Image.FromFile(bankIconPath)) {
            Rectangle destRect = CalculatePreviewBounds(original.Size, btn88.ClientSize);
            if (destRect.Width > 0 && destRect.Height > 0) {
              Bitmap preview = new Bitmap(btn88.ClientSize.Width, btn88.ClientSize.Height);
              using (Graphics g = Graphics.FromImage(preview)) {
                g.Clear(Color.Transparent);
                g.CompositingQuality = CompositingQuality.HighQuality;
                g.InterpolationMode = InterpolationMode.HighQualityBicubic;
                g.PixelOffsetMode = PixelOffsetMode.HighQuality;
                g.SmoothingMode = SmoothingMode.HighQuality;
                g.DrawImage(original, destRect);
              }
              ApplyButtonImage(btn88, preview);
            }
          }
        } catch (Exception ex) {
          Program.SaveErrorToFile(ex, "Load bank icon");
          btn88.Text = $"Bank {Settings.CurrentPresetBank}";
        }
      } else {
        btn88.Text = $"Bank {Settings.CurrentPresetBank}";
      }

      toolTip1.SetToolTip(btn88, $"Bank {Settings.CurrentPresetBank} (Click to cycle)");
    }

    private void RestoreBankSwitchButton() {
      // In command mode, restore sprite button appearance
      UpdateSpriteButtonAppearance(btn88);
    }

    private void Btn88_MouseDown(object? sender, MouseEventArgs e) {
      if (!Settings.IsPresetMode) {
        // In command mode, execute normal 88 action
        SendUnicodeChars("88");
        return;
      }

      // In preset mode, cycle banks
      if (e.Button == MouseButtons.Left) {
        Settings.CurrentPresetBank = (Settings.CurrentPresetBank % 3) + 1;
        UpdateBankSwitchButton();

        // Refresh all preset button appearances for new bank
        for (int i = 0; i < presetDeckButtons.Length; i++) {
          UpdatePresetDeckButtonAppearance(i + 1);
        }

        SaveSettingsToFile();
        SetStatusText($"Switched to Bank {Settings.CurrentPresetBank}");
      }
    }

    private void Btn88_Resize(object? sender, EventArgs e) {
      if (Settings.IsPresetMode) {
        UpdateBankSwitchButton();
      }
    }

    private int GetBankOffsetButtonIndex(int buttonIndex) {
      // buttonIndex is 1-15 (local button number)
      // Returns the global button index accounting for bank offset
      // Bank 1: 1-15, Bank 2: 16-30, Bank 3: 31-45
      return buttonIndex + ((Settings.CurrentPresetBank - 1) * 15);
    }

    private void UpdateModeToggleButton() {
      string switchIconPath = Path.Combine(BaseDir, "resources\\buttons\\btn-switch.png");

      if (File.Exists(switchIconPath) && btnSwitchMode.ClientSize.Width > 0 && btnSwitchMode.ClientSize.Height > 0) {
        try {
          using (Image original = Image.FromFile(switchIconPath)) {
            Rectangle destRect = CalculatePreviewBounds(original.Size, btnSwitchMode.ClientSize);
            if (destRect.Width > 0 && destRect.Height > 0) {
              Bitmap preview = new Bitmap(btnSwitchMode.ClientSize.Width, btnSwitchMode.ClientSize.Height);
              using (Graphics g = Graphics.FromImage(preview)) {
                g.Clear(Color.Transparent);
                g.CompositingQuality = CompositingQuality.HighQuality;
                g.InterpolationMode = InterpolationMode.HighQualityBicubic;
                g.PixelOffsetMode = PixelOffsetMode.HighQuality;
                g.SmoothingMode = SmoothingMode.HighQuality;
                g.DrawImage(original, destRect);
              }
              ApplyButtonImage(btnSwitchMode, preview);
            }
          }
        } catch (Exception ex) {
          Program.SaveErrorToFile(ex, "Load switch icon");
        }
      }

      if (Settings.IsPresetMode) {
        toolTip1.SetToolTip(btnSwitchMode, "Switch to Command mode");
      } else {
        toolTip1.SetToolTip(btnSwitchMode, "Switch to Preset mode");
      }
    }

    private void RestoreCommandButtonAppearance(int buttonArrayIndex) {
      if (buttonArrayIndex < 0 || buttonArrayIndex >= presetDeckButtons.Length) {
        return;
      }

      Button btn = presetDeckButtons[buttonArrayIndex];

      // Clear any preset deck assignment
      if (btn.Tag is int) {
        btn.Tag = null;
      }

      // Dispose any image
      if (buttonImageCache.TryGetValue(btn, out Image? cached) && cached != null) {
        if (btn.Image == cached) {
          btn.Image = null;
        }
        cached.Dispose();
        buttonImageCache.Remove(btn);
      } else {
        btn.Image = null;
      }

      // Restore original button text based on which button it is
      if (btn == btnSpace) btn.Text = "Next Preset\r\n(Space)";
      else if (btn == btnBackspace) btn.Text = "Previous Preset\r\n(Backspace)";
      else if (btn == btnN) btn.Text = "Sound Info\r\n(N)";
      else if (btn == btnB) btn.Text = "Song Info\r\n(B)";
      else if (btn == btnTilde) btn.Text = "Preset Lock \r\n(~)";
      else if (btn == btnF2) btn.Text = "Borderless \r\n(F2)";
      else if (btn == btnF3) btn.Text = "Change FPS\r\n(F3)";
      else if (btn == btnF4) btn.Text = "Preset Info\r\n(F4)";
      else if (btn == btnF7) btn.Text = "Always On Top\r\n(F7)";
      else if (btn == btnF10) btn.Text = "Toggle Spout\r\n(F10)";
      else if (btn == btnAltEnter) btn.Text = "Fullscreen\r\n(Alt+Enter)";
      else if (btn == btnK) btn.Text = "Sprite/Msg Mode\r\n(K)";
      else if (btn == btnDelete) btn.Text = "Clear Sprite/Msg\r\n(Delete)";
      else if (btn == btnWatermark) btn.Text = "Watermark Mode \r\n(Ctrl+Shift+F9)";
      else if (btn == btnTransparency) btn.Text = "Transparency\r\n(F12)";

      // Reset button styling
      btn.Padding = new Padding(0);
      btn.TextAlign = ContentAlignment.MiddleCenter;
      btn.ImageAlign = ContentAlignment.MiddleCenter;
      btn.TextImageRelation = TextImageRelation.Overlay;
      toolTip1.SetToolTip(btn, string.Empty);
    }

    private void UpdatePresetDeckButtonAppearance(int buttonIndex) {
      if (buttonIndex < 1 || buttonIndex > presetDeckButtons.Length) {
        return;
      }

      Button btn = presetDeckButtons[buttonIndex - 1];
      btn.Tag = buttonIndex;

      int globalIndex = GetBankOffsetButtonIndex(buttonIndex);

      if (presetDeck.Assignments.TryGetValue(globalIndex, out var assignment)) {
        string thumbnailPath = Path.Combine(BaseDir, assignment.ThumbnailPath);
        if (File.Exists(thumbnailPath)) {
          try {
            if (buttonImageCache.TryGetValue(btn, out Image? cached)) {
              cached?.Dispose();
              buttonImageCache.Remove(btn);
            }

            Image? thumbnail = CreatePresetThumbnail(thumbnailPath, btn.ClientSize);
            if (thumbnail != null) {
              ApplyButtonImage(btn, thumbnail);
            } else {
              ShowPresetDeckButtonNumber(btn, globalIndex);
            }
          } catch {
            ShowPresetDeckButtonNumber(btn, globalIndex);
          }
        } else {
          ShowPresetDeckButtonNumber(btn, globalIndex);
        }

        toolTip1.SetToolTip(btn, assignment.PresetDisplayName);
      } else {
        ShowPresetDeckButtonNumber(btn, globalIndex);
        toolTip1.SetToolTip(btn, "Right-click to assign preset");
      }
    }

    private void ShowPresetDeckButtonNumber(Button btn, int number) {
      if (buttonImageCache.TryGetValue(btn, out Image? cached)) {
        cached?.Dispose();
        buttonImageCache.Remove(btn);
      }
      btn.Image = null;
      btn.Text = number.ToString();
      btn.TextAlign = ContentAlignment.MiddleCenter;
    }

    private void PresetDeckButton_Resize(object? sender, EventArgs e) {
      if (!Settings.IsPresetMode || sender is not Button btn || btn.Tag is not int buttonIndex) {
        return;
      }

      int globalIndex = GetBankOffsetButtonIndex(buttonIndex);
      if (presetDeck.Assignments.ContainsKey(globalIndex)) {
        UpdatePresetDeckButtonAppearance(buttonIndex);
      }
    }

    private void PresetDeckButton_MouseDown(object sender, MouseEventArgs e) {
      if (!Settings.IsPresetMode || sender is not Button btn || btn.Tag is not int buttonIndex) {
        return;
      }

      int globalIndex = GetBankOffsetButtonIndex(buttonIndex);

      if (e.Button == MouseButtons.Left) {
        if (presetDeck.Assignments.ContainsKey(globalIndex)) {
          TriggerPresetFromDeck(globalIndex);
        } else {
          SetStatusText("Right-click to assign current preset");
        }
      } else if (e.Button == MouseButtons.Right) {
        AssignCurrentPresetToDeckButton(globalIndex);
      } else if (e.Button == MouseButtons.Middle) {
        UnassignPresetFromDeckButton(globalIndex);
      }
    }

    private void TriggerPresetFromDeck(int buttonIndex) {
      if (!presetDeck.Assignments.TryGetValue(buttonIndex, out var assignment)) {
        return;
      }

      string fullPath = assignment.PresetPath;
      if (!Path.IsPathRooted(fullPath)) {
        fullPath = Path.Combine(VisualizerPresetsFolder, assignment.PresetPath);
      }

      if (File.Exists(fullPath)) {
        SendToMilkwaveVisualizer(fullPath, MessageType.PresetFilePath);
        SetStatusText($"Triggered preset: {assignment.PresetDisplayName}");
      } else {
        SetStatusText($"Preset file not found: {fullPath}");
      }
    }

    private void AssignCurrentPresetToDeckButton(int buttonIndex) {
      string? fullPresetPath = toolTip1.GetToolTip(txtVisRunning);
      if (string.IsNullOrEmpty(fullPresetPath)) {
        SetStatusText("No preset currently running");
        return;
      }

      string presetName = Path.GetFileNameWithoutExtension(fullPresetPath);
      if (fullPresetPath.Contains("\\") || fullPresetPath.Contains("/")) {
        int lastSlash = Math.Max(fullPresetPath.LastIndexOf('\\'), fullPresetPath.LastIndexOf('/'));
        string fileNameWithExt = fullPresetPath.Substring(lastSlash + 1);
        if (fileNameWithExt.EndsWith(".milk", StringComparison.OrdinalIgnoreCase) ||
            fileNameWithExt.EndsWith(".milk2", StringComparison.OrdinalIgnoreCase)) {
          presetName = Path.GetFileNameWithoutExtension(fileNameWithExt);
        } else {
          presetName = fileNameWithExt;
        }
      }

      string maybeRelativePath = fullPresetPath;
      int relIndex = fullPresetPath.IndexOf(VisualizerPresetsFolder, StringComparison.OrdinalIgnoreCase);
      if (relIndex >= 0) {
        maybeRelativePath = fullPresetPath.Substring(relIndex + VisualizerPresetsFolder.Length);
      } else {
        // Check if it's already a relative path starting with "resources\presets\" or "resources/presets/"
        const string relativePrefix1 = "resources\\presets\\";
        const string relativePrefix2 = "resources/presets/";
        if (maybeRelativePath.StartsWith(relativePrefix1, StringComparison.OrdinalIgnoreCase)) {
          maybeRelativePath = maybeRelativePath.Substring(relativePrefix1.Length);
        } else if (maybeRelativePath.StartsWith(relativePrefix2, StringComparison.OrdinalIgnoreCase)) {
          maybeRelativePath = maybeRelativePath.Substring(relativePrefix2.Length);
        }
      }

      var pendingThumbnail = new PendingThumbnail {
        Preset = new Preset {
          MaybeRelativePath = maybeRelativePath,
          DisplayName = presetName
        },
        RequestTime = DateTime.Now
      };

      pendingThumbnailAssignments[buttonIndex] = pendingThumbnail;

      System.Diagnostics.Debug.WriteLine($"[AssignPreset] BaseDir: {BaseDir}");
      System.Diagnostics.Debug.WriteLine($"[AssignPreset] Full preset path: {fullPresetPath}");
      System.Diagnostics.Debug.WriteLine($"[AssignPreset] Relative path: {maybeRelativePath}");
      System.Diagnostics.Debug.WriteLine($"[AssignPreset] Sending CAPTURE message to visualizer");

      SetStatusText($"Requesting screenshot for '{presetName}'...");

      SendToMilkwaveVisualizer("CAPTURE", MessageType.CaptureScreenshot);

      Task.Run(() => PollForCaptureFile(buttonIndex));
    }

    private async Task PollForCaptureFile(int buttonIndex) {
      if (!pendingThumbnailAssignments.TryGetValue(buttonIndex, out var pending)) {
        return;
      }

      bool isDX12 = IsSelectedVisualizerDX12();
      string primaryCaptureDir = Path.Combine(GetVisualizerDir(isDX12), "capture");
      string? fallbackCaptureDir = isDX12 ? Path.Combine(GetVisualizerDir(false), "capture") : null;

      System.Diagnostics.Debug.WriteLine($"[PollCapture] Primary dir: {primaryCaptureDir}");
      if (fallbackCaptureDir != null)
        System.Diagnostics.Debug.WriteLine($"[PollCapture] Fallback dir: {fallbackCaptureDir}");

      if (!Directory.Exists(primaryCaptureDir))
        Directory.CreateDirectory(primaryCaptureDir);

      DateTime requestTime = pending.RequestTime;
      TimeSpan timeout = TimeSpan.FromSeconds(2);
      int pollCount = 0;
      bool fallbackEnabled = false;

      this.Invoke(() => SetStatusText("Acquiring screenshot..."));

      while (DateTime.Now - requestTime < timeout) {
        pollCount++;

        // After half a second, also start checking the fallback Milkwave capture dir (DX12 may write there)
        if (!fallbackEnabled && fallbackCaptureDir != null && DateTime.Now - requestTime >= TimeSpan.FromMilliseconds(500)) {
          fallbackEnabled = true;
          System.Diagnostics.Debug.WriteLine($"[PollCapture] Enabling fallback dir after 500ms");
        }

        try {
          FileInfo? captureFile = FindCaptureFile(primaryCaptureDir, requestTime);

          if (captureFile == null && fallbackEnabled && fallbackCaptureDir != null) {
            captureFile = FindCaptureFile(fallbackCaptureDir, requestTime);
            if (captureFile != null)
              System.Diagnostics.Debug.WriteLine($"[PollCapture] Found file in fallback dir: {captureFile.Name}");
          }

          if (captureFile != null) {
            System.Diagnostics.Debug.WriteLine($"[PollCapture] Found capture: {captureFile.FullName}");
            this.Invoke(() => SetStatusText($"Found screenshot: {captureFile.Name}"));
            await ProcessCapturedThumbnail(buttonIndex, captureFile.FullName);
            return;
          }
        } catch (Exception ex) {
          System.Diagnostics.Debug.WriteLine($"[PollCapture] Exception: {ex.Message}");
          Program.SaveErrorToFile(ex, "Poll capture file");
        }

        await Task.Delay(100);
      }

      System.Diagnostics.Debug.WriteLine($"[PollCapture] TIMEOUT after {pollCount} polls");
      this.Invoke(() => {
        pendingThumbnailAssignments.Remove(buttonIndex);
        SetStatusText("Could not acquire screenshot");
      });
    }

    private FileInfo? FindCaptureFile(string captureDir, DateTime requestTime) {
      if (!Directory.Exists(captureDir)) return null;
      return Directory.GetFiles(captureDir, "*.png")
        .Select(f => new FileInfo(f))
        .Where(fi => fi.CreationTime >= requestTime)
        .OrderByDescending(fi => fi.CreationTime)
        .FirstOrDefault();
    }

    private async Task ProcessCapturedThumbnail(int buttonIndex, string captureFilePath) {
      if (!pendingThumbnailAssignments.TryGetValue(buttonIndex, out var pending)) {
        return;
      }

      // The Visualizer may still have the file open (finishing the write). Retry for up to 2 seconds.
      const int maxWaitMs = 2000;
      const int retryIntervalMs = 100;
      int waited = 0;
      while (true) {
        try {
          using var testFs = new FileStream(captureFilePath, FileMode.Open, FileAccess.Read, FileShare.Read);
          break; // file is accessible
        } catch (IOException) when (waited < maxWaitMs) {
          waited += retryIntervalMs;
          await Task.Delay(retryIntervalMs);
        }
      }

      try {
        using (var fs = new FileStream(captureFilePath, FileMode.Open, FileAccess.Read, FileShare.Read))
        using (var img = Image.FromStream(fs, useEmbeddedColorManagement: false, validateImageData: false)) {
          using Image croppedImage = CropTo16x9(img);

          string thumbnailPath = Path.Combine(BaseDir, $"resources\\buttons\\btn-{buttonIndex:D2}.png");
          string thumbnailDir = Path.GetDirectoryName(thumbnailPath);
          if (!string.IsNullOrEmpty(thumbnailDir) && !Directory.Exists(thumbnailDir)) {
            Directory.CreateDirectory(thumbnailDir);
          }

          croppedImage.Save(thumbnailPath, ImageFormat.Png);
        }

        this.Invoke(() => {
          presetDeck.Assignments[buttonIndex] = new PresetDeckButton {
            PresetPath = pending.Preset.MaybeRelativePath,
            PresetDisplayName = pending.Preset.DisplayName,
            ThumbnailPath = $"resources\\buttons\\btn-{buttonIndex:D2}.png"
          };

          pendingThumbnailAssignments.Remove(buttonIndex);

          // Calculate local button index (1-15) from global index for UI update
          int localButtonIndex = ((buttonIndex - 1) % 15) + 1;
          int buttonBank = ((buttonIndex - 1) / 15) + 1;

          // Only update appearance if we're on the correct bank
          if (buttonBank == Settings.CurrentPresetBank) {
            UpdatePresetDeckButtonAppearance(localButtonIndex);
          }

          SavePresetDeck();

          SetStatusText($"Preset '{pending.Preset.DisplayName}' assigned to button {buttonIndex}");
        });

        // Delete the capture file after successful processing
        try {
          if (File.Exists(captureFilePath)) {
            File.Delete(captureFilePath);
          }
        } catch (Exception deleteEx) {
          Program.SaveErrorToFile(deleteEx, "Delete capture file");
        }

      } catch (Exception ex) {
        this.Invoke(() => {
          pendingThumbnailAssignments.Remove(buttonIndex);
          SetStatusText($"Failed to process thumbnail: {ex.Message}");
        });
      }
    }

    private Image CropTo16x9(Image source) {
      // Cap thumbnail resolution to avoid GDI+ out-of-memory on large captures
      const int maxWidth = 640;
      const int maxHeight = 360;

      int cropWidth = source.Width;
      int cropHeight = (int)(source.Width / 16.0 * 9.0);
      int yOffset = 0;

      if (Math.Abs(source.Height - cropHeight) >= 5 && source.Height > cropHeight) {
        yOffset = (source.Height - cropHeight) / 2;
      } else {
        cropHeight = source.Height;
      }

      float scale = Math.Min((float)maxWidth / cropWidth, (float)maxHeight / cropHeight);
      int destWidth = scale < 1.0f ? (int)(cropWidth * scale) : cropWidth;
      int destHeight = scale < 1.0f ? (int)(cropHeight * scale) : cropHeight;

      // For large sources, step down in two passes to avoid GDI+ allocating a scratch
      // buffer proportional to the full source size during HighQualityBicubic resampling.
      // Pass 1: fast Bilinear crop+scale to 2x final size. Pass 2: Bicubic to final size.
      if (cropWidth > maxWidth * 3 || cropHeight > maxHeight * 3) {
        int midWidth = destWidth * 2;
        int midHeight = destHeight * 2;
        using Bitmap mid = new Bitmap(midWidth, midHeight);
        using (Graphics gMid = Graphics.FromImage(mid)) {
          gMid.InterpolationMode = InterpolationMode.Bilinear;
          gMid.PixelOffsetMode = PixelOffsetMode.HighQuality;
          gMid.DrawImage(source,
            new Rectangle(0, 0, midWidth, midHeight),
            new Rectangle(0, yOffset, cropWidth, cropHeight),
            GraphicsUnit.Pixel);
        }
        Bitmap result = new Bitmap(destWidth, destHeight);
        using (Graphics g = Graphics.FromImage(result)) {
          g.InterpolationMode = InterpolationMode.HighQualityBicubic;
          g.PixelOffsetMode = PixelOffsetMode.HighQuality;
          g.DrawImage(mid, new Rectangle(0, 0, destWidth, destHeight));
        }
        return result;
      }

      Bitmap finalResult = new Bitmap(destWidth, destHeight);
      using (Graphics g = Graphics.FromImage(finalResult)) {
        g.InterpolationMode = InterpolationMode.HighQualityBicubic;
        g.PixelOffsetMode = PixelOffsetMode.HighQuality;
        g.DrawImage(source,
          new Rectangle(0, 0, destWidth, destHeight),
          new Rectangle(0, yOffset, cropWidth, cropHeight),
          GraphicsUnit.Pixel);
      }
      return finalResult;
    }

    private void UnassignPresetFromDeckButton(int buttonIndex) {
      if (!presetDeck.Assignments.ContainsKey(buttonIndex)) {
        return;
      }

      var assignment = presetDeck.Assignments[buttonIndex];
      string thumbnailPath = Path.Combine(BaseDir, assignment.ThumbnailPath);
      if (File.Exists(thumbnailPath)) {
        try {
          File.Delete(thumbnailPath);
        } catch (Exception ex) {
          Program.SaveErrorToFile(ex, "Delete thumbnail");
        }
      }

      presetDeck.Assignments.Remove(buttonIndex);

      // Calculate local button index (1-15) from global index for UI update
      int localButtonIndex = ((buttonIndex - 1) % 15) + 1;
      int buttonBank = ((buttonIndex - 1) / 15) + 1;

      // Only update appearance if we're on the correct bank
      if (buttonBank == Settings.CurrentPresetBank) {
        UpdatePresetDeckButtonAppearance(localButtonIndex);
      }

      SavePresetDeck();

      SetStatusText($"Preset unassigned from button {buttonIndex}");
    }

    #region Video Input Mixing

    private void UpdateInputMixControlsEnabled() {
      bool active = chkVideoMix.Checked || chkSpoutMix.Checked;
      chkInputTop.Enabled         = active;
      numInputMixOpacity.Enabled  = active;
      chkMixLumaActive.Enabled    = active;
      numLumaThreshold.Enabled    = active;
      numLumaSoftness.Enabled     = active;
    }

    private void PopulateVideoDevices() {
      try {
        DeviceManager.PopulateVideoDevices(cboVideoInput);
        chkVideoMix.Enabled = cboVideoInput.Enabled;
        if (!chkVideoMix.Enabled) chkVideoMix.Checked = false;
      } catch (Exception ex) {
        SetStatusText($"Error enumerating video devices: {ex.Message}");
      }
    }

    private void chkVideoMix_CheckedChanged(object sender, EventArgs e) {
      try {
        bool enabled = chkVideoMix.Checked;

        // Make Mix buttons mutually exclusive
        if (enabled && chkSpoutMix.Checked) {
          chkSpoutMix.Checked = false;
        }

        if (IsPipeConnected) {
          // Always send current mix settings (opacity and layer position) beforehand
          if (enabled) {
            int opacityInt = (int)numInputMixOpacity.Value;
            bool onTop = chkInputTop.Checked;
            _pipeClient!.SendSignal($"SET_INPUTMIX_OPACITY={opacityInt}");
            _pipeClient!.SendSignal($"SET_INPUTMIX_ONTOP={(onTop ? 1 : 0)}");

            // Send Luma Key settings
            bool lumaActive = chkMixLumaActive.Checked;
            int lumaThreshold = lumaActive ? (int)numLumaThreshold.Value : -1;
            int lumaSoftness = (int)numLumaSoftness.Value;
            _pipeClient!.SendSignal($"SET_INPUTMIX_LUMAKEY={lumaThreshold}|{lumaSoftness}");

            System.Threading.Thread.Sleep(50); // Small wait to ensure settings are applied
          }

          // Always send device index first (even if already sent) to ensure it's initialized
          if (enabled && cboVideoInput.SelectedIndex >= 0) {
            int deviceIndex = cboVideoInput.SelectedIndex;
            _pipeClient!.SendSignal($"SETVIDEODEVICE={deviceIndex}");
            System.Threading.Thread.Sleep(100); // Give device time to initialize
          }

          // Then enable/disable mixing
          _pipeClient!.SendSignal($"ENABLEVIDEOMIX={(enabled ? 1 : 0)}");

          if (enabled && cboVideoInput.SelectedIndex >= 0) {
            SetStatusText($"Video mixing enabled: {cboVideoInput.Text}");
          } else if (enabled) {
            SetStatusText("Video mixing enabled but no device selected");
          } else {
            SetStatusText("Video mixing disabled");
          }
        } else {
          SetStatusText(windowNotFound);
        }

        // Keep dropdown enabled so user can change device even when Mix is off
      } catch (Exception ex) {
        SetStatusText($"Error toggling video mix: {ex.Message}");
      }
      UpdateInputMixControlsEnabled();
    }

    private void cboVideoInput_SelectedIndexChanged(object sender, EventArgs e) {
      try {
        if (chkVideoMix.Checked && cboVideoInput.SelectedIndex >= 0) {
          if (IsPipeConnected) {
            int deviceIndex = cboVideoInput.SelectedIndex;
            _pipeClient!.SendSignal($"SETVIDEODEVICE={deviceIndex}");
            SetStatusText($"Video source changed: {cboVideoInput.Text}");
          } else {
            SetStatusText(windowNotFound);
          }
        }
      } catch (Exception ex) {
        SetStatusText($"Error changing video device: {ex.Message}");
      }
    }

    private void btnVideoInputScan_Click(object sender, EventArgs e) {
      PopulateVideoDevices();
      int realCount = cboVideoInput.Enabled ? cboVideoInput.Items.Count : 0;
      SetStatusText($"Video devices rescanned: {realCount} found");
    }

    private void PopulateSpoutSenders() {
      try {
        DeviceManager.PopulateSpoutSenders(cboSputInput);
        chkSpoutMix.Enabled = cboSputInput.Enabled;
        if (!chkSpoutMix.Enabled) chkSpoutMix.Checked = false;
      } catch (Exception ex) {
        SetStatusText($"Error enumerating Spout senders: {ex.Message}");
      }
    }

    private void chkSpoutMix_CheckedChanged(object sender, EventArgs e) {
      try {
        bool enabled = chkSpoutMix.Checked;

        // Make Mix buttons mutually exclusive
        if (enabled && chkVideoMix.Checked) {
          chkVideoMix.Checked = false;
        }

        if (IsPipeConnected) {
          // Always send current mix settings (opacity and layer position) beforehand
          if (enabled) {
            int opacityInt = (int)numInputMixOpacity.Value;
            bool onTop = chkInputTop.Checked;
            _pipeClient!.SendSignal($"SET_INPUTMIX_OPACITY={opacityInt}");
            _pipeClient!.SendSignal($"SET_INPUTMIX_ONTOP={(onTop ? 1 : 0)}");

            // Send Luma Key settings
            bool lumaActive = chkMixLumaActive.Checked;
            int lumaThreshold = lumaActive ? (int)numLumaThreshold.Value : -1;
            int lumaSoftness = (int)numLumaSoftness.Value;
            _pipeClient!.SendSignal($"SET_INPUTMIX_LUMAKEY={lumaThreshold}|{lumaSoftness}");

            System.Threading.Thread.Sleep(50); // Small wait to ensure settings are applied
          }

          // Send sender name and then enable/disable mixing
          if (enabled && cboSputInput.SelectedIndex >= 0) {
            string senderName = cboSputInput.Text;
            _pipeClient!.SendSpoutSender(senderName);
            System.Threading.Thread.Sleep(50);
          }

          _pipeClient!.SendSignal($"ENABLESPOUTMIX={(enabled ? 1 : 0)}");

          if (enabled && cboSputInput.SelectedIndex >= 0) {
            SetStatusText($"Spout mixing enabled: {cboSputInput.Text}");
          } else if (enabled) {
            SetStatusText("Spout mixing enabled but no sender selected");
          } else {
            SetStatusText("Spout mixing disabled");
          }
        } else {
          SetStatusText(windowNotFound);
        }
      } catch (Exception ex) {
        SetStatusText($"Error toggling Spout mix: {ex.Message}");
      }
      UpdateInputMixControlsEnabled();
    }

    private void cboSpoutInput_SelectedIndexChanged(object sender, EventArgs e) {
      try {
        if (updatingSettingsParams) return;
        if (chkSpoutMix.Checked && cboSputInput.SelectedIndex >= 0) {
          string senderName = cboSputInput.Text;
          if (senderName == lastSpoutSenderName) return; // Ignore if name hasn't changed (avoids noise from refresh timer)

          if (IsPipeConnected) {
            _pipeClient!.SendSpoutSender(senderName);
            SetStatusText($"Spout sender changed: {senderName}");
            lastSpoutSenderName = senderName;
          } else {
            SetStatusText(windowNotFound);
          }
        }
      } catch (Exception ex) {
        SetStatusText($"Error changing Spout sender: {ex.Message}");
      }
    }

    private void btnSpoutInputScan_Click(object sender, EventArgs e) {
      PopulateSpoutSenders();
      int realCount = cboSputInput.Enabled ? cboSputInput.Items.Count : 0;
      SetStatusText($"Spout senders rescanned: {realCount} found");
    }

    private void PopulateGameControllers() {
      try {
        DeviceManager.PopulateGameControllers(cboInputController);
        chkControllerActive.Enabled = cboInputController.Enabled;
        if (!chkControllerActive.Enabled) chkControllerActive.Checked = false;
      } catch (Exception ex) {
        SetStatusText($"Error enumerating game controllers: {ex.Message}");
      }
    }

    private void btnControllerInputScan_Click(object sender, EventArgs e) {
      PopulateGameControllers();
      int realCount = cboInputController.Enabled ? cboInputController.Items.Count : 0;
      SetStatusText($"Game controllers rescanned: {realCount} found");
    }

    private void cboInputController_SelectedIndexChanged(object sender, EventArgs e) {
      if (cboInputController.SelectedItem is DeviceEnumerator.DeviceItem item) {
        RemoteHelper.SetIniValue("Milkwave", "Controller", item.Name);
      }
    }

    private void chkControllerActive_CheckedChanged(object sender, EventArgs e) {
      if (chkControllerActive.Checked) {
        LoadControllerConfig();
        controllerTimer.Start();
      } else {
        controllerTimer.Stop();
      }
    }

    private void LoadControllerConfig() {
      try {
        string filePath = Path.Combine(BaseDir, milkwaveControllerFile);
        if (File.Exists(filePath)) {
          var lines = File.ReadAllLines(filePath).Where(l => !l.TrimStart().StartsWith("//"));
          string jsonString = string.Join(Environment.NewLine, lines);
          var config = JsonSerializer.Deserialize<Dictionary<string, string>>(jsonString);
          if (config != null) {
            controllerConfig = config.Where(kvp => !string.IsNullOrEmpty(kvp.Value))
                                     .ToDictionary(kvp => int.Parse(kvp.Key), kvp => kvp.Value);
          }
        }
      } catch (Exception ex) {
        SetStatusText($"Error loading controller config: {ex.Message}");
      }
    }

    private void ControllerTimer_Tick(object? sender, EventArgs? e) {
      try {
        if (cboInputController.SelectedItem is DeviceEnumerator.DeviceItem item &&
            !string.IsNullOrEmpty(item.DeviceID) &&
            int.TryParse(item.DeviceID, out int joyID)) {

          var status = DeviceEnumerator.GetJoystickStatus(joyID);
          uint buttons = status.dwButtons;

          for (int i = 1; i <= 32; i++) {
            uint mask = 1u << (i - 1);
            if ((buttons & mask) != 0 && (lastControllerButtons & mask) == 0) {
              // Button i just pressed
              Program.LogToFile($"[Controller] Button {i} pressed on '{item.Name}'");
              if (controllerConfig.TryGetValue(i, out string? command)) {
                Program.LogToFile($"[Controller] Executing command for button {i}: {command}");
                HandleScriptLine(true, command);
              }
            }
          }

          lastControllerButtons = buttons;
        }
      } catch (Exception ex) {
        Debug.WriteLine($"Error polling controller: {ex.Message}");
      }
    }

    #endregion

    private void btnControllerInputConfig_Click(object sender, EventArgs e) {
      OpenFile(milkwaveControllerFile);
    }

    private void btnMessagesEditorOpen_Click(object sender, EventArgs e) {
      // open the messages editor (messages-editor.html) in the default browser
      OpenFile(milkwaveMessagesEditorFile);
    }

    private void btnCacheCompile_Click(object sender, EventArgs e) {
      SendToMilkwaveVisualizer("", MessageType.PrecompileCache);
    }

    private void btnCacheClear_Click(object sender, EventArgs e) {
      try {
        string cacheDir = Path.Combine(BaseDir, "cache");
        if (Directory.Exists(cacheDir)) {
          string[] files = Directory.GetFiles(cacheDir);
          foreach (string file in files) {
            File.Delete(file);
          }
          SetStatusText($"Cleared {files.Length} files from cache directory");
        } else {
          SetStatusText("Cache directory not found or already empty");
        }
      } catch (Exception ex) {
        SetStatusText($"Error clearing cache: {ex.Message}");
      }
    }

    private void numLumaThreshold_Click(object sender, EventArgs e) {
      numLumaThreshold.Value = 20;
    }

    private void numLumaSoftness_Click(object sender, EventArgs e) {
      numLumaSoftness.Value = 20;
    }

    private void numInputMixOpacity_Click(object sender, EventArgs e) {
      numInputMixOpacity.Value = 100;
    }

    private void labelEQAttack_DoubleClick(object sender, EventArgs e) {
      numFFTAttack.Value = 0.5m;
    }

    private void labelEQDecay_DoubleClick(object sender, EventArgs e) {
      numFFTDecay.Value = 0.7m;
    }

    private void lblVisualizerOpacity_DoubleClick(object sender, EventArgs e) {
      numOpacity.Value = 100;
    }

  } // end class
} // end namespace