﻿namespace MilkwaveRemote.Data {
  public class Settings {
    public Settings() {
    }

    public static int Panel1DefaultHeight = 256;
    public List<Style> Styles { get; set; } = new List<Style>();
    public List<String> LoadFilters { get; set; } = new List<String>();
    public List<String> ShadertoyIDs { get; set; } = new List<String>();
    public Point RemoteWindowLocation { get; set; } = new Point(50, 50);
    public Size RemoteWindowSize { get; set; } = new Size(800, 600);
    public int SplitterDistance1 { get; set; } = Panel1DefaultHeight;
    public bool DarkMode { get; set; } = true;
    public bool ShowTabsPanel { get; set; } = true;
    public bool ShowButtonPanel { get; set; } = true;
    public bool EnableMonitorCPU { get; set; } = true;
    public bool EnableMonitorGPU { get; set; } = true;
    public int MonitorPollingInterval { get; set; } = 1500;
    public bool CloseVisualizerWithRemote { get; set; } = false;
    public int SelectedTabIndex { get; set; } = 0;
    public bool ShaderFileChecked { get; set; } = true;
    public bool WrapChecked { get; set; } = true;

    public decimal VisIntensity { get; set; } = 1.0M;
    public decimal VisShift { get; set; } = 0.0M;
    public int VisVersion { get; set; } = 1;
  }
}