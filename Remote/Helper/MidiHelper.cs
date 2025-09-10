using MilkwaveRemote.Data;
using NAudio.Midi;

namespace MilkwaveRemote.Helper {

  public class MidiHelper : IDisposable {

    public bool Learning { get; set; } = false;
    public List<MidiRow> MidiRows { get; set; } = new List<MidiRow>();

    private MidiIn? midiInDevice;

    public event Action<MidiEventInfo>? MidiMessageReceived;

    public MidiHelper() {
      for (int i = 0; i < 9*5; i++) { // 5 rows, 9 banks
        MidiRows.Add(new MidiRow());
      }
    }

    public void SelectDevice(int deviceIndex) {
      StopListening();
      midiInDevice?.Dispose();
      if (MidiIn.NumberOfDevices > 0) {
        midiInDevice = new MidiIn(deviceIndex);
        midiInDevice.MessageReceived += MidiIn_MessageReceived;
        midiInDevice.ErrorReceived += MidiIn_ErrorReceived;
        StartListening();
      }
    }

    // Begin monitoring the device
    public void StartListening() {
      if (midiInDevice != null) {
        try {
          midiInDevice.Start();
        } catch (NAudio.MmException ex) {
          // ignore
        }
      }
    }

    // Stop monitoring
    public void StopListening() {
      if (midiInDevice != null) {
        try {
          midiInDevice.Stop();
        } catch (NAudio.MmException ex) {
          // ignore
        }
      }
    }

    private void MidiIn_MessageReceived(object? sender, MidiInMessageEventArgs e) {
      MidiEvent midiEvent = e.MidiEvent;
      MidiEventInfo midiEventInfo = new MidiEventInfo();
      // Filter for NoteOn with velocity > 0
      if (midiEvent is NoteEvent eNote &&
          midiEvent.CommandCode == MidiCommandCode.NoteOn &&
          eNote.Velocity > 0) {
        midiEventInfo.Channel = eNote.Channel;
        midiEventInfo.Value = eNote.NoteNumber;
        // Capture the first note as the learned key
        if (midiEventInfo.Value >= 0) {
          MidiMessageReceived?.Invoke(midiEventInfo);
        }
      } else if (midiEvent is ControlChangeEvent eCC) {
        midiEventInfo.Channel = eCC.Channel;
        midiEventInfo.Controller = (int)eCC.Controller;
        midiEventInfo.Value = eCC.ControllerValue;
        
        MidiMessageReceived?.Invoke(midiEventInfo);
      }
    }

    private void MidiIn_ErrorReceived(object? sender, MidiInMessageEventArgs e) {
      // TODO
      Console.WriteLine($"MIDI Error: {e.RawMessage}");
    }

    public void Dispose() => midiInDevice?.Dispose();

    public static List<MidiDeviceEntry> GetInputDevices() {
      var list = new List<MidiDeviceEntry>();
      for (int index = 0; index < MidiIn.NumberOfDevices; index++) {
        var info = MidiIn.DeviceInfo(index);
        list.Add(new MidiDeviceEntry {
          DeviceIndex = index,
          DeviceName = info.ProductName
        });
      }
      return list;
    }

  }
}