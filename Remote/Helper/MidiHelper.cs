using MilkwaveRemote.Data;
using NAudio.Midi;

namespace MilkwaveRemote.Helper {

  public class MidiHelper : IDisposable {

    private MidiIn? midiInDevice;
    public event Action<MidiEventInfo>? NoteLearned;

    public void SelectDevice(int deviceIndex) {
      Stop();
      midiInDevice?.Dispose();
      if (MidiIn.NumberOfDevices > 0) {
        midiInDevice = new MidiIn(deviceIndex);
        midiInDevice.MessageReceived += MidiIn_MessageReceived;
        midiInDevice.ErrorReceived += MidiIn_ErrorReceived;
        Start();
      }
    }

    // Begin monitoring the device
    public void Start() {
      if (midiInDevice != null) {
        try {
          midiInDevice.Start();
        } catch (NAudio.MmException ex) {
          // ignore
        }
      }
    }

    // Stop monitoring
    public void Stop() {
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
          NoteLearned?.Invoke(midiEventInfo);
        }
      } else if (midiEvent is ControlChangeEvent eCC) {
        midiEventInfo.Channel = eCC.Channel;
        midiEventInfo.Controller = (int)eCC.Controller;
        midiEventInfo.Value = eCC.ControllerValue;
        
        NoteLearned?.Invoke(midiEventInfo);
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