namespace MilkwaveRemote.Data {
  public class MidiRow {
    public bool Active = true;
    public string Label = "";

    public int Value = -1;
    public int Channel = -1;
    public int Controller = -1;
    
    public MidiActionEntry.Id ActionId = MidiActionEntry.Id.Undefined;
    public MidiActionEntry.Type ActionType = MidiActionEntry.Type.Undefined;

    public string Increment = "";
  }
}
