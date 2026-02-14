using MilkwaveRemote.Helper;
using System.Windows.Forms;

namespace MilkwaveRemote
{
    partial class MilkwaveRemoteForm
    {
        /// <summary>
        ///  Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        ///  Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            } 
            base.Dispose(disposing);
        }

    #region Windows Form Designer generated code

    /// <summary>
    ///  Required method for Designer support - do not modify
    ///  the contents of this method with the code editor.
    /// </summary>
    private void InitializeComponent() {
      components = new System.ComponentModel.Container();
      System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MilkwaveRemoteForm));
      statusStrip1 = new StatusStrip();
      statusBar = new ToolStripStatusLabel();
      toolStripStatusLabelMonitorCPU = new ToolStripStatusLabel();
      toolStripStatusLabelMonitorGPU = new ToolStripStatusLabel();
      toolStripDropDownButton = new ToolStripDropDownButton();
      toolStripMenuItemHomepage = new ToolStripMenuItem();
      toolStripSeparator1 = new ToolStripSeparator();
      toolStripMenuItemHelp = new ToolStripMenuItem();
      toolStripMenuItemSupporters = new ToolStripMenuItem();
      toolStripSeparator3 = new ToolStripSeparator();
      toolStripMenuItemTabsPanel = new ToolStripMenuItem();
      toolStripMenuItemButtonPanel = new ToolStripMenuItem();
      toolStripMenuItemSpriteButtonImages = new ToolStripMenuItem();
      toolStripMenuItemColorButtonImages = new ToolStripMenuItem();
      toolStripSeparator4 = new ToolStripSeparator();
      toolStripMenuItemMonitorCPU = new ToolStripMenuItem();
      toolStripMenuItemMonitorGPU = new ToolStripMenuItem();
      toolStripSeparator2 = new ToolStripSeparator();
      toolStripMenuItemDarkMode = new ToolStripMenuItem();
      toolStripMenuItemOpenVisualizer = new ToolStripMenuItem();
      toolTip1 = new ToolTip(components);
      chkPreview = new CheckBox();
      btnAppendSize = new Button();
      numSize = new NumericUpDown();
      lblSize = new Label();
      lblStyle = new Label();
      numBPM = new NumericUpDown();
      lblBPM = new Label();
      chkFileRandom = new CheckBox();
      pnlColorMessage = new Panel();
      btnAppendColor = new Button();
      lblColor = new Label();
      btnFontAppend = new Button();
      lblFont = new Label();
      cboFonts = new ComboBox();
      numBeats = new NumericUpDown();
      label7 = new Label();
      lblFromFile = new Label();
      chkAutoplay = new CheckBox();
      btnSaveParam = new Button();
      lblParameters = new Label();
      lblWindow = new Label();
      btnSend = new Button();
      txtMessage = new TextBox();
      lblPreset = new Label();
      btnPresetLoadFile = new Button();
      btnPresetSend = new Button();
      btnPresetLoadDirectory = new Button();
      txtVisRunning = new TextBox();
      cboPresets = new ComboBox();
      numAmpLeft = new NumericUpDown();
      lblAmp = new Label();
      numAmpRight = new NumericUpDown();
      lblCurrentPreset = new Label();
      chkAmpLinked = new CheckBox();
      chkWrap = new CheckBox();
      numWrap = new NumericUpDown();
      btnSetAudioDevice = new Button();
      cboAudioDevice = new ComboBox();
      numOpacity = new NumericUpDown();
      lblPercent = new Label();
      lblAudioDevice = new Label();
      btnSendFile = new Button();
      cboAutoplay = new ComboBox();
      chkPresetLink = new CheckBox();
      btnPresetLoadTags = new Button();
      txtTags = new TextBox();
      btnTagsSave = new Button();
      chkTagsFromRunning = new CheckBox();
      btnSendWave = new Button();
      pnlColorWave = new Panel();
      numWaveAlpha = new NumericUpDown();
      lblWavemode = new Label();
      chkPresetRandom = new CheckBox();
      lblLoad = new Label();
      lblTags = new Label();
      chkWaveLink = new CheckBox();
      numWaveR = new NumericUpDown();
      numWaveB = new NumericUpDown();
      numWaveG = new NumericUpDown();
      btnWaveClear = new Button();
      lblPushX = new Label();
      lblPushY = new Label();
      lblZoom = new Label();
      lblRotation = new Label();
      lblWarp = new Label();
      lblDecay = new Label();
      btnWaveQuicksave = new Button();
      cboWindowTitle = new ComboBox();
      chkWaveVolAlpha = new CheckBox();
      lblScale = new Label();
      lblEcho = new Label();
      cboTagsFilter = new ComboBox();
      lblMostUsed = new Label();
      pnlColorFont1 = new Panel();
      numFont1 = new NumericUpDown();
      chkFontAA1 = new CheckBox();
      btnSettingsSave = new Button();
      lblFont1 = new Label();
      btnSettingsLoad = new Button();
      chkFontAA2 = new CheckBox();
      lblFont2 = new Label();
      numFont2 = new NumericUpDown();
      pnlColorFont2 = new Panel();
      chkFontAA3 = new CheckBox();
      lblFont3 = new Label();
      numFont3 = new NumericUpDown();
      pnlColorFont3 = new Panel();
      chkFontAA4 = new CheckBox();
      lblFont4 = new Label();
      numFont4 = new NumericUpDown();
      pnlColorFont4 = new Panel();
      chkFontAA5 = new CheckBox();
      lblFont5 = new Label();
      numFont5 = new NumericUpDown();
      pnlColorFont5 = new Panel();
      cboFont1 = new ComboBox();
      cboFont5 = new ComboBox();
      cboFont4 = new ComboBox();
      cboFont3 = new ComboBox();
      cboFont2 = new ComboBox();
      btnSpace = new Button();
      btnBackspace = new Button();
      btnWatermark = new Button();
      lblFactorFrame = new Label();
      lblFactorTime = new Label();
      lblFactorFPS = new Label();
      btnOpenSettingsFile = new Button();
      txtFilterTags = new TextBox();
      btnSendShader = new Button();
      txtShaderinfo = new TextBox();
      btnShaderConvert = new Button();
      txtLineNumberError = new TextBox();
      txtShaderFind = new TextBox();
      btnLoadShaderInputFromFile = new Button();
      btnShaderHelp = new Button();
      numPSVersion = new NumericUpDown();
      numOffset = new NumericUpDown();
      chkShaderFile = new CheckBox();
      chkShaderLeft = new CheckBox();
      btnHLSLSave = new Button();
      btnHLSLLoad = new Button();
      lblVisIntensity = new Label();
      lblVisShift = new Label();
      lblVisVersion = new Label();
      btnMIDIHelp = new Button();
      txtMidi1Con = new TextBox();
      txtMidi1Val = new TextBox();
      txtMidi1Ch = new TextBox();
      txtMidi1Label = new TextBox();
      chkMidi1Active = new CheckBox();
      lblMidi1Inc = new Label();
      txtMidi3Label = new TextBox();
      txtMidi4Label = new TextBox();
      txtMidi5Label = new TextBox();
      btnMIDISave = new Button();
      btnMIDILoad = new Button();
      lblMidi1Action = new Label();
      lblSpout = new Label();
      chkSpoutActive = new CheckBox();
      chkSpoutFixedSize = new CheckBox();
      cboSpoutWidth = new ComboBox();
      cboSpoutHeight = new ComboBox();
      lblQuality = new Label();
      lblMidi1Row = new Label();
      lblBrightness = new Label();
      lblSaturation = new Label();
      lblHue = new Label();
      txtFilterPresets = new TextBox();
      chkQualityAuto = new CheckBox();
      btnShadertoyFilesLoadDir = new Button();
      txtShadertoyFile = new TextBox();
      btnShadertoyFileLoadNext = new Button();
      btnShadertoyFileLoadThis = new Button();
      numShadertoyFileIndex = new NumericUpDown();
      btnFontGlobalPlus = new Button();
      btnFontGlobalMinus = new Button();
      numSettingsHueAuto = new NumericUpDown();
      chkHueAuto = new CheckBox();
      lblChangePreset = new Label();
      chkPresetLocked = new CheckBox();
      chkVideoMix = new CheckBox();
      cboSettingsOpenFile = new ComboBox();
      btn00 = new Button();
      txtShaderGLSL = new TextBox();
      txtShaderHLSL = new TextBox();
      txtMidi2Label = new TextBox();
      lblMidi2Inc = new Label();
      chkMidi2Active = new CheckBox();
      txtMidi2Con = new TextBox();
      txtMidi2Val = new TextBox();
      txtMidi2Ch = new TextBox();
      numMidiBank = new NumericUpDown();
      numVisShift = new NumericUpDown();
      numVisIntensity = new NumericUpDown();
      chkMidi1Learn = new CheckBox();
      cboMidiDevice = new ComboBox();
      lblMidiDevice = new Label();
      btnMidiDeviceScan = new Button();
      numVisVersion = new NumericUpDown();
      cboParameters = new ComboBox();
      chkWaveBrighten = new CheckBox();
      chkWaveDarken = new CheckBox();
      chkWaveSolarize = new CheckBox();
      chkWaveInvert = new CheckBox();
      chkWaveAdditive = new CheckBox();
      chkWaveDotted = new CheckBox();
      chkWaveThick = new CheckBox();
      lblRGB = new Label();
      numWaveMode = new NumericUpDown();
      colorDialogMessage = new ColorDialog();
      label2 = new Label();
      txtStyle = new TextBox();
      tableLayoutPanel1 = new TableLayoutPanel();
      btnSwitchMode = new Button();
      btn88 = new Button();
      btnTransparency = new Button();
      btnB = new Button();
      btn77 = new Button();
      btn66 = new Button();
      btn55 = new Button();
      btn44 = new Button();
      btn33 = new Button();
      btn22 = new Button();
      btnK = new Button();
      btnF2 = new Button();
      btnN = new Button();
      btnAltEnter = new Button();
      btnF10 = new Button();
      btn11 = new Button();
      btnTilde = new Button();
      btnF7 = new Button();
      btnF4 = new Button();
      btnF3 = new Button();
      btnDelete = new Button();
      colorDialogWave = new ColorDialog();
      splitContainer1 = new SplitContainer();
      tabControl = new FlatTabControl();
      tabPreset = new TabPage();
      btnTag10 = new Button();
      btnTag9 = new Button();
      btnTag8 = new Button();
      btnTag7 = new Button();
      btnTag6 = new Button();
      btnTag5 = new Button();
      btnTag4 = new Button();
      btnTag2 = new Button();
      btnTag3 = new Button();
      btnTag1 = new Button();
      tabMessage = new TabPage();
      tabWave = new TabPage();
      numWaveEcho = new NumericUpDown();
      numWaveScale = new NumericUpDown();
      numWaveDecay = new NumericUpDown();
      numWaveWarp = new NumericUpDown();
      numWaveRotation = new NumericUpDown();
      numWaveZoom = new NumericUpDown();
      numWavePushY = new NumericUpDown();
      numWavePushX = new NumericUpDown();
      tabInput = new TabPage();
      cboVideoInput = new ComboBox();
      label10 = new Label();
      tabMidi = new TabPage();
      label6 = new Label();
      label8 = new Label();
      cboMidi5Action = new ComboBox();
      chkMidi5Active = new CheckBox();
      txtMidi5Inc = new TextBox();
      txtMidi5Con = new TextBox();
      txtMidi5Val = new TextBox();
      txtMidi5Ch = new TextBox();
      chkMidi5Learn = new CheckBox();
      label4 = new Label();
      label5 = new Label();
      cboMidi4Action = new ComboBox();
      chkMidi4Active = new CheckBox();
      txtMidi4Inc = new TextBox();
      txtMidi4Con = new TextBox();
      txtMidi4Val = new TextBox();
      txtMidi4Ch = new TextBox();
      chkMidi4Learn = new CheckBox();
      label1 = new Label();
      label3 = new Label();
      cboMidi3Action = new ComboBox();
      chkMidi3Active = new CheckBox();
      txtMidi3Inc = new TextBox();
      txtMidi3Con = new TextBox();
      txtMidi3Val = new TextBox();
      txtMidi3Ch = new TextBox();
      chkMidi3Learn = new CheckBox();
      lblMidi2Action = new Label();
      cboMidi2Action = new ComboBox();
      txtMidi2Inc = new TextBox();
      chkMidi2Learn = new CheckBox();
      lblMidi5Row = new Label();
      lblMidi4Row = new Label();
      lblMidi3Row = new Label();
      lblMidi2Row = new Label();
      lblBank = new Label();
      cboMidi1Action = new ComboBox();
      txtMidi1Inc = new TextBox();
      tabSettings = new TabPage();
      numPresetChange = new NumericUpDown();
      label9 = new Label();
      numSettingsBrightness = new NumericUpDown();
      numSettingsSaturation = new NumericUpDown();
      numSettingsHue = new NumericUpDown();
      btnQualityDouble = new Button();
      btnQualityHalf = new Button();
      numQuality = new NumericUpDown();
      numFactorFPS = new NumericUpDown();
      numFactorFrame = new NumericUpDown();
      numFactorTime = new NumericUpDown();
      tabFonts = new TabPage();
      btnTestFonts = new Button();
      chkFontItalic5 = new CheckBox();
      chkFontBold5 = new CheckBox();
      chkFontItalic4 = new CheckBox();
      chkFontBold4 = new CheckBox();
      chkFontItalic3 = new CheckBox();
      chkFontBold3 = new CheckBox();
      chkFontItalic2 = new CheckBox();
      chkFontBold2 = new CheckBox();
      chkFontItalic1 = new CheckBox();
      chkFontBold1 = new CheckBox();
      tabShader = new TabPage();
      pnlTabShader = new Panel();
      panShadertoyLocal = new Panel();
      picShaderError = new PictureBox();
      splitContainerShader = new SplitContainer();
      statusStrip1.SuspendLayout();
      ((System.ComponentModel.ISupportInitialize)numSize).BeginInit();
      ((System.ComponentModel.ISupportInitialize)numBPM).BeginInit();
      ((System.ComponentModel.ISupportInitialize)numBeats).BeginInit();
      ((System.ComponentModel.ISupportInitialize)numAmpLeft).BeginInit();
      ((System.ComponentModel.ISupportInitialize)numAmpRight).BeginInit();
      ((System.ComponentModel.ISupportInitialize)numWrap).BeginInit();
      ((System.ComponentModel.ISupportInitialize)numOpacity).BeginInit();
      ((System.ComponentModel.ISupportInitialize)numWaveAlpha).BeginInit();
      ((System.ComponentModel.ISupportInitialize)numWaveR).BeginInit();
      ((System.ComponentModel.ISupportInitialize)numWaveB).BeginInit();
      ((System.ComponentModel.ISupportInitialize)numWaveG).BeginInit();
      ((System.ComponentModel.ISupportInitialize)numFont1).BeginInit();
      ((System.ComponentModel.ISupportInitialize)numFont2).BeginInit();
      ((System.ComponentModel.ISupportInitialize)numFont3).BeginInit();
      ((System.ComponentModel.ISupportInitialize)numFont4).BeginInit();
      ((System.ComponentModel.ISupportInitialize)numFont5).BeginInit();
      ((System.ComponentModel.ISupportInitialize)numPSVersion).BeginInit();
      ((System.ComponentModel.ISupportInitialize)numOffset).BeginInit();
      ((System.ComponentModel.ISupportInitialize)numShadertoyFileIndex).BeginInit();
      ((System.ComponentModel.ISupportInitialize)numSettingsHueAuto).BeginInit();
      ((System.ComponentModel.ISupportInitialize)numMidiBank).BeginInit();
      ((System.ComponentModel.ISupportInitialize)numVisShift).BeginInit();
      ((System.ComponentModel.ISupportInitialize)numVisIntensity).BeginInit();
      ((System.ComponentModel.ISupportInitialize)numVisVersion).BeginInit();
      ((System.ComponentModel.ISupportInitialize)numWaveMode).BeginInit();
      tableLayoutPanel1.SuspendLayout();
      ((System.ComponentModel.ISupportInitialize)splitContainer1).BeginInit();
      splitContainer1.Panel1.SuspendLayout();
      splitContainer1.Panel2.SuspendLayout();
      splitContainer1.SuspendLayout();
      tabControl.SuspendLayout();
      tabPreset.SuspendLayout();
      tabMessage.SuspendLayout();
      tabWave.SuspendLayout();
      ((System.ComponentModel.ISupportInitialize)numWaveEcho).BeginInit();
      ((System.ComponentModel.ISupportInitialize)numWaveScale).BeginInit();
      ((System.ComponentModel.ISupportInitialize)numWaveDecay).BeginInit();
      ((System.ComponentModel.ISupportInitialize)numWaveWarp).BeginInit();
      ((System.ComponentModel.ISupportInitialize)numWaveRotation).BeginInit();
      ((System.ComponentModel.ISupportInitialize)numWaveZoom).BeginInit();
      ((System.ComponentModel.ISupportInitialize)numWavePushY).BeginInit();
      ((System.ComponentModel.ISupportInitialize)numWavePushX).BeginInit();
      tabInput.SuspendLayout();
      tabMidi.SuspendLayout();
      tabSettings.SuspendLayout();
      ((System.ComponentModel.ISupportInitialize)numPresetChange).BeginInit();
      ((System.ComponentModel.ISupportInitialize)numSettingsBrightness).BeginInit();
      ((System.ComponentModel.ISupportInitialize)numSettingsSaturation).BeginInit();
      ((System.ComponentModel.ISupportInitialize)numSettingsHue).BeginInit();
      ((System.ComponentModel.ISupportInitialize)numQuality).BeginInit();
      ((System.ComponentModel.ISupportInitialize)numFactorFPS).BeginInit();
      ((System.ComponentModel.ISupportInitialize)numFactorFrame).BeginInit();
      ((System.ComponentModel.ISupportInitialize)numFactorTime).BeginInit();
      tabFonts.SuspendLayout();
      tabShader.SuspendLayout();
      pnlTabShader.SuspendLayout();
      panShadertoyLocal.SuspendLayout();
      ((System.ComponentModel.ISupportInitialize)picShaderError).BeginInit();
      ((System.ComponentModel.ISupportInitialize)splitContainerShader).BeginInit();
      splitContainerShader.Panel1.SuspendLayout();
      splitContainerShader.Panel2.SuspendLayout();
      splitContainerShader.SuspendLayout();
      SuspendLayout();
      // 
      // statusStrip1
      // 
      statusStrip1.ImageScalingSize = new Size(20, 20);
      statusStrip1.Items.AddRange(new ToolStripItem[] { statusBar, toolStripStatusLabelMonitorCPU, toolStripStatusLabelMonitorGPU, toolStripDropDownButton });
      statusStrip1.Location = new Point(0, 401);
      statusStrip1.Name = "statusStrip1";
      statusStrip1.Padding = new Padding(1, 0, 12, 0);
      statusStrip1.ShowItemToolTips = true;
      statusStrip1.Size = new Size(619, 26);
      statusStrip1.TabIndex = 5;
      statusStrip1.Text = "statusStrip1";
      // 
      // statusBar
      // 
      statusBar.AutoToolTip = true;
      statusBar.Margin = new Padding(7, 4, 0, 2);
      statusBar.Name = "statusBar";
      statusBar.Size = new Size(494, 20);
      statusBar.Spring = true;
      statusBar.TextAlign = ContentAlignment.TopLeft;
      statusBar.MouseDown += statusBar_MouseDown;
      // 
      // toolStripStatusLabelMonitorCPU
      // 
      toolStripStatusLabelMonitorCPU.Name = "toolStripStatusLabelMonitorCPU";
      toolStripStatusLabelMonitorCPU.Size = new Size(12, 21);
      toolStripStatusLabelMonitorCPU.Text = "-";
      toolStripStatusLabelMonitorCPU.ToolTipText = "CPU usage";
      // 
      // toolStripStatusLabelMonitorGPU
      // 
      toolStripStatusLabelMonitorGPU.Name = "toolStripStatusLabelMonitorGPU";
      toolStripStatusLabelMonitorGPU.Size = new Size(12, 21);
      toolStripStatusLabelMonitorGPU.Text = "-";
      toolStripStatusLabelMonitorGPU.ToolTipText = "GPU usage";
      // 
      // toolStripDropDownButton
      // 
      toolStripDropDownButton.DropDownItems.AddRange(new ToolStripItem[] { toolStripMenuItemHomepage, toolStripSeparator1, toolStripMenuItemHelp, toolStripMenuItemSupporters, toolStripSeparator3, toolStripMenuItemTabsPanel, toolStripMenuItemButtonPanel, toolStripMenuItemSpriteButtonImages, toolStripMenuItemColorButtonImages, toolStripSeparator4, toolStripMenuItemMonitorCPU, toolStripMenuItemMonitorGPU, toolStripSeparator2, toolStripMenuItemDarkMode, toolStripMenuItemOpenVisualizer });
      toolStripDropDownButton.Image = (Image)resources.GetObject("toolStripDropDownButton.Image");
      toolStripDropDownButton.ImageTransparentColor = Color.Magenta;
      toolStripDropDownButton.Name = "toolStripDropDownButton";
      toolStripDropDownButton.ShowDropDownArrow = false;
      toolStripDropDownButton.Size = new Size(81, 24);
      toolStripDropDownButton.Text = "Milkwave";
      // 
      // toolStripMenuItemHomepage
      // 
      toolStripMenuItemHomepage.Name = "toolStripMenuItemHomepage";
      toolStripMenuItemHomepage.Size = new Size(155, 22);
      toolStripMenuItemHomepage.Text = "Milkwave";
      toolStripMenuItemHomepage.Click += toolStripMenuItemReleases_Click;
      // 
      // toolStripSeparator1
      // 
      toolStripSeparator1.Name = "toolStripSeparator1";
      toolStripSeparator1.Size = new Size(152, 6);
      // 
      // toolStripMenuItemHelp
      // 
      toolStripMenuItemHelp.Name = "toolStripMenuItemHelp";
      toolStripMenuItemHelp.Size = new Size(155, 22);
      toolStripMenuItemHelp.Text = "Help";
      toolStripMenuItemHelp.Click += toolStripMenuItemHelp_Click;
      // 
      // toolStripMenuItemSupporters
      // 
      toolStripMenuItemSupporters.Name = "toolStripMenuItemSupporters";
      toolStripMenuItemSupporters.Size = new Size(155, 22);
      toolStripMenuItemSupporters.Text = "Supporters";
      toolStripMenuItemSupporters.Click += toolStripMenuItemSupporters_Click;
      // 
      // toolStripSeparator3
      // 
      toolStripSeparator3.Name = "toolStripSeparator3";
      toolStripSeparator3.Size = new Size(152, 6);
      // 
      // toolStripMenuItemTabsPanel
      // 
      toolStripMenuItemTabsPanel.Checked = true;
      toolStripMenuItemTabsPanel.CheckState = CheckState.Checked;
      toolStripMenuItemTabsPanel.Name = "toolStripMenuItemTabsPanel";
      toolStripMenuItemTabsPanel.Size = new Size(155, 22);
      toolStripMenuItemTabsPanel.Text = "Tabs Panel";
      toolStripMenuItemTabsPanel.Click += toolStripMenuItemTabsPanel_Click;
      // 
      // toolStripMenuItemButtonPanel
      // 
      toolStripMenuItemButtonPanel.Checked = true;
      toolStripMenuItemButtonPanel.CheckState = CheckState.Checked;
      toolStripMenuItemButtonPanel.Name = "toolStripMenuItemButtonPanel";
      toolStripMenuItemButtonPanel.Size = new Size(155, 22);
      toolStripMenuItemButtonPanel.Text = "Button Panel";
      toolStripMenuItemButtonPanel.ToolTipText = "Ctrl+B";
      toolStripMenuItemButtonPanel.Click += toolStripMenuItemButtonPanel_Click;
      // 
      // toolStripMenuItemSpriteButtonImages
      // 
      toolStripMenuItemSpriteButtonImages.Checked = true;
      toolStripMenuItemSpriteButtonImages.CheckState = CheckState.Checked;
      toolStripMenuItemSpriteButtonImages.Name = "toolStripMenuItemSpriteButtonImages";
      toolStripMenuItemSpriteButtonImages.Size = new Size(155, 22);
      toolStripMenuItemSpriteButtonImages.Text = "Sprite Images";
      toolStripMenuItemSpriteButtonImages.Click += toolStripMenuItemSpriteButtonImages_Click;
      // 
      // toolStripMenuItemColorButtonImages
      // 
      toolStripMenuItemColorButtonImages.Checked = true;
      toolStripMenuItemColorButtonImages.CheckState = CheckState.Checked;
      toolStripMenuItemColorButtonImages.Name = "toolStripMenuItemColorButtonImages";
      toolStripMenuItemColorButtonImages.Size = new Size(155, 22);
      toolStripMenuItemColorButtonImages.Text = "Color Images";
      toolStripMenuItemColorButtonImages.Click += toolStripMenuItemColorButtonImages_Click;
      // 
      // toolStripSeparator4
      // 
      toolStripSeparator4.Name = "toolStripSeparator4";
      toolStripSeparator4.Size = new Size(152, 6);
      // 
      // toolStripMenuItemMonitorCPU
      // 
      toolStripMenuItemMonitorCPU.Checked = true;
      toolStripMenuItemMonitorCPU.CheckState = CheckState.Checked;
      toolStripMenuItemMonitorCPU.Name = "toolStripMenuItemMonitorCPU";
      toolStripMenuItemMonitorCPU.Size = new Size(155, 22);
      toolStripMenuItemMonitorCPU.Text = "CPU Monitor";
      toolStripMenuItemMonitorCPU.Click += toolStripMenuItemMonitorCPU_Click;
      // 
      // toolStripMenuItemMonitorGPU
      // 
      toolStripMenuItemMonitorGPU.Checked = true;
      toolStripMenuItemMonitorGPU.CheckState = CheckState.Checked;
      toolStripMenuItemMonitorGPU.Name = "toolStripMenuItemMonitorGPU";
      toolStripMenuItemMonitorGPU.Size = new Size(155, 22);
      toolStripMenuItemMonitorGPU.Text = "GPU Monitor";
      toolStripMenuItemMonitorGPU.Click += toolStripMenuItemMonitorGPU_Click;
      // 
      // toolStripSeparator2
      // 
      toolStripSeparator2.Name = "toolStripSeparator2";
      toolStripSeparator2.Size = new Size(152, 6);
      // 
      // toolStripMenuItemDarkMode
      // 
      toolStripMenuItemDarkMode.Checked = true;
      toolStripMenuItemDarkMode.CheckState = CheckState.Checked;
      toolStripMenuItemDarkMode.Name = "toolStripMenuItemDarkMode";
      toolStripMenuItemDarkMode.Size = new Size(155, 22);
      toolStripMenuItemDarkMode.Text = "Dark Mode";
      toolStripMenuItemDarkMode.Click += toolStripMenuItemDarkMode_Click;
      // 
      // toolStripMenuItemOpenVisualizer
      // 
      toolStripMenuItemOpenVisualizer.Name = "toolStripMenuItemOpenVisualizer";
      toolStripMenuItemOpenVisualizer.Size = new Size(155, 22);
      toolStripMenuItemOpenVisualizer.Text = "Open Visualizer";
      toolStripMenuItemOpenVisualizer.ToolTipText = "Ctrl+O";
      toolStripMenuItemOpenVisualizer.Click += toolStripMenuItemOpenVisualizer_Click;
      // 
      // chkPreview
      // 
      chkPreview.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      chkPreview.Appearance = Appearance.Button;
      chkPreview.Checked = true;
      chkPreview.CheckState = CheckState.Checked;
      chkPreview.FlatStyle = FlatStyle.System;
      chkPreview.Location = new Point(518, 66);
      chkPreview.Margin = new Padding(3, 2, 3, 2);
      chkPreview.Name = "chkPreview";
      chkPreview.Size = new Size(83, 23);
      chkPreview.TabIndex = 6;
      chkPreview.Text = "Preview";
      chkPreview.TextAlign = ContentAlignment.MiddleCenter;
      chkPreview.TextImageRelation = TextImageRelation.ImageAboveText;
      toolTip1.SetToolTip(chkPreview, "Apply style to message box");
      chkPreview.UseVisualStyleBackColor = true;
      chkPreview.CheckedChanged += chkPreview_CheckedChanged;
      // 
      // btnAppendSize
      // 
      btnAppendSize.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      btnAppendSize.Location = new Point(329, 124);
      btnAppendSize.Margin = new Padding(0);
      btnAppendSize.Name = "btnAppendSize";
      btnAppendSize.Size = new Size(46, 22);
      btnAppendSize.TabIndex = 13;
      btnAppendSize.Text = "Set";
      toolTip1.SetToolTip(btnAppendSize, "Append to (or update in) parameters line");
      btnAppendSize.UseVisualStyleBackColor = true;
      btnAppendSize.Click += btnAppendSize_Click;
      // 
      // numSize
      // 
      numSize.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      numSize.Location = new Point(277, 124);
      numSize.Margin = new Padding(3, 2, 3, 2);
      numSize.Maximum = new decimal(new int[] { 999, 0, 0, 0 });
      numSize.Name = "numSize";
      numSize.Size = new Size(46, 23);
      numSize.TabIndex = 12;
      numSize.TextAlign = HorizontalAlignment.Center;
      toolTip1.SetToolTip(numSize, "Font size");
      numSize.Value = new decimal(new int[] { 25, 0, 0, 0 });
      numSize.TextChanged += txtSize_TextChanged;
      numSize.KeyDown += txtSize_KeyDown;
      // 
      // lblSize
      // 
      lblSize.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      lblSize.Location = new Point(244, 123);
      lblSize.Name = "lblSize";
      lblSize.Size = new Size(33, 23);
      lblSize.TabIndex = 98;
      lblSize.Text = "Size";
      lblSize.TextAlign = ContentAlignment.MiddleRight;
      toolTip1.SetToolTip(lblSize, "Only used if no size parameter supplied\r\nDouble-click to clear the size value from the parameters line\r\n");
      lblSize.DoubleClick += lblSize_DoubleClick;
      // 
      // lblStyle
      // 
      lblStyle.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      lblStyle.Location = new Point(378, 94);
      lblStyle.Name = "lblStyle";
      lblStyle.Size = new Size(41, 23);
      lblStyle.TabIndex = 75;
      lblStyle.Text = "Style";
      lblStyle.TextAlign = ContentAlignment.MiddleRight;
      toolTip1.SetToolTip(lblStyle, "Double-click to remove this style\r\nRight-click to fill frm elements from current parameters");
      lblStyle.DoubleClick += lblStyle_DoubleClick;
      lblStyle.MouseClick += lblStyle_MouseClick;
      // 
      // numBPM
      // 
      numBPM.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      numBPM.DecimalPlaces = 1;
      numBPM.Increment = new decimal(new int[] { 1, 0, 0, 65536 });
      numBPM.Location = new Point(318, 153);
      numBPM.Margin = new Padding(3, 2, 3, 2);
      numBPM.Maximum = new decimal(new int[] { 999, 0, 0, 0 });
      numBPM.Name = "numBPM";
      numBPM.Size = new Size(57, 23);
      numBPM.TabIndex = 18;
      numBPM.TextAlign = HorizontalAlignment.Center;
      toolTip1.SetToolTip(numBPM, "BPM");
      numBPM.Value = new decimal(new int[] { 120, 0, 0, 0 });
      numBPM.TextChanged += txtBPM_TextChanged;
      // 
      // lblBPM
      // 
      lblBPM.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      lblBPM.Location = new Point(278, 153);
      lblBPM.Name = "lblBPM";
      lblBPM.Size = new Size(38, 23);
      lblBPM.TabIndex = 97;
      lblBPM.Text = "BPM";
      lblBPM.TextAlign = ContentAlignment.MiddleRight;
      toolTip1.SetToolTip(lblBPM, "Click to reset timer");
      lblBPM.Click += lblBPM_Click;
      // 
      // chkFileRandom
      // 
      chkFileRandom.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      chkFileRandom.Appearance = Appearance.Button;
      chkFileRandom.FlatStyle = FlatStyle.System;
      chkFileRandom.Location = new Point(465, 152);
      chkFileRandom.Margin = new Padding(3, 2, 3, 2);
      chkFileRandom.Name = "chkFileRandom";
      chkFileRandom.Size = new Size(47, 23);
      chkFileRandom.TabIndex = 20;
      chkFileRandom.Text = "Rand";
      chkFileRandom.TextAlign = ContentAlignment.MiddleCenter;
      chkFileRandom.TextImageRelation = TextImageRelation.ImageAboveText;
      toolTip1.SetToolTip(chkFileRandom, "Play random line");
      chkFileRandom.UseVisualStyleBackColor = true;
      // 
      // pnlColorMessage
      // 
      pnlColorMessage.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      pnlColorMessage.BorderStyle = BorderStyle.FixedSingle;
      pnlColorMessage.Location = new Point(421, 124);
      pnlColorMessage.Name = "pnlColorMessage";
      pnlColorMessage.Size = new Size(38, 23);
      pnlColorMessage.TabIndex = 14;
      toolTip1.SetToolTip(pnlColorMessage, "Only used if no r, g, b parameters supplied");
      pnlColorMessage.Click += pnlColorMessage_Click;
      // 
      // btnAppendColor
      // 
      btnAppendColor.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      btnAppendColor.Location = new Point(465, 124);
      btnAppendColor.Margin = new Padding(0);
      btnAppendColor.Name = "btnAppendColor";
      btnAppendColor.Size = new Size(47, 22);
      btnAppendColor.TabIndex = 15;
      btnAppendColor.Text = "Set";
      toolTip1.SetToolTip(btnAppendColor, "Append to (or update in) parameters line");
      btnAppendColor.UseVisualStyleBackColor = true;
      btnAppendColor.Click += btnAppendColor_Click;
      // 
      // lblColor
      // 
      lblColor.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      lblColor.Location = new Point(380, 124);
      lblColor.Name = "lblColor";
      lblColor.Size = new Size(39, 23);
      lblColor.TabIndex = 96;
      lblColor.Text = "Color";
      lblColor.TextAlign = ContentAlignment.MiddleRight;
      toolTip1.SetToolTip(lblColor, "Only used if no r, g, b parameters supplied\r\nDouble-click to clear the color information from the parameters line\r\n");
      lblColor.DoubleClick += lblColor_DoubleClick;
      // 
      // btnFontAppend
      // 
      btnFontAppend.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      btnFontAppend.Location = new Point(192, 124);
      btnFontAppend.Margin = new Padding(0);
      btnFontAppend.Name = "btnFontAppend";
      btnFontAppend.Size = new Size(49, 22);
      btnFontAppend.TabIndex = 11;
      btnFontAppend.Text = "Set";
      toolTip1.SetToolTip(btnFontAppend, "Append to (or update in) parameters line");
      btnFontAppend.UseVisualStyleBackColor = true;
      btnFontAppend.Click += btnFontAppend_Click;
      // 
      // lblFont
      // 
      lblFont.Location = new Point(4, 124);
      lblFont.Name = "lblFont";
      lblFont.Size = new Size(66, 23);
      lblFont.TabIndex = 95;
      lblFont.Text = "Font";
      lblFont.TextAlign = ContentAlignment.MiddleRight;
      toolTip1.SetToolTip(lblFont, "Only used if no font parameter supplied\r\nDouble-click to clear the font value from the parameters line\r\n");
      lblFont.DoubleClick += lblFont_DoubleClick;
      // 
      // cboFonts
      // 
      cboFonts.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
      cboFonts.AutoCompleteMode = AutoCompleteMode.SuggestAppend;
      cboFonts.AutoCompleteSource = AutoCompleteSource.ListItems;
      cboFonts.DropDownStyle = ComboBoxStyle.DropDownList;
      cboFonts.FormattingEnabled = true;
      cboFonts.Location = new Point(74, 124);
      cboFonts.Name = "cboFonts";
      cboFonts.Size = new Size(115, 23);
      cboFonts.TabIndex = 10;
      toolTip1.SetToolTip(cboFonts, "Only used if no font parameter supplied");
      cboFonts.SelectedIndexChanged += cboFonts_SelectedIndexChanged;
      cboFonts.KeyDown += cboFonts_KeyDown;
      // 
      // numBeats
      // 
      numBeats.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      numBeats.Location = new Point(421, 153);
      numBeats.Margin = new Padding(3, 2, 3, 2);
      numBeats.Maximum = new decimal(new int[] { 99, 0, 0, 0 });
      numBeats.Name = "numBeats";
      numBeats.Size = new Size(38, 23);
      numBeats.TabIndex = 19;
      numBeats.TextAlign = HorizontalAlignment.Center;
      toolTip1.SetToolTip(numBeats, "Wait time between lines in beats");
      numBeats.Value = new decimal(new int[] { 8, 0, 0, 0 });
      // 
      // label7
      // 
      label7.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      label7.Location = new Point(380, 153);
      label7.Name = "label7";
      label7.Size = new Size(39, 23);
      label7.TabIndex = 94;
      label7.Text = "Beats";
      label7.TextAlign = ContentAlignment.MiddleRight;
      toolTip1.SetToolTip(label7, "Wait time between lines in beats");
      // 
      // lblFromFile
      // 
      lblFromFile.Location = new Point(4, 152);
      lblFromFile.Name = "lblFromFile";
      lblFromFile.Size = new Size(66, 23);
      lblFromFile.TabIndex = 93;
      lblFromFile.Text = "From File";
      lblFromFile.TextAlign = ContentAlignment.MiddleRight;
      toolTip1.SetToolTip(lblFromFile, "Double-click to reload file\r\nRight-click to load custom file");
      lblFromFile.DoubleClick += lblFromFile_DoubleClick;
      lblFromFile.MouseClick += lblFromFile_MouseClick;
      // 
      // chkAutoplay
      // 
      chkAutoplay.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      chkAutoplay.Appearance = Appearance.Button;
      chkAutoplay.FlatStyle = FlatStyle.System;
      chkAutoplay.Location = new Point(518, 124);
      chkAutoplay.Name = "chkAutoplay";
      chkAutoplay.Size = new Size(83, 23);
      chkAutoplay.TabIndex = 16;
      chkAutoplay.Text = "Autoplay";
      chkAutoplay.TextAlign = ContentAlignment.MiddleCenter;
      chkAutoplay.TextImageRelation = TextImageRelation.ImageAboveText;
      toolTip1.SetToolTip(chkAutoplay, "Autoplay on/off (Ctrl+Y)\r\nHold Alt: Reload script file");
      chkAutoplay.UseVisualStyleBackColor = true;
      chkAutoplay.CheckedChanged += chkAutoplay_CheckedChanged;
      // 
      // btnSaveParam
      // 
      btnSaveParam.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      btnSaveParam.FlatStyle = FlatStyle.System;
      btnSaveParam.Location = new Point(518, 95);
      btnSaveParam.Name = "btnSaveParam";
      btnSaveParam.Size = new Size(83, 22);
      btnSaveParam.TabIndex = 9;
      btnSaveParam.Text = "Save";
      toolTip1.SetToolTip(btnSaveParam, "Save current parameters as style");
      btnSaveParam.UseVisualStyleBackColor = true;
      btnSaveParam.Click += btnSaveParam_Click;
      // 
      // lblParameters
      // 
      lblParameters.Location = new Point(4, 94);
      lblParameters.Name = "lblParameters";
      lblParameters.Size = new Size(66, 23);
      lblParameters.TabIndex = 91;
      lblParameters.Text = "Parameters";
      lblParameters.TextAlign = ContentAlignment.MiddleRight;
      toolTip1.SetToolTip(lblParameters, "Double-click: Clear all saved styles\r\nRight-click: Open window showing all possible parameters");
      lblParameters.DoubleClick += lblParameters_DoubleClick;
      lblParameters.MouseDown += lblParameters_MouseDown;
      // 
      // lblWindow
      // 
      lblWindow.Location = new Point(4, 67);
      lblWindow.Name = "lblWindow";
      lblWindow.Size = new Size(66, 23);
      lblWindow.TabIndex = 89;
      lblWindow.Text = "Window";
      lblWindow.TextAlign = ContentAlignment.MiddleRight;
      toolTip1.SetToolTip(lblWindow, "Double-click: Start Visualizer if no window found\r\nCtrl+F2: Reset window\r\n");
      lblWindow.DoubleClick += lblWindow_DoubleClick;
      // 
      // btnSend
      // 
      btnSend.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      btnSend.Location = new Point(518, 6);
      btnSend.Name = "btnSend";
      btnSend.Size = new Size(83, 53);
      btnSend.TabIndex = 1;
      btnSend.Text = "Send";
      toolTip1.SetToolTip(btnSend, "Send to Visualizer \r\n(Ctrl+S)");
      btnSend.UseVisualStyleBackColor = true;
      btnSend.Click += btnSend_Click;
      // 
      // txtMessage
      // 
      txtMessage.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
      txtMessage.Location = new Point(74, 7);
      txtMessage.Multiline = true;
      txtMessage.Name = "txtMessage";
      txtMessage.Size = new Size(438, 54);
      txtMessage.TabIndex = 0;
      txtMessage.Text = "Hi from Milkwave Remote!";
      toolTip1.SetToolTip(txtMessage, "Ctrl+A: Select all text\r\nEnter: Send to Visualizer\r\nShift+Enter: line break (or use // in message text)");
      txtMessage.KeyDown += txtMessage_KeyDown;
      // 
      // lblPreset
      // 
      lblPreset.Location = new Point(1, 6);
      lblPreset.Name = "lblPreset";
      lblPreset.Size = new Size(67, 24);
      lblPreset.TabIndex = 98;
      lblPreset.Text = "Preset";
      lblPreset.TextAlign = ContentAlignment.MiddleRight;
      toolTip1.SetToolTip(lblPreset, "Click: Copy full path to clipboard\r\nDouble-click: Clear all items\r\nCtrl+Click: Open file in editor");
      lblPreset.Click += lblPreset_Click;
      lblPreset.DoubleClick += lblPreset_DoubleClick;
      // 
      // btnPresetLoadFile
      // 
      btnPresetLoadFile.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      btnPresetLoadFile.FlatStyle = FlatStyle.System;
      btnPresetLoadFile.Location = new Point(456, 65);
      btnPresetLoadFile.Name = "btnPresetLoadFile";
      btnPresetLoadFile.Size = new Size(70, 23);
      btnPresetLoadFile.TabIndex = 23;
      btnPresetLoadFile.Text = "File";
      toolTip1.SetToolTip(btnPresetLoadFile, "Load a single preset");
      btnPresetLoadFile.UseVisualStyleBackColor = true;
      btnPresetLoadFile.Click += btnPresetLoadFile_Click;
      // 
      // btnPresetSend
      // 
      btnPresetSend.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      btnPresetSend.FlatStyle = FlatStyle.System;
      btnPresetSend.Location = new Point(532, 7);
      btnPresetSend.Name = "btnPresetSend";
      btnPresetSend.Size = new Size(70, 52);
      btnPresetSend.TabIndex = 25;
      btnPresetSend.Text = "Send";
      toolTip1.SetToolTip(btnPresetSend, "Send to Visualizer (Ctrl+P)\r\nRight-click: Send next to Visualizer (Ctrl+N)\r\nMiddle-click: Send previous to Visualizer");
      btnPresetSend.UseVisualStyleBackColor = true;
      btnPresetSend.Click += btnPresetSend_Click;
      btnPresetSend.MouseDown += btnPresetSend_MouseDown;
      // 
      // btnPresetLoadDirectory
      // 
      btnPresetLoadDirectory.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      btnPresetLoadDirectory.FlatStyle = FlatStyle.System;
      btnPresetLoadDirectory.Location = new Point(532, 65);
      btnPresetLoadDirectory.Name = "btnPresetLoadDirectory";
      btnPresetLoadDirectory.Size = new Size(70, 23);
      btnPresetLoadDirectory.TabIndex = 24;
      btnPresetLoadDirectory.Text = "Dir";
      toolTip1.SetToolTip(btnPresetLoadDirectory, "Load presets from a directory (Ctrl+D)\r\n\r\n");
      btnPresetLoadDirectory.UseVisualStyleBackColor = true;
      btnPresetLoadDirectory.Click += btnPresetLoadDirectory_Click;
      // 
      // txtVisRunning
      // 
      txtVisRunning.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
      txtVisRunning.Location = new Point(71, 36);
      txtVisRunning.Name = "txtVisRunning";
      txtVisRunning.ReadOnly = true;
      txtVisRunning.Size = new Size(379, 23);
      txtVisRunning.TabIndex = 96;
      toolTip1.SetToolTip(txtVisRunning, "Currently running Visualizer preset");
      // 
      // cboPresets
      // 
      cboPresets.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
      cboPresets.AutoCompleteMode = AutoCompleteMode.SuggestAppend;
      cboPresets.AutoCompleteSource = AutoCompleteSource.ListItems;
      cboPresets.DropDownStyle = ComboBoxStyle.DropDownList;
      cboPresets.FormattingEnabled = true;
      cboPresets.Location = new Point(71, 7);
      cboPresets.Name = "cboPresets";
      cboPresets.Size = new Size(303, 23);
      cboPresets.TabIndex = 22;
      toolTip1.SetToolTip(cboPresets, "Alt+Mousewheel: Send to Visualizer");
      cboPresets.SelectedIndexChanged += cboPresets_SelectedIndexChanged;
      cboPresets.KeyDown += cboPresets_KeyDown;
      // 
      // numAmpLeft
      // 
      numAmpLeft.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      numAmpLeft.DecimalPlaces = 2;
      numAmpLeft.Increment = new decimal(new int[] { 1, 0, 0, 65536 });
      numAmpLeft.Location = new Point(416, 153);
      numAmpLeft.Maximum = new decimal(new int[] { 9999, 0, 0, 131072 });
      numAmpLeft.Name = "numAmpLeft";
      numAmpLeft.Size = new Size(52, 23);
      numAmpLeft.TabIndex = 27;
      numAmpLeft.TextAlign = HorizontalAlignment.Center;
      toolTip1.SetToolTip(numAmpLeft, "Amplification factor for left channel");
      numAmpLeft.Value = new decimal(new int[] { 10, 0, 0, 65536 });
      numAmpLeft.ValueChanged += numAmpLeft_ValueChanged;
      // 
      // lblAmp
      // 
      lblAmp.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      lblAmp.Location = new Point(378, 153);
      lblAmp.Name = "lblAmp";
      lblAmp.Size = new Size(37, 23);
      lblAmp.TabIndex = 104;
      lblAmp.Text = "Amp";
      lblAmp.TextAlign = ContentAlignment.MiddleRight;
      toolTip1.SetToolTip(lblAmp, "Double-click to reset");
      lblAmp.Click += lblAmpLeft_Click;
      // 
      // numAmpRight
      // 
      numAmpRight.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      numAmpRight.DecimalPlaces = 2;
      numAmpRight.Increment = new decimal(new int[] { 1, 0, 0, 65536 });
      numAmpRight.Location = new Point(474, 153);
      numAmpRight.Margin = new Padding(3, 2, 3, 2);
      numAmpRight.Maximum = new decimal(new int[] { 9999, 0, 0, 131072 });
      numAmpRight.Name = "numAmpRight";
      numAmpRight.Size = new Size(52, 23);
      numAmpRight.TabIndex = 28;
      numAmpRight.TextAlign = HorizontalAlignment.Center;
      toolTip1.SetToolTip(numAmpRight, "Amplification factor for right channel");
      numAmpRight.Value = new decimal(new int[] { 1, 0, 0, 0 });
      numAmpRight.ValueChanged += numAmpRight_ValueChanged;
      // 
      // lblCurrentPreset
      // 
      lblCurrentPreset.Location = new Point(1, 34);
      lblCurrentPreset.Name = "lblCurrentPreset";
      lblCurrentPreset.Size = new Size(67, 23);
      lblCurrentPreset.TabIndex = 95;
      lblCurrentPreset.Text = "Running";
      lblCurrentPreset.TextAlign = ContentAlignment.MiddleRight;
      toolTip1.SetToolTip(lblCurrentPreset, "Click: Copy full path to clipboard\r\nCtrl+Click: Open file in editor\r\n");
      lblCurrentPreset.Click += lblCurrentPreset_Click;
      // 
      // chkAmpLinked
      // 
      chkAmpLinked.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      chkAmpLinked.Appearance = Appearance.Button;
      chkAmpLinked.Checked = true;
      chkAmpLinked.CheckState = CheckState.Checked;
      chkAmpLinked.FlatStyle = FlatStyle.System;
      chkAmpLinked.Location = new Point(532, 152);
      chkAmpLinked.Margin = new Padding(3, 2, 3, 2);
      chkAmpLinked.Name = "chkAmpLinked";
      chkAmpLinked.Size = new Size(70, 23);
      chkAmpLinked.TabIndex = 29;
      chkAmpLinked.Text = "Link";
      chkAmpLinked.TextAlign = ContentAlignment.MiddleCenter;
      chkAmpLinked.TextImageRelation = TextImageRelation.ImageAboveText;
      toolTip1.SetToolTip(chkAmpLinked, "Link amp for both channels");
      chkAmpLinked.UseVisualStyleBackColor = true;
      // 
      // chkWrap
      // 
      chkWrap.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      chkWrap.Appearance = Appearance.Button;
      chkWrap.Checked = true;
      chkWrap.CheckState = CheckState.Checked;
      chkWrap.FlatStyle = FlatStyle.System;
      chkWrap.Location = new Point(465, 66);
      chkWrap.Margin = new Padding(3, 2, 3, 2);
      chkWrap.Name = "chkWrap";
      chkWrap.Size = new Size(46, 23);
      chkWrap.TabIndex = 5;
      chkWrap.Text = "Wrap";
      chkWrap.TextAlign = ContentAlignment.MiddleCenter;
      chkWrap.TextImageRelation = TextImageRelation.ImageAboveText;
      toolTip1.SetToolTip(chkWrap, "If the line is longer than this value, try to wrap it in the middle");
      chkWrap.UseVisualStyleBackColor = true;
      // 
      // numWrap
      // 
      numWrap.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      numWrap.Location = new Point(421, 67);
      numWrap.Margin = new Padding(3, 2, 3, 2);
      numWrap.Maximum = new decimal(new int[] { 99, 0, 0, 0 });
      numWrap.Minimum = new decimal(new int[] { 10, 0, 0, 0 });
      numWrap.Name = "numWrap";
      numWrap.Size = new Size(38, 23);
      numWrap.TabIndex = 4;
      numWrap.TextAlign = HorizontalAlignment.Center;
      toolTip1.SetToolTip(numWrap, "If the line is longer than this value, try to wrap it in the middle");
      numWrap.Value = new decimal(new int[] { 30, 0, 0, 0 });
      // 
      // btnSetAudioDevice
      // 
      btnSetAudioDevice.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      btnSetAudioDevice.Location = new Point(330, 152);
      btnSetAudioDevice.Name = "btnSetAudioDevice";
      btnSetAudioDevice.Size = new Size(46, 23);
      btnSetAudioDevice.TabIndex = 35;
      btnSetAudioDevice.Text = "Set";
      toolTip1.SetToolTip(btnSetAudioDevice, "Set this device as Visualizer audio source");
      btnSetAudioDevice.UseVisualStyleBackColor = true;
      btnSetAudioDevice.Click += btnSetAudioDevice_Click;
      // 
      // cboAudioDevice
      // 
      cboAudioDevice.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
      cboAudioDevice.DropDownStyle = ComboBoxStyle.DropDownList;
      cboAudioDevice.FormattingEnabled = true;
      cboAudioDevice.Location = new Point(71, 153);
      cboAudioDevice.Name = "cboAudioDevice";
      cboAudioDevice.Size = new Size(253, 23);
      cboAudioDevice.TabIndex = 34;
      toolTip1.SetToolTip(cboAudioDevice, "Alt+Mousewheel: Set in Visualizer");
      cboAudioDevice.SelectedIndexChanged += cboAudioDevice_SelectedIndexChanged;
      // 
      // numOpacity
      // 
      numOpacity.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      numOpacity.Increment = new decimal(new int[] { 2, 0, 0, 0 });
      numOpacity.Location = new Point(334, 67);
      numOpacity.Name = "numOpacity";
      numOpacity.Size = new Size(46, 23);
      numOpacity.TabIndex = 3;
      numOpacity.TextAlign = HorizontalAlignment.Center;
      toolTip1.SetToolTip(numOpacity, "Opacity");
      numOpacity.Value = new decimal(new int[] { 100, 0, 0, 0 });
      numOpacity.ValueChanged += numOpacity_ValueChanged;
      // 
      // lblPercent
      // 
      lblPercent.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      lblPercent.Location = new Point(381, 67);
      lblPercent.Name = "lblPercent";
      lblPercent.Size = new Size(23, 23);
      lblPercent.TabIndex = 105;
      lblPercent.Text = "%";
      lblPercent.TextAlign = ContentAlignment.MiddleLeft;
      toolTip1.SetToolTip(lblPercent, "Click to reset");
      lblPercent.Click += lblPercent_Click;
      // 
      // lblAudioDevice
      // 
      lblAudioDevice.Location = new Point(1, 153);
      lblAudioDevice.Name = "lblAudioDevice";
      lblAudioDevice.Size = new Size(67, 23);
      lblAudioDevice.TabIndex = 115;
      lblAudioDevice.Text = "Device";
      lblAudioDevice.TextAlign = ContentAlignment.MiddleRight;
      toolTip1.SetToolTip(lblAudioDevice, "Double-click to reload list\r\nRight-click to select and set default device\r\n");
      lblAudioDevice.DoubleClick += lblAudioDevice_DoubleClick;
      lblAudioDevice.MouseClick += lblAudioDevice_MouseClick;
      // 
      // btnSendFile
      // 
      btnSendFile.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      btnSendFile.Location = new Point(518, 153);
      btnSendFile.Name = "btnSendFile";
      btnSendFile.Size = new Size(83, 22);
      btnSendFile.TabIndex = 21;
      btnSendFile.Text = "Send";
      toolTip1.SetToolTip(btnSendFile, "Send to Visualizer \r\n(Ctrl+X)\r\nRight-click: Send to Visualizer and select next\r\n(Shift+Ctrl+X)\r\n\r\n");
      btnSendFile.UseVisualStyleBackColor = true;
      btnSendFile.Click += btnSendFile_Click;
      btnSendFile.MouseDown += btnSendFile_MouseDown;
      // 
      // cboAutoplay
      // 
      cboAutoplay.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
      cboAutoplay.AutoCompleteMode = AutoCompleteMode.SuggestAppend;
      cboAutoplay.Location = new Point(74, 153);
      cboAutoplay.Name = "cboAutoplay";
      cboAutoplay.Size = new Size(203, 23);
      cboAutoplay.TabIndex = 107;
      toolTip1.SetToolTip(cboAutoplay, "From file");
      cboAutoplay.SelectedIndexChanged += cboAutoplay_SelectedIndexChanged;
      // 
      // chkPresetLink
      // 
      chkPresetLink.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      chkPresetLink.Appearance = Appearance.Button;
      chkPresetLink.FlatStyle = FlatStyle.System;
      chkPresetLink.Location = new Point(456, 6);
      chkPresetLink.Margin = new Padding(3, 2, 3, 2);
      chkPresetLink.Name = "chkPresetLink";
      chkPresetLink.Size = new Size(70, 23);
      chkPresetLink.TabIndex = 119;
      chkPresetLink.Text = "Link";
      chkPresetLink.TextAlign = ContentAlignment.MiddleCenter;
      chkPresetLink.TextImageRelation = TextImageRelation.ImageAboveText;
      toolTip1.SetToolTip(chkPresetLink, "When checked, Next/Previous Preset commands in Visualizer trigger this list");
      chkPresetLink.UseVisualStyleBackColor = true;
      chkPresetLink.CheckedChanged += chkPresetLink_CheckedChanged;
      // 
      // btnPresetLoadTags
      // 
      btnPresetLoadTags.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      btnPresetLoadTags.FlatStyle = FlatStyle.System;
      btnPresetLoadTags.Location = new Point(380, 65);
      btnPresetLoadTags.Name = "btnPresetLoadTags";
      btnPresetLoadTags.Size = new Size(70, 23);
      btnPresetLoadTags.TabIndex = 121;
      btnPresetLoadTags.Text = "Tags";
      toolTip1.SetToolTip(btnPresetLoadTags, resources.GetString("btnPresetLoadTags.ToolTip"));
      btnPresetLoadTags.UseVisualStyleBackColor = true;
      btnPresetLoadTags.Click += btnPresetLoadTags_Click;
      // 
      // txtTags
      // 
      txtTags.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
      txtTags.Location = new Point(71, 95);
      txtTags.Name = "txtTags";
      txtTags.Size = new Size(379, 23);
      txtTags.TabIndex = 123;
      toolTip1.SetToolTip(txtTags, "Enter: Save\r\nCtrl+Enter: Save and select next preset");
      txtTags.Enter += txtTags_Enter;
      txtTags.KeyDown += txtFilterTags_KeyDown;
      txtTags.Leave += txtTags_Leave;
      // 
      // btnTagsSave
      // 
      btnTagsSave.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      btnTagsSave.FlatStyle = FlatStyle.System;
      btnTagsSave.Location = new Point(532, 95);
      btnTagsSave.Name = "btnTagsSave";
      btnTagsSave.Size = new Size(70, 51);
      btnTagsSave.TabIndex = 124;
      btnTagsSave.Text = "Save";
      toolTip1.SetToolTip(btnTagsSave, "Save tags for running or selected preset (Ctrl+T)");
      btnTagsSave.UseVisualStyleBackColor = true;
      btnTagsSave.Click += btnTagsSave_Click;
      // 
      // chkTagsFromRunning
      // 
      chkTagsFromRunning.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      chkTagsFromRunning.Appearance = Appearance.Button;
      chkTagsFromRunning.Checked = true;
      chkTagsFromRunning.CheckState = CheckState.Checked;
      chkTagsFromRunning.FlatStyle = FlatStyle.System;
      chkTagsFromRunning.Location = new Point(456, 94);
      chkTagsFromRunning.Margin = new Padding(3, 2, 3, 2);
      chkTagsFromRunning.Name = "chkTagsFromRunning";
      chkTagsFromRunning.Size = new Size(70, 23);
      chkTagsFromRunning.TabIndex = 125;
      chkTagsFromRunning.Text = "Running";
      chkTagsFromRunning.TextAlign = ContentAlignment.MiddleCenter;
      chkTagsFromRunning.TextImageRelation = TextImageRelation.ImageAboveText;
      toolTip1.SetToolTip(chkTagsFromRunning, "Checked: Get tags from (and save to) currently running preset\r\nUnchecked: Get tags from (and save to) selected preset above\r\n");
      chkTagsFromRunning.UseVisualStyleBackColor = true;
      chkTagsFromRunning.CheckedChanged += chkTagsFromRunning_CheckedChanged;
      // 
      // btnSendWave
      // 
      btnSendWave.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      btnSendWave.FlatStyle = FlatStyle.System;
      btnSendWave.Location = new Point(518, 6);
      btnSendWave.Name = "btnSendWave";
      btnSendWave.Size = new Size(83, 53);
      btnSendWave.TabIndex = 116;
      btnSendWave.Text = "Send";
      toolTip1.SetToolTip(btnSendWave, "Send to Visualizer\r\n(only alters wave already defined in preset)");
      btnSendWave.UseVisualStyleBackColor = true;
      btnSendWave.Click += btnSendWave_Click;
      // 
      // pnlColorWave
      // 
      pnlColorWave.BorderStyle = BorderStyle.FixedSingle;
      pnlColorWave.Location = new Point(416, 6);
      pnlColorWave.Name = "pnlColorWave";
      pnlColorWave.Size = new Size(50, 23);
      pnlColorWave.TabIndex = 114;
      toolTip1.SetToolTip(pnlColorWave, "Wave color");
      pnlColorWave.Click += pnlColorWave_Click;
      // 
      // numWaveAlpha
      // 
      numWaveAlpha.DecimalPlaces = 2;
      numWaveAlpha.Increment = new decimal(new int[] { 1, 0, 0, 65536 });
      numWaveAlpha.Location = new Point(186, 7);
      numWaveAlpha.Margin = new Padding(3, 2, 3, 2);
      numWaveAlpha.Maximum = new decimal(new int[] { 9999, 0, 0, 0 });
      numWaveAlpha.Name = "numWaveAlpha";
      numWaveAlpha.Size = new Size(56, 23);
      numWaveAlpha.TabIndex = 115;
      numWaveAlpha.TextAlign = HorizontalAlignment.Center;
      toolTip1.SetToolTip(numWaveAlpha, "Alpha (opacity)");
      numWaveAlpha.Value = new decimal(new int[] { 10, 0, 0, 65536 });
      numWaveAlpha.ValueChanged += ctrlWave_ValueChanged;
      numWaveAlpha.KeyDown += numWave_KeyDown;
      // 
      // lblWavemode
      // 
      lblWavemode.Location = new Point(4, 5);
      lblWavemode.Name = "lblWavemode";
      lblWavemode.Size = new Size(65, 24);
      lblWavemode.TabIndex = 117;
      lblWavemode.Text = "Mode";
      lblWavemode.TextAlign = ContentAlignment.MiddleRight;
      toolTip1.SetToolTip(lblWavemode, "Wave Mode (0-15)");
      // 
      // chkPresetRandom
      // 
      chkPresetRandom.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      chkPresetRandom.Appearance = Appearance.Button;
      chkPresetRandom.FlatStyle = FlatStyle.System;
      chkPresetRandom.Location = new Point(456, 35);
      chkPresetRandom.Margin = new Padding(3, 2, 3, 2);
      chkPresetRandom.Name = "chkPresetRandom";
      chkPresetRandom.Size = new Size(70, 23);
      chkPresetRandom.TabIndex = 120;
      chkPresetRandom.Text = "Random";
      chkPresetRandom.TextAlign = ContentAlignment.MiddleCenter;
      chkPresetRandom.TextImageRelation = TextImageRelation.ImageAboveText;
      toolTip1.SetToolTip(chkPresetRandom, "When checked AND linked, Next Preset commands in Visualizer will trgger a random preset from the preset list");
      chkPresetRandom.UseVisualStyleBackColor = true;
      // 
      // lblLoad
      // 
      lblLoad.Location = new Point(1, 64);
      lblLoad.Name = "lblLoad";
      lblLoad.Size = new Size(67, 24);
      lblLoad.TabIndex = 116;
      lblLoad.Text = "Load";
      lblLoad.TextAlign = ContentAlignment.MiddleRight;
      toolTip1.SetToolTip(lblLoad, "Double-click: Clear filter\r\nRight-click: Clear history");
      lblLoad.DoubleClick += lblLoad_DoubleClick;
      lblLoad.MouseDown += lblLoad_MouseDown;
      // 
      // lblTags
      // 
      lblTags.Location = new Point(1, 94);
      lblTags.Name = "lblTags";
      lblTags.Size = new Size(67, 23);
      lblTags.TabIndex = 122;
      lblTags.Text = "Tags";
      lblTags.TextAlign = ContentAlignment.MiddleRight;
      toolTip1.SetToolTip(lblTags, "Double-click: Clear tags\r\nCtrl+Click: Clear ALL tags");
      lblTags.DoubleClick += lblTags_DoubleClick;
      lblTags.MouseDown += lblTags_MouseDown;
      // 
      // chkWaveLink
      // 
      chkWaveLink.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      chkWaveLink.Appearance = Appearance.Button;
      chkWaveLink.Checked = true;
      chkWaveLink.CheckState = CheckState.Checked;
      chkWaveLink.FlatStyle = FlatStyle.System;
      chkWaveLink.Location = new Point(518, 66);
      chkWaveLink.Name = "chkWaveLink";
      chkWaveLink.Size = new Size(83, 23);
      chkWaveLink.TabIndex = 120;
      chkWaveLink.Text = "Link";
      chkWaveLink.TextAlign = ContentAlignment.MiddleCenter;
      chkWaveLink.TextImageRelation = TextImageRelation.ImageAboveText;
      toolTip1.SetToolTip(chkWaveLink, "Send and receive values from/to running preset instantly");
      chkWaveLink.UseVisualStyleBackColor = true;
      // 
      // numWaveR
      // 
      numWaveR.Increment = new decimal(new int[] { 5, 0, 0, 0 });
      numWaveR.Location = new Point(248, 7);
      numWaveR.Margin = new Padding(3, 2, 3, 2);
      numWaveR.Maximum = new decimal(new int[] { 255, 0, 0, 0 });
      numWaveR.Name = "numWaveR";
      numWaveR.Size = new Size(50, 23);
      numWaveR.TabIndex = 121;
      numWaveR.TextAlign = HorizontalAlignment.Center;
      toolTip1.SetToolTip(numWaveR, "R");
      numWaveR.Value = new decimal(new int[] { 255, 0, 0, 0 });
      numWaveR.ValueChanged += numWaveR_ValueChanged;
      // 
      // numWaveB
      // 
      numWaveB.Increment = new decimal(new int[] { 5, 0, 0, 0 });
      numWaveB.Location = new Point(360, 7);
      numWaveB.Margin = new Padding(3, 2, 3, 2);
      numWaveB.Maximum = new decimal(new int[] { 255, 0, 0, 0 });
      numWaveB.Name = "numWaveB";
      numWaveB.Size = new Size(50, 23);
      numWaveB.TabIndex = 124;
      numWaveB.TextAlign = HorizontalAlignment.Center;
      toolTip1.SetToolTip(numWaveB, "B");
      numWaveB.Value = new decimal(new int[] { 255, 0, 0, 0 });
      numWaveB.ValueChanged += numWaveB_ValueChanged;
      // 
      // numWaveG
      // 
      numWaveG.Increment = new decimal(new int[] { 5, 0, 0, 0 });
      numWaveG.Location = new Point(304, 7);
      numWaveG.Margin = new Padding(3, 2, 3, 2);
      numWaveG.Maximum = new decimal(new int[] { 255, 0, 0, 0 });
      numWaveG.Name = "numWaveG";
      numWaveG.Size = new Size(50, 23);
      numWaveG.TabIndex = 125;
      numWaveG.TextAlign = HorizontalAlignment.Center;
      toolTip1.SetToolTip(numWaveG, "G");
      numWaveG.Value = new decimal(new int[] { 255, 0, 0, 0 });
      numWaveG.ValueChanged += numWaveG_ValueChanged;
      // 
      // btnWaveClear
      // 
      btnWaveClear.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      btnWaveClear.FlatStyle = FlatStyle.System;
      btnWaveClear.Location = new Point(518, 95);
      btnWaveClear.Name = "btnWaveClear";
      btnWaveClear.Size = new Size(83, 22);
      btnWaveClear.TabIndex = 126;
      btnWaveClear.Text = "Clear";
      toolTip1.SetToolTip(btnWaveClear, "Clear the current preset");
      btnWaveClear.UseVisualStyleBackColor = true;
      btnWaveClear.Click += btnWaveClear_Click;
      // 
      // lblPushX
      // 
      lblPushX.Location = new Point(4, 35);
      lblPushX.Name = "lblPushX";
      lblPushX.Size = new Size(66, 23);
      lblPushX.TabIndex = 128;
      lblPushX.Text = "Push X";
      lblPushX.TextAlign = ContentAlignment.MiddleRight;
      toolTip1.SetToolTip(lblPushX, "Double-click: Set 0");
      lblPushX.DoubleClick += lblPushX_DoubleClick;
      // 
      // lblPushY
      // 
      lblPushY.Location = new Point(132, 35);
      lblPushY.Name = "lblPushY";
      lblPushY.Size = new Size(48, 23);
      lblPushY.TabIndex = 130;
      lblPushY.Text = "Push Y";
      lblPushY.TextAlign = ContentAlignment.MiddleRight;
      toolTip1.SetToolTip(lblPushY, "Double-click: Set 0");
      lblPushY.DoubleClick += lblPushY_DoubleClick;
      // 
      // lblZoom
      // 
      lblZoom.Location = new Point(248, 35);
      lblZoom.Name = "lblZoom";
      lblZoom.Size = new Size(48, 23);
      lblZoom.TabIndex = 132;
      lblZoom.Text = "Zoom";
      lblZoom.TextAlign = ContentAlignment.MiddleRight;
      toolTip1.SetToolTip(lblZoom, "Double-click: Set 1");
      lblZoom.DoubleClick += lblZoom_DoubleClick;
      // 
      // lblRotation
      // 
      lblRotation.Location = new Point(5, 65);
      lblRotation.Name = "lblRotation";
      lblRotation.Size = new Size(65, 23);
      lblRotation.TabIndex = 134;
      lblRotation.Text = "Rotation";
      lblRotation.TextAlign = ContentAlignment.MiddleRight;
      toolTip1.SetToolTip(lblRotation, "Double-click: Set 0");
      lblRotation.DoubleClick += lblRotation_DoubleClick;
      // 
      // lblWarp
      // 
      lblWarp.Location = new Point(248, 65);
      lblWarp.Name = "lblWarp";
      lblWarp.Size = new Size(46, 23);
      lblWarp.TabIndex = 136;
      lblWarp.Text = "Warp";
      lblWarp.TextAlign = ContentAlignment.MiddleRight;
      toolTip1.SetToolTip(lblWarp, "Double-click: Set 0");
      lblWarp.DoubleClick += lblWarp_DoubleClick;
      // 
      // lblDecay
      // 
      lblDecay.Location = new Point(133, 65);
      lblDecay.Name = "lblDecay";
      lblDecay.Size = new Size(48, 23);
      lblDecay.TabIndex = 138;
      lblDecay.Text = "Decay";
      lblDecay.TextAlign = ContentAlignment.MiddleRight;
      toolTip1.SetToolTip(lblDecay, "Double-click: Set 0");
      lblDecay.DoubleClick += lblDecay_DoubleClick;
      // 
      // btnWaveQuicksave
      // 
      btnWaveQuicksave.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      btnWaveQuicksave.FlatStyle = FlatStyle.System;
      btnWaveQuicksave.Location = new Point(518, 124);
      btnWaveQuicksave.Name = "btnWaveQuicksave";
      btnWaveQuicksave.Size = new Size(83, 22);
      btnWaveQuicksave.TabIndex = 139;
      btnWaveQuicksave.Text = "Quicksave";
      toolTip1.SetToolTip(btnWaveQuicksave, "Quicksave the current preset");
      btnWaveQuicksave.UseVisualStyleBackColor = true;
      btnWaveQuicksave.Click += btnWaveQuicksave_Click;
      // 
      // cboWindowTitle
      // 
      cboWindowTitle.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
      cboWindowTitle.AutoCompleteMode = AutoCompleteMode.SuggestAppend;
      cboWindowTitle.AutoCompleteSource = AutoCompleteSource.ListItems;
      cboWindowTitle.DropDownStyle = ComboBoxStyle.DropDownList;
      cboWindowTitle.FormattingEnabled = true;
      cboWindowTitle.Items.AddRange(new object[] { "Milkwave Visualizer", "Milkwave Visualizer 2", "Milkwave Visualizer 3", "Milkwave Visualizer 4", "Milkwave Visualizer 5", "Milkwave Visualizer 6", "Milkwave Visualizer 7", "Milkwave Visualizer 8", "Milkwave Visualizer 9" });
      cboWindowTitle.Location = new Point(74, 67);
      cboWindowTitle.Name = "cboWindowTitle";
      cboWindowTitle.Size = new Size(254, 23);
      cboWindowTitle.TabIndex = 108;
      toolTip1.SetToolTip(cboWindowTitle, "Target window title");
      // 
      // chkWaveVolAlpha
      // 
      chkWaveVolAlpha.Appearance = Appearance.Button;
      chkWaveVolAlpha.FlatStyle = FlatStyle.System;
      chkWaveVolAlpha.Location = new Point(360, 123);
      chkWaveVolAlpha.Name = "chkWaveVolAlpha";
      chkWaveVolAlpha.Size = new Size(106, 23);
      chkWaveVolAlpha.TabIndex = 147;
      chkWaveVolAlpha.Text = "Alpha";
      chkWaveVolAlpha.TextAlign = ContentAlignment.MiddleCenter;
      chkWaveVolAlpha.TextImageRelation = TextImageRelation.ImageAboveText;
      toolTip1.SetToolTip(chkWaveVolAlpha, "Change Alpha With Volume");
      chkWaveVolAlpha.UseVisualStyleBackColor = true;
      chkWaveVolAlpha.CheckedChanged += ctrlWave_ValueChanged;
      // 
      // lblScale
      // 
      lblScale.Location = new Point(360, 35);
      lblScale.Name = "lblScale";
      lblScale.Size = new Size(48, 23);
      lblScale.TabIndex = 149;
      lblScale.Text = "Scale";
      lblScale.TextAlign = ContentAlignment.MiddleRight;
      toolTip1.SetToolTip(lblScale, "Double-click: Set 1");
      lblScale.DoubleClick += lblScale_DoubleClick;
      // 
      // lblEcho
      // 
      lblEcho.Location = new Point(360, 66);
      lblEcho.Name = "lblEcho";
      lblEcho.Size = new Size(48, 23);
      lblEcho.TabIndex = 151;
      lblEcho.Text = "Echo";
      lblEcho.TextAlign = ContentAlignment.MiddleRight;
      toolTip1.SetToolTip(lblEcho, "Double-click: Set 2");
      lblEcho.DoubleClick += lblEcho_DoubleClick;
      // 
      // cboTagsFilter
      // 
      cboTagsFilter.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
      cboTagsFilter.Location = new Point(71, 65);
      cboTagsFilter.Name = "cboTagsFilter";
      cboTagsFilter.Size = new Size(227, 23);
      cboTagsFilter.TabIndex = 137;
      toolTip1.SetToolTip(cboTagsFilter, "Tags filter");
      cboTagsFilter.KeyDown += cboDirOrTagsFilter_KeyDown;
      // 
      // lblMostUsed
      // 
      lblMostUsed.Location = new Point(1, 124);
      lblMostUsed.Name = "lblMostUsed";
      lblMostUsed.Size = new Size(67, 23);
      lblMostUsed.TabIndex = 136;
      lblMostUsed.Text = "Most used";
      lblMostUsed.TextAlign = ContentAlignment.MiddleRight;
      toolTip1.SetToolTip(lblMostUsed, "Double-click: Show tag statistics");
      lblMostUsed.DoubleClick += lblMostUsed_DoubleClick;
      // 
      // pnlColorFont1
      // 
      pnlColorFont1.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      pnlColorFont1.BorderStyle = BorderStyle.FixedSingle;
      pnlColorFont1.Location = new Point(307, 7);
      pnlColorFont1.Name = "pnlColorFont1";
      pnlColorFont1.Size = new Size(38, 23);
      pnlColorFont1.TabIndex = 119;
      toolTip1.SetToolTip(pnlColorFont1, "Font color");
      pnlColorFont1.Click += pnlColorFont_Click;
      // 
      // numFont1
      // 
      numFont1.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      numFont1.Location = new Point(255, 7);
      numFont1.Margin = new Padding(3, 2, 3, 2);
      numFont1.Maximum = new decimal(new int[] { 99, 0, 0, 0 });
      numFont1.Name = "numFont1";
      numFont1.Size = new Size(46, 23);
      numFont1.TabIndex = 121;
      numFont1.TextAlign = HorizontalAlignment.Center;
      toolTip1.SetToolTip(numFont1, "Font size\r\nAlt+Mousewheel: Save and preview instantly");
      numFont1.Value = new decimal(new int[] { 30, 0, 0, 0 });
      numFont1.ValueChanged += numFont1_ValueChanged;
      // 
      // chkFontAA1
      // 
      chkFontAA1.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      chkFontAA1.Appearance = Appearance.Button;
      chkFontAA1.FlatStyle = FlatStyle.System;
      chkFontAA1.Location = new Point(461, 7);
      chkFontAA1.Margin = new Padding(3, 2, 3, 2);
      chkFontAA1.Name = "chkFontAA1";
      chkFontAA1.Size = new Size(49, 23);
      chkFontAA1.TabIndex = 127;
      chkFontAA1.Text = "AA";
      chkFontAA1.TextAlign = ContentAlignment.MiddleCenter;
      chkFontAA1.TextImageRelation = TextImageRelation.ImageAboveText;
      toolTip1.SetToolTip(chkFontAA1, "Anti-Aliased");
      chkFontAA1.UseVisualStyleBackColor = true;
      // 
      // btnSettingsSave
      // 
      btnSettingsSave.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      btnSettingsSave.FlatStyle = FlatStyle.System;
      btnSettingsSave.Location = new Point(518, 6);
      btnSettingsSave.Name = "btnSettingsSave";
      btnSettingsSave.Size = new Size(83, 53);
      btnSettingsSave.TabIndex = 128;
      btnSettingsSave.Text = "Save";
      toolTip1.SetToolTip(btnSettingsSave, "Save values to settings.ini and reload settings in Visualizer\r\n");
      btnSettingsSave.UseVisualStyleBackColor = true;
      btnSettingsSave.Click += btnSettingsSave_Click;
      // 
      // lblFont1
      // 
      lblFont1.Location = new Point(1, 6);
      lblFont1.Name = "lblFont1";
      lblFont1.Size = new Size(67, 24);
      lblFont1.TabIndex = 124;
      lblFont1.Text = "Notify";
      lblFont1.TextAlign = ContentAlignment.MiddleRight;
      toolTip1.SetToolTip(lblFont1, "Simple Font: Most messages and notifications\r\nDouble-click: Set default values\r\n");
      lblFont1.DoubleClick += lblFont1_DoubleClick;
      // 
      // btnSettingsLoad
      // 
      btnSettingsLoad.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      btnSettingsLoad.FlatStyle = FlatStyle.System;
      btnSettingsLoad.Location = new Point(518, 65);
      btnSettingsLoad.Name = "btnSettingsLoad";
      btnSettingsLoad.Size = new Size(83, 23);
      btnSettingsLoad.TabIndex = 130;
      btnSettingsLoad.Text = "Load";
      toolTip1.SetToolTip(btnSettingsLoad, "Load values from settings.ini");
      btnSettingsLoad.UseVisualStyleBackColor = true;
      btnSettingsLoad.Click += btnSettingsLoad_Click;
      // 
      // chkFontAA2
      // 
      chkFontAA2.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      chkFontAA2.Appearance = Appearance.Button;
      chkFontAA2.FlatStyle = FlatStyle.System;
      chkFontAA2.Location = new Point(461, 36);
      chkFontAA2.Margin = new Padding(3, 2, 3, 2);
      chkFontAA2.Name = "chkFontAA2";
      chkFontAA2.Size = new Size(49, 23);
      chkFontAA2.TabIndex = 137;
      chkFontAA2.Text = "AA";
      chkFontAA2.TextAlign = ContentAlignment.MiddleCenter;
      chkFontAA2.TextImageRelation = TextImageRelation.ImageAboveText;
      toolTip1.SetToolTip(chkFontAA2, "Anti-Aliased");
      chkFontAA2.UseVisualStyleBackColor = true;
      // 
      // lblFont2
      // 
      lblFont2.Location = new Point(1, 35);
      lblFont2.Name = "lblFont2";
      lblFont2.Size = new Size(67, 24);
      lblFont2.TabIndex = 134;
      lblFont2.Text = "Preset";
      lblFont2.TextAlign = ContentAlignment.MiddleRight;
      toolTip1.SetToolTip(lblFont2, "Decorative Font: Preset name\r\nDouble-click: Set default values");
      lblFont2.DoubleClick += lblFont2_DoubleClick;
      // 
      // numFont2
      // 
      numFont2.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      numFont2.Location = new Point(255, 36);
      numFont2.Margin = new Padding(3, 2, 3, 2);
      numFont2.Maximum = new decimal(new int[] { 99, 0, 0, 0 });
      numFont2.Name = "numFont2";
      numFont2.Size = new Size(46, 23);
      numFont2.TabIndex = 133;
      numFont2.TextAlign = HorizontalAlignment.Center;
      toolTip1.SetToolTip(numFont2, "Font size\r\nAlt+Mousewheel: Save and preview instantly\r\n");
      numFont2.Value = new decimal(new int[] { 30, 0, 0, 0 });
      numFont2.ValueChanged += numFont2_ValueChanged;
      // 
      // pnlColorFont2
      // 
      pnlColorFont2.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      pnlColorFont2.BorderStyle = BorderStyle.FixedSingle;
      pnlColorFont2.Location = new Point(307, 36);
      pnlColorFont2.Name = "pnlColorFont2";
      pnlColorFont2.Size = new Size(38, 23);
      pnlColorFont2.TabIndex = 132;
      toolTip1.SetToolTip(pnlColorFont2, "Font color");
      pnlColorFont2.Click += pnlColorFont_Click;
      // 
      // chkFontAA3
      // 
      chkFontAA3.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      chkFontAA3.Appearance = Appearance.Button;
      chkFontAA3.FlatStyle = FlatStyle.System;
      chkFontAA3.Location = new Point(461, 65);
      chkFontAA3.Margin = new Padding(3, 2, 3, 2);
      chkFontAA3.Name = "chkFontAA3";
      chkFontAA3.Size = new Size(49, 23);
      chkFontAA3.TabIndex = 144;
      chkFontAA3.Text = "AA";
      chkFontAA3.TextAlign = ContentAlignment.MiddleCenter;
      chkFontAA3.TextImageRelation = TextImageRelation.ImageAboveText;
      toolTip1.SetToolTip(chkFontAA3, "Anti-Aliased");
      chkFontAA3.UseVisualStyleBackColor = true;
      // 
      // lblFont3
      // 
      lblFont3.Location = new Point(1, 64);
      lblFont3.Name = "lblFont3";
      lblFont3.Size = new Size(67, 24);
      lblFont3.TabIndex = 141;
      lblFont3.Text = "Artist";
      lblFont3.TextAlign = ContentAlignment.MiddleRight;
      toolTip1.SetToolTip(lblFont3, "Extra Font 1: Artist\r\nDouble-click: Set default values");
      lblFont3.DoubleClick += lblFont3_DoubleClick;
      // 
      // numFont3
      // 
      numFont3.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      numFont3.Location = new Point(255, 65);
      numFont3.Margin = new Padding(3, 2, 3, 2);
      numFont3.Maximum = new decimal(new int[] { 99, 0, 0, 0 });
      numFont3.Name = "numFont3";
      numFont3.Size = new Size(46, 23);
      numFont3.TabIndex = 140;
      numFont3.TextAlign = HorizontalAlignment.Center;
      toolTip1.SetToolTip(numFont3, "Font size\r\nAlt+Mousewheel: Save and preview instantly\r\n");
      numFont3.Value = new decimal(new int[] { 30, 0, 0, 0 });
      numFont3.ValueChanged += numFont3_ValueChanged;
      // 
      // pnlColorFont3
      // 
      pnlColorFont3.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      pnlColorFont3.BorderStyle = BorderStyle.FixedSingle;
      pnlColorFont3.Location = new Point(307, 65);
      pnlColorFont3.Name = "pnlColorFont3";
      pnlColorFont3.Size = new Size(38, 23);
      pnlColorFont3.TabIndex = 139;
      toolTip1.SetToolTip(pnlColorFont3, "Font color");
      pnlColorFont3.Click += pnlColorFont_Click;
      // 
      // chkFontAA4
      // 
      chkFontAA4.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      chkFontAA4.Appearance = Appearance.Button;
      chkFontAA4.FlatStyle = FlatStyle.System;
      chkFontAA4.Location = new Point(461, 94);
      chkFontAA4.Margin = new Padding(3, 2, 3, 2);
      chkFontAA4.Name = "chkFontAA4";
      chkFontAA4.Size = new Size(49, 23);
      chkFontAA4.TabIndex = 151;
      chkFontAA4.Text = "AA";
      chkFontAA4.TextAlign = ContentAlignment.MiddleCenter;
      chkFontAA4.TextImageRelation = TextImageRelation.ImageAboveText;
      toolTip1.SetToolTip(chkFontAA4, "Anti-Aliased");
      chkFontAA4.UseVisualStyleBackColor = true;
      // 
      // lblFont4
      // 
      lblFont4.Location = new Point(1, 93);
      lblFont4.Name = "lblFont4";
      lblFont4.Size = new Size(67, 24);
      lblFont4.TabIndex = 148;
      lblFont4.Text = "Title";
      lblFont4.TextAlign = ContentAlignment.MiddleRight;
      toolTip1.SetToolTip(lblFont4, "Extra Font 2: Title\r\nDouble-click: Set default values\r\n");
      lblFont4.DoubleClick += lblFont4_DoubleClick;
      // 
      // numFont4
      // 
      numFont4.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      numFont4.Location = new Point(255, 94);
      numFont4.Margin = new Padding(3, 2, 3, 2);
      numFont4.Maximum = new decimal(new int[] { 99, 0, 0, 0 });
      numFont4.Name = "numFont4";
      numFont4.Size = new Size(46, 23);
      numFont4.TabIndex = 147;
      numFont4.TextAlign = HorizontalAlignment.Center;
      toolTip1.SetToolTip(numFont4, "Font size\r\nAlt+Mousewheel: Save and preview instantly\r\n");
      numFont4.Value = new decimal(new int[] { 30, 0, 0, 0 });
      numFont4.ValueChanged += numFont4_ValueChanged;
      // 
      // pnlColorFont4
      // 
      pnlColorFont4.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      pnlColorFont4.BorderStyle = BorderStyle.FixedSingle;
      pnlColorFont4.Location = new Point(307, 94);
      pnlColorFont4.Name = "pnlColorFont4";
      pnlColorFont4.Size = new Size(38, 23);
      pnlColorFont4.TabIndex = 146;
      toolTip1.SetToolTip(pnlColorFont4, "Font color");
      pnlColorFont4.Click += pnlColorFont_Click;
      // 
      // chkFontAA5
      // 
      chkFontAA5.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      chkFontAA5.Appearance = Appearance.Button;
      chkFontAA5.FlatStyle = FlatStyle.System;
      chkFontAA5.Location = new Point(461, 123);
      chkFontAA5.Margin = new Padding(3, 2, 3, 2);
      chkFontAA5.Name = "chkFontAA5";
      chkFontAA5.Size = new Size(49, 23);
      chkFontAA5.TabIndex = 158;
      chkFontAA5.Text = "AA";
      chkFontAA5.TextAlign = ContentAlignment.MiddleCenter;
      chkFontAA5.TextImageRelation = TextImageRelation.ImageAboveText;
      toolTip1.SetToolTip(chkFontAA5, "Anti-Aliased");
      chkFontAA5.UseVisualStyleBackColor = true;
      // 
      // lblFont5
      // 
      lblFont5.Location = new Point(1, 122);
      lblFont5.Name = "lblFont5";
      lblFont5.Size = new Size(67, 24);
      lblFont5.TabIndex = 155;
      lblFont5.Text = "Album";
      lblFont5.TextAlign = ContentAlignment.MiddleRight;
      toolTip1.SetToolTip(lblFont5, "Extra Font 3: Album\r\nDouble-click: Set default values");
      lblFont5.DoubleClick += lblFont5_DoubleClick;
      // 
      // numFont5
      // 
      numFont5.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      numFont5.Location = new Point(255, 123);
      numFont5.Margin = new Padding(3, 2, 3, 2);
      numFont5.Maximum = new decimal(new int[] { 99, 0, 0, 0 });
      numFont5.Name = "numFont5";
      numFont5.Size = new Size(46, 23);
      numFont5.TabIndex = 154;
      numFont5.TextAlign = HorizontalAlignment.Center;
      toolTip1.SetToolTip(numFont5, "Font size\r\nAlt+Mousewheel: Save and preview instantly\r\n");
      numFont5.Value = new decimal(new int[] { 30, 0, 0, 0 });
      numFont5.ValueChanged += numFont5_ValueChanged;
      // 
      // pnlColorFont5
      // 
      pnlColorFont5.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      pnlColorFont5.BorderStyle = BorderStyle.FixedSingle;
      pnlColorFont5.Location = new Point(307, 123);
      pnlColorFont5.Name = "pnlColorFont5";
      pnlColorFont5.Size = new Size(38, 23);
      pnlColorFont5.TabIndex = 153;
      toolTip1.SetToolTip(pnlColorFont5, "Font color");
      pnlColorFont5.Click += pnlColorFont_Click;
      // 
      // cboFont1
      // 
      cboFont1.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
      cboFont1.AutoCompleteMode = AutoCompleteMode.SuggestAppend;
      cboFont1.AutoCompleteSource = AutoCompleteSource.ListItems;
      cboFont1.DropDownStyle = ComboBoxStyle.DropDownList;
      cboFont1.FormattingEnabled = true;
      cboFont1.Location = new Point(71, 7);
      cboFont1.Name = "cboFont1";
      cboFont1.Size = new Size(178, 23);
      cboFont1.TabIndex = 118;
      toolTip1.SetToolTip(cboFont1, "Font face\r\nAlt+Mousewheel: Save and preview instantly");
      cboFont1.SelectedIndexChanged += cboFont1_SelectedIndexChanged;
      // 
      // cboFont5
      // 
      cboFont5.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
      cboFont5.AutoCompleteMode = AutoCompleteMode.SuggestAppend;
      cboFont5.AutoCompleteSource = AutoCompleteSource.ListItems;
      cboFont5.DropDownStyle = ComboBoxStyle.DropDownList;
      cboFont5.FormattingEnabled = true;
      cboFont5.Location = new Point(71, 123);
      cboFont5.Name = "cboFont5";
      cboFont5.Size = new Size(178, 23);
      cboFont5.TabIndex = 152;
      toolTip1.SetToolTip(cboFont5, "Font face\r\nAlt+Mousewheel: Save and preview instantly\r\n");
      cboFont5.SelectedIndexChanged += cboFont5_SelectedIndexChanged;
      // 
      // cboFont4
      // 
      cboFont4.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
      cboFont4.AutoCompleteMode = AutoCompleteMode.SuggestAppend;
      cboFont4.AutoCompleteSource = AutoCompleteSource.ListItems;
      cboFont4.DropDownStyle = ComboBoxStyle.DropDownList;
      cboFont4.FormattingEnabled = true;
      cboFont4.Location = new Point(71, 94);
      cboFont4.Name = "cboFont4";
      cboFont4.Size = new Size(178, 23);
      cboFont4.TabIndex = 145;
      toolTip1.SetToolTip(cboFont4, "Font face\r\nAlt+Mousewheel: Save and preview instantly\r\n");
      cboFont4.SelectedIndexChanged += cboFont4_SelectedIndexChanged;
      // 
      // cboFont3
      // 
      cboFont3.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
      cboFont3.AutoCompleteMode = AutoCompleteMode.SuggestAppend;
      cboFont3.AutoCompleteSource = AutoCompleteSource.ListItems;
      cboFont3.DropDownStyle = ComboBoxStyle.DropDownList;
      cboFont3.FormattingEnabled = true;
      cboFont3.Location = new Point(71, 65);
      cboFont3.Name = "cboFont3";
      cboFont3.Size = new Size(178, 23);
      cboFont3.TabIndex = 138;
      toolTip1.SetToolTip(cboFont3, "Font face\r\nAlt+Mousewheel: Save and preview instantly\r\n");
      cboFont3.SelectedIndexChanged += cboFont3_SelectedIndexChanged;
      // 
      // cboFont2
      // 
      cboFont2.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
      cboFont2.AutoCompleteMode = AutoCompleteMode.SuggestAppend;
      cboFont2.AutoCompleteSource = AutoCompleteSource.ListItems;
      cboFont2.DropDownStyle = ComboBoxStyle.DropDownList;
      cboFont2.FormattingEnabled = true;
      cboFont2.Location = new Point(71, 36);
      cboFont2.Name = "cboFont2";
      cboFont2.Size = new Size(178, 23);
      cboFont2.TabIndex = 131;
      toolTip1.SetToolTip(cboFont2, "Font face\r\nAlt+Mousewheel: Save and preview instantly\r\n");
      cboFont2.SelectedIndexChanged += cboFont2_SelectedIndexChanged;
      // 
      // btnSpace
      // 
      tableLayoutPanel1.SetColumnSpan(btnSpace, 2);
      btnSpace.Dock = DockStyle.Fill;
      btnSpace.Location = new Point(7, 6);
      btnSpace.Margin = new Padding(3, 2, 3, 2);
      btnSpace.Name = "btnSpace";
      btnSpace.Size = new Size(116, 40);
      btnSpace.TabIndex = 0;
      btnSpace.Text = "Next Preset\r\n(Space)";
      toolTip1.SetToolTip(btnSpace, "Ctrl+Space");
      btnSpace.UseVisualStyleBackColor = true;
      btnSpace.Click += btnSpace_Click;
      // 
      // btnBackspace
      // 
      tableLayoutPanel1.SetColumnSpan(btnBackspace, 2);
      btnBackspace.Dock = DockStyle.Fill;
      btnBackspace.Location = new Point(129, 6);
      btnBackspace.Margin = new Padding(3, 2, 3, 2);
      btnBackspace.Name = "btnBackspace";
      btnBackspace.Size = new Size(116, 40);
      btnBackspace.TabIndex = 1;
      btnBackspace.Text = "Previous Preset\r\n(Backspace)";
      toolTip1.SetToolTip(btnBackspace, "Shift+Ctrl+Space");
      btnBackspace.UseVisualStyleBackColor = true;
      btnBackspace.Click += btnBackspace_Click;
      // 
      // btnWatermark
      // 
      tableLayoutPanel1.SetColumnSpan(btnWatermark, 2);
      btnWatermark.Dock = DockStyle.Fill;
      btnWatermark.Location = new Point(373, 94);
      btnWatermark.Margin = new Padding(3, 2, 3, 2);
      btnWatermark.Name = "btnWatermark";
      btnWatermark.Size = new Size(116, 40);
      btnWatermark.TabIndex = 22;
      btnWatermark.Text = "Watermark Mode \r\n(Ctrl+Shift+F9)";
      toolTip1.SetToolTip(btnWatermark, "Right-click:\r\nSwitch to Desktop Mode instead (Ctrl+F9)");
      btnWatermark.UseVisualStyleBackColor = true;
      btnWatermark.Click += btnWatermark_Click;
      btnWatermark.MouseDown += btnWatermark_MouseDown;
      // 
      // lblFactorFrame
      // 
      lblFactorFrame.Location = new Point(4, 63);
      lblFactorFrame.Name = "lblFactorFrame";
      lblFactorFrame.Size = new Size(65, 23);
      lblFactorFrame.TabIndex = 140;
      lblFactorFrame.Text = "Frame";
      lblFactorFrame.TextAlign = ContentAlignment.MiddleRight;
      toolTip1.SetToolTip(lblFactorFrame, "Value by which the internal frame counter is incremented on every loop\r\nHigher values may trigger counter-based preset events more often\r\nClick: Set 1");
      lblFactorFrame.Click += lblFactorFrame_Click;
      // 
      // lblFactorTime
      // 
      lblFactorTime.Location = new Point(4, 5);
      lblFactorTime.Name = "lblFactorTime";
      lblFactorTime.Size = new Size(65, 23);
      lblFactorTime.TabIndex = 138;
      lblFactorTime.Text = "Time";
      lblFactorTime.TextAlign = ContentAlignment.MiddleRight;
      toolTip1.SetToolTip(lblFactorTime, "Values < 1 may slow down rendering of the preset, sprites and notifications\r\nClick: Set 1");
      lblFactorTime.Click += lblFactorTime_Click;
      // 
      // lblFactorFPS
      // 
      lblFactorFPS.Location = new Point(4, 35);
      lblFactorFPS.Name = "lblFactorFPS";
      lblFactorFPS.Size = new Size(65, 23);
      lblFactorFPS.TabIndex = 142;
      lblFactorFPS.Text = "FPS";
      lblFactorFPS.TextAlign = ContentAlignment.MiddleRight;
      toolTip1.SetToolTip(lblFactorFPS, "Values < 1 may speed up rendering of the preset, not affecting sprites and notifications\r\nClick: Set 1");
      lblFactorFPS.Click += lblFactorFPS_Click;
      // 
      // btnOpenSettingsFile
      // 
      btnOpenSettingsFile.Location = new Point(212, 151);
      btnOpenSettingsFile.Name = "btnOpenSettingsFile";
      btnOpenSettingsFile.Size = new Size(56, 23);
      btnOpenSettingsFile.TabIndex = 143;
      btnOpenSettingsFile.Text = "Open";
      toolTip1.SetToolTip(btnOpenSettingsFile, "Open file in associated editor");
      btnOpenSettingsFile.UseVisualStyleBackColor = true;
      btnOpenSettingsFile.Click += btnOpenSettingsIni_Click;
      // 
      // txtFilterTags
      // 
      txtFilterTags.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      txtFilterTags.Location = new Point(304, 65);
      txtFilterTags.Name = "txtFilterTags";
      txtFilterTags.Size = new Size(70, 23);
      txtFilterTags.TabIndex = 138;
      toolTip1.SetToolTip(txtFilterTags, "Only load presets containing this text in filename\r\nPress Enter: Load and filter all presets including subdirs\r\n\"age=X\": Load only presets modified within the last X days");
      txtFilterTags.KeyDown += txtFilter_KeyDown;
      // 
      // btnSendShader
      // 
      btnSendShader.Anchor = AnchorStyles.Bottom | AnchorStyles.Right;
      btnSendShader.FlatStyle = FlatStyle.System;
      btnSendShader.Location = new Point(530, 154);
      btnSendShader.Name = "btnSendShader";
      btnSendShader.Size = new Size(70, 23);
      btnSendShader.TabIndex = 26;
      btnSendShader.Text = "Send";
      toolTip1.SetToolTip(btnSendShader, "Save HLSL as preset and send to Visualizer \r\n(Ctrl+S)");
      btnSendShader.UseVisualStyleBackColor = true;
      btnSendShader.Click += btnSendShader_Click;
      // 
      // txtShaderinfo
      // 
      txtShaderinfo.Anchor = AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
      txtShaderinfo.Location = new Point(117, 155);
      txtShaderinfo.Multiline = true;
      txtShaderinfo.Name = "txtShaderinfo";
      txtShaderinfo.ScrollBars = ScrollBars.Vertical;
      txtShaderinfo.Size = new Size(309, 23);
      txtShaderinfo.TabIndex = 35;
      toolTip1.SetToolTip(txtShaderinfo, "Shaderinfo used for filename and embedded into generated preset file\r\nCan be multiple lines, use cursor keys to scroll");
      // 
      // btnShaderConvert
      // 
      btnShaderConvert.Anchor = AnchorStyles.Bottom | AnchorStyles.Left;
      btnShaderConvert.FlatStyle = FlatStyle.System;
      btnShaderConvert.Location = new Point(35, 154);
      btnShaderConvert.Name = "btnShaderConvert";
      btnShaderConvert.Size = new Size(76, 23);
      btnShaderConvert.TabIndex = 32;
      btnShaderConvert.Text = "Convert";
      toolTip1.SetToolTip(btnShaderConvert, "Convert GLSL (left) to HLSL (right)");
      btnShaderConvert.UseVisualStyleBackColor = true;
      btnShaderConvert.Click += btnShaderConvert_Click;
      // 
      // txtLineNumberError
      // 
      txtLineNumberError.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      txtLineNumberError.Location = new Point(492, 7);
      txtLineNumberError.Name = "txtLineNumberError";
      txtLineNumberError.ReadOnly = true;
      txtLineNumberError.Size = new Size(28, 23);
      txtLineNumberError.TabIndex = 30;
      txtLineNumberError.TextAlign = HorizontalAlignment.Center;
      toolTip1.SetToolTip(txtLineNumberError, "Approx. line in Milkwave generated shader code\r\nUse this to find the matching line from shader compilation error messages");
      // 
      // txtShaderFind
      // 
      txtShaderFind.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      txtShaderFind.Location = new Point(392, 7);
      txtShaderFind.Name = "txtShaderFind";
      txtShaderFind.Size = new Size(43, 23);
      txtShaderFind.TabIndex = 29;
      toolTip1.SetToolTip(txtShaderFind, "Ctrl+F: Find next occurence of this string\r\nCtrl+Shift+F: Focus search box\r\nCtrl+Alt+F: Reformat HLSL code");
      txtShaderFind.KeyDown += txtShaderFind_KeyDown;
      // 
      // btnLoadShaderInputFromFile
      // 
      btnLoadShaderInputFromFile.FlatStyle = FlatStyle.System;
      btnLoadShaderInputFromFile.Location = new Point(318, 5);
      btnLoadShaderInputFromFile.Name = "btnLoadShaderInputFromFile";
      btnLoadShaderInputFromFile.Size = new Size(49, 23);
      btnLoadShaderInputFromFile.TabIndex = 28;
      btnLoadShaderInputFromFile.Text = "File";
      toolTip1.SetToolTip(btnLoadShaderInputFromFile, "Load .glsl or .json (Shadertoy) file into left pane");
      btnLoadShaderInputFromFile.UseVisualStyleBackColor = true;
      btnLoadShaderInputFromFile.Click += btnLoadShaderInput_Click;
      // 
      // btnShaderHelp
      // 
      btnShaderHelp.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      btnShaderHelp.FlatStyle = FlatStyle.System;
      btnShaderHelp.Location = new Point(579, 6);
      btnShaderHelp.Name = "btnShaderHelp";
      btnShaderHelp.Size = new Size(23, 23);
      btnShaderHelp.TabIndex = 36;
      btnShaderHelp.Text = "?";
      toolTip1.SetToolTip(btnShaderHelp, "Help");
      btnShaderHelp.UseVisualStyleBackColor = true;
      btnShaderHelp.Click += btnShaderHelp_Click;
      // 
      // numPSVersion
      // 
      numPSVersion.Anchor = AnchorStyles.Bottom | AnchorStyles.Right;
      numPSVersion.Location = new Point(482, 156);
      numPSVersion.Margin = new Padding(3, 2, 3, 2);
      numPSVersion.Maximum = new decimal(new int[] { 4, 0, 0, 0 });
      numPSVersion.Minimum = new decimal(new int[] { 2, 0, 0, 0 });
      numPSVersion.Name = "numPSVersion";
      numPSVersion.Size = new Size(42, 23);
      numPSVersion.TabIndex = 140;
      numPSVersion.TextAlign = HorizontalAlignment.Center;
      toolTip1.SetToolTip(numPSVersion, "Pixel shader version");
      numPSVersion.Value = new decimal(new int[] { 4, 0, 0, 0 });
      // 
      // numOffset
      // 
      numOffset.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      numOffset.Location = new Point(439, 7);
      numOffset.Margin = new Padding(3, 2, 3, 2);
      numOffset.Maximum = new decimal(new int[] { 999, 0, 0, 0 });
      numOffset.Minimum = new decimal(new int[] { 150, 0, 0, 0 });
      numOffset.Name = "numOffset";
      numOffset.Size = new Size(47, 23);
      numOffset.TabIndex = 141;
      numOffset.TextAlign = HorizontalAlignment.Center;
      toolTip1.SetToolTip(numOffset, "Adjust this value if the line marked as error is wrong");
      numOffset.Value = new decimal(new int[] { 227, 0, 0, 0 });
      numOffset.ValueChanged += numOffset_ValueChanged;
      // 
      // chkShaderFile
      // 
      chkShaderFile.Anchor = AnchorStyles.Bottom | AnchorStyles.Right;
      chkShaderFile.AutoSize = true;
      chkShaderFile.Checked = true;
      chkShaderFile.CheckState = CheckState.Checked;
      chkShaderFile.Location = new Point(432, 156);
      chkShaderFile.Name = "chkShaderFile";
      chkShaderFile.Size = new Size(44, 19);
      chkShaderFile.TabIndex = 145;
      chkShaderFile.Text = "File";
      toolTip1.SetToolTip(chkShaderFile, "If checked, use left line for generated preset file\r\nIf unchecked, filename will be \"Shader.milk\"");
      chkShaderFile.UseVisualStyleBackColor = true;
      // 
      // chkShaderLeft
      // 
      chkShaderLeft.Anchor = AnchorStyles.Bottom | AnchorStyles.Left;
      chkShaderLeft.Appearance = Appearance.Button;
      chkShaderLeft.FlatStyle = FlatStyle.System;
      chkShaderLeft.Font = new Font("Segoe UI", 9F, FontStyle.Bold, GraphicsUnit.Point, 0);
      chkShaderLeft.Location = new Point(7, 154);
      chkShaderLeft.Margin = new Padding(3, 2, 3, 2);
      chkShaderLeft.Name = "chkShaderLeft";
      chkShaderLeft.Size = new Size(23, 23);
      chkShaderLeft.TabIndex = 147;
      chkShaderLeft.Text = "<";
      chkShaderLeft.TextAlign = ContentAlignment.MiddleCenter;
      chkShaderLeft.TextImageRelation = TextImageRelation.ImageAboveText;
      toolTip1.SetToolTip(chkShaderLeft, "Show/hide left pane (Ctrl-Y)\r\n\r\n");
      chkShaderLeft.UseVisualStyleBackColor = true;
      chkShaderLeft.CheckedChanged += chkShaderLeft_CheckedChanged;
      // 
      // btnHLSLSave
      // 
      btnHLSLSave.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      btnHLSLSave.FlatStyle = FlatStyle.System;
      btnHLSLSave.Location = new Point(551, 6);
      btnHLSLSave.Name = "btnHLSLSave";
      btnHLSLSave.Size = new Size(23, 23);
      btnHLSLSave.TabIndex = 148;
      btnHLSLSave.Text = "S";
      toolTip1.SetToolTip(btnHLSLSave, "Save HLSL to file\r\n(including Shaderinfo lines)\r\n");
      btnHLSLSave.UseVisualStyleBackColor = true;
      btnHLSLSave.Click += btnHLSLSave_Click;
      // 
      // btnHLSLLoad
      // 
      btnHLSLLoad.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      btnHLSLLoad.FlatStyle = FlatStyle.System;
      btnHLSLLoad.Location = new Point(523, 6);
      btnHLSLLoad.Name = "btnHLSLLoad";
      btnHLSLLoad.Size = new Size(23, 23);
      btnHLSLLoad.TabIndex = 149;
      btnHLSLLoad.Text = "L";
      toolTip1.SetToolTip(btnHLSLLoad, "Load HLSL shader code from .hlsl or .milk file\r\n(restoring Shaderinfo lines)");
      btnHLSLLoad.UseVisualStyleBackColor = true;
      btnHLSLLoad.Click += btnHLSLLoad_Click;
      // 
      // lblVisIntensity
      // 
      lblVisIntensity.Location = new Point(148, 5);
      lblVisIntensity.Name = "lblVisIntensity";
      lblVisIntensity.Size = new Size(58, 23);
      lblVisIntensity.TabIndex = 150;
      lblVisIntensity.Text = "Intensity";
      lblVisIntensity.TextAlign = ContentAlignment.MiddleRight;
      toolTip1.SetToolTip(lblVisIntensity, resources.GetString("lblVisIntensity.ToolTip"));
      lblVisIntensity.Click += lblVisIntensity_Click;
      // 
      // lblVisShift
      // 
      lblVisShift.Location = new Point(155, 34);
      lblVisShift.Name = "lblVisShift";
      lblVisShift.Size = new Size(51, 23);
      lblVisShift.TabIndex = 152;
      lblVisShift.Text = "Shift";
      lblVisShift.TextAlign = ContentAlignment.MiddleRight;
      toolTip1.SetToolTip(lblVisShift, resources.GetString("lblVisShift.ToolTip"));
      lblVisShift.Click += lblVisShift_Click;
      // 
      // lblVisVersion
      // 
      lblVisVersion.Location = new Point(155, 63);
      lblVisVersion.Name = "lblVisVersion";
      lblVisVersion.Size = new Size(51, 23);
      lblVisVersion.TabIndex = 154;
      lblVisVersion.Text = "Version";
      lblVisVersion.TextAlign = ContentAlignment.MiddleRight;
      toolTip1.SetToolTip(lblVisVersion, resources.GetString("lblVisVersion.ToolTip"));
      lblVisVersion.Click += lblVisVersion_Click;
      // 
      // btnMIDIHelp
      // 
      btnMIDIHelp.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      btnMIDIHelp.FlatStyle = FlatStyle.System;
      btnMIDIHelp.Location = new Point(579, 6);
      btnMIDIHelp.Name = "btnMIDIHelp";
      btnMIDIHelp.Size = new Size(23, 23);
      btnMIDIHelp.TabIndex = 150;
      btnMIDIHelp.Text = "?";
      toolTip1.SetToolTip(btnMIDIHelp, "Help");
      btnMIDIHelp.UseVisualStyleBackColor = true;
      btnMIDIHelp.Click += btnMIDIHelp_Click;
      // 
      // txtMidi1Con
      // 
      txtMidi1Con.Location = new Point(258, 38);
      txtMidi1Con.Name = "txtMidi1Con";
      txtMidi1Con.ReadOnly = true;
      txtMidi1Con.Size = new Size(30, 23);
      txtMidi1Con.TabIndex = 8;
      txtMidi1Con.TextAlign = HorizontalAlignment.Center;
      toolTip1.SetToolTip(txtMidi1Con, "Controller");
      // 
      // txtMidi1Val
      // 
      txtMidi1Val.Location = new Point(222, 38);
      txtMidi1Val.Name = "txtMidi1Val";
      txtMidi1Val.ReadOnly = true;
      txtMidi1Val.Size = new Size(30, 23);
      txtMidi1Val.TabIndex = 7;
      txtMidi1Val.TextAlign = HorizontalAlignment.Center;
      toolTip1.SetToolTip(txtMidi1Val, "Value");
      // 
      // txtMidi1Ch
      // 
      txtMidi1Ch.Location = new Point(186, 38);
      txtMidi1Ch.Name = "txtMidi1Ch";
      txtMidi1Ch.ReadOnly = true;
      txtMidi1Ch.Size = new Size(30, 23);
      txtMidi1Ch.TabIndex = 6;
      txtMidi1Ch.TextAlign = HorizontalAlignment.Center;
      toolTip1.SetToolTip(txtMidi1Ch, "Channel");
      // 
      // txtMidi1Label
      // 
      txtMidi1Label.Location = new Point(34, 38);
      txtMidi1Label.Name = "txtMidi1Label";
      txtMidi1Label.Size = new Size(95, 23);
      txtMidi1Label.TabIndex = 4;
      toolTip1.SetToolTip(txtMidi1Label, "Set a custom label");
      txtMidi1Label.TextChanged += txtMidiLabel_TextChanged;
      // 
      // chkMidi1Active
      // 
      chkMidi1Active.Appearance = Appearance.Button;
      chkMidi1Active.FlatStyle = FlatStyle.System;
      chkMidi1Active.Location = new Point(134, 37);
      chkMidi1Active.Margin = new Padding(3, 2, 3, 2);
      chkMidi1Active.Name = "chkMidi1Active";
      chkMidi1Active.Size = new Size(45, 23);
      chkMidi1Active.TabIndex = 5;
      chkMidi1Active.Text = "Active";
      chkMidi1Active.TextAlign = ContentAlignment.MiddleCenter;
      chkMidi1Active.TextImageRelation = TextImageRelation.ImageAboveText;
      toolTip1.SetToolTip(chkMidi1Active, "Action will be triggered when matching MIDI event is received");
      chkMidi1Active.UseVisualStyleBackColor = true;
      chkMidi1Active.CheckedChanged += chkMidiActive_CheckedChanged;
      // 
      // lblMidi1Inc
      // 
      lblMidi1Inc.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      lblMidi1Inc.Location = new Point(538, 36);
      lblMidi1Inc.Name = "lblMidi1Inc";
      lblMidi1Inc.Size = new Size(27, 24);
      lblMidi1Inc.TabIndex = 158;
      lblMidi1Inc.Text = "Inc";
      lblMidi1Inc.TextAlign = ContentAlignment.MiddleRight;
      toolTip1.SetToolTip(lblMidi1Inc, resources.GetString("lblMidi1Inc.ToolTip"));
      // 
      // txtMidi3Label
      // 
      txtMidi3Label.Location = new Point(34, 96);
      txtMidi3Label.Name = "txtMidi3Label";
      txtMidi3Label.Size = new Size(95, 23);
      txtMidi3Label.TabIndex = 20;
      toolTip1.SetToolTip(txtMidi3Label, "Set a custom label");
      // 
      // txtMidi4Label
      // 
      txtMidi4Label.Location = new Point(34, 125);
      txtMidi4Label.Name = "txtMidi4Label";
      txtMidi4Label.Size = new Size(95, 23);
      txtMidi4Label.TabIndex = 28;
      toolTip1.SetToolTip(txtMidi4Label, "Set a custom label");
      // 
      // txtMidi5Label
      // 
      txtMidi5Label.Location = new Point(34, 154);
      txtMidi5Label.Name = "txtMidi5Label";
      txtMidi5Label.Size = new Size(95, 23);
      txtMidi5Label.TabIndex = 36;
      toolTip1.SetToolTip(txtMidi5Label, "Set a custom label");
      // 
      // btnMIDISave
      // 
      btnMIDISave.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      btnMIDISave.FlatStyle = FlatStyle.System;
      btnMIDISave.Location = new Point(551, 6);
      btnMIDISave.Name = "btnMIDISave";
      btnMIDISave.Size = new Size(23, 23);
      btnMIDISave.TabIndex = 176;
      btnMIDISave.Text = "S";
      toolTip1.SetToolTip(btnMIDISave, "Save MIDI settings to midi-remote.json\r\nThis is done automatically when Remote is closed\r\n");
      btnMIDISave.UseVisualStyleBackColor = true;
      btnMIDISave.Click += btnMIDISave_Click;
      // 
      // btnMIDILoad
      // 
      btnMIDILoad.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      btnMIDILoad.FlatStyle = FlatStyle.System;
      btnMIDILoad.Location = new Point(523, 6);
      btnMIDILoad.Name = "btnMIDILoad";
      btnMIDILoad.Size = new Size(23, 23);
      btnMIDILoad.TabIndex = 177;
      btnMIDILoad.Text = "L";
      toolTip1.SetToolTip(btnMIDILoad, "Reload MIDI settings from midi-remote.json\r\nThis is done automatically when Remote is started");
      btnMIDILoad.UseVisualStyleBackColor = true;
      btnMIDILoad.Click += btnMIDILoad_Click;
      // 
      // lblMidi1Action
      // 
      lblMidi1Action.Location = new Point(342, 36);
      lblMidi1Action.Name = "lblMidi1Action";
      lblMidi1Action.Size = new Size(48, 24);
      lblMidi1Action.TabIndex = 157;
      lblMidi1Action.Text = "Action";
      lblMidi1Action.TextAlign = ContentAlignment.MiddleRight;
      toolTip1.SetToolTip(lblMidi1Action, "Actions become selectable as soon as a MIDI control is learned\r\nDifferent actions are available for MIDI buttons/notes and faders/knobs\r\nSee the Milkwave manual for details (press \"?\" button)");
      // 
      // lblSpout
      // 
      lblSpout.Location = new Point(24, 121);
      lblSpout.Name = "lblSpout";
      lblSpout.Size = new Size(45, 23);
      lblSpout.TabIndex = 155;
      lblSpout.Text = "Spout";
      lblSpout.TextAlign = ContentAlignment.MiddleRight;
      toolTip1.SetToolTip(lblSpout, "Values < 1 may slow down rendering of the preset, sprites and notifications\r\nClick: Set 1");
      // 
      // chkSpoutActive
      // 
      chkSpoutActive.Appearance = Appearance.Button;
      chkSpoutActive.FlatStyle = FlatStyle.System;
      chkSpoutActive.Location = new Point(212, 122);
      chkSpoutActive.Name = "chkSpoutActive";
      chkSpoutActive.Size = new Size(56, 23);
      chkSpoutActive.TabIndex = 156;
      chkSpoutActive.Text = "Active";
      chkSpoutActive.TextAlign = ContentAlignment.MiddleCenter;
      chkSpoutActive.TextImageRelation = TextImageRelation.ImageAboveText;
      toolTip1.SetToolTip(chkSpoutActive, "Activate Spout output (F10)");
      chkSpoutActive.UseVisualStyleBackColor = true;
      chkSpoutActive.CheckedChanged += chkSpoutActive_CheckedChanged;
      // 
      // chkSpoutFixedSize
      // 
      chkSpoutFixedSize.Appearance = Appearance.Button;
      chkSpoutFixedSize.FlatStyle = FlatStyle.System;
      chkSpoutFixedSize.Location = new Point(279, 122);
      chkSpoutFixedSize.Name = "chkSpoutFixedSize";
      chkSpoutFixedSize.Size = new Size(56, 23);
      chkSpoutFixedSize.TabIndex = 157;
      chkSpoutFixedSize.Text = "Fixed";
      chkSpoutFixedSize.TextAlign = ContentAlignment.MiddleCenter;
      chkSpoutFixedSize.TextImageRelation = TextImageRelation.ImageAboveText;
      toolTip1.SetToolTip(chkSpoutFixedSize, "Check to force Spout output to this resolution\r\nIf unchecked, output will depend on the Visualizer window size");
      chkSpoutFixedSize.UseVisualStyleBackColor = true;
      chkSpoutFixedSize.CheckedChanged += chkSpoutFixedSize_CheckedChanged;
      // 
      // cboSpoutWidth
      // 
      cboSpoutWidth.Items.AddRange(new object[] { "3840", "2560", "1920", "1600", "1280", "1024", "800", "640", "320" });
      cboSpoutWidth.Location = new Point(75, 122);
      cboSpoutWidth.Name = "cboSpoutWidth";
      cboSpoutWidth.Size = new Size(58, 23);
      cboSpoutWidth.TabIndex = 158;
      toolTip1.SetToolTip(cboSpoutWidth, "Fixed output width");
      cboSpoutWidth.TextUpdate += cboSpoutRes_ValueChanged;
      cboSpoutWidth.SelectedValueChanged += cboSpoutRes_ValueChanged;
      // 
      // cboSpoutHeight
      // 
      cboSpoutHeight.Items.AddRange(new object[] { "2160", "1440", "1080", "900", "720", "600", "480", "240", "200" });
      cboSpoutHeight.Location = new Point(142, 122);
      cboSpoutHeight.Name = "cboSpoutHeight";
      cboSpoutHeight.Size = new Size(58, 23);
      cboSpoutHeight.TabIndex = 159;
      toolTip1.SetToolTip(cboSpoutHeight, "Fixed output height");
      cboSpoutHeight.TextUpdate += cboSpoutRes_ValueChanged;
      cboSpoutHeight.SelectedValueChanged += cboSpoutRes_ValueChanged;
      // 
      // lblQuality
      // 
      lblQuality.Location = new Point(24, 92);
      lblQuality.Name = "lblQuality";
      lblQuality.Size = new Size(45, 23);
      lblQuality.TabIndex = 161;
      lblQuality.Text = "Quality";
      lblQuality.TextAlign = ContentAlignment.MiddleRight;
      toolTip1.SetToolTip(lblQuality, resources.GetString("lblQuality.ToolTip"));
      lblQuality.Click += lblQuality_Click;
      // 
      // lblMidi1Row
      // 
      lblMidi1Row.Location = new Point(8, 36);
      lblMidi1Row.Name = "lblMidi1Row";
      lblMidi1Row.Size = new Size(20, 24);
      lblMidi1Row.TabIndex = 156;
      lblMidi1Row.Text = "1";
      lblMidi1Row.TextAlign = ContentAlignment.MiddleRight;
      toolTip1.SetToolTip(lblMidi1Row, "Double-click: Clear assignment for row");
      lblMidi1Row.DoubleClick += lblMidiRow_DoubleClick;
      // 
      // lblBrightness
      // 
      lblBrightness.Location = new Point(274, 63);
      lblBrightness.Name = "lblBrightness";
      lblBrightness.Size = new Size(74, 23);
      lblBrightness.TabIndex = 169;
      lblBrightness.Text = "Brightness";
      lblBrightness.TextAlign = ContentAlignment.MiddleRight;
      toolTip1.SetToolTip(lblBrightness, "Shift Color Brightness\r\nClick: Set 0\r\nHold Alt while changing value: Set increment to 0.05\r\n");
      lblBrightness.Click += lblValue_Click;
      // 
      // lblSaturation
      // 
      lblSaturation.Location = new Point(279, 34);
      lblSaturation.Name = "lblSaturation";
      lblSaturation.Size = new Size(69, 23);
      lblSaturation.TabIndex = 167;
      lblSaturation.Text = "Saturation";
      lblSaturation.TextAlign = ContentAlignment.MiddleRight;
      toolTip1.SetToolTip(lblSaturation, "Shift Color Saturation\r\nClick: Set 0\r\nHold Alt while changing value: Set increment to 0.05\r\n\r\n");
      lblSaturation.Click += lblSaturation_Click;
      // 
      // lblHue
      // 
      lblHue.Location = new Point(290, 5);
      lblHue.Name = "lblHue";
      lblHue.Size = new Size(58, 23);
      lblHue.TabIndex = 165;
      lblHue.Text = "Hue";
      lblHue.TextAlign = ContentAlignment.MiddleRight;
      toolTip1.SetToolTip(lblHue, "Shift Color Hue\r\nClick: Set 0\r\nHold Alt while changing value: Set increment to 0.05\r\n");
      lblHue.Click += lblHue_Click;
      // 
      // txtFilterPresets
      // 
      txtFilterPresets.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      txtFilterPresets.Location = new Point(380, 7);
      txtFilterPresets.Name = "txtFilterPresets";
      txtFilterPresets.Size = new Size(70, 23);
      txtFilterPresets.TabIndex = 139;
      toolTip1.SetToolTip(txtFilterPresets, "Only show presets containing this text (Ctrl+F)\r\n");
      txtFilterPresets.TextChanged += txtPreset_TextChanged;
      txtFilterPresets.KeyDown += txtFilterPresets_KeyDown;
      // 
      // chkQualityAuto
      // 
      chkQualityAuto.Appearance = Appearance.Button;
      chkQualityAuto.FlatStyle = FlatStyle.System;
      chkQualityAuto.Location = new Point(279, 92);
      chkQualityAuto.Name = "chkQualityAuto";
      chkQualityAuto.Size = new Size(56, 23);
      chkQualityAuto.TabIndex = 172;
      chkQualityAuto.Text = "Auto";
      chkQualityAuto.TextAlign = ContentAlignment.MiddleCenter;
      chkQualityAuto.TextImageRelation = TextImageRelation.ImageAboveText;
      toolTip1.SetToolTip(chkQualityAuto, "Adjust set quality based on window size, so the percieved quality stays the same");
      chkQualityAuto.UseVisualStyleBackColor = true;
      chkQualityAuto.CheckedChanged += chkQualityAuto_CheckedChanged;
      // 
      // btnShadertoyFilesLoadDir
      // 
      btnShadertoyFilesLoadDir.FlatStyle = FlatStyle.System;
      btnShadertoyFilesLoadDir.Location = new Point(5, 5);
      btnShadertoyFilesLoadDir.Name = "btnShadertoyFilesLoadDir";
      btnShadertoyFilesLoadDir.Size = new Size(38, 23);
      btnShadertoyFilesLoadDir.TabIndex = 139;
      btnShadertoyFilesLoadDir.Text = "Dir";
      toolTip1.SetToolTip(btnShadertoyFilesLoadDir, "Open directory containing Shadertoy .json files\r\nRead Milkwave manual for possible sources");
      btnShadertoyFilesLoadDir.UseVisualStyleBackColor = true;
      btnShadertoyFilesLoadDir.Click += btnShadertoyFilesLoad_Click;
      // 
      // txtShadertoyFile
      // 
      txtShadertoyFile.Location = new Point(49, 6);
      txtShadertoyFile.Name = "txtShadertoyFile";
      txtShadertoyFile.ReadOnly = true;
      txtShadertoyFile.Size = new Size(79, 23);
      txtShadertoyFile.TabIndex = 143;
      txtShadertoyFile.TextAlign = HorizontalAlignment.Center;
      toolTip1.SetToolTip(txtShadertoyFile, "Shadertoy ID\r\nMiddle click: Open Shadertoy ID on website");
      txtShadertoyFile.MouseDown += txtShadertoyFile_MouseDown;
      // 
      // btnShadertoyFileLoadNext
      // 
      btnShadertoyFileLoadNext.FlatStyle = FlatStyle.System;
      btnShadertoyFileLoadNext.Location = new Point(257, 5);
      btnShadertoyFileLoadNext.Name = "btnShadertoyFileLoadNext";
      btnShadertoyFileLoadNext.Size = new Size(55, 23);
      btnShadertoyFileLoadNext.TabIndex = 145;
      btnShadertoyFileLoadNext.Text = "Next";
      toolTip1.SetToolTip(btnShadertoyFileLoadNext, "Go to next file, then load that shader\r\nCtrl+Click: Go to previous\r\nAlt-Click: Also open on Shadertoy.com");
      btnShadertoyFileLoadNext.UseVisualStyleBackColor = true;
      btnShadertoyFileLoadNext.Click += btnShadertoyFileLoadNext_Click;
      // 
      // btnShadertoyFileLoadThis
      // 
      btnShadertoyFileLoadThis.FlatStyle = FlatStyle.System;
      btnShadertoyFileLoadThis.Location = new Point(196, 5);
      btnShadertoyFileLoadThis.Name = "btnShadertoyFileLoadThis";
      btnShadertoyFileLoadThis.Size = new Size(55, 23);
      btnShadertoyFileLoadThis.TabIndex = 144;
      btnShadertoyFileLoadThis.Text = "Load";
      toolTip1.SetToolTip(btnShadertoyFileLoadThis, "Load the shader with the displayed ID");
      btnShadertoyFileLoadThis.UseVisualStyleBackColor = true;
      btnShadertoyFileLoadThis.Click += btnShadertoyFileLoadThis_Click;
      // 
      // numShadertoyFileIndex
      // 
      numShadertoyFileIndex.Location = new Point(133, 6);
      numShadertoyFileIndex.Margin = new Padding(3, 2, 3, 2);
      numShadertoyFileIndex.Maximum = new decimal(new int[] { 99999, 0, 0, 0 });
      numShadertoyFileIndex.Minimum = new decimal(new int[] { 1, 0, 0, 0 });
      numShadertoyFileIndex.Name = "numShadertoyFileIndex";
      numShadertoyFileIndex.Size = new Size(56, 23);
      numShadertoyFileIndex.TabIndex = 142;
      numShadertoyFileIndex.TextAlign = HorizontalAlignment.Center;
      toolTip1.SetToolTip(numShadertoyFileIndex, "Directory file index");
      numShadertoyFileIndex.Value = new decimal(new int[] { 1, 0, 0, 0 });
      numShadertoyFileIndex.ValueChanged += numShadertoyFileIndex_ValueChanged;
      // 
      // btnFontGlobalPlus
      // 
      btnFontGlobalPlus.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      btnFontGlobalPlus.FlatStyle = FlatStyle.System;
      btnFontGlobalPlus.Location = new Point(563, 124);
      btnFontGlobalPlus.Name = "btnFontGlobalPlus";
      btnFontGlobalPlus.Size = new Size(38, 22);
      btnFontGlobalPlus.TabIndex = 161;
      btnFontGlobalPlus.Text = "+5";
      toolTip1.SetToolTip(btnFontGlobalPlus, "Increase all, save and preview");
      btnFontGlobalPlus.UseVisualStyleBackColor = true;
      btnFontGlobalPlus.Click += btnFontGlobalPlus_Click;
      // 
      // btnFontGlobalMinus
      // 
      btnFontGlobalMinus.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      btnFontGlobalMinus.FlatStyle = FlatStyle.System;
      btnFontGlobalMinus.Location = new Point(518, 124);
      btnFontGlobalMinus.Name = "btnFontGlobalMinus";
      btnFontGlobalMinus.Size = new Size(38, 22);
      btnFontGlobalMinus.TabIndex = 160;
      btnFontGlobalMinus.Text = "-5";
      toolTip1.SetToolTip(btnFontGlobalMinus, "Decrease all, save and preview");
      btnFontGlobalMinus.UseVisualStyleBackColor = true;
      btnFontGlobalMinus.Click += btnFontGlobalMinus_Click;
      // 
      // numSettingsHueAuto
      // 
      numSettingsHueAuto.DecimalPlaces = 2;
      numSettingsHueAuto.Increment = new decimal(new int[] { 1, 0, 0, 131072 });
      numSettingsHueAuto.Location = new Point(488, 7);
      numSettingsHueAuto.Margin = new Padding(3, 2, 3, 2);
      numSettingsHueAuto.Maximum = new decimal(new int[] { 999, 0, 0, 131072 });
      numSettingsHueAuto.Minimum = new decimal(new int[] { 1, 0, 0, 131072 });
      numSettingsHueAuto.Name = "numSettingsHueAuto";
      numSettingsHueAuto.Size = new Size(56, 23);
      numSettingsHueAuto.TabIndex = 173;
      numSettingsHueAuto.TextAlign = HorizontalAlignment.Center;
      toolTip1.SetToolTip(numSettingsHueAuto, "When Auto is selected: Seconds until Hue is incremented by 0.01\r\n");
      numSettingsHueAuto.Value = new decimal(new int[] { 2, 0, 0, 65536 });
      numSettingsHueAuto.ValueChanged += numSettingsHueAuto_ValueChanged;
      // 
      // chkHueAuto
      // 
      chkHueAuto.Appearance = Appearance.Button;
      chkHueAuto.FlatStyle = FlatStyle.System;
      chkHueAuto.Location = new Point(421, 6);
      chkHueAuto.Name = "chkHueAuto";
      chkHueAuto.Size = new Size(56, 23);
      chkHueAuto.TabIndex = 174;
      chkHueAuto.Text = "Auto";
      chkHueAuto.TextAlign = ContentAlignment.MiddleCenter;
      chkHueAuto.TextImageRelation = TextImageRelation.ImageAboveText;
      toolTip1.SetToolTip(chkHueAuto, "Automatically shift Hue over time");
      chkHueAuto.UseVisualStyleBackColor = true;
      chkHueAuto.CheckedChanged += chkHueAuto_CheckedChanged;
      // 
      // lblChangePreset
      // 
      lblChangePreset.Location = new Point(279, 150);
      lblChangePreset.Name = "lblChangePreset";
      lblChangePreset.Size = new Size(136, 23);
      lblChangePreset.TabIndex = 176;
      lblChangePreset.Text = "Change Preset after";
      lblChangePreset.TextAlign = ContentAlignment.MiddleRight;
      toolTip1.SetToolTip(lblChangePreset, "Next Preset after this many seconds (unless locked)\r\nNote that fBlendTimeAuto and 0..fTimeBetweenPresetsRand are added to determine the actual duration (see settings.ini)");
      // 
      // chkPresetLocked
      // 
      chkPresetLocked.Appearance = Appearance.Button;
      chkPresetLocked.FlatStyle = FlatStyle.System;
      chkPresetLocked.Location = new Point(488, 150);
      chkPresetLocked.Name = "chkPresetLocked";
      chkPresetLocked.Size = new Size(56, 23);
      chkPresetLocked.TabIndex = 177;
      chkPresetLocked.Text = "Locked";
      chkPresetLocked.TextAlign = ContentAlignment.MiddleCenter;
      chkPresetLocked.TextImageRelation = TextImageRelation.ImageAboveText;
      toolTip1.SetToolTip(chkPresetLocked, "Lock/Unlock Preset\r\n(same as pressing ~ in Visualizer)");
      chkPresetLocked.UseVisualStyleBackColor = true;
      chkPresetLocked.CheckedChanged += chkPresetLocked_CheckedChanged;
      // 
      // chkVideoMix
      // 
      chkVideoMix.Appearance = Appearance.Button;
      chkVideoMix.FlatStyle = FlatStyle.System;
      chkVideoMix.Location = new Point(294, 9);
      chkVideoMix.Margin = new Padding(3, 2, 3, 2);
      chkVideoMix.Name = "chkVideoMix";
      chkVideoMix.Size = new Size(70, 23);
      chkVideoMix.TabIndex = 126;
      chkVideoMix.Text = "Mix";
      chkVideoMix.TextAlign = ContentAlignment.MiddleCenter;
      chkVideoMix.TextImageRelation = TextImageRelation.ImageAboveText;
      toolTip1.SetToolTip(chkVideoMix, "Checked: Get tags from (and save to) currently running preset\r\nUnchecked: Get tags from (and save to) selected preset above\r\n");
      chkVideoMix.UseVisualStyleBackColor = true;
      // 
      // cboSettingsOpenFile
      // 
      cboSettingsOpenFile.AutoCompleteMode = AutoCompleteMode.SuggestAppend;
      cboSettingsOpenFile.AutoCompleteSource = AutoCompleteSource.ListItems;
      cboSettingsOpenFile.DropDownStyle = ComboBoxStyle.DropDownList;
      cboSettingsOpenFile.FormattingEnabled = true;
      cboSettingsOpenFile.Items.AddRange(new object[] { "settings.ini", "sprites.ini", "messages.ini", "script-default.txt", "settings-remote.json", "tags-remote.json" });
      cboSettingsOpenFile.Location = new Point(75, 152);
      cboSettingsOpenFile.Name = "cboSettingsOpenFile";
      cboSettingsOpenFile.Size = new Size(125, 23);
      cboSettingsOpenFile.TabIndex = 170;
      // 
      // btn00
      // 
      btn00.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
      btn00.Location = new Point(7, 138);
      btn00.Margin = new Padding(3, 2, 3, 2);
      btn00.Name = "btn00";
      btn00.Size = new Size(55, 41);
      btn00.TabIndex = 12;
      btn00.Text = "00";
      btn00.UseVisualStyleBackColor = true;
      btn00.Click += btn00_Click;
      // 
      // txtShaderGLSL
      // 
      txtShaderGLSL.Dock = DockStyle.Fill;
      txtShaderGLSL.Font = new Font("Consolas", 9F, FontStyle.Regular, GraphicsUnit.Point, 0);
      txtShaderGLSL.Location = new Point(0, 0);
      txtShaderGLSL.Multiline = true;
      txtShaderGLSL.Name = "txtShaderGLSL";
      txtShaderGLSL.ScrollBars = ScrollBars.Both;
      txtShaderGLSL.Size = new Size(286, 114);
      txtShaderGLSL.TabIndex = 28;
      txtShaderGLSL.Text = resources.GetString("txtShaderGLSL.Text");
      txtShaderGLSL.MouseWheel += txtShader_MouseWheel;
      // 
      // txtShaderHLSL
      // 
      txtShaderHLSL.Dock = DockStyle.Fill;
      txtShaderHLSL.Font = new Font("Consolas", 9F, FontStyle.Regular, GraphicsUnit.Point, 0);
      txtShaderHLSL.Location = new Point(0, 0);
      txtShaderHLSL.Multiline = true;
      txtShaderHLSL.Name = "txtShaderHLSL";
      txtShaderHLSL.ScrollBars = ScrollBars.Both;
      txtShaderHLSL.Size = new Size(306, 114);
      txtShaderHLSL.TabIndex = 27;
      txtShaderHLSL.Text = "/*\r\nRight pane: Converted HLSL code\r\n*/";
      txtShaderHLSL.Click += txtShaderSetLineNumber;
      txtShaderHLSL.Enter += txtShaderSetLineNumber;
      txtShaderHLSL.KeyDown += txtShaderHLSL_KeyDown;
      txtShaderHLSL.KeyUp += txtShaderSetLineNumber;
      txtShaderHLSL.MouseWheel += txtShader_MouseWheel;
      txtShaderHLSL.PreviewKeyDown += txtShaderSetLineNumber;
      // 
      // txtMidi2Label
      // 
      txtMidi2Label.Location = new Point(34, 67);
      txtMidi2Label.Name = "txtMidi2Label";
      txtMidi2Label.Size = new Size(95, 23);
      txtMidi2Label.TabIndex = 12;
      txtMidi2Label.TextChanged += txtMidiLabel_TextChanged;
      // 
      // lblMidi2Inc
      // 
      lblMidi2Inc.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      lblMidi2Inc.Location = new Point(538, 65);
      lblMidi2Inc.Name = "lblMidi2Inc";
      lblMidi2Inc.Size = new Size(27, 24);
      lblMidi2Inc.TabIndex = 175;
      lblMidi2Inc.Text = "Inc";
      lblMidi2Inc.TextAlign = ContentAlignment.MiddleRight;
      // 
      // chkMidi2Active
      // 
      chkMidi2Active.Appearance = Appearance.Button;
      chkMidi2Active.FlatStyle = FlatStyle.System;
      chkMidi2Active.Location = new Point(134, 66);
      chkMidi2Active.Margin = new Padding(3, 2, 3, 2);
      chkMidi2Active.Name = "chkMidi2Active";
      chkMidi2Active.Size = new Size(45, 23);
      chkMidi2Active.TabIndex = 13;
      chkMidi2Active.Text = "Active";
      chkMidi2Active.TextAlign = ContentAlignment.MiddleCenter;
      chkMidi2Active.TextImageRelation = TextImageRelation.ImageAboveText;
      chkMidi2Active.UseVisualStyleBackColor = true;
      chkMidi2Active.CheckedChanged += chkMidiActive_CheckedChanged;
      // 
      // txtMidi2Con
      // 
      txtMidi2Con.Location = new Point(258, 67);
      txtMidi2Con.Name = "txtMidi2Con";
      txtMidi2Con.ReadOnly = true;
      txtMidi2Con.Size = new Size(30, 23);
      txtMidi2Con.TabIndex = 16;
      txtMidi2Con.TextAlign = HorizontalAlignment.Center;
      // 
      // txtMidi2Val
      // 
      txtMidi2Val.Location = new Point(222, 67);
      txtMidi2Val.Name = "txtMidi2Val";
      txtMidi2Val.ReadOnly = true;
      txtMidi2Val.Size = new Size(30, 23);
      txtMidi2Val.TabIndex = 15;
      txtMidi2Val.TextAlign = HorizontalAlignment.Center;
      // 
      // txtMidi2Ch
      // 
      txtMidi2Ch.Location = new Point(186, 67);
      txtMidi2Ch.Name = "txtMidi2Ch";
      txtMidi2Ch.ReadOnly = true;
      txtMidi2Ch.Size = new Size(30, 23);
      txtMidi2Ch.TabIndex = 14;
      txtMidi2Ch.TextAlign = HorizontalAlignment.Center;
      // 
      // numMidiBank
      // 
      numMidiBank.Location = new Point(396, 9);
      numMidiBank.Margin = new Padding(3, 2, 3, 2);
      numMidiBank.Maximum = new decimal(new int[] { 10, 0, 0, 0 });
      numMidiBank.Minimum = new decimal(new int[] { 1, 0, 0, 0 });
      numMidiBank.Name = "numMidiBank";
      numMidiBank.Size = new Size(47, 23);
      numMidiBank.TabIndex = 3;
      numMidiBank.TextAlign = HorizontalAlignment.Center;
      numMidiBank.Value = new decimal(new int[] { 1, 0, 0, 0 });
      numMidiBank.ValueChanged += numMidiBank_ValueChanged;
      // 
      // numVisShift
      // 
      numVisShift.DecimalPlaces = 2;
      numVisShift.Increment = new decimal(new int[] { 2, 0, 0, 131072 });
      numVisShift.Location = new Point(212, 36);
      numVisShift.Margin = new Padding(3, 2, 3, 2);
      numVisShift.Maximum = new decimal(new int[] { 99999, 0, 0, 131072 });
      numVisShift.Minimum = new decimal(new int[] { 99999, 0, 0, -2147352576 });
      numVisShift.Name = "numVisShift";
      numVisShift.Size = new Size(56, 23);
      numVisShift.TabIndex = 151;
      numVisShift.TextAlign = HorizontalAlignment.Center;
      numVisShift.ValueChanged += numVisShift_ValueChanged;
      // 
      // numVisIntensity
      // 
      numVisIntensity.DecimalPlaces = 2;
      numVisIntensity.Increment = new decimal(new int[] { 2, 0, 0, 131072 });
      numVisIntensity.Location = new Point(212, 7);
      numVisIntensity.Margin = new Padding(3, 2, 3, 2);
      numVisIntensity.Maximum = new decimal(new int[] { 99999, 0, 0, 131072 });
      numVisIntensity.Name = "numVisIntensity";
      numVisIntensity.Size = new Size(56, 23);
      numVisIntensity.TabIndex = 149;
      numVisIntensity.TextAlign = HorizontalAlignment.Center;
      numVisIntensity.Value = new decimal(new int[] { 1, 0, 0, 0 });
      numVisIntensity.ValueChanged += numVisIntensity_ValueChanged;
      // 
      // chkMidi1Learn
      // 
      chkMidi1Learn.Appearance = Appearance.Button;
      chkMidi1Learn.FlatStyle = FlatStyle.System;
      chkMidi1Learn.Location = new Point(294, 37);
      chkMidi1Learn.Margin = new Padding(3, 2, 3, 2);
      chkMidi1Learn.Name = "chkMidi1Learn";
      chkMidi1Learn.Size = new Size(45, 23);
      chkMidi1Learn.TabIndex = 9;
      chkMidi1Learn.Text = "Learn";
      chkMidi1Learn.TextAlign = ContentAlignment.MiddleCenter;
      chkMidi1Learn.TextImageRelation = TextImageRelation.ImageAboveText;
      chkMidi1Learn.UseVisualStyleBackColor = true;
      chkMidi1Learn.CheckedChanged += chkMidiLearn_CheckedChanged;
      // 
      // cboMidiDevice
      // 
      cboMidiDevice.DropDownStyle = ComboBoxStyle.DropDownList;
      cboMidiDevice.FormattingEnabled = true;
      cboMidiDevice.Location = new Point(56, 9);
      cboMidiDevice.Name = "cboMidiDevice";
      cboMidiDevice.Size = new Size(232, 23);
      cboMidiDevice.TabIndex = 1;
      cboMidiDevice.SelectedIndexChanged += cboMidiDevice_SelectedIndexChanged;
      // 
      // lblMidiDevice
      // 
      lblMidiDevice.Location = new Point(5, 8);
      lblMidiDevice.Name = "lblMidiDevice";
      lblMidiDevice.Size = new Size(48, 24);
      lblMidiDevice.TabIndex = 99;
      lblMidiDevice.Text = "Device";
      lblMidiDevice.TextAlign = ContentAlignment.MiddleRight;
      // 
      // btnMidiDeviceScan
      // 
      btnMidiDeviceScan.FlatStyle = FlatStyle.System;
      btnMidiDeviceScan.Location = new Point(294, 8);
      btnMidiDeviceScan.Name = "btnMidiDeviceScan";
      btnMidiDeviceScan.Size = new Size(45, 23);
      btnMidiDeviceScan.TabIndex = 2;
      btnMidiDeviceScan.Text = "Scan";
      btnMidiDeviceScan.UseVisualStyleBackColor = true;
      btnMidiDeviceScan.Click += btnMidiDeviceScan_Click;
      // 
      // numVisVersion
      // 
      numVisVersion.Location = new Point(212, 65);
      numVisVersion.Margin = new Padding(3, 2, 3, 2);
      numVisVersion.Maximum = new decimal(new int[] { 999, 0, 0, 0 });
      numVisVersion.Minimum = new decimal(new int[] { 1, 0, 0, 0 });
      numVisVersion.Name = "numVisVersion";
      numVisVersion.Size = new Size(56, 23);
      numVisVersion.TabIndex = 153;
      numVisVersion.TextAlign = HorizontalAlignment.Center;
      numVisVersion.Value = new decimal(new int[] { 1, 0, 0, 0 });
      numVisVersion.ValueChanged += numVisVersion_ValueChanged;
      // 
      // cboParameters
      // 
      cboParameters.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
      cboParameters.AutoCompleteMode = AutoCompleteMode.SuggestAppend;
      cboParameters.Location = new Point(74, 95);
      cboParameters.Name = "cboParameters";
      cboParameters.Size = new Size(306, 23);
      cboParameters.TabIndex = 7;
      cboParameters.SelectedIndexChanged += cboParameters_SelectedIndexChanged;
      cboParameters.TextChanged += cboParameters_TextChanged;
      cboParameters.KeyDown += cboParameters_KeyDown;
      // 
      // chkWaveBrighten
      // 
      chkWaveBrighten.Appearance = Appearance.Button;
      chkWaveBrighten.FlatStyle = FlatStyle.System;
      chkWaveBrighten.Location = new Point(25, 94);
      chkWaveBrighten.Name = "chkWaveBrighten";
      chkWaveBrighten.Size = new Size(106, 23);
      chkWaveBrighten.TabIndex = 140;
      chkWaveBrighten.Text = "Brighten";
      chkWaveBrighten.TextAlign = ContentAlignment.MiddleCenter;
      chkWaveBrighten.TextImageRelation = TextImageRelation.ImageAboveText;
      chkWaveBrighten.UseVisualStyleBackColor = true;
      chkWaveBrighten.CheckedChanged += ctrlWave_ValueChanged;
      // 
      // chkWaveDarken
      // 
      chkWaveDarken.Appearance = Appearance.Button;
      chkWaveDarken.FlatStyle = FlatStyle.System;
      chkWaveDarken.Location = new Point(136, 94);
      chkWaveDarken.Name = "chkWaveDarken";
      chkWaveDarken.Size = new Size(106, 23);
      chkWaveDarken.TabIndex = 141;
      chkWaveDarken.Text = "Darken";
      chkWaveDarken.TextAlign = ContentAlignment.MiddleCenter;
      chkWaveDarken.TextImageRelation = TextImageRelation.ImageAboveText;
      chkWaveDarken.UseVisualStyleBackColor = true;
      chkWaveDarken.CheckedChanged += ctrlWave_ValueChanged;
      // 
      // chkWaveSolarize
      // 
      chkWaveSolarize.Appearance = Appearance.Button;
      chkWaveSolarize.FlatStyle = FlatStyle.System;
      chkWaveSolarize.Location = new Point(25, 123);
      chkWaveSolarize.Name = "chkWaveSolarize";
      chkWaveSolarize.Size = new Size(106, 23);
      chkWaveSolarize.TabIndex = 142;
      chkWaveSolarize.Text = "Solarize";
      chkWaveSolarize.TextAlign = ContentAlignment.MiddleCenter;
      chkWaveSolarize.TextImageRelation = TextImageRelation.ImageAboveText;
      chkWaveSolarize.UseVisualStyleBackColor = true;
      chkWaveSolarize.CheckedChanged += ctrlWave_ValueChanged;
      // 
      // chkWaveInvert
      // 
      chkWaveInvert.Appearance = Appearance.Button;
      chkWaveInvert.FlatStyle = FlatStyle.System;
      chkWaveInvert.Location = new Point(136, 123);
      chkWaveInvert.Name = "chkWaveInvert";
      chkWaveInvert.Size = new Size(106, 23);
      chkWaveInvert.TabIndex = 143;
      chkWaveInvert.Text = "Invert";
      chkWaveInvert.TextAlign = ContentAlignment.MiddleCenter;
      chkWaveInvert.TextImageRelation = TextImageRelation.ImageAboveText;
      chkWaveInvert.UseVisualStyleBackColor = true;
      chkWaveInvert.CheckedChanged += ctrlWave_ValueChanged;
      // 
      // chkWaveAdditive
      // 
      chkWaveAdditive.Appearance = Appearance.Button;
      chkWaveAdditive.FlatStyle = FlatStyle.System;
      chkWaveAdditive.Location = new Point(248, 94);
      chkWaveAdditive.Name = "chkWaveAdditive";
      chkWaveAdditive.Size = new Size(106, 23);
      chkWaveAdditive.TabIndex = 144;
      chkWaveAdditive.Text = "Additive";
      chkWaveAdditive.TextAlign = ContentAlignment.MiddleCenter;
      chkWaveAdditive.TextImageRelation = TextImageRelation.ImageAboveText;
      chkWaveAdditive.UseVisualStyleBackColor = true;
      chkWaveAdditive.CheckedChanged += ctrlWave_ValueChanged;
      // 
      // chkWaveDotted
      // 
      chkWaveDotted.Appearance = Appearance.Button;
      chkWaveDotted.FlatStyle = FlatStyle.System;
      chkWaveDotted.Location = new Point(360, 94);
      chkWaveDotted.Name = "chkWaveDotted";
      chkWaveDotted.Size = new Size(106, 23);
      chkWaveDotted.TabIndex = 145;
      chkWaveDotted.Text = "Dotted";
      chkWaveDotted.TextAlign = ContentAlignment.MiddleCenter;
      chkWaveDotted.TextImageRelation = TextImageRelation.ImageAboveText;
      chkWaveDotted.UseVisualStyleBackColor = true;
      chkWaveDotted.CheckedChanged += ctrlWave_ValueChanged;
      // 
      // chkWaveThick
      // 
      chkWaveThick.Appearance = Appearance.Button;
      chkWaveThick.FlatStyle = FlatStyle.System;
      chkWaveThick.Location = new Point(248, 123);
      chkWaveThick.Name = "chkWaveThick";
      chkWaveThick.Size = new Size(106, 23);
      chkWaveThick.TabIndex = 146;
      chkWaveThick.Text = "Thick";
      chkWaveThick.TextAlign = ContentAlignment.MiddleCenter;
      chkWaveThick.TextImageRelation = TextImageRelation.ImageAboveText;
      chkWaveThick.UseVisualStyleBackColor = true;
      chkWaveThick.CheckedChanged += ctrlWave_ValueChanged;
      // 
      // lblRGB
      // 
      lblRGB.Location = new Point(133, 5);
      lblRGB.Name = "lblRGB";
      lblRGB.Size = new Size(48, 24);
      lblRGB.TabIndex = 122;
      lblRGB.Text = "ARGB";
      lblRGB.TextAlign = ContentAlignment.MiddleRight;
      // 
      // numWaveMode
      // 
      numWaveMode.Location = new Point(75, 7);
      numWaveMode.Margin = new Padding(3, 2, 3, 2);
      numWaveMode.Maximum = new decimal(new int[] { 17, 0, 0, 0 });
      numWaveMode.Name = "numWaveMode";
      numWaveMode.Size = new Size(56, 23);
      numWaveMode.TabIndex = 113;
      numWaveMode.TextAlign = HorizontalAlignment.Center;
      numWaveMode.Value = new decimal(new int[] { 7, 0, 0, 0 });
      numWaveMode.ValueChanged += numWavemode_ValueChanged;
      // 
      // colorDialogMessage
      // 
      colorDialogMessage.AnyColor = true;
      colorDialogMessage.Color = Color.White;
      colorDialogMessage.FullOpen = true;
      colorDialogMessage.SolidColorOnly = true;
      // 
      // label2
      // 
      label2.Location = new Point(4, 4);
      label2.Name = "label2";
      label2.Size = new Size(66, 24);
      label2.TabIndex = 90;
      label2.Text = "Message";
      label2.TextAlign = ContentAlignment.MiddleRight;
      // 
      // txtStyle
      // 
      txtStyle.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      txtStyle.Location = new Point(421, 95);
      txtStyle.Margin = new Padding(3, 2, 3, 2);
      txtStyle.Name = "txtStyle";
      txtStyle.Size = new Size(91, 23);
      txtStyle.TabIndex = 8;
      txtStyle.Text = "Style A";
      txtStyle.KeyDown += txtStyle_KeyDown;
      // 
      // tableLayoutPanel1
      // 
      tableLayoutPanel1.ColumnCount = 10;
      tableLayoutPanel1.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 10F));
      tableLayoutPanel1.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 10F));
      tableLayoutPanel1.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 10F));
      tableLayoutPanel1.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 10F));
      tableLayoutPanel1.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 10F));
      tableLayoutPanel1.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 10F));
      tableLayoutPanel1.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 10F));
      tableLayoutPanel1.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 10F));
      tableLayoutPanel1.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 10.2874432F));
      tableLayoutPanel1.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 9.833586F));
      tableLayoutPanel1.Controls.Add(btnSwitchMode, 9, 3);
      tableLayoutPanel1.Controls.Add(btn88, 8, 3);
      tableLayoutPanel1.Controls.Add(btnWatermark, 6, 2);
      tableLayoutPanel1.Controls.Add(btnTransparency, 8, 2);
      tableLayoutPanel1.Controls.Add(btnB, 6, 0);
      tableLayoutPanel1.Controls.Add(btn77, 7, 3);
      tableLayoutPanel1.Controls.Add(btn66, 6, 3);
      tableLayoutPanel1.Controls.Add(btn55, 5, 3);
      tableLayoutPanel1.Controls.Add(btn44, 4, 3);
      tableLayoutPanel1.Controls.Add(btn33, 3, 3);
      tableLayoutPanel1.Controls.Add(btn22, 2, 3);
      tableLayoutPanel1.Controls.Add(btnK, 2, 2);
      tableLayoutPanel1.Controls.Add(btnF2, 0, 1);
      tableLayoutPanel1.Controls.Add(btnN, 4, 0);
      tableLayoutPanel1.Controls.Add(btnAltEnter, 0, 2);
      tableLayoutPanel1.Controls.Add(btnF10, 8, 1);
      tableLayoutPanel1.Controls.Add(btn11, 1, 3);
      tableLayoutPanel1.Controls.Add(btnTilde, 8, 0);
      tableLayoutPanel1.Controls.Add(btn00, 0, 3);
      tableLayoutPanel1.Controls.Add(btnSpace, 0, 0);
      tableLayoutPanel1.Controls.Add(btnF7, 6, 1);
      tableLayoutPanel1.Controls.Add(btnF4, 4, 1);
      tableLayoutPanel1.Controls.Add(btnF3, 2, 1);
      tableLayoutPanel1.Controls.Add(btnBackspace, 2, 0);
      tableLayoutPanel1.Controls.Add(btnDelete, 4, 2);
      tableLayoutPanel1.Dock = DockStyle.Fill;
      tableLayoutPanel1.Location = new Point(0, 0);
      tableLayoutPanel1.Margin = new Padding(3, 2, 3, 2);
      tableLayoutPanel1.Name = "tableLayoutPanel1";
      tableLayoutPanel1.Padding = new Padding(4);
      tableLayoutPanel1.RowCount = 4;
      tableLayoutPanel1.RowStyles.Add(new RowStyle(SizeType.Percent, 25F));
      tableLayoutPanel1.RowStyles.Add(new RowStyle(SizeType.Percent, 25F));
      tableLayoutPanel1.RowStyles.Add(new RowStyle(SizeType.Percent, 25F));
      tableLayoutPanel1.RowStyles.Add(new RowStyle(SizeType.Percent, 25F));
      tableLayoutPanel1.RowStyles.Add(new RowStyle(SizeType.Absolute, 20F));
      tableLayoutPanel1.Size = new Size(619, 185);
      tableLayoutPanel1.TabIndex = 34;
      // 
      // btnSwitchMode
      // 
      btnSwitchMode.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
      btnSwitchMode.Location = new Point(557, 138);
      btnSwitchMode.Margin = new Padding(3, 2, 3, 2);
      btnSwitchMode.Name = "btnSwitchMode";
      btnSwitchMode.Size = new Size(55, 41);
      btnSwitchMode.TabIndex = 24;
      btnSwitchMode.Text = "SW";
      btnSwitchMode.Click += btnSwitchMode_Click;
      // 
      // btn88
      // 
      btn88.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
      btn88.Location = new Point(495, 138);
      btn88.Margin = new Padding(3, 2, 3, 2);
      btn88.Name = "btn88";
      btn88.Size = new Size(56, 41);
      btn88.TabIndex = 23;
      btn88.Text = "88";
      btn88.UseVisualStyleBackColor = true;
      btn88.Click += btn88_Click;
      // 
      // btnTransparency
      // 
      tableLayoutPanel1.SetColumnSpan(btnTransparency, 2);
      btnTransparency.Dock = DockStyle.Fill;
      btnTransparency.Location = new Point(495, 94);
      btnTransparency.Margin = new Padding(3, 2, 3, 2);
      btnTransparency.Name = "btnTransparency";
      btnTransparency.Size = new Size(117, 40);
      btnTransparency.TabIndex = 21;
      btnTransparency.Text = "Transparency\r\n(F12)";
      btnTransparency.UseVisualStyleBackColor = true;
      btnTransparency.Click += btnTransparency_Click;
      // 
      // btnB
      // 
      tableLayoutPanel1.SetColumnSpan(btnB, 2);
      btnB.Dock = DockStyle.Fill;
      btnB.Location = new Point(373, 6);
      btnB.Margin = new Padding(3, 2, 3, 2);
      btnB.Name = "btnB";
      btnB.Size = new Size(116, 40);
      btnB.TabIndex = 20;
      btnB.Text = "Song Info\r\n(B)";
      btnB.UseVisualStyleBackColor = true;
      btnB.Click += btnB_Click;
      // 
      // btn77
      // 
      btn77.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
      btn77.Location = new Point(434, 138);
      btn77.Margin = new Padding(3, 2, 3, 2);
      btn77.Name = "btn77";
      btn77.Size = new Size(55, 41);
      btn77.TabIndex = 19;
      btn77.Text = "77";
      btn77.UseVisualStyleBackColor = true;
      btn77.Click += btn77_Click;
      // 
      // btn66
      // 
      btn66.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
      btn66.Location = new Point(373, 138);
      btn66.Margin = new Padding(3, 2, 3, 2);
      btn66.Name = "btn66";
      btn66.Size = new Size(55, 41);
      btn66.TabIndex = 18;
      btn66.Text = "66";
      btn66.UseVisualStyleBackColor = true;
      btn66.Click += btn66_Click;
      // 
      // btn55
      // 
      btn55.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
      btn55.Location = new Point(312, 138);
      btn55.Margin = new Padding(3, 2, 3, 2);
      btn55.Name = "btn55";
      btn55.Size = new Size(55, 41);
      btn55.TabIndex = 17;
      btn55.Text = "55";
      btn55.UseVisualStyleBackColor = true;
      btn55.Click += btn55_Click;
      // 
      // btn44
      // 
      btn44.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
      btn44.Location = new Point(251, 138);
      btn44.Margin = new Padding(3, 2, 3, 2);
      btn44.Name = "btn44";
      btn44.Size = new Size(55, 41);
      btn44.TabIndex = 16;
      btn44.Text = "44";
      btn44.UseVisualStyleBackColor = true;
      btn44.Click += btn44_Click;
      // 
      // btn33
      // 
      btn33.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
      btn33.Location = new Point(190, 138);
      btn33.Margin = new Padding(3, 2, 3, 2);
      btn33.Name = "btn33";
      btn33.Size = new Size(55, 41);
      btn33.TabIndex = 15;
      btn33.Text = "33";
      btn33.UseVisualStyleBackColor = true;
      btn33.Click += btn33_Click;
      // 
      // btn22
      // 
      btn22.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
      btn22.Location = new Point(129, 138);
      btn22.Margin = new Padding(3, 2, 3, 2);
      btn22.Name = "btn22";
      btn22.Size = new Size(55, 41);
      btn22.TabIndex = 14;
      btn22.Text = "22";
      btn22.UseVisualStyleBackColor = true;
      btn22.Click += btn22_Click;
      // 
      // btnK
      // 
      tableLayoutPanel1.SetColumnSpan(btnK, 2);
      btnK.Dock = DockStyle.Fill;
      btnK.Location = new Point(129, 94);
      btnK.Margin = new Padding(3, 2, 3, 2);
      btnK.Name = "btnK";
      btnK.Size = new Size(116, 40);
      btnK.TabIndex = 9;
      btnK.Text = "Sprite/Msg Mode\r\n(K)";
      btnK.UseVisualStyleBackColor = true;
      btnK.Click += btnK_Click;
      // 
      // btnF2
      // 
      tableLayoutPanel1.SetColumnSpan(btnF2, 2);
      btnF2.Dock = DockStyle.Fill;
      btnF2.Location = new Point(7, 50);
      btnF2.Margin = new Padding(3, 2, 3, 2);
      btnF2.Name = "btnF2";
      btnF2.Size = new Size(116, 40);
      btnF2.TabIndex = 4;
      btnF2.Text = "Borderless \r\n(F2)";
      btnF2.UseVisualStyleBackColor = true;
      btnF2.Click += btnF2_Click;
      // 
      // btnN
      // 
      tableLayoutPanel1.SetColumnSpan(btnN, 2);
      btnN.Dock = DockStyle.Fill;
      btnN.Location = new Point(251, 6);
      btnN.Margin = new Padding(3, 2, 3, 2);
      btnN.Name = "btnN";
      btnN.Size = new Size(116, 40);
      btnN.TabIndex = 2;
      btnN.Text = "Sound Info\r\n(N)";
      btnN.UseVisualStyleBackColor = true;
      btnN.Click += btnN_Click;
      // 
      // btnAltEnter
      // 
      tableLayoutPanel1.SetColumnSpan(btnAltEnter, 2);
      btnAltEnter.Dock = DockStyle.Fill;
      btnAltEnter.Location = new Point(7, 94);
      btnAltEnter.Margin = new Padding(3, 2, 3, 2);
      btnAltEnter.Name = "btnAltEnter";
      btnAltEnter.Size = new Size(116, 40);
      btnAltEnter.TabIndex = 8;
      btnAltEnter.Text = "Fullscreen\r\n(Alt+Enter)";
      btnAltEnter.UseVisualStyleBackColor = true;
      btnAltEnter.Click += btnAltEnter_Click;
      // 
      // btnF10
      // 
      tableLayoutPanel1.SetColumnSpan(btnF10, 2);
      btnF10.Dock = DockStyle.Fill;
      btnF10.Location = new Point(495, 50);
      btnF10.Margin = new Padding(3, 2, 3, 2);
      btnF10.Name = "btnF10";
      btnF10.Size = new Size(117, 40);
      btnF10.TabIndex = 10;
      btnF10.Text = "Toggle Spout\r\n(F10)";
      btnF10.UseVisualStyleBackColor = true;
      btnF10.Click += btnF10_Click;
      // 
      // btn11
      // 
      btn11.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
      btn11.Location = new Point(68, 138);
      btn11.Margin = new Padding(3, 2, 3, 2);
      btn11.Name = "btn11";
      btn11.Size = new Size(55, 41);
      btn11.TabIndex = 13;
      btn11.Text = "11";
      btn11.UseVisualStyleBackColor = true;
      btn11.Click += btn11_Click;
      // 
      // btnTilde
      // 
      tableLayoutPanel1.SetColumnSpan(btnTilde, 2);
      btnTilde.Dock = DockStyle.Fill;
      btnTilde.Location = new Point(495, 6);
      btnTilde.Margin = new Padding(3, 2, 3, 2);
      btnTilde.Name = "btnTilde";
      btnTilde.Size = new Size(117, 40);
      btnTilde.TabIndex = 3;
      btnTilde.Text = "Preset Lock \r\n(~)";
      btnTilde.UseVisualStyleBackColor = true;
      btnTilde.Click += btnTilde_Click;
      // 
      // btnF7
      // 
      tableLayoutPanel1.SetColumnSpan(btnF7, 2);
      btnF7.Dock = DockStyle.Fill;
      btnF7.Location = new Point(373, 50);
      btnF7.Margin = new Padding(3, 2, 3, 2);
      btnF7.Name = "btnF7";
      btnF7.Size = new Size(116, 40);
      btnF7.TabIndex = 7;
      btnF7.Text = "Always On Top\r\n(F7)";
      btnF7.UseVisualStyleBackColor = true;
      btnF7.Click += btnF7_Click;
      // 
      // btnF4
      // 
      tableLayoutPanel1.SetColumnSpan(btnF4, 2);
      btnF4.Dock = DockStyle.Fill;
      btnF4.Location = new Point(251, 50);
      btnF4.Margin = new Padding(3, 2, 3, 2);
      btnF4.Name = "btnF4";
      btnF4.Size = new Size(116, 40);
      btnF4.TabIndex = 6;
      btnF4.Text = "Preset Info\r\n(F4)";
      btnF4.UseVisualStyleBackColor = true;
      btnF4.Click += btnF4_Click;
      // 
      // btnF3
      // 
      tableLayoutPanel1.SetColumnSpan(btnF3, 2);
      btnF3.Dock = DockStyle.Fill;
      btnF3.Location = new Point(129, 50);
      btnF3.Margin = new Padding(3, 2, 3, 2);
      btnF3.Name = "btnF3";
      btnF3.Size = new Size(116, 40);
      btnF3.TabIndex = 5;
      btnF3.Text = "Change FPS\r\n(F3)";
      btnF3.UseVisualStyleBackColor = true;
      btnF3.Click += btnF3_Click;
      // 
      // btnDelete
      // 
      tableLayoutPanel1.SetColumnSpan(btnDelete, 2);
      btnDelete.Dock = DockStyle.Fill;
      btnDelete.Location = new Point(251, 94);
      btnDelete.Margin = new Padding(3, 2, 3, 2);
      btnDelete.Name = "btnDelete";
      btnDelete.Size = new Size(116, 40);
      btnDelete.TabIndex = 11;
      btnDelete.Text = "Clear Sprite/Msg\r\n(Delete)";
      btnDelete.UseVisualStyleBackColor = true;
      btnDelete.Click += btnDelete_Click;
      // 
      // colorDialogWave
      // 
      colorDialogWave.AnyColor = true;
      colorDialogWave.Color = Color.White;
      colorDialogWave.FullOpen = true;
      colorDialogWave.SolidColorOnly = true;
      // 
      // splitContainer1
      // 
      splitContainer1.Dock = DockStyle.Fill;
      splitContainer1.Location = new Point(0, 0);
      splitContainer1.Margin = new Padding(0);
      splitContainer1.Name = "splitContainer1";
      splitContainer1.Orientation = Orientation.Horizontal;
      // 
      // splitContainer1.Panel1
      // 
      splitContainer1.Panel1.Controls.Add(tabControl);
      // 
      // splitContainer1.Panel2
      // 
      splitContainer1.Panel2.Controls.Add(tableLayoutPanel1);
      splitContainer1.Size = new Size(619, 401);
      splitContainer1.SplitterDistance = 211;
      splitContainer1.SplitterWidth = 5;
      splitContainer1.TabIndex = 115;
      // 
      // tabControl
      // 
      tabControl.Appearance = TabAppearance.Buttons;
      tabControl.BorderColor = SystemColors.ControlLightLight;
      tabControl.Controls.Add(tabPreset);
      tabControl.Controls.Add(tabMessage);
      tabControl.Controls.Add(tabWave);
      tabControl.Controls.Add(tabInput);
      tabControl.Controls.Add(tabMidi);
      tabControl.Controls.Add(tabSettings);
      tabControl.Controls.Add(tabFonts);
      tabControl.Controls.Add(tabShader);
      tabControl.Dock = DockStyle.Fill;
      tabControl.DrawMode = TabDrawMode.OwnerDrawFixed;
      tabControl.ItemSize = new Size(73, 20);
      tabControl.LineColor = SystemColors.ControlDark;
      tabControl.Location = new Point(0, 0);
      tabControl.Margin = new Padding(0);
      tabControl.Name = "tabControl";
      tabControl.Padding = new Point(2, 0);
      tabControl.SelectedForeColor = SystemColors.ControlText;
      tabControl.SelectedIndex = 0;
      tabControl.SelectTabColor = Color.LightGray;
      tabControl.ShowTabCloseButton = false;
      tabControl.Size = new Size(619, 211);
      tabControl.SizeMode = TabSizeMode.Fixed;
      tabControl.TabCloseColor = SystemColors.ControlText;
      tabControl.TabColor = SystemColors.ControlLight;
      tabControl.TabIndex = 0;
      // 
      // tabPreset
      // 
      tabPreset.BackColor = SystemColors.ControlLight;
      tabPreset.BorderStyle = BorderStyle.FixedSingle;
      tabPreset.Controls.Add(lblMostUsed);
      tabPreset.Controls.Add(lblTags);
      tabPreset.Controls.Add(lblLoad);
      tabPreset.Controls.Add(lblPreset);
      tabPreset.Controls.Add(lblAudioDevice);
      tabPreset.Controls.Add(lblCurrentPreset);
      tabPreset.Controls.Add(txtFilterPresets);
      tabPreset.Controls.Add(txtFilterTags);
      tabPreset.Controls.Add(cboTagsFilter);
      tabPreset.Controls.Add(btnTagsSave);
      tabPreset.Controls.Add(btnTag10);
      tabPreset.Controls.Add(btnTag9);
      tabPreset.Controls.Add(btnTag8);
      tabPreset.Controls.Add(btnTag7);
      tabPreset.Controls.Add(btnTag6);
      tabPreset.Controls.Add(btnTag5);
      tabPreset.Controls.Add(btnTag4);
      tabPreset.Controls.Add(btnTag2);
      tabPreset.Controls.Add(btnTag3);
      tabPreset.Controls.Add(btnTag1);
      tabPreset.Controls.Add(chkTagsFromRunning);
      tabPreset.Controls.Add(txtTags);
      tabPreset.Controls.Add(btnPresetLoadTags);
      tabPreset.Controls.Add(chkPresetRandom);
      tabPreset.Controls.Add(chkPresetLink);
      tabPreset.Controls.Add(btnSetAudioDevice);
      tabPreset.Controls.Add(cboAudioDevice);
      tabPreset.Controls.Add(numAmpRight);
      tabPreset.Controls.Add(numAmpLeft);
      tabPreset.Controls.Add(btnPresetLoadDirectory);
      tabPreset.Controls.Add(chkAmpLinked);
      tabPreset.Controls.Add(btnPresetSend);
      tabPreset.Controls.Add(btnPresetLoadFile);
      tabPreset.Controls.Add(txtVisRunning);
      tabPreset.Controls.Add(cboPresets);
      tabPreset.Controls.Add(lblAmp);
      tabPreset.Location = new Point(4, 24);
      tabPreset.Margin = new Padding(0);
      tabPreset.Name = "tabPreset";
      tabPreset.Size = new Size(611, 183);
      tabPreset.TabIndex = 1;
      tabPreset.Text = "Preset";
      // 
      // btnTag10
      // 
      btnTag10.FlatStyle = FlatStyle.System;
      btnTag10.Location = new Point(485, 123);
      btnTag10.Name = "btnTag10";
      btnTag10.Size = new Size(40, 23);
      btnTag10.TabIndex = 135;
      btnTag10.UseVisualStyleBackColor = true;
      btnTag10.Click += btnTag_Click;
      // 
      // btnTag9
      // 
      btnTag9.FlatStyle = FlatStyle.System;
      btnTag9.Location = new Point(439, 123);
      btnTag9.Name = "btnTag9";
      btnTag9.Size = new Size(40, 23);
      btnTag9.TabIndex = 134;
      btnTag9.UseVisualStyleBackColor = true;
      btnTag9.Click += btnTag_Click;
      // 
      // btnTag8
      // 
      btnTag8.FlatStyle = FlatStyle.System;
      btnTag8.Location = new Point(393, 123);
      btnTag8.Name = "btnTag8";
      btnTag8.Size = new Size(40, 23);
      btnTag8.TabIndex = 133;
      btnTag8.UseVisualStyleBackColor = true;
      btnTag8.Click += btnTag_Click;
      // 
      // btnTag7
      // 
      btnTag7.FlatStyle = FlatStyle.System;
      btnTag7.Location = new Point(347, 123);
      btnTag7.Name = "btnTag7";
      btnTag7.Size = new Size(40, 23);
      btnTag7.TabIndex = 132;
      btnTag7.UseVisualStyleBackColor = true;
      btnTag7.Click += btnTag_Click;
      // 
      // btnTag6
      // 
      btnTag6.FlatStyle = FlatStyle.System;
      btnTag6.Location = new Point(301, 123);
      btnTag6.Name = "btnTag6";
      btnTag6.Size = new Size(40, 23);
      btnTag6.TabIndex = 131;
      btnTag6.UseVisualStyleBackColor = true;
      btnTag6.Click += btnTag_Click;
      // 
      // btnTag5
      // 
      btnTag5.FlatStyle = FlatStyle.System;
      btnTag5.Location = new Point(255, 123);
      btnTag5.Name = "btnTag5";
      btnTag5.Size = new Size(40, 23);
      btnTag5.TabIndex = 130;
      btnTag5.UseVisualStyleBackColor = true;
      btnTag5.Click += btnTag_Click;
      // 
      // btnTag4
      // 
      btnTag4.FlatStyle = FlatStyle.System;
      btnTag4.Location = new Point(209, 123);
      btnTag4.Name = "btnTag4";
      btnTag4.Size = new Size(40, 23);
      btnTag4.TabIndex = 129;
      btnTag4.UseVisualStyleBackColor = true;
      btnTag4.Click += btnTag_Click;
      // 
      // btnTag2
      // 
      btnTag2.FlatStyle = FlatStyle.System;
      btnTag2.Location = new Point(117, 123);
      btnTag2.Name = "btnTag2";
      btnTag2.Size = new Size(40, 23);
      btnTag2.TabIndex = 128;
      btnTag2.UseVisualStyleBackColor = true;
      btnTag2.Click += btnTag_Click;
      // 
      // btnTag3
      // 
      btnTag3.FlatStyle = FlatStyle.System;
      btnTag3.Location = new Point(163, 123);
      btnTag3.Name = "btnTag3";
      btnTag3.Size = new Size(40, 23);
      btnTag3.TabIndex = 127;
      btnTag3.UseVisualStyleBackColor = true;
      btnTag3.Click += btnTag_Click;
      // 
      // btnTag1
      // 
      btnTag1.FlatStyle = FlatStyle.System;
      btnTag1.Location = new Point(71, 123);
      btnTag1.Name = "btnTag1";
      btnTag1.Size = new Size(40, 23);
      btnTag1.TabIndex = 126;
      btnTag1.UseVisualStyleBackColor = true;
      btnTag1.Click += btnTag_Click;
      // 
      // tabMessage
      // 
      tabMessage.BackColor = SystemColors.ControlLight;
      tabMessage.BorderStyle = BorderStyle.FixedSingle;
      tabMessage.Controls.Add(cboWindowTitle);
      tabMessage.Controls.Add(cboAutoplay);
      tabMessage.Controls.Add(btnSendFile);
      tabMessage.Controls.Add(btnFontAppend);
      tabMessage.Controls.Add(btnSaveParam);
      tabMessage.Controls.Add(numOpacity);
      tabMessage.Controls.Add(chkAutoplay);
      tabMessage.Controls.Add(cboFonts);
      tabMessage.Controls.Add(numBeats);
      tabMessage.Controls.Add(btnAppendColor);
      tabMessage.Controls.Add(cboParameters);
      tabMessage.Controls.Add(pnlColorMessage);
      tabMessage.Controls.Add(chkFileRandom);
      tabMessage.Controls.Add(numBPM);
      tabMessage.Controls.Add(btnSend);
      tabMessage.Controls.Add(txtStyle);
      tabMessage.Controls.Add(txtMessage);
      tabMessage.Controls.Add(numWrap);
      tabMessage.Controls.Add(chkWrap);
      tabMessage.Controls.Add(numSize);
      tabMessage.Controls.Add(chkPreview);
      tabMessage.Controls.Add(btnAppendSize);
      tabMessage.Controls.Add(label2);
      tabMessage.Controls.Add(lblPercent);
      tabMessage.Controls.Add(lblFont);
      tabMessage.Controls.Add(label7);
      tabMessage.Controls.Add(lblFromFile);
      tabMessage.Controls.Add(lblColor);
      tabMessage.Controls.Add(lblParameters);
      tabMessage.Controls.Add(lblWindow);
      tabMessage.Controls.Add(lblBPM);
      tabMessage.Controls.Add(lblStyle);
      tabMessage.Controls.Add(lblSize);
      tabMessage.Location = new Point(4, 24);
      tabMessage.Margin = new Padding(0);
      tabMessage.Name = "tabMessage";
      tabMessage.Size = new Size(611, 183);
      tabMessage.TabIndex = 0;
      tabMessage.Text = "Message";
      // 
      // tabWave
      // 
      tabWave.BackColor = SystemColors.ControlLight;
      tabWave.BorderStyle = BorderStyle.FixedSingle;
      tabWave.Controls.Add(pnlColorWave);
      tabWave.Controls.Add(numWaveEcho);
      tabWave.Controls.Add(lblEcho);
      tabWave.Controls.Add(numWaveScale);
      tabWave.Controls.Add(lblScale);
      tabWave.Controls.Add(chkWaveVolAlpha);
      tabWave.Controls.Add(chkWaveThick);
      tabWave.Controls.Add(chkWaveDotted);
      tabWave.Controls.Add(chkWaveAdditive);
      tabWave.Controls.Add(chkWaveInvert);
      tabWave.Controls.Add(chkWaveSolarize);
      tabWave.Controls.Add(chkWaveDarken);
      tabWave.Controls.Add(chkWaveBrighten);
      tabWave.Controls.Add(btnWaveQuicksave);
      tabWave.Controls.Add(numWaveDecay);
      tabWave.Controls.Add(numWaveWarp);
      tabWave.Controls.Add(numWaveRotation);
      tabWave.Controls.Add(numWaveZoom);
      tabWave.Controls.Add(numWavePushY);
      tabWave.Controls.Add(numWavePushX);
      tabWave.Controls.Add(btnWaveClear);
      tabWave.Controls.Add(numWaveG);
      tabWave.Controls.Add(numWaveB);
      tabWave.Controls.Add(numWaveR);
      tabWave.Controls.Add(chkWaveLink);
      tabWave.Controls.Add(numWaveMode);
      tabWave.Controls.Add(btnSendWave);
      tabWave.Controls.Add(numWaveAlpha);
      tabWave.Controls.Add(lblDecay);
      tabWave.Controls.Add(lblWarp);
      tabWave.Controls.Add(lblRotation);
      tabWave.Controls.Add(lblZoom);
      tabWave.Controls.Add(lblPushY);
      tabWave.Controls.Add(lblPushX);
      tabWave.Controls.Add(lblRGB);
      tabWave.Controls.Add(lblWavemode);
      tabWave.Location = new Point(4, 24);
      tabWave.Margin = new Padding(0);
      tabWave.Name = "tabWave";
      tabWave.Size = new Size(611, 183);
      tabWave.TabIndex = 2;
      tabWave.Text = "Wave";
      // 
      // numWaveEcho
      // 
      numWaveEcho.DecimalPlaces = 3;
      numWaveEcho.Increment = new decimal(new int[] { 5, 0, 0, 196608 });
      numWaveEcho.Location = new Point(410, 68);
      numWaveEcho.Margin = new Padding(3, 2, 3, 2);
      numWaveEcho.Maximum = new decimal(new int[] { 9999, 0, 0, 196608 });
      numWaveEcho.Name = "numWaveEcho";
      numWaveEcho.Size = new Size(56, 23);
      numWaveEcho.TabIndex = 150;
      numWaveEcho.TextAlign = HorizontalAlignment.Center;
      numWaveEcho.Value = new decimal(new int[] { 1, 0, 0, 0 });
      numWaveEcho.ValueChanged += ctrlWave_ValueChanged;
      // 
      // numWaveScale
      // 
      numWaveScale.DecimalPlaces = 3;
      numWaveScale.Increment = new decimal(new int[] { 5, 0, 0, 196608 });
      numWaveScale.Location = new Point(410, 37);
      numWaveScale.Margin = new Padding(3, 2, 3, 2);
      numWaveScale.Maximum = new decimal(new int[] { 9999, 0, 0, 196608 });
      numWaveScale.Name = "numWaveScale";
      numWaveScale.Size = new Size(56, 23);
      numWaveScale.TabIndex = 148;
      numWaveScale.TextAlign = HorizontalAlignment.Center;
      numWaveScale.Value = new decimal(new int[] { 1, 0, 0, 0 });
      numWaveScale.ValueChanged += ctrlWave_ValueChanged;
      // 
      // numWaveDecay
      // 
      numWaveDecay.DecimalPlaces = 3;
      numWaveDecay.Increment = new decimal(new int[] { 5, 0, 0, 196608 });
      numWaveDecay.Location = new Point(186, 67);
      numWaveDecay.Margin = new Padding(3, 2, 3, 2);
      numWaveDecay.Maximum = new decimal(new int[] { 1, 0, 0, 0 });
      numWaveDecay.Name = "numWaveDecay";
      numWaveDecay.Size = new Size(56, 23);
      numWaveDecay.TabIndex = 137;
      numWaveDecay.TextAlign = HorizontalAlignment.Center;
      numWaveDecay.ValueChanged += ctrlWave_ValueChanged;
      // 
      // numWaveWarp
      // 
      numWaveWarp.DecimalPlaces = 3;
      numWaveWarp.Increment = new decimal(new int[] { 5, 0, 0, 196608 });
      numWaveWarp.Location = new Point(298, 67);
      numWaveWarp.Margin = new Padding(3, 2, 3, 2);
      numWaveWarp.Maximum = new decimal(new int[] { 9999, 0, 0, 196608 });
      numWaveWarp.Name = "numWaveWarp";
      numWaveWarp.Size = new Size(56, 23);
      numWaveWarp.TabIndex = 135;
      numWaveWarp.TextAlign = HorizontalAlignment.Center;
      numWaveWarp.ValueChanged += ctrlWave_ValueChanged;
      numWaveWarp.KeyDown += numWave_KeyDown;
      // 
      // numWaveRotation
      // 
      numWaveRotation.DecimalPlaces = 3;
      numWaveRotation.Increment = new decimal(new int[] { 5, 0, 0, 196608 });
      numWaveRotation.Location = new Point(75, 67);
      numWaveRotation.Margin = new Padding(3, 2, 3, 2);
      numWaveRotation.Maximum = new decimal(new int[] { 9999, 0, 0, 196608 });
      numWaveRotation.Minimum = new decimal(new int[] { 9999, 0, 0, -2147287040 });
      numWaveRotation.Name = "numWaveRotation";
      numWaveRotation.Size = new Size(56, 23);
      numWaveRotation.TabIndex = 133;
      numWaveRotation.TextAlign = HorizontalAlignment.Center;
      numWaveRotation.ValueChanged += ctrlWave_ValueChanged;
      numWaveRotation.KeyDown += numWave_KeyDown;
      // 
      // numWaveZoom
      // 
      numWaveZoom.DecimalPlaces = 3;
      numWaveZoom.Increment = new decimal(new int[] { 5, 0, 0, 196608 });
      numWaveZoom.Location = new Point(298, 37);
      numWaveZoom.Margin = new Padding(3, 2, 3, 2);
      numWaveZoom.Maximum = new decimal(new int[] { 9999, 0, 0, 196608 });
      numWaveZoom.Name = "numWaveZoom";
      numWaveZoom.Size = new Size(56, 23);
      numWaveZoom.TabIndex = 131;
      numWaveZoom.TextAlign = HorizontalAlignment.Center;
      numWaveZoom.Value = new decimal(new int[] { 1, 0, 0, 0 });
      numWaveZoom.ValueChanged += ctrlWave_ValueChanged;
      numWaveZoom.KeyDown += numWave_KeyDown;
      // 
      // numWavePushY
      // 
      numWavePushY.DecimalPlaces = 3;
      numWavePushY.Increment = new decimal(new int[] { 5, 0, 0, 196608 });
      numWavePushY.Location = new Point(186, 37);
      numWavePushY.Margin = new Padding(3, 2, 3, 2);
      numWavePushY.Maximum = new decimal(new int[] { 1, 0, 0, 0 });
      numWavePushY.Minimum = new decimal(new int[] { 1, 0, 0, int.MinValue });
      numWavePushY.Name = "numWavePushY";
      numWavePushY.Size = new Size(56, 23);
      numWavePushY.TabIndex = 129;
      numWavePushY.TextAlign = HorizontalAlignment.Center;
      numWavePushY.ValueChanged += ctrlWave_ValueChanged;
      numWavePushY.KeyDown += numWave_KeyDown;
      // 
      // numWavePushX
      // 
      numWavePushX.DecimalPlaces = 3;
      numWavePushX.Increment = new decimal(new int[] { 5, 0, 0, 196608 });
      numWavePushX.Location = new Point(75, 37);
      numWavePushX.Margin = new Padding(3, 2, 3, 2);
      numWavePushX.Maximum = new decimal(new int[] { 1, 0, 0, 0 });
      numWavePushX.Minimum = new decimal(new int[] { 1, 0, 0, int.MinValue });
      numWavePushX.Name = "numWavePushX";
      numWavePushX.Size = new Size(56, 23);
      numWavePushX.TabIndex = 127;
      numWavePushX.TextAlign = HorizontalAlignment.Center;
      numWavePushX.ValueChanged += ctrlWave_ValueChanged;
      numWavePushX.KeyDown += numWave_KeyDown;
      // 
      // tabInput
      // 
      tabInput.BackColor = SystemColors.ControlLight;
      tabInput.BorderStyle = BorderStyle.FixedSingle;
      tabInput.Controls.Add(chkVideoMix);
      tabInput.Controls.Add(cboVideoInput);
      tabInput.Controls.Add(label10);
      tabInput.Location = new Point(4, 24);
      tabInput.Margin = new Padding(0);
      tabInput.Name = "tabInput";
      tabInput.Size = new Size(611, 183);
      tabInput.TabIndex = 7;
      tabInput.Text = "Input";
      // 
      // cboVideoInput
      // 
      cboVideoInput.DropDownStyle = ComboBoxStyle.DropDownList;
      cboVideoInput.FormattingEnabled = true;
      cboVideoInput.Location = new Point(56, 9);
      cboVideoInput.Name = "cboVideoInput";
      cboVideoInput.Size = new Size(232, 23);
      cboVideoInput.TabIndex = 101;
      // 
      // label10
      // 
      label10.Location = new Point(5, 8);
      label10.Name = "label10";
      label10.Size = new Size(48, 24);
      label10.TabIndex = 100;
      label10.Text = "Device";
      label10.TextAlign = ContentAlignment.MiddleRight;
      // 
      // tabMidi
      // 
      tabMidi.BackColor = SystemColors.ControlLight;
      tabMidi.BorderStyle = BorderStyle.FixedSingle;
      tabMidi.Controls.Add(label6);
      tabMidi.Controls.Add(label8);
      tabMidi.Controls.Add(cboMidi5Action);
      tabMidi.Controls.Add(chkMidi5Active);
      tabMidi.Controls.Add(txtMidi5Inc);
      tabMidi.Controls.Add(txtMidi5Con);
      tabMidi.Controls.Add(txtMidi5Val);
      tabMidi.Controls.Add(txtMidi5Ch);
      tabMidi.Controls.Add(chkMidi5Learn);
      tabMidi.Controls.Add(label4);
      tabMidi.Controls.Add(label5);
      tabMidi.Controls.Add(cboMidi4Action);
      tabMidi.Controls.Add(chkMidi4Active);
      tabMidi.Controls.Add(txtMidi4Inc);
      tabMidi.Controls.Add(txtMidi4Con);
      tabMidi.Controls.Add(txtMidi4Val);
      tabMidi.Controls.Add(txtMidi4Ch);
      tabMidi.Controls.Add(chkMidi4Learn);
      tabMidi.Controls.Add(label1);
      tabMidi.Controls.Add(label3);
      tabMidi.Controls.Add(cboMidi3Action);
      tabMidi.Controls.Add(chkMidi3Active);
      tabMidi.Controls.Add(txtMidi3Inc);
      tabMidi.Controls.Add(txtMidi3Con);
      tabMidi.Controls.Add(txtMidi3Val);
      tabMidi.Controls.Add(txtMidi3Ch);
      tabMidi.Controls.Add(chkMidi3Learn);
      tabMidi.Controls.Add(btnMIDILoad);
      tabMidi.Controls.Add(btnMIDISave);
      tabMidi.Controls.Add(lblMidi2Inc);
      tabMidi.Controls.Add(lblMidi2Action);
      tabMidi.Controls.Add(cboMidi2Action);
      tabMidi.Controls.Add(chkMidi2Active);
      tabMidi.Controls.Add(txtMidi2Inc);
      tabMidi.Controls.Add(txtMidi2Con);
      tabMidi.Controls.Add(txtMidi2Val);
      tabMidi.Controls.Add(txtMidi2Ch);
      tabMidi.Controls.Add(chkMidi2Learn);
      tabMidi.Controls.Add(lblMidi5Row);
      tabMidi.Controls.Add(txtMidi5Label);
      tabMidi.Controls.Add(lblMidi4Row);
      tabMidi.Controls.Add(txtMidi4Label);
      tabMidi.Controls.Add(lblMidi3Row);
      tabMidi.Controls.Add(txtMidi3Label);
      tabMidi.Controls.Add(lblMidi2Row);
      tabMidi.Controls.Add(txtMidi2Label);
      tabMidi.Controls.Add(lblMidi1Inc);
      tabMidi.Controls.Add(lblMidi1Action);
      tabMidi.Controls.Add(lblMidi1Row);
      tabMidi.Controls.Add(lblBank);
      tabMidi.Controls.Add(numMidiBank);
      tabMidi.Controls.Add(cboMidi1Action);
      tabMidi.Controls.Add(btnMIDIHelp);
      tabMidi.Controls.Add(txtMidi1Label);
      tabMidi.Controls.Add(chkMidi1Active);
      tabMidi.Controls.Add(txtMidi1Inc);
      tabMidi.Controls.Add(txtMidi1Con);
      tabMidi.Controls.Add(txtMidi1Val);
      tabMidi.Controls.Add(txtMidi1Ch);
      tabMidi.Controls.Add(btnMidiDeviceScan);
      tabMidi.Controls.Add(lblMidiDevice);
      tabMidi.Controls.Add(cboMidiDevice);
      tabMidi.Controls.Add(chkMidi1Learn);
      tabMidi.Location = new Point(4, 24);
      tabMidi.Margin = new Padding(0);
      tabMidi.Name = "tabMidi";
      tabMidi.Size = new Size(611, 183);
      tabMidi.TabIndex = 6;
      tabMidi.Text = "MIDI";
      // 
      // label6
      // 
      label6.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      label6.Location = new Point(538, 152);
      label6.Name = "label6";
      label6.Size = new Size(27, 24);
      label6.TabIndex = 204;
      label6.Text = "Inc";
      label6.TextAlign = ContentAlignment.MiddleRight;
      // 
      // label8
      // 
      label8.Location = new Point(342, 152);
      label8.Name = "label8";
      label8.Size = new Size(48, 24);
      label8.TabIndex = 203;
      label8.Text = "Action";
      label8.TextAlign = ContentAlignment.MiddleRight;
      // 
      // cboMidi5Action
      // 
      cboMidi5Action.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
      cboMidi5Action.AutoCompleteMode = AutoCompleteMode.SuggestAppend;
      cboMidi5Action.Location = new Point(395, 154);
      cboMidi5Action.Name = "cboMidi5Action";
      cboMidi5Action.Size = new Size(143, 23);
      cboMidi5Action.TabIndex = 42;
      cboMidi5Action.TextUpdate += cboMidiAction_SelectedValueChanged;
      cboMidi5Action.SelectedValueChanged += cboMidiAction_SelectedValueChanged;
      // 
      // chkMidi5Active
      // 
      chkMidi5Active.Appearance = Appearance.Button;
      chkMidi5Active.FlatStyle = FlatStyle.System;
      chkMidi5Active.Location = new Point(134, 153);
      chkMidi5Active.Margin = new Padding(3, 2, 3, 2);
      chkMidi5Active.Name = "chkMidi5Active";
      chkMidi5Active.Size = new Size(45, 23);
      chkMidi5Active.TabIndex = 37;
      chkMidi5Active.Text = "Active";
      chkMidi5Active.TextAlign = ContentAlignment.MiddleCenter;
      chkMidi5Active.TextImageRelation = TextImageRelation.ImageAboveText;
      chkMidi5Active.UseVisualStyleBackColor = true;
      chkMidi5Active.CheckStateChanged += chkMidiActive_CheckedChanged;
      // 
      // txtMidi5Inc
      // 
      txtMidi5Inc.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      txtMidi5Inc.Location = new Point(571, 154);
      txtMidi5Inc.Name = "txtMidi5Inc";
      txtMidi5Inc.Size = new Size(30, 23);
      txtMidi5Inc.TabIndex = 43;
      txtMidi5Inc.TextAlign = HorizontalAlignment.Center;
      txtMidi5Inc.TextChanged += txtMidiInc_TextChanged;
      // 
      // txtMidi5Con
      // 
      txtMidi5Con.Location = new Point(258, 154);
      txtMidi5Con.Name = "txtMidi5Con";
      txtMidi5Con.ReadOnly = true;
      txtMidi5Con.Size = new Size(30, 23);
      txtMidi5Con.TabIndex = 40;
      txtMidi5Con.TextAlign = HorizontalAlignment.Center;
      // 
      // txtMidi5Val
      // 
      txtMidi5Val.Location = new Point(222, 154);
      txtMidi5Val.Name = "txtMidi5Val";
      txtMidi5Val.ReadOnly = true;
      txtMidi5Val.Size = new Size(30, 23);
      txtMidi5Val.TabIndex = 39;
      txtMidi5Val.TextAlign = HorizontalAlignment.Center;
      // 
      // txtMidi5Ch
      // 
      txtMidi5Ch.Location = new Point(186, 154);
      txtMidi5Ch.Name = "txtMidi5Ch";
      txtMidi5Ch.ReadOnly = true;
      txtMidi5Ch.Size = new Size(30, 23);
      txtMidi5Ch.TabIndex = 38;
      txtMidi5Ch.TextAlign = HorizontalAlignment.Center;
      // 
      // chkMidi5Learn
      // 
      chkMidi5Learn.Appearance = Appearance.Button;
      chkMidi5Learn.FlatStyle = FlatStyle.System;
      chkMidi5Learn.Location = new Point(294, 153);
      chkMidi5Learn.Margin = new Padding(3, 2, 3, 2);
      chkMidi5Learn.Name = "chkMidi5Learn";
      chkMidi5Learn.Size = new Size(45, 23);
      chkMidi5Learn.TabIndex = 41;
      chkMidi5Learn.Text = "Learn";
      chkMidi5Learn.TextAlign = ContentAlignment.MiddleCenter;
      chkMidi5Learn.TextImageRelation = TextImageRelation.ImageAboveText;
      chkMidi5Learn.UseVisualStyleBackColor = true;
      chkMidi5Learn.CheckedChanged += chkMidiLearn_CheckedChanged;
      // 
      // label4
      // 
      label4.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      label4.Location = new Point(538, 123);
      label4.Name = "label4";
      label4.Size = new Size(27, 24);
      label4.TabIndex = 195;
      label4.Text = "Inc";
      label4.TextAlign = ContentAlignment.MiddleRight;
      // 
      // label5
      // 
      label5.Location = new Point(342, 123);
      label5.Name = "label5";
      label5.Size = new Size(48, 24);
      label5.TabIndex = 194;
      label5.Text = "Action";
      label5.TextAlign = ContentAlignment.MiddleRight;
      // 
      // cboMidi4Action
      // 
      cboMidi4Action.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
      cboMidi4Action.AutoCompleteMode = AutoCompleteMode.SuggestAppend;
      cboMidi4Action.Location = new Point(395, 125);
      cboMidi4Action.Name = "cboMidi4Action";
      cboMidi4Action.Size = new Size(143, 23);
      cboMidi4Action.TabIndex = 34;
      cboMidi4Action.TextUpdate += cboMidiAction_SelectedValueChanged;
      cboMidi4Action.SelectedValueChanged += cboMidiAction_SelectedValueChanged;
      // 
      // chkMidi4Active
      // 
      chkMidi4Active.Appearance = Appearance.Button;
      chkMidi4Active.FlatStyle = FlatStyle.System;
      chkMidi4Active.Location = new Point(134, 124);
      chkMidi4Active.Margin = new Padding(3, 2, 3, 2);
      chkMidi4Active.Name = "chkMidi4Active";
      chkMidi4Active.Size = new Size(45, 23);
      chkMidi4Active.TabIndex = 29;
      chkMidi4Active.Text = "Active";
      chkMidi4Active.TextAlign = ContentAlignment.MiddleCenter;
      chkMidi4Active.TextImageRelation = TextImageRelation.ImageAboveText;
      chkMidi4Active.UseVisualStyleBackColor = true;
      chkMidi4Active.CheckedChanged += chkMidiActive_CheckedChanged;
      // 
      // txtMidi4Inc
      // 
      txtMidi4Inc.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      txtMidi4Inc.Location = new Point(571, 125);
      txtMidi4Inc.Name = "txtMidi4Inc";
      txtMidi4Inc.Size = new Size(30, 23);
      txtMidi4Inc.TabIndex = 35;
      txtMidi4Inc.TextAlign = HorizontalAlignment.Center;
      txtMidi4Inc.TextChanged += txtMidiInc_TextChanged;
      // 
      // txtMidi4Con
      // 
      txtMidi4Con.Location = new Point(258, 125);
      txtMidi4Con.Name = "txtMidi4Con";
      txtMidi4Con.ReadOnly = true;
      txtMidi4Con.Size = new Size(30, 23);
      txtMidi4Con.TabIndex = 32;
      txtMidi4Con.TextAlign = HorizontalAlignment.Center;
      // 
      // txtMidi4Val
      // 
      txtMidi4Val.Location = new Point(222, 125);
      txtMidi4Val.Name = "txtMidi4Val";
      txtMidi4Val.ReadOnly = true;
      txtMidi4Val.Size = new Size(30, 23);
      txtMidi4Val.TabIndex = 31;
      txtMidi4Val.TextAlign = HorizontalAlignment.Center;
      // 
      // txtMidi4Ch
      // 
      txtMidi4Ch.Location = new Point(186, 125);
      txtMidi4Ch.Name = "txtMidi4Ch";
      txtMidi4Ch.ReadOnly = true;
      txtMidi4Ch.Size = new Size(30, 23);
      txtMidi4Ch.TabIndex = 30;
      txtMidi4Ch.TextAlign = HorizontalAlignment.Center;
      // 
      // chkMidi4Learn
      // 
      chkMidi4Learn.Appearance = Appearance.Button;
      chkMidi4Learn.FlatStyle = FlatStyle.System;
      chkMidi4Learn.Location = new Point(294, 124);
      chkMidi4Learn.Margin = new Padding(3, 2, 3, 2);
      chkMidi4Learn.Name = "chkMidi4Learn";
      chkMidi4Learn.Size = new Size(45, 23);
      chkMidi4Learn.TabIndex = 33;
      chkMidi4Learn.Text = "Learn";
      chkMidi4Learn.TextAlign = ContentAlignment.MiddleCenter;
      chkMidi4Learn.TextImageRelation = TextImageRelation.ImageAboveText;
      chkMidi4Learn.UseVisualStyleBackColor = true;
      chkMidi4Learn.CheckedChanged += chkMidiLearn_CheckedChanged;
      // 
      // label1
      // 
      label1.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      label1.Location = new Point(538, 94);
      label1.Name = "label1";
      label1.Size = new Size(27, 24);
      label1.TabIndex = 186;
      label1.Text = "Inc";
      label1.TextAlign = ContentAlignment.MiddleRight;
      // 
      // label3
      // 
      label3.Location = new Point(342, 94);
      label3.Name = "label3";
      label3.Size = new Size(48, 24);
      label3.TabIndex = 185;
      label3.Text = "Action";
      label3.TextAlign = ContentAlignment.MiddleRight;
      // 
      // cboMidi3Action
      // 
      cboMidi3Action.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
      cboMidi3Action.AutoCompleteMode = AutoCompleteMode.SuggestAppend;
      cboMidi3Action.Location = new Point(395, 96);
      cboMidi3Action.Name = "cboMidi3Action";
      cboMidi3Action.Size = new Size(143, 23);
      cboMidi3Action.TabIndex = 26;
      cboMidi3Action.TextUpdate += cboMidiAction_SelectedValueChanged;
      cboMidi3Action.SelectedValueChanged += cboMidiAction_SelectedValueChanged;
      // 
      // chkMidi3Active
      // 
      chkMidi3Active.Appearance = Appearance.Button;
      chkMidi3Active.FlatStyle = FlatStyle.System;
      chkMidi3Active.Location = new Point(134, 95);
      chkMidi3Active.Margin = new Padding(3, 2, 3, 2);
      chkMidi3Active.Name = "chkMidi3Active";
      chkMidi3Active.Size = new Size(45, 23);
      chkMidi3Active.TabIndex = 21;
      chkMidi3Active.Text = "Active";
      chkMidi3Active.TextAlign = ContentAlignment.MiddleCenter;
      chkMidi3Active.TextImageRelation = TextImageRelation.ImageAboveText;
      chkMidi3Active.UseVisualStyleBackColor = true;
      chkMidi3Active.CheckedChanged += chkMidiActive_CheckedChanged;
      // 
      // txtMidi3Inc
      // 
      txtMidi3Inc.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      txtMidi3Inc.Location = new Point(571, 96);
      txtMidi3Inc.Name = "txtMidi3Inc";
      txtMidi3Inc.Size = new Size(30, 23);
      txtMidi3Inc.TabIndex = 27;
      txtMidi3Inc.TextAlign = HorizontalAlignment.Center;
      txtMidi3Inc.TextChanged += txtMidiInc_TextChanged;
      // 
      // txtMidi3Con
      // 
      txtMidi3Con.Location = new Point(258, 96);
      txtMidi3Con.Name = "txtMidi3Con";
      txtMidi3Con.ReadOnly = true;
      txtMidi3Con.Size = new Size(30, 23);
      txtMidi3Con.TabIndex = 24;
      txtMidi3Con.TextAlign = HorizontalAlignment.Center;
      // 
      // txtMidi3Val
      // 
      txtMidi3Val.Location = new Point(222, 96);
      txtMidi3Val.Name = "txtMidi3Val";
      txtMidi3Val.ReadOnly = true;
      txtMidi3Val.Size = new Size(30, 23);
      txtMidi3Val.TabIndex = 23;
      txtMidi3Val.TextAlign = HorizontalAlignment.Center;
      // 
      // txtMidi3Ch
      // 
      txtMidi3Ch.Location = new Point(186, 96);
      txtMidi3Ch.Name = "txtMidi3Ch";
      txtMidi3Ch.ReadOnly = true;
      txtMidi3Ch.Size = new Size(30, 23);
      txtMidi3Ch.TabIndex = 22;
      txtMidi3Ch.TextAlign = HorizontalAlignment.Center;
      // 
      // chkMidi3Learn
      // 
      chkMidi3Learn.Appearance = Appearance.Button;
      chkMidi3Learn.FlatStyle = FlatStyle.System;
      chkMidi3Learn.Location = new Point(294, 95);
      chkMidi3Learn.Margin = new Padding(3, 2, 3, 2);
      chkMidi3Learn.Name = "chkMidi3Learn";
      chkMidi3Learn.Size = new Size(45, 23);
      chkMidi3Learn.TabIndex = 25;
      chkMidi3Learn.Text = "Learn";
      chkMidi3Learn.TextAlign = ContentAlignment.MiddleCenter;
      chkMidi3Learn.TextImageRelation = TextImageRelation.ImageAboveText;
      chkMidi3Learn.UseVisualStyleBackColor = true;
      chkMidi3Learn.CheckedChanged += chkMidiLearn_CheckedChanged;
      // 
      // lblMidi2Action
      // 
      lblMidi2Action.Location = new Point(342, 65);
      lblMidi2Action.Name = "lblMidi2Action";
      lblMidi2Action.Size = new Size(48, 24);
      lblMidi2Action.TabIndex = 174;
      lblMidi2Action.Text = "Action";
      lblMidi2Action.TextAlign = ContentAlignment.MiddleRight;
      // 
      // cboMidi2Action
      // 
      cboMidi2Action.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
      cboMidi2Action.AutoCompleteMode = AutoCompleteMode.SuggestAppend;
      cboMidi2Action.Location = new Point(395, 67);
      cboMidi2Action.Name = "cboMidi2Action";
      cboMidi2Action.Size = new Size(143, 23);
      cboMidi2Action.TabIndex = 18;
      cboMidi2Action.TextUpdate += cboMidiAction_SelectedValueChanged;
      cboMidi2Action.SelectedValueChanged += cboMidiAction_SelectedValueChanged;
      // 
      // txtMidi2Inc
      // 
      txtMidi2Inc.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      txtMidi2Inc.Location = new Point(571, 67);
      txtMidi2Inc.Name = "txtMidi2Inc";
      txtMidi2Inc.Size = new Size(30, 23);
      txtMidi2Inc.TabIndex = 19;
      txtMidi2Inc.TextAlign = HorizontalAlignment.Center;
      txtMidi2Inc.TextChanged += txtMidiInc_TextChanged;
      // 
      // chkMidi2Learn
      // 
      chkMidi2Learn.Appearance = Appearance.Button;
      chkMidi2Learn.FlatStyle = FlatStyle.System;
      chkMidi2Learn.Location = new Point(294, 66);
      chkMidi2Learn.Margin = new Padding(3, 2, 3, 2);
      chkMidi2Learn.Name = "chkMidi2Learn";
      chkMidi2Learn.Size = new Size(45, 23);
      chkMidi2Learn.TabIndex = 17;
      chkMidi2Learn.Text = "Learn";
      chkMidi2Learn.TextAlign = ContentAlignment.MiddleCenter;
      chkMidi2Learn.TextImageRelation = TextImageRelation.ImageAboveText;
      chkMidi2Learn.UseVisualStyleBackColor = true;
      chkMidi2Learn.CheckedChanged += chkMidiLearn_CheckedChanged;
      // 
      // lblMidi5Row
      // 
      lblMidi5Row.Location = new Point(8, 152);
      lblMidi5Row.Name = "lblMidi5Row";
      lblMidi5Row.Size = new Size(20, 24);
      lblMidi5Row.TabIndex = 166;
      lblMidi5Row.Text = "5";
      lblMidi5Row.TextAlign = ContentAlignment.MiddleRight;
      lblMidi5Row.DoubleClick += lblMidiRow_DoubleClick;
      // 
      // lblMidi4Row
      // 
      lblMidi4Row.Location = new Point(8, 123);
      lblMidi4Row.Name = "lblMidi4Row";
      lblMidi4Row.Size = new Size(20, 24);
      lblMidi4Row.TabIndex = 164;
      lblMidi4Row.Text = "4";
      lblMidi4Row.TextAlign = ContentAlignment.MiddleRight;
      lblMidi4Row.DoubleClick += lblMidiRow_DoubleClick;
      // 
      // lblMidi3Row
      // 
      lblMidi3Row.Location = new Point(8, 94);
      lblMidi3Row.Name = "lblMidi3Row";
      lblMidi3Row.Size = new Size(20, 24);
      lblMidi3Row.TabIndex = 162;
      lblMidi3Row.Text = "3";
      lblMidi3Row.TextAlign = ContentAlignment.MiddleRight;
      lblMidi3Row.DoubleClick += lblMidiRow_DoubleClick;
      // 
      // lblMidi2Row
      // 
      lblMidi2Row.Location = new Point(8, 65);
      lblMidi2Row.Name = "lblMidi2Row";
      lblMidi2Row.Size = new Size(20, 24);
      lblMidi2Row.TabIndex = 160;
      lblMidi2Row.Text = "2";
      lblMidi2Row.TextAlign = ContentAlignment.MiddleRight;
      lblMidi2Row.DoubleClick += lblMidiRow_DoubleClick;
      // 
      // lblBank
      // 
      lblBank.Location = new Point(342, 7);
      lblBank.Name = "lblBank";
      lblBank.Size = new Size(48, 24);
      lblBank.TabIndex = 155;
      lblBank.Text = "Bank";
      lblBank.TextAlign = ContentAlignment.MiddleRight;
      // 
      // cboMidi1Action
      // 
      cboMidi1Action.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
      cboMidi1Action.Location = new Point(395, 38);
      cboMidi1Action.Name = "cboMidi1Action";
      cboMidi1Action.Size = new Size(143, 23);
      cboMidi1Action.TabIndex = 10;
      cboMidi1Action.TextUpdate += cboMidiAction_SelectedValueChanged;
      cboMidi1Action.SelectedValueChanged += cboMidiAction_SelectedValueChanged;
      // 
      // txtMidi1Inc
      // 
      txtMidi1Inc.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      txtMidi1Inc.Location = new Point(571, 38);
      txtMidi1Inc.Name = "txtMidi1Inc";
      txtMidi1Inc.Size = new Size(30, 23);
      txtMidi1Inc.TabIndex = 11;
      txtMidi1Inc.TextAlign = HorizontalAlignment.Center;
      txtMidi1Inc.TextChanged += txtMidiInc_TextChanged;
      // 
      // tabSettings
      // 
      tabSettings.BackColor = SystemColors.ControlLight;
      tabSettings.BorderStyle = BorderStyle.FixedSingle;
      tabSettings.Controls.Add(chkPresetLocked);
      tabSettings.Controls.Add(numPresetChange);
      tabSettings.Controls.Add(lblChangePreset);
      tabSettings.Controls.Add(chkHueAuto);
      tabSettings.Controls.Add(numSettingsHueAuto);
      tabSettings.Controls.Add(chkQualityAuto);
      tabSettings.Controls.Add(label9);
      tabSettings.Controls.Add(cboSettingsOpenFile);
      tabSettings.Controls.Add(numSettingsBrightness);
      tabSettings.Controls.Add(lblBrightness);
      tabSettings.Controls.Add(numSettingsSaturation);
      tabSettings.Controls.Add(lblSaturation);
      tabSettings.Controls.Add(numSettingsHue);
      tabSettings.Controls.Add(lblHue);
      tabSettings.Controls.Add(btnQualityDouble);
      tabSettings.Controls.Add(btnQualityHalf);
      tabSettings.Controls.Add(numQuality);
      tabSettings.Controls.Add(lblQuality);
      tabSettings.Controls.Add(cboSpoutHeight);
      tabSettings.Controls.Add(cboSpoutWidth);
      tabSettings.Controls.Add(chkSpoutFixedSize);
      tabSettings.Controls.Add(chkSpoutActive);
      tabSettings.Controls.Add(lblSpout);
      tabSettings.Controls.Add(numVisVersion);
      tabSettings.Controls.Add(lblVisVersion);
      tabSettings.Controls.Add(numVisShift);
      tabSettings.Controls.Add(lblVisShift);
      tabSettings.Controls.Add(numVisIntensity);
      tabSettings.Controls.Add(lblVisIntensity);
      tabSettings.Controls.Add(btnOpenSettingsFile);
      tabSettings.Controls.Add(numFactorFPS);
      tabSettings.Controls.Add(lblFactorFPS);
      tabSettings.Controls.Add(numFactorFrame);
      tabSettings.Controls.Add(numFactorTime);
      tabSettings.Controls.Add(lblFactorFrame);
      tabSettings.Controls.Add(lblFactorTime);
      tabSettings.Location = new Point(4, 24);
      tabSettings.Margin = new Padding(0);
      tabSettings.Name = "tabSettings";
      tabSettings.Size = new Size(611, 183);
      tabSettings.TabIndex = 4;
      tabSettings.Text = "Settings";
      // 
      // numPresetChange
      // 
      numPresetChange.DecimalPlaces = 1;
      numPresetChange.Increment = new decimal(new int[] { 5, 0, 0, 0 });
      numPresetChange.Location = new Point(421, 151);
      numPresetChange.Margin = new Padding(3, 2, 3, 2);
      numPresetChange.Maximum = new decimal(new int[] { 999, 0, 0, 0 });
      numPresetChange.Minimum = new decimal(new int[] { 1, 0, 0, 65536 });
      numPresetChange.Name = "numPresetChange";
      numPresetChange.Size = new Size(56, 23);
      numPresetChange.TabIndex = 175;
      numPresetChange.TextAlign = HorizontalAlignment.Center;
      numPresetChange.Value = new decimal(new int[] { 60, 0, 0, 0 });
      numPresetChange.ValueChanged += numPresetChange_ValueChanged;
      // 
      // label9
      // 
      label9.Location = new Point(24, 150);
      label9.Name = "label9";
      label9.Size = new Size(45, 23);
      label9.TabIndex = 171;
      label9.Text = "File";
      label9.TextAlign = ContentAlignment.MiddleRight;
      // 
      // numSettingsBrightness
      // 
      numSettingsBrightness.DecimalPlaces = 2;
      numSettingsBrightness.Increment = new decimal(new int[] { 2, 0, 0, 131072 });
      numSettingsBrightness.Location = new Point(354, 65);
      numSettingsBrightness.Margin = new Padding(3, 2, 3, 2);
      numSettingsBrightness.Maximum = new decimal(new int[] { 1, 0, 0, 0 });
      numSettingsBrightness.Minimum = new decimal(new int[] { 1, 0, 0, int.MinValue });
      numSettingsBrightness.Name = "numSettingsBrightness";
      numSettingsBrightness.Size = new Size(56, 23);
      numSettingsBrightness.TabIndex = 168;
      numSettingsBrightness.TextAlign = HorizontalAlignment.Center;
      numSettingsBrightness.ValueChanged += numSettingsBrightness_ValueChanged;
      // 
      // numSettingsSaturation
      // 
      numSettingsSaturation.DecimalPlaces = 2;
      numSettingsSaturation.Increment = new decimal(new int[] { 2, 0, 0, 131072 });
      numSettingsSaturation.Location = new Point(354, 36);
      numSettingsSaturation.Margin = new Padding(3, 2, 3, 2);
      numSettingsSaturation.Maximum = new decimal(new int[] { 1, 0, 0, 0 });
      numSettingsSaturation.Minimum = new decimal(new int[] { 1, 0, 0, int.MinValue });
      numSettingsSaturation.Name = "numSettingsSaturation";
      numSettingsSaturation.Size = new Size(56, 23);
      numSettingsSaturation.TabIndex = 166;
      numSettingsSaturation.TextAlign = HorizontalAlignment.Center;
      numSettingsSaturation.ValueChanged += numSettingsSaturation_ValueChanged;
      // 
      // numSettingsHue
      // 
      numSettingsHue.DecimalPlaces = 2;
      numSettingsHue.Increment = new decimal(new int[] { 2, 0, 0, 131072 });
      numSettingsHue.Location = new Point(354, 7);
      numSettingsHue.Margin = new Padding(3, 2, 3, 2);
      numSettingsHue.Maximum = new decimal(new int[] { 1, 0, 0, 0 });
      numSettingsHue.Minimum = new decimal(new int[] { 1, 0, 0, int.MinValue });
      numSettingsHue.Name = "numSettingsHue";
      numSettingsHue.Size = new Size(56, 23);
      numSettingsHue.TabIndex = 164;
      numSettingsHue.TextAlign = HorizontalAlignment.Center;
      numSettingsHue.ValueChanged += numSettingsHue_ValueChanged;
      // 
      // btnQualityDouble
      // 
      btnQualityDouble.Location = new Point(212, 92);
      btnQualityDouble.Name = "btnQualityDouble";
      btnQualityDouble.Size = new Size(56, 23);
      btnQualityDouble.TabIndex = 163;
      btnQualityDouble.Text = "Double";
      btnQualityDouble.UseVisualStyleBackColor = true;
      btnQualityDouble.Click += btnQualityDouble_Click;
      // 
      // btnQualityHalf
      // 
      btnQualityHalf.Location = new Point(142, 92);
      btnQualityHalf.Name = "btnQualityHalf";
      btnQualityHalf.Size = new Size(58, 23);
      btnQualityHalf.TabIndex = 162;
      btnQualityHalf.Text = "Half";
      btnQualityHalf.UseVisualStyleBackColor = true;
      btnQualityHalf.Click += btnQualityHalf_Click;
      // 
      // numQuality
      // 
      numQuality.DecimalPlaces = 2;
      numQuality.Increment = new decimal(new int[] { 2, 0, 0, 131072 });
      numQuality.Location = new Point(75, 93);
      numQuality.Margin = new Padding(3, 2, 3, 2);
      numQuality.Maximum = new decimal(new int[] { 1, 0, 0, 0 });
      numQuality.Minimum = new decimal(new int[] { 1, 0, 0, 131072 });
      numQuality.Name = "numQuality";
      numQuality.Size = new Size(56, 23);
      numQuality.TabIndex = 160;
      numQuality.TextAlign = HorizontalAlignment.Center;
      numQuality.Value = new decimal(new int[] { 1, 0, 0, 0 });
      numQuality.ValueChanged += numQuality_ValueChanged;
      // 
      // numFactorFPS
      // 
      numFactorFPS.DecimalPlaces = 2;
      numFactorFPS.Increment = new decimal(new int[] { 1, 0, 0, 131072 });
      numFactorFPS.Location = new Point(75, 36);
      numFactorFPS.Margin = new Padding(3, 2, 3, 2);
      numFactorFPS.Maximum = new decimal(new int[] { 99999, 0, 0, 131072 });
      numFactorFPS.Name = "numFactorFPS";
      numFactorFPS.Size = new Size(56, 23);
      numFactorFPS.TabIndex = 141;
      numFactorFPS.TextAlign = HorizontalAlignment.Center;
      numFactorFPS.Value = new decimal(new int[] { 1, 0, 0, 0 });
      numFactorFPS.ValueChanged += numFactorFPS_ValueChanged;
      // 
      // numFactorFrame
      // 
      numFactorFrame.Location = new Point(75, 65);
      numFactorFrame.Margin = new Padding(3, 2, 3, 2);
      numFactorFrame.Maximum = new decimal(new int[] { 99999, 0, 0, 131072 });
      numFactorFrame.Name = "numFactorFrame";
      numFactorFrame.Size = new Size(56, 23);
      numFactorFrame.TabIndex = 139;
      numFactorFrame.TextAlign = HorizontalAlignment.Center;
      numFactorFrame.Value = new decimal(new int[] { 1, 0, 0, 0 });
      numFactorFrame.ValueChanged += munFactorFrame_ValueChanged;
      // 
      // numFactorTime
      // 
      numFactorTime.DecimalPlaces = 2;
      numFactorTime.Increment = new decimal(new int[] { 1, 0, 0, 131072 });
      numFactorTime.Location = new Point(75, 7);
      numFactorTime.Margin = new Padding(3, 2, 3, 2);
      numFactorTime.Maximum = new decimal(new int[] { 99999, 0, 0, 131072 });
      numFactorTime.Name = "numFactorTime";
      numFactorTime.Size = new Size(56, 23);
      numFactorTime.TabIndex = 137;
      numFactorTime.TextAlign = HorizontalAlignment.Center;
      numFactorTime.Value = new decimal(new int[] { 1, 0, 0, 0 });
      numFactorTime.ValueChanged += numFactorTime_ValueChanged;
      // 
      // tabFonts
      // 
      tabFonts.BackColor = SystemColors.ControlLight;
      tabFonts.BorderStyle = BorderStyle.FixedSingle;
      tabFonts.Controls.Add(btnFontGlobalPlus);
      tabFonts.Controls.Add(btnFontGlobalMinus);
      tabFonts.Controls.Add(btnTestFonts);
      tabFonts.Controls.Add(chkFontAA5);
      tabFonts.Controls.Add(chkFontItalic5);
      tabFonts.Controls.Add(chkFontBold5);
      tabFonts.Controls.Add(lblFont5);
      tabFonts.Controls.Add(numFont5);
      tabFonts.Controls.Add(cboFont5);
      tabFonts.Controls.Add(pnlColorFont5);
      tabFonts.Controls.Add(chkFontAA4);
      tabFonts.Controls.Add(chkFontItalic4);
      tabFonts.Controls.Add(chkFontBold4);
      tabFonts.Controls.Add(lblFont4);
      tabFonts.Controls.Add(numFont4);
      tabFonts.Controls.Add(cboFont4);
      tabFonts.Controls.Add(pnlColorFont4);
      tabFonts.Controls.Add(chkFontAA3);
      tabFonts.Controls.Add(chkFontItalic3);
      tabFonts.Controls.Add(chkFontBold3);
      tabFonts.Controls.Add(lblFont3);
      tabFonts.Controls.Add(numFont3);
      tabFonts.Controls.Add(cboFont3);
      tabFonts.Controls.Add(pnlColorFont3);
      tabFonts.Controls.Add(chkFontAA2);
      tabFonts.Controls.Add(chkFontItalic2);
      tabFonts.Controls.Add(chkFontBold2);
      tabFonts.Controls.Add(lblFont2);
      tabFonts.Controls.Add(numFont2);
      tabFonts.Controls.Add(cboFont2);
      tabFonts.Controls.Add(pnlColorFont2);
      tabFonts.Controls.Add(btnSettingsSave);
      tabFonts.Controls.Add(chkFontAA1);
      tabFonts.Controls.Add(chkFontItalic1);
      tabFonts.Controls.Add(chkFontBold1);
      tabFonts.Controls.Add(lblFont1);
      tabFonts.Controls.Add(numFont1);
      tabFonts.Controls.Add(cboFont1);
      tabFonts.Controls.Add(pnlColorFont1);
      tabFonts.Controls.Add(btnSettingsLoad);
      tabFonts.Location = new Point(4, 24);
      tabFonts.Margin = new Padding(0);
      tabFonts.Name = "tabFonts";
      tabFonts.Size = new Size(611, 183);
      tabFonts.TabIndex = 3;
      tabFonts.Text = "Fonts";
      // 
      // btnTestFonts
      // 
      btnTestFonts.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      btnTestFonts.FlatStyle = FlatStyle.System;
      btnTestFonts.Location = new Point(516, 95);
      btnTestFonts.Name = "btnTestFonts";
      btnTestFonts.Size = new Size(85, 22);
      btnTestFonts.TabIndex = 159;
      btnTestFonts.Text = "Test";
      btnTestFonts.UseVisualStyleBackColor = true;
      btnTestFonts.Click += btnTestFonts_Click;
      // 
      // chkFontItalic5
      // 
      chkFontItalic5.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      chkFontItalic5.Appearance = Appearance.Button;
      chkFontItalic5.FlatStyle = FlatStyle.System;
      chkFontItalic5.Location = new Point(406, 123);
      chkFontItalic5.Margin = new Padding(3, 2, 3, 2);
      chkFontItalic5.Name = "chkFontItalic5";
      chkFontItalic5.Size = new Size(49, 23);
      chkFontItalic5.TabIndex = 157;
      chkFontItalic5.Text = "Italic";
      chkFontItalic5.TextAlign = ContentAlignment.MiddleCenter;
      chkFontItalic5.TextImageRelation = TextImageRelation.ImageAboveText;
      chkFontItalic5.UseVisualStyleBackColor = true;
      // 
      // chkFontBold5
      // 
      chkFontBold5.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      chkFontBold5.Appearance = Appearance.Button;
      chkFontBold5.FlatStyle = FlatStyle.System;
      chkFontBold5.Location = new Point(351, 123);
      chkFontBold5.Margin = new Padding(3, 2, 3, 2);
      chkFontBold5.Name = "chkFontBold5";
      chkFontBold5.Size = new Size(49, 23);
      chkFontBold5.TabIndex = 156;
      chkFontBold5.Text = "Bold";
      chkFontBold5.TextAlign = ContentAlignment.MiddleCenter;
      chkFontBold5.TextImageRelation = TextImageRelation.ImageAboveText;
      chkFontBold5.UseVisualStyleBackColor = true;
      // 
      // chkFontItalic4
      // 
      chkFontItalic4.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      chkFontItalic4.Appearance = Appearance.Button;
      chkFontItalic4.FlatStyle = FlatStyle.System;
      chkFontItalic4.Location = new Point(406, 94);
      chkFontItalic4.Margin = new Padding(3, 2, 3, 2);
      chkFontItalic4.Name = "chkFontItalic4";
      chkFontItalic4.Size = new Size(49, 23);
      chkFontItalic4.TabIndex = 150;
      chkFontItalic4.Text = "Italic";
      chkFontItalic4.TextAlign = ContentAlignment.MiddleCenter;
      chkFontItalic4.TextImageRelation = TextImageRelation.ImageAboveText;
      chkFontItalic4.UseVisualStyleBackColor = true;
      // 
      // chkFontBold4
      // 
      chkFontBold4.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      chkFontBold4.Appearance = Appearance.Button;
      chkFontBold4.FlatStyle = FlatStyle.System;
      chkFontBold4.Location = new Point(351, 94);
      chkFontBold4.Margin = new Padding(3, 2, 3, 2);
      chkFontBold4.Name = "chkFontBold4";
      chkFontBold4.Size = new Size(49, 23);
      chkFontBold4.TabIndex = 149;
      chkFontBold4.Text = "Bold";
      chkFontBold4.TextAlign = ContentAlignment.MiddleCenter;
      chkFontBold4.TextImageRelation = TextImageRelation.ImageAboveText;
      chkFontBold4.UseVisualStyleBackColor = true;
      // 
      // chkFontItalic3
      // 
      chkFontItalic3.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      chkFontItalic3.Appearance = Appearance.Button;
      chkFontItalic3.FlatStyle = FlatStyle.System;
      chkFontItalic3.Location = new Point(406, 65);
      chkFontItalic3.Margin = new Padding(3, 2, 3, 2);
      chkFontItalic3.Name = "chkFontItalic3";
      chkFontItalic3.Size = new Size(49, 23);
      chkFontItalic3.TabIndex = 143;
      chkFontItalic3.Text = "Italic";
      chkFontItalic3.TextAlign = ContentAlignment.MiddleCenter;
      chkFontItalic3.TextImageRelation = TextImageRelation.ImageAboveText;
      chkFontItalic3.UseVisualStyleBackColor = true;
      // 
      // chkFontBold3
      // 
      chkFontBold3.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      chkFontBold3.Appearance = Appearance.Button;
      chkFontBold3.FlatStyle = FlatStyle.System;
      chkFontBold3.Location = new Point(351, 65);
      chkFontBold3.Margin = new Padding(3, 2, 3, 2);
      chkFontBold3.Name = "chkFontBold3";
      chkFontBold3.Size = new Size(49, 23);
      chkFontBold3.TabIndex = 142;
      chkFontBold3.Text = "Bold";
      chkFontBold3.TextAlign = ContentAlignment.MiddleCenter;
      chkFontBold3.TextImageRelation = TextImageRelation.ImageAboveText;
      chkFontBold3.UseVisualStyleBackColor = true;
      // 
      // chkFontItalic2
      // 
      chkFontItalic2.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      chkFontItalic2.Appearance = Appearance.Button;
      chkFontItalic2.FlatStyle = FlatStyle.System;
      chkFontItalic2.Location = new Point(406, 36);
      chkFontItalic2.Margin = new Padding(3, 2, 3, 2);
      chkFontItalic2.Name = "chkFontItalic2";
      chkFontItalic2.Size = new Size(49, 23);
      chkFontItalic2.TabIndex = 136;
      chkFontItalic2.Text = "Italic";
      chkFontItalic2.TextAlign = ContentAlignment.MiddleCenter;
      chkFontItalic2.TextImageRelation = TextImageRelation.ImageAboveText;
      chkFontItalic2.UseVisualStyleBackColor = true;
      // 
      // chkFontBold2
      // 
      chkFontBold2.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      chkFontBold2.Appearance = Appearance.Button;
      chkFontBold2.FlatStyle = FlatStyle.System;
      chkFontBold2.Location = new Point(351, 36);
      chkFontBold2.Margin = new Padding(3, 2, 3, 2);
      chkFontBold2.Name = "chkFontBold2";
      chkFontBold2.Size = new Size(49, 23);
      chkFontBold2.TabIndex = 135;
      chkFontBold2.Text = "Bold";
      chkFontBold2.TextAlign = ContentAlignment.MiddleCenter;
      chkFontBold2.TextImageRelation = TextImageRelation.ImageAboveText;
      chkFontBold2.UseVisualStyleBackColor = true;
      // 
      // chkFontItalic1
      // 
      chkFontItalic1.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      chkFontItalic1.Appearance = Appearance.Button;
      chkFontItalic1.FlatStyle = FlatStyle.System;
      chkFontItalic1.Location = new Point(406, 7);
      chkFontItalic1.Margin = new Padding(3, 2, 3, 2);
      chkFontItalic1.Name = "chkFontItalic1";
      chkFontItalic1.Size = new Size(49, 23);
      chkFontItalic1.TabIndex = 126;
      chkFontItalic1.Text = "Italic";
      chkFontItalic1.TextAlign = ContentAlignment.MiddleCenter;
      chkFontItalic1.TextImageRelation = TextImageRelation.ImageAboveText;
      chkFontItalic1.UseVisualStyleBackColor = true;
      // 
      // chkFontBold1
      // 
      chkFontBold1.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      chkFontBold1.Appearance = Appearance.Button;
      chkFontBold1.FlatStyle = FlatStyle.System;
      chkFontBold1.Location = new Point(351, 7);
      chkFontBold1.Margin = new Padding(3, 2, 3, 2);
      chkFontBold1.Name = "chkFontBold1";
      chkFontBold1.Size = new Size(49, 23);
      chkFontBold1.TabIndex = 125;
      chkFontBold1.Text = "Bold";
      chkFontBold1.TextAlign = ContentAlignment.MiddleCenter;
      chkFontBold1.TextImageRelation = TextImageRelation.ImageAboveText;
      chkFontBold1.UseVisualStyleBackColor = true;
      // 
      // tabShader
      // 
      tabShader.BackColor = SystemColors.ControlLight;
      tabShader.BorderStyle = BorderStyle.FixedSingle;
      tabShader.Controls.Add(pnlTabShader);
      tabShader.Location = new Point(4, 24);
      tabShader.Margin = new Padding(0);
      tabShader.Name = "tabShader";
      tabShader.Size = new Size(611, 183);
      tabShader.TabIndex = 5;
      tabShader.Text = "Shader";
      // 
      // pnlTabShader
      // 
      pnlTabShader.Controls.Add(panShadertoyLocal);
      pnlTabShader.Controls.Add(btnHLSLLoad);
      pnlTabShader.Controls.Add(chkShaderLeft);
      pnlTabShader.Controls.Add(btnHLSLSave);
      pnlTabShader.Controls.Add(picShaderError);
      pnlTabShader.Controls.Add(txtShaderFind);
      pnlTabShader.Controls.Add(numOffset);
      pnlTabShader.Controls.Add(txtLineNumberError);
      pnlTabShader.Controls.Add(btnShaderHelp);
      pnlTabShader.Controls.Add(chkShaderFile);
      pnlTabShader.Controls.Add(numPSVersion);
      pnlTabShader.Controls.Add(txtShaderinfo);
      pnlTabShader.Controls.Add(splitContainerShader);
      pnlTabShader.Controls.Add(btnSendShader);
      pnlTabShader.Controls.Add(btnShaderConvert);
      pnlTabShader.Dock = DockStyle.Fill;
      pnlTabShader.Location = new Point(0, 0);
      pnlTabShader.Name = "pnlTabShader";
      pnlTabShader.Size = new Size(609, 181);
      pnlTabShader.TabIndex = 140;
      // 
      // panShadertoyLocal
      // 
      panShadertoyLocal.Controls.Add(btnShadertoyFileLoadNext);
      panShadertoyLocal.Controls.Add(btnShadertoyFileLoadThis);
      panShadertoyLocal.Controls.Add(txtShadertoyFile);
      panShadertoyLocal.Controls.Add(numShadertoyFileIndex);
      panShadertoyLocal.Controls.Add(btnShadertoyFilesLoadDir);
      panShadertoyLocal.Controls.Add(btnLoadShaderInputFromFile);
      panShadertoyLocal.Location = new Point(0, 1);
      panShadertoyLocal.Name = "panShadertoyLocal";
      panShadertoyLocal.Size = new Size(370, 32);
      panShadertoyLocal.TabIndex = 150;
      // 
      // picShaderError
      // 
      picShaderError.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      picShaderError.Location = new Point(365, 7);
      picShaderError.Margin = new Padding(0);
      picShaderError.Name = "picShaderError";
      picShaderError.Size = new Size(20, 20);
      picShaderError.SizeMode = PictureBoxSizeMode.Zoom;
      picShaderError.TabIndex = 146;
      picShaderError.TabStop = false;
      // 
      // splitContainerShader
      // 
      splitContainerShader.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
      splitContainerShader.Location = new Point(6, 35);
      splitContainerShader.Margin = new Padding(0);
      splitContainerShader.Name = "splitContainerShader";
      // 
      // splitContainerShader.Panel1
      // 
      splitContainerShader.Panel1.Controls.Add(txtShaderGLSL);
      // 
      // splitContainerShader.Panel2
      // 
      splitContainerShader.Panel2.Controls.Add(txtShaderHLSL);
      splitContainerShader.Size = new Size(596, 114);
      splitContainerShader.SplitterDistance = 286;
      splitContainerShader.TabIndex = 31;
      // 
      // MilkwaveRemoteForm
      // 
      AutoScaleDimensions = new SizeF(96F, 96F);
      AutoScaleMode = AutoScaleMode.Dpi;
      ClientSize = new Size(619, 427);
      Controls.Add(splitContainer1);
      Controls.Add(statusStrip1);
      Icon = (Icon)resources.GetObject("$this.Icon");
      KeyPreview = true;
      Margin = new Padding(3, 2, 3, 2);
      MinimumSize = new Size(264, 84);
      Name = "MilkwaveRemoteForm";
      Text = "Milkwave Remote";
      FormClosing += MainForm_FormClosing;
      Load += MilkwaveRemoteForm_Load;
      Shown += MainForm_Shown;
      KeyDown += MainForm_KeyDown;
      statusStrip1.ResumeLayout(false);
      statusStrip1.PerformLayout();
      ((System.ComponentModel.ISupportInitialize)numSize).EndInit();
      ((System.ComponentModel.ISupportInitialize)numBPM).EndInit();
      ((System.ComponentModel.ISupportInitialize)numBeats).EndInit();
      ((System.ComponentModel.ISupportInitialize)numAmpLeft).EndInit();
      ((System.ComponentModel.ISupportInitialize)numAmpRight).EndInit();
      ((System.ComponentModel.ISupportInitialize)numWrap).EndInit();
      ((System.ComponentModel.ISupportInitialize)numOpacity).EndInit();
      ((System.ComponentModel.ISupportInitialize)numWaveAlpha).EndInit();
      ((System.ComponentModel.ISupportInitialize)numWaveR).EndInit();
      ((System.ComponentModel.ISupportInitialize)numWaveB).EndInit();
      ((System.ComponentModel.ISupportInitialize)numWaveG).EndInit();
      ((System.ComponentModel.ISupportInitialize)numFont1).EndInit();
      ((System.ComponentModel.ISupportInitialize)numFont2).EndInit();
      ((System.ComponentModel.ISupportInitialize)numFont3).EndInit();
      ((System.ComponentModel.ISupportInitialize)numFont4).EndInit();
      ((System.ComponentModel.ISupportInitialize)numFont5).EndInit();
      ((System.ComponentModel.ISupportInitialize)numPSVersion).EndInit();
      ((System.ComponentModel.ISupportInitialize)numOffset).EndInit();
      ((System.ComponentModel.ISupportInitialize)numShadertoyFileIndex).EndInit();
      ((System.ComponentModel.ISupportInitialize)numSettingsHueAuto).EndInit();
      ((System.ComponentModel.ISupportInitialize)numMidiBank).EndInit();
      ((System.ComponentModel.ISupportInitialize)numVisShift).EndInit();
      ((System.ComponentModel.ISupportInitialize)numVisIntensity).EndInit();
      ((System.ComponentModel.ISupportInitialize)numVisVersion).EndInit();
      ((System.ComponentModel.ISupportInitialize)numWaveMode).EndInit();
      tableLayoutPanel1.ResumeLayout(false);
      splitContainer1.Panel1.ResumeLayout(false);
      splitContainer1.Panel2.ResumeLayout(false);
      ((System.ComponentModel.ISupportInitialize)splitContainer1).EndInit();
      splitContainer1.ResumeLayout(false);
      tabControl.ResumeLayout(false);
      tabPreset.ResumeLayout(false);
      tabPreset.PerformLayout();
      tabMessage.ResumeLayout(false);
      tabMessage.PerformLayout();
      tabWave.ResumeLayout(false);
      ((System.ComponentModel.ISupportInitialize)numWaveEcho).EndInit();
      ((System.ComponentModel.ISupportInitialize)numWaveScale).EndInit();
      ((System.ComponentModel.ISupportInitialize)numWaveDecay).EndInit();
      ((System.ComponentModel.ISupportInitialize)numWaveWarp).EndInit();
      ((System.ComponentModel.ISupportInitialize)numWaveRotation).EndInit();
      ((System.ComponentModel.ISupportInitialize)numWaveZoom).EndInit();
      ((System.ComponentModel.ISupportInitialize)numWavePushY).EndInit();
      ((System.ComponentModel.ISupportInitialize)numWavePushX).EndInit();
      tabInput.ResumeLayout(false);
      tabMidi.ResumeLayout(false);
      tabMidi.PerformLayout();
      tabSettings.ResumeLayout(false);
      ((System.ComponentModel.ISupportInitialize)numPresetChange).EndInit();
      ((System.ComponentModel.ISupportInitialize)numSettingsBrightness).EndInit();
      ((System.ComponentModel.ISupportInitialize)numSettingsSaturation).EndInit();
      ((System.ComponentModel.ISupportInitialize)numSettingsHue).EndInit();
      ((System.ComponentModel.ISupportInitialize)numQuality).EndInit();
      ((System.ComponentModel.ISupportInitialize)numFactorFPS).EndInit();
      ((System.ComponentModel.ISupportInitialize)numFactorFrame).EndInit();
      ((System.ComponentModel.ISupportInitialize)numFactorTime).EndInit();
      tabFonts.ResumeLayout(false);
      tabShader.ResumeLayout(false);
      pnlTabShader.ResumeLayout(false);
      pnlTabShader.PerformLayout();
      panShadertoyLocal.ResumeLayout(false);
      panShadertoyLocal.PerformLayout();
      ((System.ComponentModel.ISupportInitialize)picShaderError).EndInit();
      splitContainerShader.Panel1.ResumeLayout(false);
      splitContainerShader.Panel1.PerformLayout();
      splitContainerShader.Panel2.ResumeLayout(false);
      splitContainerShader.Panel2.PerformLayout();
      ((System.ComponentModel.ISupportInitialize)splitContainerShader).EndInit();
      splitContainerShader.ResumeLayout(false);
      ResumeLayout(false);
      PerformLayout();
    }

    private void TxtBPM_TextChanged(object sender, EventArgs e) {
      throw new NotImplementedException();
    }

    #endregion
    private StatusStrip statusStrip1;
    private ToolStripStatusLabel statusBar;
    private ToolTip toolTip1;
    private ColorDialog colorDialogMessage;
    private ToolStripStatusLabel statusHelp;
    private ToolStripStatusLabel statusSupporters;
    private SplitContainer splitContainer1;
    private CheckBox chkPreview;
    private Button btnAppendSize;
    private NumericUpDown numSize;
    private Label lblSize;
    private Label lblStyle;
    private TextBox txtStyle;
    private NumericUpDown numBPM;
    private Label lblBPM;
    private CheckBox chkFileRandom;
    private Panel pnlColorMessage;
    private Button btnAppendColor;
    private Label lblColor;
    private Button btnFontAppend;
    private Label lblFont;
    private ComboBox cboFonts;
    private NumericUpDown numBeats;
    private Label label7;
    private Label lblFromFile;
    private CheckBox chkAutoplay;
    private TextBox txtAutoplay;
    private Button btnSaveParam;
    private Label lblParameters;
    private ComboBox cboParameters;
    private Label label2;
    private Label lblWindow;
    private Button btnSend;
    private TextBox txtMessage;
    private TableLayoutPanel tableLayoutPanel1;
    private Button btn77;
    private Button btn66;
    private Button btn55;
    private Button btn44;
    private Button btn33;
    private Button btn22;
    private Button btnK;
    private Button btnF2;
    private Button btnN;
    private Button btnAltEnter;
    private Button btnF10;
    private Button btn11;
    private Button btnTilde;
    private Button btn00;
    private Button btnSpace;
    private Button btnF7;
    private Button btnF4;
    private Button btnF3;
    private Button btnBackspace;
    private Button btnDelete;
    private Label lblCurrentPreset;
    private TextBox txtVisRunning;
    private ToolStripDropDownButton toolStripDropDownButton;
    private ToolStripMenuItem toolStripMenuItemHelp;
    private ToolStripMenuItem toolStripMenuItemSupporters;
    private ToolStripMenuItem toolStripMenuItemHomepage;
    private ToolStripSeparator toolStripSeparator1;
    private ToolStripMenuItem toolStripMenuItemDarkMode;
    private ToolStripMenuItem toolStripMenuItemOpenVisualizer;
    private ToolStripMenuItem toolStripMenuItemButtonPanel;
    private ToolStripMenuItem toolStripMenuItemSpriteButtonImages;
    private ToolStripMenuItem toolStripMenuItemColorButtonImages;
    private ToolStripMenuItem toolStripMenuItemVisualizerPanel;
    private ToolStripSeparator toolStripSeparator2;
    private ToolStripMenuItem toolStripMenuItemTabsPanel;
    private Label lblPreset;
    private ComboBox cboPresets;
    private Button btnPresetLoadFile;
    private Button btnPresetSend;
    private Button btnPresetLoadDirectory;
    private NumericUpDown numAmpLeft;
    private Label lblAmp;
    private NumericUpDown numAmpRight;
    private ColorDialog colorDialogWave;
    private CheckBox chkAmpLinked;
    private NumericUpDown numWrap;
    private CheckBox chkWrap;
    private Button btnSetAudioDevice;
    private Label lblAudioDevice;
    private ComboBox cboAudioDevice;
    private NumericUpDown numOpacity;
    private Label lblPercent;
    private ToolStripSeparator toolStripSeparator3;
    private Button btnSendFile;
    private ComboBox cboAutoplay;
    private Button btnSwitchMode;
    private Button btn88;
    private Button btnWatermark;
    private Button btnTransparency;
    private Button btnB;
    private TabPage tabMessage;
    private TabPage tabPreset;
    private CheckBox chkTagsFromRunning;
    private CheckBox chkWaveDarken;
    private Label lblLoad;
    private TextBox txtTags;
    private Label lblTags;
    private Button btnPresetLoadTags;
    private CheckBox chkPresetRandom;
    private CheckBox chkPresetLink;
    private Button btnTagsSave;
    private FlatTabControl tabControl;
    private Button btnTag2;
    private Button btnTag3;
    private Button btnTag1;
    private TabPage tabWave;
    private NumericUpDown numWaveMode;
    private Button btnSendWave;
    private Panel pnlColorWave;
    private NumericUpDown numWaveAlpha;
    private Label lblWavemode;
    private Button btnTag5;
    private Button btnTag4;
    private CheckBox chkWaveLink;
    private Label lblRGB;
    private NumericUpDown numWaveR;
    private NumericUpDown numWaveB;
    private NumericUpDown numWaveG;
    private Button btnWaveClear;
    private NumericUpDown numWavePushX;
    private Label lblPushX;
    private Label lblPushY;
    private NumericUpDown numWavePushY;
    private Label lblDecay;
    private NumericUpDown numWaveDecay;
    private Label lblWarp;
    private NumericUpDown numWaveWarp;
    private Label lblRotation;
    private NumericUpDown numWaveRotation;
    private Label lblZoom;
    private NumericUpDown numWaveZoom;
    private Button btnWaveQuicksave;
    private ComboBox cboWindowTitle;
    private CheckBox chkWaveInvert;
    private CheckBox chkWaveSolarize;
    private CheckBox chkWaveBrighten;
    private CheckBox chkWaveVolAlpha;
    private CheckBox chkWaveThick;
    private CheckBox chkWaveDotted;
    private CheckBox chkWaveAdditive;
    private NumericUpDown numWaveEcho;
    private Label lblEcho;
    private NumericUpDown numWaveScale;
    private Label lblScale;
    private Button btnTag6;
    private Button btnTag10;
    private Button btnTag9;
    private Button btnTag8;
    private Button btnTag7;
    private Label lblMostUsed;
    private ComboBox cboTagsFilter;
    private TabPage tabFonts;
    private ComboBox cboFont1;
    private Panel pnlColorFont1;
    private NumericUpDown numFont1;
    private Label lblFont1;
    private CheckBox chkFontBold1;
    private CheckBox chkFontAA1;
    private CheckBox chkFontItalic1;
    private Button btnSettingsLoad;
    private Button btnSettingsSave;
    private CheckBox chkFontAA5;
    private CheckBox chkFontItalic5;
    private CheckBox chkFontBold5;
    private Label lblFont5;
    private NumericUpDown numFont5;
    private ComboBox cboFont5;
    private Panel pnlColorFont5;
    private CheckBox chkFontAA4;
    private CheckBox chkFontItalic4;
    private CheckBox chkFontBold4;
    private Label lblFont4;
    private NumericUpDown numFont4;
    private ComboBox cboFont4;
    private Panel pnlColorFont4;
    private CheckBox chkFontAA3;
    private CheckBox chkFontItalic3;
    private CheckBox chkFontBold3;
    private Label lblFont3;
    private NumericUpDown numFont3;
    private ComboBox cboFont3;
    private Panel pnlColorFont3;
    private CheckBox chkFontAA2;
    private CheckBox chkFontItalic2;
    private CheckBox chkFontBold2;
    private Label lblFont2;
    private NumericUpDown numFont2;
    private ComboBox cboFont2;
    private Panel pnlColorFont2;
    private Button btnTestFonts;
    private TabPage tabSettings;
    private NumericUpDown numFactorFrame;
    private NumericUpDown numFactorTime;
    private Label lblFactorFrame;
    private Label lblFactorTime;
    private NumericUpDown numFactorFPS;
    private Label lblFactorFPS;
    private Button btnOpenSettingsFile;
    private TextBox txtFilterTags;
    private TabPage tabShader;
    private TextBox txtShaderHLSL;
    private Button btnSendShader;
    private Button btnLoadShaderInputFromFile;
    private TextBox txtShaderFind;
    private TextBox txtLineNumberError;
    private SplitContainer splitContainerShader;
    private Button btnShaderConvert;
    private TextBox txtShaderGLSL;
    private TextBox txtShaderinfo;
    private Button btnShaderHelp;
    private Panel pnlTabShader;
    private NumericUpDown numPSVersion;
    private NumericUpDown numOffset;
    private CheckBox chkShaderFile;
    private Button btnShaderError;
    private PictureBox picShaderError;
    private CheckBox chkShaderLeft;
    private Button btnHLSLLoad;
    private Button btnHLSLSave;
    private NumericUpDown numVisShift;
    private Label lblVisShift;
    private NumericUpDown numVisIntensity;
    private Label lblVisIntensity;
    private NumericUpDown numVisVersion;
    private Label lblVisVersion;
    private TabPage tabMidi;
    private CheckBox chkMidi1Learn;
    private Label lblMidiDevice;
    private ComboBox cboMidiDevice;
    private Button btnMidiDeviceScan;
    private TextBox txtMidi1Ch;
    private TextBox txtMidi1Con;
    private TextBox txtMidi1Val;
    private CheckBox chkMidi1Active;
    private TextBox txtMidi1Inc;
    private TextBox txtMidi1Label;
    private Button btnMIDIHelp;
    private ComboBox cboMidi1Action;
    private NumericUpDown numMidiBank;
    private Label lblMidi1Row;
    private Label lblBank;
    private Label lblMidi1Inc;
    private Label lblMidi1Action;
    private Label lblMidi5Row;
    private TextBox txtMidi5Label;
    private Label lblMidi4Row;
    private TextBox txtMidi4Label;
    private Label lblMidi3Row;
    private TextBox txtMidi3Label;
    private Label lblMidi2Row;
    private TextBox txtMidi2Label;
    private Label lblMidi2Inc;
    private Label lblMidi2Action;
    private ComboBox cboMidi2Action;
    private CheckBox chkMidi2Active;
    private TextBox txtMidi2Inc;
    private TextBox txtMidi2Con;
    private TextBox txtMidi2Val;
    private TextBox txtMidi2Ch;
    private CheckBox chkMidi2Learn;
    private Button btnMIDISave;
    private Button btnMIDILoad;
    private Label label6;
    private Label label8;
    private ComboBox cboMidi5Action;
    private CheckBox chkMidi5Active;
    private TextBox txtMidi5Inc;
    private TextBox txtMidi5Con;
    private TextBox txtMidi5Val;
    private TextBox txtMidi5Ch;
    private CheckBox chkMidi5Learn;
    private Label label4;
    private Label label5;
    private ComboBox cboMidi4Action;
    private CheckBox chkMidi4Active;
    private TextBox txtMidi4Inc;
    private TextBox txtMidi4Con;
    private TextBox txtMidi4Val;
    private TextBox txtMidi4Ch;
    private CheckBox chkMidi4Learn;
    private Label label1;
    private Label label3;
    private ComboBox cboMidi3Action;
    private CheckBox chkMidi3Active;
    private TextBox txtMidi3Inc;
    private TextBox txtMidi3Con;
    private TextBox txtMidi3Val;
    private TextBox txtMidi3Ch;
    private CheckBox chkMidi3Learn;
    private CheckBox chkSpoutFixedSize;
    private CheckBox chkSpoutActive;
    private Label lblSpout;
    private ComboBox cboSpoutWidth;
    private ComboBox cboSpoutHeight;
    private NumericUpDown numQuality;
    private Label lblQuality;
    private ToolStripStatusLabel toolStripStatusLabelMonitorCPU;
    private ToolStripMenuItem toolStripMenuItemMonitorCPU;
    private ToolStripStatusLabel toolStripStatusLabelMonitorGPU;
    private ToolStripMenuItem toolStripMenuItemMonitorGPU;
    private ToolStripSeparator toolStripSeparator4;
    private Button btnQualityHalf;
    private Button btnQualityDouble;
    private NumericUpDown numSettingsBrightness;
    private Label lblBrightness;
    private NumericUpDown numSettingsSaturation;
    private Label lblSaturation;
    private NumericUpDown numSettingsHue;
    private Label lblHue;
    private TextBox txtFilterPresets;
    private Label label9;
    private ComboBox cboSettingsOpenFile;
    private CheckBox chkQualityAuto;
    private Panel panShadertoyLocal;
    private Button btnShadertoyFilesLoadDir;
    private TextBox txtShadertoyFile;
    private NumericUpDown numShadertoyFileIndex;
    private Button btnShadertoyFileLoadNext;
    private Button btnShadertoyFileLoadThis;
    private Button btnFontGlobalPlus;
    private Button btnFontGlobalMinus;
    private NumericUpDown numSettingsHueAuto;
    private CheckBox chkHueAuto;
    private NumericUpDown numPresetChange;
    private Label lblChangePreset;
    private CheckBox chkPresetLocked;
    private TabPage tabInput;
    private Button btnVideoInputSet;
    private ComboBox cboVideoInput;
    private Label label10;
    private CheckBox chkVideoMix;
  }
}
