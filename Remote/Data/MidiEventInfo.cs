namespace MilkwaveRemote.Data {
  public class MidiEventInfo {
    public int Value { get; set; } 
    public int Channel { get; set; }
    public int Controller { get; set; }

    public override string ToString() {
      return $"Channel: {Channel} Controller: {Controller} Value: {Value}";
    }
  }
}
