namespace MilkwaveRemote.Data {
  public class Tags {
    public Tags() {
    }
    public Dictionary<string, TagEntry> TagEntries { get; set; } = new Dictionary<string, TagEntry>();
  }
}
