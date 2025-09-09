namespace MilkwaveRemote.Data {
  public class MidiDeviceEntry {
    public int DeviceIndex { get; set; }
    public string DeviceName { get; set; } = "";
    public override string ToString() => DeviceName;
  }
}
