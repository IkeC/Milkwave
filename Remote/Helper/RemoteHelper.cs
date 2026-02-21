using NAudio.CoreAudioApi;
using System.Runtime.InteropServices;
using System.Text;
using System.Management;

namespace MilkwaveRemote.Helper {

  // COM interface for DirectShow property bag
  [ComImport, Guid("55272A00-42CB-11CE-8135-00AA004BB851")]
  [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
  internal interface IPropertyBag {
    [PreserveSig]
    int Read(
      [In, MarshalAs(UnmanagedType.LPWStr)] string propertyName,
      [In, Out] ref object? value,
      [In] IntPtr errorLog);

    [PreserveSig]
    int Write(
      [In, MarshalAs(UnmanagedType.LPWStr)] string propertyName,
      [In] ref object value);
  }

  [ComImport, Guid("29840822-5B84-11D0-BD3B-00A0C911CE86")]
  [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
  internal interface ICreateDevEnum {
    [PreserveSig]
    int CreateClassEnumerator(
      [In] ref Guid clsidDeviceClass,
      [Out] out IEnumMoniker? enumMoniker,
      [In] int flags);
  }

  [ComImport, Guid("00000102-0000-0000-C000-000000000046")]
  [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
  internal interface IEnumMoniker {
    [PreserveSig]
    int Next(
      [In] int celt,
      [Out, MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 0)] IMoniker[] rgelt,
      [Out] out int pceltFetched);

    [PreserveSig]
    int Skip([In] int celt);

    void Reset();
    void Clone([Out] out IEnumMoniker ppenum);
  }

  [ComImport, Guid("0000000f-0000-0000-C000-000000000046")]
  [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
  internal interface IMoniker {
    void GetClassID(out Guid pClassID);
    void IsDirty();
    void Load(IStream pStm);
    void Save(IStream pStm, bool fClearDirty);
    void GetSizeMax(out long pcbSize);

    [PreserveSig]
    int BindToObject(
      IBindCtx? pbc,
      IMoniker? pmkToLeft,
      [In] ref Guid riidResult,
      [Out, MarshalAs(UnmanagedType.IUnknown)] out object? ppvResult);

    [PreserveSig]
    int BindToStorage(
      IBindCtx? pbc,
      IMoniker? pmkToLeft,
      [In] ref Guid riid,
      [Out, MarshalAs(UnmanagedType.Interface)] out object? ppvObj);

    void Reduce(IBindCtx pbc, int dwReduceHowFar, ref IMoniker ppmkToLeft, out IMoniker ppmkReduced);
    void ComposeWith(IMoniker pmkRight, bool fOnlyIfNotGeneric, out IMoniker ppmkComposite);
    void Enum(bool fForward, out IEnumMoniker ppenumMoniker);
    void IsEqual(IMoniker pmkOtherMoniker);
    void Hash(out int pdwHash);
    void IsRunning(IBindCtx pbc, IMoniker pmkToLeft, IMoniker pmkNewlyRunning);
    void GetTimeOfLastChange(IBindCtx pbc, IMoniker pmkToLeft, out System.Runtime.InteropServices.ComTypes.FILETIME pFileTime);
    void Inverse(out IMoniker ppmk);
    void CommonPrefixWith(IMoniker pmkOther, out IMoniker ppmkPrefix);
    void RelativePathTo(IMoniker pmkOther, out IMoniker ppmkRelPath);
    void GetDisplayName(IBindCtx pbc, IMoniker pmkToLeft, out string ppszDisplayName);
    void ParseDisplayName(IBindCtx pbc, IMoniker pmkToLeft, string pszDisplayName, out int pchEaten, out IMoniker ppmkOut);
    void IsSystemMoniker(out int pdwMonikerType);
  }

  [ComImport, Guid("0000000e-0000-0000-C000-000000000046")]
  [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
  internal interface IBindCtx {
  }

  [ComImport, Guid("0000000c-0000-0000-C000-000000000046")]
  [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
  internal interface IStream {
  }

  // CLSID for System Device Enumerator
  internal static class DsGuid {
    public static readonly Guid CLSID_SystemDeviceEnum = new Guid("62BE5D10-60EB-11d0-BD3B-00A0C911CE86");
    public static readonly Guid CLSID_VideoInputDeviceCategory = new Guid("860BB310-5D01-11d0-BD3B-00A0C911CE86");
    public static readonly Guid IID_IPropertyBag = new Guid("55272A00-42CB-11CE-8135-00AA004BB851");
  }

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
        // Create System Device Enumerator
        var deviceEnumType = Type.GetTypeFromCLSID(DsGuid.CLSID_SystemDeviceEnum);
        if (deviceEnumType == null) {
          return devices;
        }

        var deviceEnumerator = (ICreateDevEnum?)Activator.CreateInstance(deviceEnumType);
        if (deviceEnumerator == null) {
          return devices;
        }

        try {
          // Enumerate video input devices
          Guid videoInputCategory = DsGuid.CLSID_VideoInputDeviceCategory;
          int hr = deviceEnumerator.CreateClassEnumerator(ref videoInputCategory, out IEnumMoniker? enumMoniker, 0);

          if (hr != 0 || enumMoniker == null) {
            return devices;
          }

          try {
            IMoniker[] monikers = new IMoniker[1];
            int fetched = 0;

            while (enumMoniker.Next(1, monikers, out fetched) == 0 && fetched > 0) {
              IMoniker moniker = monikers[0];
              try {
                // Get property bag from moniker
                Guid propBagGuid = DsGuid.IID_IPropertyBag;
                hr = moniker.BindToStorage(null, null, ref propBagGuid, out object? bagObj);

                if (hr == 0 && bagObj is IPropertyBag propertyBag) {
                  try {
                    // Try to read FriendlyName first
                    object? nameObj = null;
                    hr = propertyBag.Read("FriendlyName", ref nameObj, IntPtr.Zero);

                    string? deviceName = null;
                    if (hr == 0 && nameObj != null) {
                      deviceName = nameObj.ToString();
                    }

                    // If FriendlyName failed, try Description
                    if (string.IsNullOrEmpty(deviceName)) {
                      nameObj = null;
                      hr = propertyBag.Read("Description", ref nameObj, IntPtr.Zero);
                      if (hr == 0 && nameObj != null) {
                        deviceName = nameObj.ToString();
                      }
                    }

                    if (!string.IsNullOrEmpty(deviceName) && !devices.Contains(deviceName)) {
                      devices.Add(deviceName);
                    }
                  } finally {
                    Marshal.ReleaseComObject(propertyBag);
                  }
                }
              } finally {
                Marshal.ReleaseComObject(moniker);
              }
            }
          } finally {
            Marshal.ReleaseComObject(enumMoniker);
          }
        } finally {
          Marshal.ReleaseComObject(deviceEnumerator);
        }
      } catch (Exception ex) {
        // If DirectShow enumeration fails completely, fall back to WMI
        System.Diagnostics.Debug.WriteLine($"DirectShow enumeration failed: {ex.Message}");
        try {
          using (var searcher = new ManagementObjectSearcher("SELECT * FROM Win32_PnPEntity WHERE (PNPClass = 'Image' OR PNPClass = 'Camera')")) {
            foreach (ManagementObject device in searcher.Get()) {
              var name = device["Caption"]?.ToString();
              if (!string.IsNullOrEmpty(name) && !devices.Contains(name)) {
                devices.Add(name);
              }
            }
          }
        } catch { }
      }

      return devices;
    }

    // P/Invoke for Spout sender enumeration
    // Spout uses shared memory to track active senders
    [DllImport("kernel32.dll", SetLastError = true)]
    private static extern IntPtr CreateFileMapping(
        IntPtr hFile,
        IntPtr lpFileMappingAttributes,
        uint flProtect,
        uint dwMaximumSizeHigh,
        uint dwMaximumSizeLow,
        string lpName);

    [DllImport("kernel32.dll", SetLastError = true)]
    private static extern IntPtr MapViewOfFile(
        IntPtr hFileMappingObject,
        uint dwDesiredAccess,
        uint dwFileOffsetHigh,
        uint dwFileOffsetLow,
        IntPtr dwNumberOfBytesToMap);

    [DllImport("kernel32.dll", SetLastError = true)]
    private static extern bool UnmapViewOfFile(IntPtr lpBaseAddress);

    [DllImport("kernel32.dll", SetLastError = true)]
    private static extern bool CloseHandle(IntPtr hObject);

    private const uint FILE_MAP_READ = 4;
    private const uint PAGE_READONLY = 2;

    public static List<string> GetSpoutSenders() {
      var senders = new List<string>();
      System.Diagnostics.Debug.WriteLine("=== GetSpoutSenders: Starting Spout sender enumeration ===");

      try {
        IntPtr hMapFile = OpenFileMapping(FILE_MAP_READ, false, "SpoutSenderNames");

        if (hMapFile != IntPtr.Zero) {
          System.Diagnostics.Debug.WriteLine("GetSpoutSenders: Found SpoutSenderNames shared memory");

          // Standard Spout 2 shared memory layout for "SpoutSenderNames":
          // No header! Just slots of 256 bytes each.
          // SpoutSDK iterates up to m_MaxSenders (usually 64 or 256).
          const int maxSlots = 64; 
          const int bufferSize = maxSlots * 256;
          IntPtr pBuf = MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, new IntPtr(bufferSize));

          if (pBuf != IntPtr.Zero) {
            try {
              byte[] buffer = new byte[bufferSize];
              System.Runtime.InteropServices.Marshal.Copy(pBuf, buffer, 0, bufferSize);

              int offset = 0; // Spout starts names at offset 0
              for (int i = 0; i < maxSlots; i++) {
                int slotStart = offset + (i * 256);
                if (slotStart >= bufferSize - 1) break;

                // Spout SDK specifies that an empty first byte terminates the active list
                if (buffer[slotStart] == 0) {
                  System.Diagnostics.Debug.WriteLine($"GetSpoutSenders: Slot {i} is empty, stopping.");
                  break; 
                }

                // Find null terminator within this 256-byte slot
                int nullPos = System.Array.IndexOf(buffer, (byte)0, slotStart, 256);
                int strLen = (nullPos >= 0) ? (nullPos - slotStart) : 256;

                if (strLen > 0) {
                  try {
                    string senderName = System.Text.Encoding.ASCII.GetString(buffer, slotStart, strLen).Trim();
                    // Validation: must not be empty and must look like a valid name
                    if (!string.IsNullOrWhiteSpace(senderName) && char.IsLetterOrDigit(senderName[0])) {
                      if (!senders.Contains(senderName)) {
                        System.Diagnostics.Debug.WriteLine($"GetSpoutSenders: Found sender '{senderName}' in slot {i}");
                        senders.Add(senderName);
                      }
                    }
                  } catch (Exception ex) {
                    System.Diagnostics.Debug.WriteLine($"GetSpoutSenders: Error decoding slot {i}: {ex.Message}");
                  }
                }
              }
            } finally {
              UnmapViewOfFile(pBuf);
            }
          }
          CloseHandle(hMapFile);
        }
      } catch (Exception ex) {
        System.Diagnostics.Debug.WriteLine($"GetSpoutSenders Exception: {ex.Message}");
      }

      System.Diagnostics.Debug.WriteLine($"=== GetSpoutSenders: Found {senders.Count} senders ===");
      return senders;
    }

    [DllImport("kernel32.dll", SetLastError = true)]
    private static extern IntPtr OpenFileMapping(
        uint dwDesiredAccess,
        bool bInheritHandle,
        string lpName);

    /// <summary>
    /// DIAGNOSTIC ONLY: Write a test sender to the registry for debugging Spout sender discovery.
    /// This simulates what a Spout sender application should do when it activates.
    /// </summary>
    public static void WriteTestSpoutSender(string senderName = "MilkwaveVisualizer") {
      try {
        using (var key = Microsoft.Win32.Registry.CurrentUser.CreateSubKey(@"Software\Leading Edge\Spout")) {
          if (key != null) {
            // Write the sender name with a dummy value (Spout just checks for presence)
            key.SetValue(senderName, 1);
            System.Diagnostics.Debug.WriteLine($"WriteTestSpoutSender: Registered '{senderName}' in registry");
          }
        }
      } catch (Exception ex) {
        System.Diagnostics.Debug.WriteLine($"WriteTestSpoutSender: Failed to write registry: {ex.Message}");
      }
    }
  }
}



