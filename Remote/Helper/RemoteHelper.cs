using NAudio.CoreAudioApi;
using System.Runtime.InteropServices;
using System.Text;

namespace MilkwaveRemote.Helper {

  public class RemoteHelper {

    private string iniFile;
    private bool includeInputDevices = true;

    public RemoteHelper(string iniFile) {
      this.iniFile = iniFile;
      this.includeInputDevices = GetIniValue("Milkwave", "IncludeInputDevices", "1") == "1";
    }
    
    [DllImport("kernel32")]
    private static extern int GetPrivateProfileString(
       string section, string key, string defaultValue,
       StringBuilder returnValue, int size, string filePath);

    [DllImport("kernel32")]
    private static extern long WritePrivateProfileString(
      string section, string key, string val, string filePath);


    public string GetIniValue(string section, string key, string defaultValue) {
      StringBuilder returnValue = new StringBuilder(256);
      if (File.Exists(iniFile)) {
        int result = GetPrivateProfileString(section, key, defaultValue, returnValue, 256, iniFile);
      }
      return returnValue.ToString();
    }

    public long SetIniValue(string section, string key, string value) {
      return WritePrivateProfileString(section, key, value, iniFile);
    }

    public string GetIniValueFonts(string key, string defaultValue) {
      return GetIniValue("Fonts", key, defaultValue);
    }

    public long SetIniValueFonts(string key, string value) {
      return SetIniValue("Fonts", key, value);
    }

    public string ReadMilkwaveAudioDevice() {
      return GetIniValue("Milkwave", "AudioDevice", "");
    }

    public void FillAudioDevices(ComboBox cbo) {
      cbo.Items.Clear(); // Clear existing items

      MMDevice defaultMDevice;
      string iniMilkwaveAudioDevice = ReadMilkwaveAudioDevice();

      using (var enumerator = new MMDeviceEnumerator()) {
        defaultMDevice = enumerator.GetDefaultAudioEndpoint(DataFlow.Render, Role.Multimedia);
        var devices = enumerator.EnumerateAudioEndPoints(DataFlow.Render, DeviceState.Active);

        foreach (var device in devices) {
          bool isDefaultDevice = device.ID == defaultMDevice.ID;
          string name = includeInputDevices ? "Out: " + device.FriendlyName : device.FriendlyName;
          cbo.Items.Add(new ComboBoxItemDevice(name, device, isDefaultDevice)); // Add device names to ComboBox
        }

        if (includeInputDevices) {
          devices = enumerator.EnumerateAudioEndPoints(DataFlow.Capture, DeviceState.Active);
          foreach (var device in devices) {
            cbo.Items.Add(new ComboBoxItemDevice("In: " + device.FriendlyName, device, false)); // Add device names to ComboBox
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
        if (item.Device.FriendlyName.Equals(deviceName) || deviceName.Length == 0 && item.IsDefaultDevice) {
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

