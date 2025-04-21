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
        int result = GetPrivateProfileString("Settings", "MilkwaveAudioDevice", string.Empty, returnValue, 256, iniFile);
      }
      return returnValue.ToString();
    }

    public void FillAudioDevices(ComboBox cbo) {
      cbo.Items.Clear(); // Clear existing items

      MMDevice defaultMDevice;
      string iniMilkwaveAudioDevice = ReadMilkwaveAudioDevice();

      using (var enumerator = new MMDeviceEnumerator()) {
        var devices = enumerator.EnumerateAudioEndPoints(DataFlow.Render, DeviceState.Active);
        defaultMDevice = enumerator.GetDefaultAudioEndpoint(DataFlow.Render, Role.Multimedia);
        foreach (var device in devices) {
          bool isDefaultDevice = device.ID == defaultMDevice.ID;
          cbo.Items.Add(new ComboBoxItem(device.FriendlyName, device, isDefaultDevice)); // Add device names to ComboBox
        }
      }

      // Sort items alphabetically
      var sortedItems = cbo.Items.Cast<ComboBoxItem>().OrderBy(item => item.Text).ToList();
      cbo.Items.Clear();
      foreach (var item in sortedItems) {
        cbo.Items.Add(item);
      }

      if (cbo.Items.Count > 0) {
        bool found = false;
        if (iniMilkwaveAudioDevice.Length > 0) {
          foreach (ComboBoxItem item in cbo.Items) {
            if (item.Device.FriendlyName.Equals(iniMilkwaveAudioDevice)) {
              cbo.SelectedItem = item;
              found = true;
              break;
            }
          }
        }

        if (!found) {
          foreach (ComboBoxItem item in cbo.Items) {
            if (item.IsDefaultDevice) {
              cbo.SelectedItem = item;
              break;
            }
          }
        }

      }
    }

    public void SelectDeviceByName(ComboBox cbo, string deviceName) {
      foreach (ComboBoxItem item in cbo.Items) {
        if (item.Device.FriendlyName.Equals(deviceName) || (deviceName.Length == 0 && item.IsDefaultDevice)) {
          cbo.SelectedItem = item;
          break;
        }
      }
    }

    protected class ComboBoxItem {

      public ComboBoxItem(string text, MMDevice device, bool isDefaultDevice) {
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

