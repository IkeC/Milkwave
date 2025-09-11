
using static MilkwaveRemote.Data.MidiRow;

namespace MilkwaveRemote.Data {
  public class MidiActionEntry {

    public MidiActionEntry(string actionText, MidiActionId actionId) {
      ActionText = actionText;
      ActionId = actionId;
    }

    public string ActionText { get; set; } = "";
    public MidiActionId ActionId { get; set; } = MidiActionId.Undefined;
  } 
}
