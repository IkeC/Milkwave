using System.Runtime.InteropServices;
using System.Text;
using System.Collections.Generic;

namespace MilkwaveRemote.Helper {
  /// <summary>
  /// Device enumerator following OBS Studio's pattern for device discovery.
  /// Supports DirectShow video devices and Spout senders enumeration.
  /// </summary>
  public static class DeviceEnumerator {

    #region COM Interfaces - DirectShow

    /// <summary>COM interface for DirectShow property bag</summary>
    [ComImport, Guid("55272A00-42CB-11CE-8135-00AA004BB851")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    private interface IPropertyBag {
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

    /// <summary>COM interface for DirectShow device enumerator</summary>
    [ComImport, Guid("29840822-5B84-11D0-BD3B-00A0C911CE86")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    private interface ICreateDevEnum {
      [PreserveSig]
      int CreateClassEnumerator(
        [In] ref Guid clsidDeviceClass,
        [Out] out IEnumMoniker? enumMoniker,
        [In] int flags);
    }

    /// <summary>COM interface for moniker enumeration</summary>
    [ComImport, Guid("00000102-0000-0000-C000-000000000046")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    private interface IEnumMoniker {
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

    /// <summary>COM interface for moniker</summary>
    [ComImport, Guid("0000000f-0000-0000-C000-000000000046")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    private interface IMoniker {
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

    /// <summary>COM interface for bind context</summary>
    [ComImport, Guid("0000000e-0000-0000-C000-000000000046")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    private interface IBindCtx {
    }

    /// <summary>COM interface for stream</summary>
    [ComImport, Guid("0000000c-0000-0000-C000-000000000046")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    private interface IStream {
    }

    #endregion

    #region DirectShow GUIDs

    private static class DirectShowGUIDs {
      public static readonly Guid CLSID_SystemDeviceEnum = new Guid("62BE5D10-60EB-11d0-BD3B-00A0C911CE86");
      public static readonly Guid CLSID_VideoInputDeviceCategory = new Guid("860BB310-5D01-11d0-BD3B-00A0C911CE86");
      public static readonly Guid CLSID_AudioInputDeviceCategory = new Guid("33D9A762-90C8-11d0-BD43-00A0C911CE86");
      public static readonly Guid IID_IPropertyBag = new Guid("55272A00-42CB-11CE-8135-00AA004BB851");
    }

    #endregion

    #region Device Classes

    /// <summary>
    /// Represents a device item with name and optional path/ID
    /// </summary>
    public class DeviceItem {
      public string Name { get; set; }
      public string? DevicePath { get; set; }
      public string? DeviceID { get; set; }
      public string? FriendlyName { get; set; }
      public bool IsDefault { get; set; }

      public DeviceItem(string name, string? devicePath = null, string? deviceID = null) {
        Name = name;
        DevicePath = devicePath;
        DeviceID = deviceID;
      }

      public override string ToString() => Name;
    }

    #endregion

    #region Public Methods - DirectShow Video Devices

    /// <summary>
    /// Enumerate DirectShow video input devices (cameras, capture cards)
    /// Follows OBS Studio's pattern for device discovery
    /// </summary>
    /// <returns>List of available video input devices</returns>
    public static List<DeviceItem> EnumerateVideoDevices() {
      var devices = new List<DeviceItem>();

      try {
        // Create System Device Enumerator (OBS pattern)
        var deviceEnumType = Type.GetTypeFromCLSID(DirectShowGUIDs.CLSID_SystemDeviceEnum);
        if (deviceEnumType == null) {
          return devices; // Return empty list if enumerator unavailable
        }

        var deviceEnumerator = (ICreateDevEnum?)Activator.CreateInstance(deviceEnumType);
        if (deviceEnumerator == null) {
          return devices;
        }

        try {
          // Enumerate video input devices (OBS pattern)
          Guid videoInputCategory = DirectShowGUIDs.CLSID_VideoInputDeviceCategory;
          int hr = deviceEnumerator.CreateClassEnumerator(ref videoInputCategory, out IEnumMoniker? enumMoniker, 0);

          if (hr != 0 || enumMoniker == null) {
            return devices; // No devices found or error
          }

          try {
            // Iterate through monikers
            IMoniker[] monikers = new IMoniker[1];
            int fetched = 0;

            while (enumMoniker.Next(1, monikers, out fetched) == 0 && fetched > 0) {
              IMoniker moniker = monikers[0];
              try {
                string? deviceName = ExtractDeviceProperty(moniker, "FriendlyName");
                if (deviceName != null) {
                  string? devicePath = ExtractDeviceProperty(moniker, "DevicePath");
                  var device = new DeviceItem(deviceName, devicePath);
                  devices.Add(device);
                }
              } catch (Exception ex) {
                // Log error but continue enumeration (OBS pattern)
                System.Diagnostics.Debug.WriteLine($"Error enumerating video device: {ex.Message}");
              }
            }
          } finally {
            Marshal.ReleaseComObject(enumMoniker);
          }
        } finally {
          Marshal.ReleaseComObject(deviceEnumerator);
        }
      } catch (Exception ex) {
        System.Diagnostics.Debug.WriteLine($"Error in video device enumeration: {ex.Message}");
      }

      return devices;
    }

    /// <summary>
    /// Enumerate DirectShow audio input devices
    /// </summary>
    /// <returns>List of available audio input devices</returns>
    public static List<DeviceItem> EnumerateAudioInputDevices() {
      var devices = new List<DeviceItem>();

      try {
        var deviceEnumType = Type.GetTypeFromCLSID(DirectShowGUIDs.CLSID_SystemDeviceEnum);
        if (deviceEnumType == null) {
          return devices;
        }

        var deviceEnumerator = (ICreateDevEnum?)Activator.CreateInstance(deviceEnumType);
        if (deviceEnumerator == null) {
          return devices;
        }

        try {
          Guid audioInputCategory = DirectShowGUIDs.CLSID_AudioInputDeviceCategory;
          int hr = deviceEnumerator.CreateClassEnumerator(ref audioInputCategory, out IEnumMoniker? enumMoniker, 0);

          if (hr != 0 || enumMoniker == null) {
            return devices;
          }

          try {
            IMoniker[] monikers = new IMoniker[1];
            int fetched = 0;

            while (enumMoniker.Next(1, monikers, out fetched) == 0 && fetched > 0) {
              IMoniker moniker = monikers[0];
              try {
                string? deviceName = ExtractDeviceProperty(moniker, "FriendlyName");
                if (deviceName != null) {
                  var device = new DeviceItem(deviceName);
                  devices.Add(device);
                }
              } catch (Exception ex) {
                System.Diagnostics.Debug.WriteLine($"Error enumerating audio device: {ex.Message}");
              }
            }
          } finally {
            Marshal.ReleaseComObject(enumMoniker);
          }
        } finally {
          Marshal.ReleaseComObject(deviceEnumerator);
        }
      } catch (Exception ex) {
        System.Diagnostics.Debug.WriteLine($"Error in audio device enumeration: {ex.Message}");
      }

      return devices;
    }

    #endregion

    #region Public Methods - Spout Senders

    /// <summary>
    /// Enumerate available Spout senders via shared memory registry
    /// </summary>
    /// <returns>List of available Spout sender names</returns>
    public static List<DeviceItem> EnumerateSpoutSenders() {
      var senders = new List<DeviceItem>();

      try {
        // Access Spout registry for max senders
        int maxSenders = GetSpoutMaxSenderCount();
        if (maxSenders <= 0) {
          return senders; // No Spout configuration found
        }

        // Read sender names from registry
        var senderSet = ReadSpoutSenderNamesFromRegistry();
        foreach (var senderName in senderSet) {
          var sender = new DeviceItem(senderName);
          senders.Add(sender);
        }
      } catch (Exception ex) {
        System.Diagnostics.Debug.WriteLine($"Error enumerating Spout senders: {ex.Message}");
      }

      return senders;
    }

    /// <summary>
    /// Get the currently active Spout sender
    /// </summary>
    /// <returns>Active sender name or null if none</returns>
    public static string? GetActiveSpoutSender() {
      try {
        using (var key = Microsoft.Win32.Registry.CurrentUser.OpenSubKey(@"Software\Leading Edge\Spout")) {
          if (key == null) {
            return null;
          }

          var activeSender = key.GetValue("ActiveSender");
          return activeSender?.ToString();
        }
      } catch (Exception ex) {
        System.Diagnostics.Debug.WriteLine($"Error getting active Spout sender: {ex.Message}");
        return null;
      }
    }

    /// <summary>
    /// Get Spout sender information (resolution, etc.)
    /// </summary>
    public static (int Width, int Height, bool Success) GetSpoutSenderInfo(string senderName) {
      try {
        using (var key = Microsoft.Win32.Registry.CurrentUser.OpenSubKey($@"Software\Leading Edge\Spout\Senders\{senderName}")) {
          if (key == null) {
            return (0, 0, false);
          }

          var width = key.GetValue("Width");
          var height = key.GetValue("Height");

          if (width is int w && height is int h) {
            return (w, h, true);
          }
        }
      } catch (Exception ex) {
        System.Diagnostics.Debug.WriteLine($"Error getting Spout sender info for '{senderName}': {ex.Message}");
      }

      return (0, 0, false);
    }

    #endregion

    #region Private Helper Methods

    /// <summary>
    /// Extract a property from a DirectShow moniker (OBS pattern)
    /// </summary>
    private static string? ExtractDeviceProperty(IMoniker moniker, string propertyName) {
      try {
        Guid propBagGuid = DirectShowGUIDs.IID_IPropertyBag;
        int hr = moniker.BindToStorage(null, null, ref propBagGuid, out object? bagObj);

        if (hr == 0 && bagObj is IPropertyBag propertyBag) {
          try {
            object? propValue = null;
            hr = propertyBag.Read(propertyName, ref propValue, IntPtr.Zero);

            if (hr == 0 && propValue != null) {
              return propValue.ToString();
            }
          } finally {
            Marshal.ReleaseComObject(propertyBag);
          }
        }
      } catch (Exception ex) {
        System.Diagnostics.Debug.WriteLine($"Error extracting device property '{propertyName}': {ex.Message}");
      }

      return null;
    }

    /// <summary>
    /// Get maximum number of Spout senders from registry
    /// </summary>
    private static int GetSpoutMaxSenderCount() {
      try {
        using (var key = Microsoft.Win32.Registry.CurrentUser.OpenSubKey(@"Software\Leading Edge\Spout")) {
          if (key == null) {
            return 64; // Default
          }

          var maxSenders = key.GetValue("MaxSenders");
          if (maxSenders is int count) {
            return Math.Max(1, count); // Ensure at least 1
          }
        }
      } catch (Exception ex) {
        System.Diagnostics.Debug.WriteLine($"Error getting Spout max senders: {ex.Message}");
      }

      return 64; // Default to 64 if registry not found
    }

    /// <summary>
    /// Read Spout sender names from registry
    /// </summary>
    private static HashSet<string> ReadSpoutSenderNamesFromRegistry() {
      var senderNames = new HashSet<string>();

      try {
        using (var key = Microsoft.Win32.Registry.CurrentUser.OpenSubKey(@"Software\Leading Edge\Spout\Senders")) {
          if (key == null) {
            return senderNames;
          }

          // Get all sender subkeys
          var subKeyNames = key.GetSubKeyNames();
          foreach (var subKeyName in subKeyNames) {
            // Filter out system entries
            if (!subKeyName.StartsWith("_") && !string.IsNullOrWhiteSpace(subKeyName)) {
              senderNames.Add(subKeyName);
            }
          }
        }
      } catch (Exception ex) {
        System.Diagnostics.Debug.WriteLine($"Error reading Spout senders from registry: {ex.Message}");
      }

      return senderNames;
    }

    #endregion

  }
}
