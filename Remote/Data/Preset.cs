namespace MilkwaveRemote.Data {
  public class Preset {
    public string DisplayName { get; set; } = "";
    public string MaybeRelativePath { get; set; } = "";
    public override string ToString() {
      return DisplayName;
    }
  }
}
