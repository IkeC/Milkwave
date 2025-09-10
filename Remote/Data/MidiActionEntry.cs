
namespace MilkwaveRemote.Data {
  public class MidiActionEntry {

    public enum Type {
      Undefined,
      Button,
      Knob
    }

    public enum Id {
      Undefined,
      Message,
      KnobIntensity,
      KnobShift
    }

    public MidiActionEntry(string actionText, Type actionType, Id actionId) {
      ActionText = actionText;
      ActionType = actionType;
      ActionId = actionId;
    }

    public string ActionText { get; set; } = "";
    public Type ActionType { get; set; } = Type.Undefined;
    public Id ActionId { get; set; } = Id.Undefined;
  } 
}
