using NAudio.CoreAudioApi;
using System.Runtime.InteropServices;
using System.Text;
using System.Management;

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

    public void FillAudioDevices(ComboBox cbo) {
      cbo.Items.Clear(); // Clear existing items

      MMDevice defaultMDevice;
      string iniMilkwaveAudioDevice = GetIniValue("Milkwave", "AudioDevice", "");
      string iniAudioDeviceRequestType = GetIniValue("Milkwave", "AudioDeviceRequestType", "0"); // 0: Undefined, 1: Capture (in), 2: Render (out)

      using (var enumerator = new MMDeviceEnumerator()) {
        defaultMDevice = enumerator.GetDefaultAudioEndpoint(DataFlow.Render, Role.Multimedia);
        var devices = enumerator.EnumerateAudioEndPoints(DataFlow.Render, DeviceState.Active);

        foreach (var device in devices) {
          bool isDefaultDevice = device.ID == defaultMDevice.ID;
          string name = includeInputDevices ? "Out: " + device.FriendlyName : device.FriendlyName;
          cbo.Items.Add(new ComboBoxItemDevice(name, device, isDefaultDevice, false)); // Add device names to ComboBox
        }

        if (includeInputDevices) {
          devices = enumerator.EnumerateAudioEndPoints(DataFlow.Capture, DeviceState.Active);
          foreach (var device in devices) {
            cbo.Items.Add(new ComboBoxItemDevice("In: " + device.FriendlyName, device, false, true)); // Add device names to ComboBox
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
                if (!item.IsInputDevice && iniAudioDeviceRequestType == "1" || item.IsInputDevice && iniAudioDeviceRequestType == "2") {
                  // the item device type is not the requested type, skip it
                  continue;
                }
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

      public ComboBoxItemDevice(string text, MMDevice device, bool isDefaultDevice, bool isInputDevice) {
        Text = text;
        Device = device;
        IsDefaultDevice = isDefaultDevice;
        IsInputDevice = isInputDevice;
      }

      public string Text { get; set; }
      public MMDevice Device { get; set; }
      public bool IsDefaultDevice { get; set; } = false;
      public bool IsInputDevice { get; set; } = false;
      public override string ToString() {
        return Text;
      }

    }

    public static List<string> GetVideoInputDevices() {
      var devices = new List<string>();
      try {
        using (var searcher = new ManagementObjectSearcher("SELECT * FROM Win32_PnPEntity WHERE (PNPClass = 'Image' OR PNPClass = 'Camera')")) {
          foreach (ManagementObject device in searcher.Get()) {
            var name = device["Caption"]?.ToString();
            if (!string.IsNullOrEmpty(name)) {
              devices.Add(name);
            }
          }
        }
      } catch (Exception) {
        // Fallback: return empty list if enumeration fails
      }
      return devices;
    }
  }
}

