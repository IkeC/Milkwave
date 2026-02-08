namespace MilkwaveRemote.Data {
  public class PresetDeck {
    public Dictionary<int, PresetDeckButton> Assignments { get; set; } = new Dictionary<int, PresetDeckButton>();
  }

  public class PresetDeckButton {
    public string PresetPath { get; set; } = "";
    public string PresetDisplayName { get; set; } = "";
    public string ThumbnailPath { get; set; } = "";
  }
}
