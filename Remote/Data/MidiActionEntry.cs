
namespace MilkwaveRemote.Data {
  public class MidiActionEntry {

    public enum Type {
      Undefined = 0 ,
      Button = 1,
      Knob = 2
    }

    public enum Id {
      Undefined = 0,
      Message = 1,
      KnobIntensity = 100,
      KnobShift = 101,
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
