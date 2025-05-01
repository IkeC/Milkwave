namespace MilkwaveRemote.Data {
  public class Settings {
    public Settings() {
    }

    public static int Panel1DefaultHeight = 230;
    public static int Panel2DefaultHeight = 79;

    public List<Style> Styles { get; set; } = new List<Style>();
    public Point RemoteWindowLocation { get; set; } = new Point(50, 50);
    public Size RemoteWindowSize { get; set; } = new Size(800, 600);
    public int SplitterDistance1 { get; set; } = Panel1DefaultHeight;
    public int SplitterDistance2 { get; set; } = Panel2DefaultHeight;
    public bool DarkMode { get; set; } = true;
    public bool ShowTabsPanel { get; set; } = true;
    public bool ShowButtonPanel { get; set; } = true;
    public bool CloseVisualizerWithRemote { get; set; } = false;
    public int SelectedTabIndex { get; set; } = 0;
  }
}