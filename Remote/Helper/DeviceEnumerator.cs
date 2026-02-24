using System.Runtime.InteropServices;
using System.Text;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;

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

    #region Win32 API - Shared Memory (Spout)

    private const uint FILE_MAP_READ = 4;

    [DllImport("kernel32.dll", SetLastError = true)]
    private static extern IntPtr OpenFileMapping(uint dwDesiredAccess, bool bInheritHandle, string lpName);

    [DllImport("kernel32.dll", SetLastError = true)]
    private static extern IntPtr MapViewOfFile(IntPtr hFileMappingObject, uint dwDesiredAccess, uint dwFileOffsetHigh, uint dwFileOffsetLow, IntPtr dwNumberOfBytesToMap);

    [DllImport("kernel32.dll", SetLastError = true)]
    private static extern bool UnmapViewOfFile(IntPtr lpBaseAddress);

    [DllImport("kernel32.dll", SetLastError = true)]
    private static extern bool CloseHandle(IntPtr hObject);

    #endregion

    #region Win32 API - Game Controllers (winmm.dll)

    [DllImport("winmm.dll")]
    private static extern int joyGetNumDevs();

    [DllImport("winmm.dll", CharSet = CharSet.Auto)]
    private static extern int joyGetDevCaps(int uJoyID, out JOYCAPS pjc, int cbjc);

    [DllImport("winmm.dll")]
    private static extern int joyGetPosEx(int uJoyID, ref JOYINFOEX pji);

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Auto)]
    private struct JOYCAPS {
      public ushort wMid;
      public ushort wPid;
      [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
      public string szPname;
      public uint wXmin;
      public uint wXmax;
      public uint wYmin;
      public uint wYmax;
      public uint wZmin;
      public uint wZmax;
      public uint wNumButtons;
      public uint wPeriodMin;
      public uint wPeriodMax;
      public uint wRmin;
      public uint wRmax;
      public uint wUmin;
      public uint wUmax;
      public uint wVmin;
      public uint wVmax;
      public uint wCaps;
      public uint wMaxAxes;
      public uint wNumAxes;
      public uint wMaxButtons;
      [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
      public string szRegKey;
      [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 260)]
      public string szOEMVxD;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct JOYINFOEX {
      public uint dwSize;
      public uint dwFlags;
      public uint dwXpos;
      public uint dwYpos;
      public uint dwZpos;
      public uint dwRpos;
      public uint dwUpos;
      public uint dwVpos;
      public uint dwButtons;
      public uint dwButtonNumber;
      public uint dwPOV;
      public uint dwReserved1;
      public uint dwReserved2;
    }

    public const uint JOY_RETURNBUTTONS = 0x80;

    #endregion

    #region Win32 API - Raw Input (user32.dll)

    private const uint RID_INPUT = 0x10000003;
    private const uint RIDI_DEVICENAME = 0x20000007;
    private const uint RIDI_DEVICEINFO = 0x2000000b;

    [DllImport("user32.dll", SetLastError = true)]
    private static extern uint GetRawInputDeviceList(IntPtr pRawInputDeviceList, ref uint puiNumDevices, uint cbSize);

    [DllImport("user32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    private static extern uint GetRawInputDeviceInfo(IntPtr hDevice, uint uiCommand, IntPtr pData, ref uint pcbSize);

    [DllImport("user32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    private static extern uint GetRawInputDeviceInfo(IntPtr hDevice, uint uiCommand, ref RID_DEVICE_INFO pData, ref uint pcbSize);

    [StructLayout(LayoutKind.Sequential)]
    private struct RID_DEVICE_INFO_HID {
      public uint dwVendorId;
      public uint dwProductId;
      public uint dwVersionNumber;
      public ushort usUsagePage;
      public ushort usUsage;
    }

    [StructLayout(LayoutKind.Explicit)]
    private struct RID_DEVICE_INFO_UNION {
      [FieldOffset(0)] public RID_DEVICE_INFO_MOUSE mouse;
      [FieldOffset(0)] public RID_DEVICE_INFO_KEYBOARD keyboard;
      [FieldOffset(0)] public RID_DEVICE_INFO_HID hid;
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct RID_DEVICE_INFO_MOUSE {
      public uint dwId;
      public uint dwNumberOfButtons;
      public uint dwSampleRate;
      public bool fHasHorizontalWheel;
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct RID_DEVICE_INFO_KEYBOARD {
      public uint dwType;
      public uint dwSubType;
      public uint dwKeyboardMode;
      public uint dwNumberOfFunctionKeys;
      public uint dwNumberOfIndicators;
      public uint dwNumberOfKeysTotal;
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct RID_DEVICE_INFO {
      public uint cbSize;
      public uint dwType;
      public RID_DEVICE_INFO_UNION u;
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct RAWINPUTDEVICELIST {
      public IntPtr hDevice;
      public uint dwType;
    }

    private const uint RIM_TYPEHID = 2;

    #endregion

    #region Win32 API - XInput (xinput1_4.dll)

    [DllImport("xinput1_4.dll", EntryPoint = "XInputGetState")]
    private static extern int XInputGetState(int dwUserIndex, ref XINPUT_STATE pState);

    [StructLayout(LayoutKind.Sequential)]
    private struct XINPUT_STATE {
      public uint dwPacketNumber;
      public XINPUT_GAMEPAD Gamepad;
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct XINPUT_GAMEPAD {
      public ushort wButtons;
      public byte bLeftTrigger;
      public byte bRightTrigger;
      public short sThumbLX;
      public short sThumbLY;
      public short sThumbRX;
      public short sThumbRY;
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
      var senderSet = new HashSet<string>(StringComparer.OrdinalIgnoreCase);

      try {
        // Method 1: Shared Memory Discovery (Active senders)
        ReadSpoutSenderNamesFromSharedMemory(senderSet);

        // Method 2: Registry Discovery (Registered senders)
        var registrySenders = ReadSpoutSenderNamesFromRegistry();
        foreach (var name in registrySenders) {
          senderSet.Add(name);
        }
      } catch (Exception ex) {
        System.Diagnostics.Debug.WriteLine($"Error enumerating Spout senders: {ex.Message}");
      }

      var items = new List<DeviceItem>();
      foreach (var name in senderSet) {
        items.Add(new DeviceItem(name));
      }
      return items;
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

    #region Public Methods - Game Controllers

    /// <summary>
    /// Enumerate available game controllers via winmm.dll
    /// </summary>
    /// <returns>List of available game controllers</returns>
    public static List<DeviceItem> EnumerateGameControllers() {
      var controllers = new List<DeviceItem>();
      Debug.WriteLine("[Controller Discovery] Starting enumeration...");

      // 1. Get robust names from Raw Input and XInput
      var rawNames = GetRawInputControllerNames();
      foreach(var n in rawNames) Debug.WriteLine($"[Controller Discovery] Raw Input found: {n}");

      var xinputControllers = GetXInputControllerNames();
      foreach(var n in xinputControllers) Debug.WriteLine($"[Controller Discovery] XInput found: {n}");

      try {
        int numDevs = joyGetNumDevs();
        int xinputIdx = 0;

        for (int i = 0; i < numDevs; i++) {
          if (joyGetDevCaps(i, out JOYCAPS caps, Marshal.SizeOf(typeof(JOYCAPS))) == 0) {
            if (!string.IsNullOrWhiteSpace(caps.szPname)) {
              string displayName = caps.szPname;
              string strategy = "winmm-default";

              // If it's a generic name, seek a better one from ANY source
              if (!IsValidName(displayName)) {
                // Priority 1: XInput matching
                if (IsXboxGenericName(displayName) && xinputIdx < xinputControllers.Count) {
                  displayName = xinputControllers[xinputIdx++];
                  strategy = "XInput-match";
                }
                // Priority 2: Precise Registry/OEM lookup
                else {
                  string? oemName = GetJoystickOEMName(caps, i);
                  if (IsValidName(oemName)) {
                    displayName = oemName!;
                    strategy = "Registry-OEM";
                  } 
                  // Priority 3: fallback to Raw Input
                  else if (rawNames.Count > i && IsValidName(rawNames[i])) {
                    displayName = rawNames[i];
                    strategy = "Raw-Input";
                  }
                  // Priority 4: WMI Fallback
                  else {
                    string? wmiName = GetFriendlyNameFromWmi(caps.wMid, caps.wPid);
                    if (IsValidName(wmiName)) {
                      displayName = wmiName!;
                      strategy = "WMI-PnP";
                    }
                    // Priority 5: Deep Registry Enum Search
                    else {
                      string? deepName = DeepSearchEnumForName(caps.wMid, caps.wPid);
                      if (IsValidName(deepName)) {
                          displayName = deepName!;
                          strategy = "Deep-Registry";
                      }
                    }
                  }
                }
              }

              Debug.WriteLine($"[Controller Discovery] Final Assignment -> '{displayName}' via {strategy}");
              controllers.Add(new DeviceItem(displayName, deviceID: i.ToString()));
            }
          }
        }
      } catch (Exception ex) {
        Debug.WriteLine($"[Controller Discovery] Error: {ex.Message}");
      }

      return controllers;
    }

    /// <summary>
    /// Gets connected XInput controllers
    /// </summary>
    public static List<string> GetXInputControllerNames() {
      var names = new List<string>();
      for (int i = 0; i < 4; i++) {
        XINPUT_STATE state = new XINPUT_STATE();
        if (XInputGetState(i, ref state) == 0) {
          names.Add($"Xbox Controller ({i + 1})");
        }
      }
      return names;
    }

    private static bool IsXboxGenericName(string name) {
      if (string.IsNullOrEmpty(name)) return false;
      return name.Contains("XINPUT", StringComparison.OrdinalIgnoreCase) || 
             name.Contains("Xbox", StringComparison.OrdinalIgnoreCase) || 
             name.Contains("Microsoft PC-Joystick", StringComparison.OrdinalIgnoreCase) ||
             name.Contains("Microsoft-PC-Joystick", StringComparison.OrdinalIgnoreCase);
    }

    /// <summary>
    /// Uses Raw Input API to get friendly names of connected game controllers
    /// </summary>
    private static List<string> GetRawInputControllerNames() {
      var names = new List<string>();
      uint numDevices = 0;
      uint size = (uint)Marshal.SizeOf(typeof(RAWINPUTDEVICELIST));

      if (GetRawInputDeviceList(IntPtr.Zero, ref numDevices, size) == unchecked((uint)-1)) {
        Debug.WriteLine("[Controller Discovery] GetRawInputDeviceList count query failed");
        return names;
      }

      Debug.WriteLine($"[Controller Discovery] Raw Input total devices: {numDevices}");
      if (numDevices == 0) return names;

      IntPtr pRawInputDeviceList = Marshal.AllocHGlobal((int)(size * numDevices));
      try {
        uint fetched = GetRawInputDeviceList(pRawInputDeviceList, ref numDevices, size);
        if (fetched == unchecked((uint)-1)) return names;

        for (int i = 0; i < fetched; i++) {
          RAWINPUTDEVICELIST device = Marshal.PtrToStructure<RAWINPUTDEVICELIST>(pRawInputDeviceList + (int)(i * size));
          if (device.dwType == RIM_TYPEHID) {
             RID_DEVICE_INFO deviceInfo = new RID_DEVICE_INFO();
             deviceInfo.cbSize = (uint)Marshal.SizeOf(typeof(RID_DEVICE_INFO));
             uint infoSize = deviceInfo.cbSize;
             if (GetRawInputDeviceInfo(device.hDevice, RIDI_DEVICEINFO, ref deviceInfo, ref infoSize) != unchecked((uint)-1)) {
                if (deviceInfo.u.hid.usUsagePage == 1 && (deviceInfo.u.hid.usUsage == 4 || deviceInfo.u.hid.usUsage == 5 || deviceInfo.u.hid.usUsage == 8)) {
                    string? name = GetProductNameFromHandle(device.hDevice);
                    Debug.WriteLine($"[Controller Discovery] Raw Input Gamepad match: {name}");
                    if (IsValidName(name)) names.Add(name!);
                }
             }
          }
        }
      } catch (Exception ex) {
        Debug.WriteLine($"[Controller Discovery] Raw Input Error: {ex.Message}");
      } finally {
        Marshal.FreeHGlobal(pRawInputDeviceList);
      }
      return names;
    }

    /// <summary>
    /// Gets the product name from a Raw Input device handle by looking up its registry path
    /// </summary>
    private static string? GetProductNameFromHandle(IntPtr hDevice) {
      uint pcbSize = 0;
      if (GetRawInputDeviceInfo(hDevice, RIDI_DEVICENAME, IntPtr.Zero, ref pcbSize) == unchecked((uint)-1)) return null;
      if (pcbSize == 0) return null;

      IntPtr pData = Marshal.AllocHGlobal((int)(pcbSize * 2));
      try {
        if (GetRawInputDeviceInfo(hDevice, RIDI_DEVICENAME, pData, ref pcbSize) != unchecked((uint)-1)) {
          string deviceName = Marshal.PtrToStringUni(pData)!;
          Debug.WriteLine($"[Controller Discovery] Raw Input device path: {deviceName}");

          if (deviceName.Contains("#") || deviceName.Contains("&")) {
             string regPath = deviceName;
             // Remove various NT path prefixes
             if (regPath.StartsWith(@"\\?\")) regPath = regPath.Substring(4);
             else if (regPath.StartsWith(@"\??\")) regPath = regPath.Substring(4);

             // Change # to \ for registry path
             regPath = regPath.Replace("#", @"\");

             // Strip trailing class GUID if present
             if (regPath.EndsWith("}") && regPath.Contains("{")) {
                int lastSlash = regPath.LastIndexOf('\\');
                if (lastSlash > 0) regPath = regPath.Substring(0, lastSlash);
             }

             Debug.WriteLine($"[Controller Discovery] Registry lookup: HKLM\\System\\CurrentControlSet\\Enum\\{regPath}");

             // Try to look up the name in the Enum registry
             using (var key = Microsoft.Win32.Registry.LocalMachine.OpenSubKey($@"System\CurrentControlSet\Enum\{regPath}")) {
                if (key != null) {
                  string? name = key.GetValue("FriendlyName")?.ToString() ?? key.GetValue("DeviceDesc")?.ToString();
                  if (name != null && name.Contains(";")) name = name.Split(';').Last();
                  if (IsValidName(name)) {
                    Debug.WriteLine($"[Controller Discovery] Found name in Registry: {name}");
                    return name;
                  }
                }
             }
          }
        }
      } finally {
        Marshal.FreeHGlobal(pData);
      }
      return null;
    }

    /// <summary>
    /// Look up the OEM name for a joystick using multiple registry strategies
    /// </summary>
    private static string? GetJoystickOEMName(JOYCAPS caps, int joyID) {
      Debug.WriteLine($"[Controller Discovery] GetJoystickOEMName for JoyID {joyID} (VID_{caps.wMid:X4} PID_{caps.wPid:X4})");
      try {
        // Strategy 0: WMI Lookup (High Priority)
        if (caps.wMid > 1 && caps.wPid > 1) {
          string? wmiName = GetFriendlyNameFromWmi(caps.wMid, caps.wPid);
          if (IsValidName(wmiName)) {
            Debug.WriteLine($"[Controller Discovery] -> Found via Strategy 0 (WMI): {wmiName}");
            return wmiName;
          }
        }

        // Strategy 1: regKey
        if (!string.IsNullOrEmpty(caps.szRegKey)) {
          string? name = ReadOEMNameFromPath(@$"System\CurrentControlSet\Control\MediaProperties\PrivateProperties\Joystick\DirectInput\{caps.szRegKey}");
          if (IsValidName(name)) {
            Debug.WriteLine($"[Controller Discovery] -> Found via Strategy 1 (RegKey): {name}");
            return name;
          }
        }

        // Strategy 2: Legacy
        string? legacyName = ReadOEMNameFromPath(@$"System\CurrentControlSet\Control\MediaProperties\PrivateProperties\Joystick\DirectInput\Config\0\Joystick{joyID + 1}");
        if (IsValidName(legacyName)) {
          Debug.WriteLine($"[Controller Discovery] -> Found via Strategy 2 (Legacy): {legacyName}");
          return legacyName;
        }

        // Strategy 3: Use VID/PID pattern if Mid/Pid are available
        if (caps.wMid != 0 && caps.wPid != 0) {
          string vidA = caps.wMid.ToString("X4");
          string pidA = caps.wPid.ToString("X4");
          string vidB = caps.wMid.ToString("X8");
          string pidB = caps.wPid.ToString("X8");

          string[] vids = { $"VID_{vidA}", $"VID_{vidB}", $"VID&{vidA}", $"VID&{vidB}" };
          string[] pids = { $"PID_{pidA}", $"PID_{pidB}", $"PID&{pidA}", $"PID&{pidB}" };

          foreach (var v in vids) {
            foreach (var p in pids) {
               string vidPidStr = $"{v}&{p}";
               string path = @$"System\CurrentControlSet\Control\MediaProperties\PrivateProperties\Joystick\DirectInput\{vidPidStr}";
               string? name = ReadOEMNameFromPath(path);
               if (IsValidName(name)) {
                  Debug.WriteLine($"[Controller Discovery] -> Found via Strategy 3a (Joystick/{vidPidStr}): {name}");
                  return name;
               }

               path = @$"System\CurrentControlSet\Control\MediaProperties\PrivateProperties\DirectInput\{vidPidStr}";
               name = ReadOEMNameFromPath(path);
               if (IsValidName(name)) {
                  Debug.WriteLine($"[Controller Discovery] -> Found via Strategy 3b (DirectInput/{vidPidStr}): {name}");
                  return name;
               }
            }
          }
        }

        // Strategy 4: Deep search in CU and LM - iterate all active DirectInput device keys
        string[] bases = { 
          @"System\CurrentControlSet\Control\MediaProperties\PrivateProperties\Joystick\DirectInput",
          @"System\CurrentControlSet\Control\MediaProperties\PrivateProperties\DirectInput"
        };
        Microsoft.Win32.RegistryKey[] hives = { Microsoft.Win32.Registry.CurrentUser, Microsoft.Win32.Registry.LocalMachine };
        string vid4 = caps.wMid.ToString("X4");
        string pid4 = caps.wPid.ToString("X4");

        foreach (var hive in hives) {
          foreach (var b in bases) {
            using (var baseKey = hive.OpenSubKey(b)) {
              if (baseKey == null) continue;
              foreach (var subKeyName in baseKey.GetSubKeyNames()) {
                // Handle various ID formats in subkey names (VID_054C, VID&0002054c etc)
                if (subKeyName.Contains(vid4, StringComparison.OrdinalIgnoreCase) && 
                    subKeyName.Contains(pid4, StringComparison.OrdinalIgnoreCase)) {
                  using (var subKey = baseKey.OpenSubKey(subKeyName)) {
                    if (subKey == null) continue;
                    string? name = subKey.GetValue("OEMName")?.ToString() ?? 
                                   subKey.GetValue("FriendlyName")?.ToString() ?? 
                                   subKey.GetValue("DeviceDesc")?.ToString();
                    if (name != null && name.Contains(";")) name = name.Split(';').Last();
                    if (IsValidName(name)) {
                      Debug.WriteLine($"[Controller Discovery] -> Found via Strategy 4 (Deep/{subKeyName}): {name}");
                      return name;
                    }
                  }
                }
              }
            }
          }
        }

        // Strategy 5: Heuristic fallback - if we still have a generic name, search for ANY valid OEMName in the registry.
        // This is often correct if only one specific gaming controller is connected.
        foreach (var hive in hives) {
          using (var baseKey = hive.OpenSubKey(bases[0])) {
            if (baseKey == null) continue;
            string? lastValid = null;
            int validCount = 0;
            foreach (var subKeyName in baseKey.GetSubKeyNames()) {
              if (subKeyName.Contains("VID_", StringComparison.OrdinalIgnoreCase) || subKeyName.Contains("VID&", StringComparison.OrdinalIgnoreCase)) {
                using (var subKey = baseKey.OpenSubKey(subKeyName)) {
                  if (subKey == null) continue;
                  var name = subKey.GetValue("OEMName")?.ToString() ?? 
                             subKey.GetValue("FriendlyName")?.ToString() ?? 
                             subKey.GetValue("DeviceDesc")?.ToString();
                  if (name != null && name.Contains(";")) name = name.Split(';').Last();
                  if (IsValidName(name)) {
                    lastValid = name;
                    validCount++;
                  }
                }
              }
            }
            if (validCount == 1) return lastValid;
          }
        }
      } catch { /* ignore registry errors */ }
      return null;
    }

    /// <summary>
    /// Use WMI to find the PNP friendly name of a device based on its IDs
    /// </summary>
    private static string? GetFriendlyNameFromWmi(ushort mid, ushort pid) {
      Debug.WriteLine($"[Controller Discovery] WMI scored search for VID_{mid:X4} PID_{pid:X4}");
      var potentialNames = new Dictionary<string, int>(StringComparer.OrdinalIgnoreCase);

      try {
        string vid = mid.ToString("X4");
        string pidS = pid.ToString("X4");

        // Match by Hardware ID pattern
        string query = $"SELECT Name, DeviceID, PNPDeviceID FROM Win32_PnPEntity WHERE DeviceID LIKE '%{vid}%{pidS}%' OR PNPDeviceID LIKE '%{vid}%{pidS}%'";
        using (var searcher = new System.Management.ManagementObjectSearcher(query)) {
          foreach (var device in searcher.Get()) {
            string? name = device["Name"]?.ToString();
            if (!string.IsNullOrEmpty(name)) {
                int score = ScoreName(name);
                if (score > -50) potentialNames[name] = score;
            }

            // Try to find parent/associated marketing name by MAC address if Bluetooth
            string? deviceId = device["DeviceID"]?.ToString();
            if (deviceId != null && deviceId.Contains("Dev_")) {
               int devIdx = deviceId.IndexOf("Dev_");
               string macPart = deviceId.Substring(devIdx, 16); // e.g. Dev_4CB99BB63E44
               using (var pSearcher = new System.Management.ManagementObjectSearcher($"SELECT Name FROM Win32_PnPEntity WHERE DeviceID LIKE '%{macPart}%'")) {
                 foreach (var pDevice in pSearcher.Get()) {
                   string? pName = pDevice["Name"]?.ToString();
                   if (!string.IsNullOrEmpty(pName)) {
                      int pScore = ScoreName(pName) + 20; // Boost parent names
                      potentialNames[pName] = pScore;
                   }
                 }
               }
            }
          }
        }

        // Broad fallback for Sony
        if (mid == 0x054C) {
           using (var searcher = new System.Management.ManagementObjectSearcher("SELECT Name FROM Win32_PnPEntity WHERE Name LIKE '%DualSense%' OR Name LIKE '%Wireless Controller%'")) {
             foreach (var device in searcher.Get()) {
               string? name = device["Name"]?.ToString();
               if (!string.IsNullOrEmpty(name) && !name.Contains("Audio", StringComparison.OrdinalIgnoreCase)) {
                  potentialNames[name] = ScoreName(name) + 10;
               }
             }
           }
        }
      } catch (Exception ex) {
        Debug.WriteLine($"[Controller Discovery] WMI Error: {ex.Message}");
      }

      if (potentialNames.Count > 0) {
         var best = potentialNames.OrderByDescending(kv => kv.Value).First();
         Debug.WriteLine($"[Controller Discovery] WMI Best Pick: '{best.Key}' (Score: {best.Value})");
         return best.Key;
      }
      return null;
    }

    private static int ScoreName(string name) {
      if (string.IsNullOrWhiteSpace(name)) return -100;
      if (!IsValidName(name)) return -100;

      int score = 0;
      if (name.Contains("DualSense", StringComparison.OrdinalIgnoreCase)) score += 100;
      if (name.Contains("DualShock", StringComparison.OrdinalIgnoreCase)) score += 80;
      if (name.Contains("Controller", StringComparison.OrdinalIgnoreCase)) score += 50;
      if (name.Contains("Gamepad", StringComparison.OrdinalIgnoreCase)) score += 50;
      if (name.Contains("Wireless", StringComparison.OrdinalIgnoreCase)) score += 20;

      // Penalize generic sounding things
      if (name.Contains("HID", StringComparison.OrdinalIgnoreCase)) score -= 30;
      if (name.Contains("VBus", StringComparison.OrdinalIgnoreCase)) score -= 40;

      return score;
    }

    /// <summary>
    /// Check if a name is valid and not a generic driver name
    /// </summary>
    private static bool IsValidName(string? name) {
      if (string.IsNullOrWhiteSpace(name)) return false;
      string n = name.Trim();
      if (n.Length < 3) return false;

      // Filter out generic driver names, software components, and technical system services
      string[] genericTerms = { 
        "Joysticktreiber", "Joystick-Treiber", "Joystick driver", 
        "HID-konform", "HID-compliant", "Eingabegerät", "Input Device",
        "PC-Joystick", "Microsoft PC", "Assistant", "Software", "Download",
        "Utility", "Control Center", "Gaming Software", "Geräteerkennungsdienst",
        "Device Discovery", "Service", "Enumeration", "Enumerator", "PnP-Monitor",
        "BluetoothDevice", "BTHENUM", "Generic", "Microsoft-PC"
      };

      foreach (var term in genericTerms) {
        if (n.Contains(term, StringComparison.OrdinalIgnoreCase)) return false;
      }

      // "Wireless Controller" is a valid name for DualSense when not using DS4Windows
      if (n.Equals("Wireless Controller", StringComparison.OrdinalIgnoreCase)) return true;

      return true;
    }

    private static string? DeepSearchEnumForName(ushort mid, ushort pid) {
      Debug.WriteLine($"[Controller Discovery] Starting DeepSearchEnum for VID_{mid:X4} PID_{pid:X4}");
      try {
        string vid = mid.ToString("X4");
        string pidS = pid.ToString("X4");
        string[] bases = { "HID", "BTHENUM", "USB" };

        using (var enumKey = Microsoft.Win32.Registry.LocalMachine.OpenSubKey(@"System\CurrentControlSet\Enum")) {
          if (enumKey == null) return null;
          foreach (var b in bases) {
            using (var baseKey = enumKey.OpenSubKey(b)) {
              if (baseKey == null) continue;
              foreach (var subKeyName in baseKey.GetSubKeyNames()) {
                if (subKeyName.Contains(vid, StringComparison.OrdinalIgnoreCase) && subKeyName.Contains(pidS, StringComparison.OrdinalIgnoreCase)) {
                  using (var subKey = baseKey.OpenSubKey(subKeyName)) {
                    if (subKey == null) continue;
                    foreach (var instanceName in subKey.GetSubKeyNames()) {
                      using (var instanceKey = subKey.OpenSubKey(instanceName)) {
                        if (instanceKey == null) continue;
                        string? name = instanceKey.GetValue("FriendlyName")?.ToString() ?? instanceKey.GetValue("DeviceDesc")?.ToString();
                        if (name != null && name.Contains(";")) name = name.Split(';').Last();
                        if (IsValidName(name)) {
                          Debug.WriteLine($"[Controller Discovery] Found name via deep search: {name}");
                          return name;
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      } catch (Exception ex) {
        Debug.WriteLine($"[Controller Discovery] DeepSearch Error: {ex.Message}");
      }
      return null;
    }

    /// <summary>
    /// Helper to read OEMName from a specific registry path in CU or LM
    /// </summary>
    private static string? ReadOEMNameFromPath(string path) {
      try {
        string[] values = { "OEMName", "FriendlyName", "DeviceDesc" };
        using (var key = Microsoft.Win32.Registry.CurrentUser.OpenSubKey(path)) {
          if (key != null) {
            foreach (var v in values) {
              var val = key.GetValue(v)?.ToString();
              if (val != null && val.Contains(";")) val = val.Split(';').Last();
              if (IsValidName(val)) return val;
            }
          }
        }
        using (var key = Microsoft.Win32.Registry.LocalMachine.OpenSubKey(path)) {
          if (key != null) {
            foreach (var v in values) {
              var val = key.GetValue(v)?.ToString();
              if (val != null && val.Contains(";")) val = val.Split(';').Last();
              if (IsValidName(val)) return val;
            }
          }
        }
      } catch { }
      return null;
    }

    /// <summary>
    /// Get the status of a specific game controller
    /// </summary>
    /// <param name="joyID">Numeric ID of the joystick</param>
    /// <returns>JOYINFOEX structure with current status</returns>
    public static JOYINFOEX GetJoystickStatus(int joyID) {
      JOYINFOEX info = new JOYINFOEX();
      info.dwSize = (uint)Marshal.SizeOf(typeof(JOYINFOEX));
      info.dwFlags = JOY_RETURNBUTTONS;
      joyGetPosEx(joyID, ref info);
      return info;
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
            // Filter out system entries and Milkwave senders
            if (!subKeyName.StartsWith("_") && !string.IsNullOrWhiteSpace(subKeyName) && !subKeyName.StartsWith("Milkwave", StringComparison.OrdinalIgnoreCase)) {
              senderNames.Add(subKeyName);
            }
          }
        }
      } catch (Exception ex) {
        System.Diagnostics.Debug.WriteLine($"Error reading Spout senders from registry: {ex.Message}");
      }

      return senderNames;
    }

    /// <summary>
    /// Read Spout sender names from shared memory (active list)
    /// </summary>
    private static void ReadSpoutSenderNamesFromSharedMemory(HashSet<string> senderNames) {
      try {
        IntPtr hMapFile = OpenFileMapping(FILE_MAP_READ, false, "SpoutSenderNames");
        if (hMapFile == IntPtr.Zero) return;

        try {
          // Standard Spout 2 shared memory layout for "SpoutSenderNames":
          // No header! Just slots of 256 bytes each.
          int maxSlots = GetSpoutMaxSenderCount();
          int bufferSize = maxSlots * 256;
          IntPtr pBuf = MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, new IntPtr(bufferSize));

          if (pBuf != IntPtr.Zero) {
            try {
              byte[] buffer = new byte[bufferSize];
              Marshal.Copy(pBuf, buffer, 0, bufferSize);

              for (int i = 0; i < maxSlots; i++) {
                int slotStart = (i * 256);
                if (slotStart >= bufferSize - 1) break;

                // Spout SDK specifies that an empty first byte terminates the active list
                if (buffer[slotStart] == 0) break;

                // Find null terminator within this 256-byte slot
                int nullPos = System.Array.IndexOf(buffer, (byte)0, slotStart, 256);
                int strLen = (nullPos >= 0) ? (nullPos - slotStart) : 256;

                if (strLen > 0) {
                  string senderName = System.Text.Encoding.ASCII.GetString(buffer, slotStart, strLen).Trim();
                  if (!string.IsNullOrWhiteSpace(senderName) && char.IsLetterOrDigit(senderName[0]) && !senderName.StartsWith("Milkwave", StringComparison.OrdinalIgnoreCase)) {
                    senderNames.Add(senderName);
                  }
                }
              }
            } finally {
              UnmapViewOfFile(pBuf);
            }
          }
        } finally {
          CloseHandle(hMapFile);
        }
      } catch (Exception ex) {
        System.Diagnostics.Debug.WriteLine($"Error reading Spout shared memory: {ex.Message}");
      }
    }

    #endregion

  }
}
