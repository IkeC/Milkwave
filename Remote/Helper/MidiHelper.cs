using MilkwaveRemote.Data;
using NAudio.Midi;

namespace MilkwaveRemote.Helper {

  public class MidiHelper : IDisposable {

    private MidiIn? midiInDevice;
    public event Action<int>? NoteLearned;

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
      // Filter for NoteOn with velocity > 0
      if (e.MidiEvent is NoteEvent note &&
          note.CommandCode == MidiCommandCode.NoteOn &&
          note.Velocity > 0) {
        int noteNumber = note.NoteNumber;

        // Capture the first note as the learned key
        if (noteNumber >= 0) {
          NoteLearned?.Invoke(noteNumber);
        }
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