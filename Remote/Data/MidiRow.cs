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
      KnobPresetAmpL = 100,
      KnobPresetAmpR = 101,
      KnobMessageBPM = 200,
      KnobMessageBeats = 201,
      KnobSettingsTime = 600,
      KnobSettingsFPS = 601,
      KnobSettingsIntensity = 610,
      KnobSettingsShift = 611,
      KnobSettingsQuality = 620,
      KnobSettingsHue = 630,
      KnobSettingsSaturation = 631,
      KnobSettingsBrightness = 632
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
