namespace MilkwaveRemote.Data {
  public class MidiRow {

    public enum MidiActionType {
      Undefined = 0,
      Button = 1,
      Knob = 2
    }

    public enum MidiActionId {
      Undefined = 0,
      Message = 1,
      KnobIntensity = 100,
      KnobShift = 101
    }

    public int Row { get; set; } = 0;
    public bool Active { get; set; } = false;
    public string Label { get; set; } = "";

    public int? Value { get; set; } = null;
    public int? Channel { get; set; } = null;
    public int? Controller { get; set; } = null;
    
    public MidiActionId ActionId { get; set; } = MidiActionId.Undefined;
    public MidiActionType ActionType { get; set; } = MidiActionType.Undefined;
    public string ActionText { get; set; } = "";

    public string Increment { get; set; } = "";
  }
}
