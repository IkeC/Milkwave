using System.Collections.Generic;
using System.Linq;

namespace MilkwaveRemote.Helper {
  /// <summary>
  /// Device manager for UI integration
  /// Provides methods to populate ComboBoxes and handle device selection
  /// Follows OBS Studio's device enumeration patterns
  /// </summary>
  public class DeviceManager {

    #region Video Devices

    /// <summary>
    /// Populate a ComboBox with available DirectShow video devices
    /// </summary>
    /// <param name="comboBox">Target ComboBox to populate</param>
    /// <param name="selectedDeviceName">Previously selected device name to restore</param>
    public static void PopulateVideoDevices(ComboBox comboBox, string? selectedDeviceName = null) {
      comboBox.Items.Clear();

      try {
        var devices = DeviceEnumerator.EnumerateVideoDevices();

        if (devices.Count == 0) {
          comboBox.Items.Add("No video devices found");
          comboBox.SelectedIndex = 0;
          comboBox.Enabled = false;
          return;
        }

        comboBox.Enabled = true;

        // Add devices sorted by name
        foreach (var device in devices.OrderBy(d => d.Name)) {
          comboBox.Items.Add(device);
        }

        // Restore previous selection or select first device
        if (!string.IsNullOrEmpty(selectedDeviceName)) {
          foreach (DeviceEnumerator.DeviceItem item in comboBox.Items) {
            if (item.Name == selectedDeviceName) {
              comboBox.SelectedItem = item;
              return;
            }
          }
        }

        // Default to first device
        if (comboBox.Items.Count > 0) {
          comboBox.SelectedIndex = 0;
        }
      } catch (Exception ex) {
        System.Diagnostics.Debug.WriteLine($"Error populating video devices: {ex.Message}");
        comboBox.Items.Add("Error loading devices");
        comboBox.Enabled = false;
      }
    }

    /// <summary>
    /// Populate a ComboBox with available DirectShow audio devices
    /// </summary>
    /// <param name="comboBox">Target ComboBox to populate</param>
    /// <param name="selectedDeviceName">Previously selected device name to restore</param>
    public static void PopulateAudioInputDevices(ComboBox comboBox, string? selectedDeviceName = null) {
      comboBox.Items.Clear();

      try {
        var devices = DeviceEnumerator.EnumerateAudioInputDevices();

        if (devices.Count == 0) {
          comboBox.Items.Add("No audio input devices found");
          comboBox.SelectedIndex = 0;
          comboBox.Enabled = false;
          return;
        }

        comboBox.Enabled = true;

        // Add devices sorted by name
        foreach (var device in devices.OrderBy(d => d.Name)) {
          comboBox.Items.Add(device);
        }

        // Restore previous selection
        if (!string.IsNullOrEmpty(selectedDeviceName)) {
          foreach (DeviceEnumerator.DeviceItem item in comboBox.Items) {
            if (item.Name == selectedDeviceName) {
              comboBox.SelectedItem = item;
              return;
            }
          }
        }

        // Default to first device
        if (comboBox.Items.Count > 0) {
          comboBox.SelectedIndex = 0;
        }
      } catch (Exception ex) {
        System.Diagnostics.Debug.WriteLine($"Error populating audio devices: {ex.Message}");
        comboBox.Items.Add("Error loading devices");
        comboBox.Enabled = false;
      }
    }

    #endregion

    #region Spout Senders

    /// <summary>
    /// Populate a ComboBox with available Spout senders
    /// </summary>
    /// <param name="comboBox">Target ComboBox to populate</param>
    /// <param name="selectedSenderName">Previously selected sender name to restore</param>
    public static void PopulateSpoutSenders(ComboBox comboBox, string? selectedSenderName = null) {
      comboBox.Items.Clear();

      try {
        var senders = DeviceEnumerator.EnumerateSpoutSenders();

        if (senders.Count == 0) {
          comboBox.Items.Add("No Spout senders available");
          comboBox.SelectedIndex = 0;
          comboBox.Enabled = false;
          return;
        }

        comboBox.Enabled = true;

        // Add "None" option first
        comboBox.Items.Add(new DeviceEnumerator.DeviceItem("(None)"));

        // Add senders sorted by name
        foreach (var sender in senders.OrderBy(s => s.Name)) {
          comboBox.Items.Add(sender);
        }

        // Try to restore previous selection
        if (!string.IsNullOrEmpty(selectedSenderName)) {
          foreach (DeviceEnumerator.DeviceItem item in comboBox.Items) {
            if (item.Name == selectedSenderName) {
              comboBox.SelectedItem = item;
              return;
            }
          }
        }

        // Try to select active sender
        var activeSender = DeviceEnumerator.GetActiveSpoutSender();
        if (!string.IsNullOrEmpty(activeSender)) {
          foreach (DeviceEnumerator.DeviceItem item in comboBox.Items) {
            if (item.Name == activeSender) {
              comboBox.SelectedItem = item;
              return;
            }
          }
        }

        // Default to first item
        if (comboBox.Items.Count > 0) {
          comboBox.SelectedIndex = 0;
        }
      } catch (Exception ex) {
        System.Diagnostics.Debug.WriteLine($"Error populating Spout senders: {ex.Message}");
        comboBox.Items.Add("Error loading senders");
        comboBox.Enabled = false;
      }
    }

    /// <summary>
    /// Refresh Spout sender list, attempting to preserve current selection
    /// </summary>
    /// <param name="comboBox">ComboBox to refresh</param>
    public static void RefreshSpoutSenders(ComboBox comboBox) {
      string? currentSelection = null;

      // Save current selection
      if (comboBox.SelectedItem is DeviceEnumerator.DeviceItem currentDevice) {
        currentSelection = currentDevice.Name;
      }

      // Repopulate with saved selection
      PopulateSpoutSenders(comboBox, currentSelection);
    }

    /// <summary>
    /// Get the currently selected Spout sender name from a ComboBox
    /// </summary>
    /// <param name="comboBox">Source ComboBox</param>
    /// <returns>Selected sender name or null</returns>
    public static string? GetSelectedSpoutSender(ComboBox comboBox) {
      if (comboBox.SelectedItem is DeviceEnumerator.DeviceItem device) {
        // Return null for "(None)" option
        return device.Name == "(None)" ? null : device.Name;
      }

      return null;
    }

    /// <summary>
    /// Get information about a specific Spout sender
    /// </summary>
    public static (int Width, int Height, bool Success) GetSpoutSenderInfo(string senderName) {
      return DeviceEnumerator.GetSpoutSenderInfo(senderName);
    }

    #endregion

    #region Utilities

    /// <summary>
    /// Get a device by name from a list of devices
    /// </summary>
    public static DeviceEnumerator.DeviceItem? FindDeviceByName(
      List<DeviceEnumerator.DeviceItem> devices,
      string deviceName) {
      return devices.FirstOrDefault(d => d.Name == deviceName);
    }

    /// <summary>
    /// Check if device enumeration is available on this system
    /// </summary>
    public static bool IsDeviceEnumerationAvailable() {
      try {
        var devices = DeviceEnumerator.EnumerateVideoDevices();
        return true;
      } catch {
        return false;
      }
    }

    #endregion

  }
}
