using NAudio.CoreAudioApi;
using System.Runtime.InteropServices;
using System.Text;

namespace MilkwaveRemote {

  public class RemoteHelper {
    public RemoteHelper() {
    }

    [DllImport("kernel32")]
    private static extern int GetPrivateProfileString(
       string section, string key, string defaultValue,
       StringBuilder returnValue, int size, string filePath);

    public string ReadMilkwaveAudioDevice() {
      StringBuilder returnValue = new StringBuilder(256);
      string iniFile = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "settings.ini");
      if (File.Exists(iniFile)) {
        int result = GetPrivateProfileString("Milkwave", "AudioDevice", string.Empty, returnValue, 256, iniFile);
      }
      return returnValue.ToString();
    }

    public void FillAudioDevices(ComboBox cbo) {
      cbo.Items.Clear(); // Clear existing items

      MMDevice defaultMDevice;
      string iniMilkwaveAudioDevice = ReadMilkwaveAudioDevice();

      using (var enumerator = new MMDeviceEnumerator()) {
        defaultMDevice = enumerator.GetDefaultAudioEndpoint(DataFlow.Render, Role.Multimedia);
        var devices = enumerator.EnumerateAudioEndPoints(DataFlow.Render, DeviceState.Active);

        bool includeCaptureDevices = false;
#if DEBUG
        includeCaptureDevices = true; // Include capture devices in debug mode
#endif
        foreach (var device in devices) {
          bool isDefaultDevice = device.ID == defaultMDevice.ID;
          string name = includeCaptureDevices ? "Output: " + device.FriendlyName : device.FriendlyName;
          cbo.Items.Add(new ComboBoxItemDevice(name, device, isDefaultDevice)); // Add device names to ComboBox
        }

        if (includeCaptureDevices) {
          devices = enumerator.EnumerateAudioEndPoints(DataFlow.Capture, DeviceState.Active);
          foreach (var device in devices) {
            cbo.Items.Add(new ComboBoxItemDevice("Input: " + device.FriendlyName, device, false)); // Add device names to ComboBox
          }
        }

        // Sort items alphabetically
        var sortedItems = cbo.Items.Cast<ComboBoxItemDevice>().OrderBy(item => item.Text).ToList();
        cbo.Items.Clear();
        foreach (var item in sortedItems) {
          cbo.Items.Add(item);
        }

        if (cbo.Items.Count > 0) {
          bool found = false;
          if (iniMilkwaveAudioDevice.Length > 0) {
            foreach (ComboBoxItemDevice item in cbo.Items) {
              if (item.Device.FriendlyName.Equals(iniMilkwaveAudioDevice)) {
                cbo.SelectedItem = item;
                found = true;
                break;
              }
            }
          }

          if (!found) {
            foreach (ComboBoxItemDevice item in cbo.Items) {
              if (item.IsDefaultDevice) {
                cbo.SelectedItem = item;
                break;
              }
            }
          }
        }
      }
    }

    public void ReloadAudioDevices(ComboBox cbo) {
      if (cbo.SelectedItem is ComboBoxItemDevice currentItem) { // Use pattern matching to check and cast
        FillAudioDevices(cbo);
        foreach (ComboBoxItemDevice item in cbo.Items) {
          if (item.Device.ID == currentItem.Device.ID) {
            cbo.SelectedItem = item;
            break;
          }
        }
      } else {
        FillAudioDevices(cbo);
        if (cbo.Items.Count > 0) {
          cbo.SelectedIndex = 0; // Select the first item if no previous selection
        }
      }
    }

    public void SelectDeviceByName(ComboBox cbo, string deviceName) {
      foreach (ComboBoxItemDevice item in cbo.Items) {
        if (item.Device.FriendlyName.Equals(deviceName) || (deviceName.Length == 0 && item.IsDefaultDevice)) {
          cbo.SelectedItem = item;
          break;
        }
      }
    }

    public void SelectDefaultDevice(ComboBox cbo) {
      foreach (ComboBoxItemDevice item in cbo.Items) {
        if (item.IsDefaultDevice) {
          cbo.SelectedItem = item;
          break;
        }
      }
    }

    public class ComboBoxItemDevice {

      public ComboBoxItemDevice(string text, MMDevice device, bool isDefaultDevice) {
        Text = text;
        Device = device;
        IsDefaultDevice = isDefaultDevice;
      }

      public string Text { get; set; }
      public MMDevice Device { get; set; }
      public bool IsDefaultDevice { get; set; } = false;

      public override string ToString() {
        return Text;
      }

    }
  }
}

