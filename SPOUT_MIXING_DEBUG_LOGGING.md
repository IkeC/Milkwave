# Spout Mixing Debugging Guide

## Overview
Comprehensive logging has been added to the visualizer to help track the Spout mixing flow when clicking "Mix" in the Remote form.

---

## Logging Output Locations

### 1. **Debug Output Window (Visual Studio)**
All logging goes to `OutputDebugString()`, visible in:
- **Visual Studio → Debug → Windows → Output** 
- Look for output from "Milkwave Visualizer"

### 2. **Milkwave Log File**
All logging also goes to the Milkwave log system via `milkwave->LogInfo()`:
- Check the Milkwave log file (typically in Release folder or app directory)

### 3. **In-Game Notifications**
Major events show as in-game notifications:
- "Spout mixing enabled"
- "Spout mixing disabled"
- Sender connection/disconnection events
- Size changes

---

## Key Logging Points

### Message Handler → Visualizer
**File**: `Visualizer/vis_milk2/Milkdrop2PcmVisualizer.cpp` (Lines ~1160-1230)

When user clicks "Mix" button in Remote:

```
[MSG HANDLER] ========== WM_ENABLESPOUTMIX received ==========
[MSG HANDLER]   wParam=0x1 (enable=1), lParam=0x0
[MSG HANDLER]   Current state: m_bSpoutInputEnabled=0
[MSG HANDLER]   Calling EnableSpoutMixing(1)...
```

**What to look for:**
- `wParam` should be 0 or 1 (enable/disable flag)
- `Current state` shows the state before change
- Should NOT see any "ERROR" messages

---

### SetSpoutSender Flow
**File**: `Visualizer/vis_milk2/plugin_inputmix.cpp` (Lines ~104-151)

When sender name is set:

```
========================= SetSpoutSender START =========================
  Input parameter (senderName): 0xABCDEF01
  Sender name length: 10 chars
  Sender name (wide): 'OBS_Spout'
  Storing sender name in m_szSpoutSenderName...
  m_szSpoutSenderName now: 'OBS_Spout'
  Converted to narrow string (ANSI): 'OBS_Spout' (10 chars converted)
  m_pSpoutReceiver: 0x0
  Receiver doesn't exist yet - sender name will be used when receiver is created
========================= SetSpoutSender END =========================
```

**What to look for:**
- Sender name should match the selected sender in Remote form
- String conversion should work (ANSI conversion shows char count)
- If `m_pSpoutReceiver: 0x0`, receiver will be created later
- If `m_pSpoutReceiver: 0xXXXXXXXX`, should see `SetReceiverName` call

---

### EnableSpoutMixing Flow
**File**: `Visualizer/vis_milk2/plugin_inputmix.cpp` (Lines ~153-248)

Main enable/disable logic:

```
================================ EnableSpoutMixing START ================================
  Parameter: enable=1
  Current state: m_bSpoutInputEnabled=0
  State changed to: m_bSpoutInputEnabled=1
  *** ENABLING SPOUT MIXING ***
    No receiver exists - creating spoutDX9 receiver object
    Setting DX9 device: m_lpDX->m_lpDevice=0x12345678
    SetDX9device() called successfully
    Specific sender name is set: 'OBS_Spout'
    Converted to narrow string: 'OBS_Spout' (10 chars converted)
    Calling SetReceiverName('OBS_Spout')...
    SetReceiverName() returned
  Notification added
  m_bSpoutInputEnabled is now: 1
  m_pSpoutReceiver is: 0x87654321
================================ EnableSpoutMixing END ================================
```

**Critical Things to Check:**
1. **DX9 Device**: `m_lpDX->m_lpDevice` should NOT be NULL
   - If NULL → `WARNING - No DX9 device available!`
   - This is likely the problem

2. **Receiver Creation**: Should see `Creating new spoutDX9 receiver object`
   - If NOT seen → already created (probably won't be an issue first time)

3. **Receiver Address**: `m_pSpoutReceiver is: 0x...` should not be NULL
   - If 0x0 → receiver creation failed!

4. **SetReceiverName**: Should see call and return
   - If not called → sender name might be empty

---

### Render Loop - Spout Update
**File**: `Visualizer/vis_milk2/plugin_inputmix_render.cpp` (Lines ~18-100)

Called every frame while Spout mixing is enabled:

```
UpdateSpoutInputTexture: Frame 0 - received=0, connected=0, texture=0x0
UpdateSpoutInputTexture: Frame 30 - received=1, connected=1, texture=0x11223344
  [SENDER] Name: OBS_Spout, Size: 1920x1080, NewFrame: 1

[SPOUT CONNECTION] Connected to: OBS_Spout (1920x1080)
```

**What to look for:**
- **`connected=1`**: Good! Receiving from sender
- **`connected=0`**: Problem! Not connecting to sender
- **`texture=0x...`**: Should eventually get a non-zero address
- **`NewFrame: 1`**: Receiving new frames from sender
- **Connection event**: Should see when first connected

---

## Troubleshooting Checklist

### If Nothing Appears in Logs After Clicking "Mix":

1. **Visual Studio Debug Output Not Showing**
   - Rebuild the project
   - Ensure Debug build configuration
   - Attach debugger to running visualizer
   - Visual Studio → Debug → Attach to Process → MilkwaveVisualizer.exe

2. **Messages Not Arriving at Visualizer**
   - Check Remote form's SetStatusText
   - Should show "Spout mixing enabled"
   - If not visible → message not reaching visualizer

3. **EnableSpoutMixing Not Called**
   - Check MSG HANDLER logs for `WM_ENABLESPOUTMIX received`
   - If not present → message not being sent from Remote

---

### If EnableSpoutMixing Called But No Receiver Created:

Look for:
- `m_lpDX->m_lpDevice` is NULL → **DX9 device not available!**
- `m_pSpoutReceiver is: 0x0` → **Receiver creation failed!**

### If Receiver Created But No Connection:

Look for:
- `connected=0` in UpdateSpoutInputTexture logs
- `NewFrame: 0` or missing
- Check Spout sender is actually running
- Verify sender name matches exactly (case-sensitive on some systems)

### If Connected But Texture Not Updated:

Look for:
- `received=1` but `texture=0x0`
- `texture` address stops updating
- May indicate spoutDX9 library issue

---

## Log Analysis Example

### Successful Spout Mixing:

```
[MSG HANDLER] ========== WM_ENABLESPOUTMIX received ==========
[MSG HANDLER]   wParam=0x1 (enable=1), lParam=0x0
[MSG HANDLER]   Current state: m_bSpoutInputEnabled=0
[MSG HANDLER]   Calling EnableSpoutMixing(1)...
================================ EnableSpoutMixing START ================================
  Parameter: enable=1
  Current state: m_bSpoutInputEnabled=0
  State changed to: m_bSpoutInputEnabled=1
  *** ENABLING SPOUT MIXING ***
    No receiver exists - creating spoutDX9 receiver object
    Setting DX9 device: m_lpDX->m_lpDevice=0xDEADBEEF
    SetDX9device() called successfully
    Specific sender name is set: 'OBS_Spout'
    Calling SetReceiverName('OBS_Spout')...
    SetReceiverName() returned
  m_pSpoutReceiver is: 0xCAFEBABE
================================ EnableSpoutMixing END ================================
[SPOUT CONNECTION] Connected to: OBS_Spout (1920x1080)
UpdateSpoutInputTexture: Frame 30 - received=1, connected=1, texture=0x12345678
```

✅ **All green!** Spout mixing should work.

### Failed - No DX9 Device:

```
[MSG HANDLER] Calling EnableSpoutMixing(1)...
================================ EnableSpoutMixing START ================================
  Setting DX9 device: m_lpDX->m_lpDevice=0x0
    WARNING - m_lpDX=0x0 or m_lpDX->m_lpDevice=NULL!
  m_pSpoutReceiver is: 0x0
```

❌ **Problem found!** DX9 device is NULL - receiver not initialized.

---

## Next Steps

1. **Run Visualizer** in Debug mode
2. **Attach Visual Studio Debugger**
3. **Enable Debug Output Window** in Visual Studio
4. **Click "Mix" in Remote form**
5. **Check Debug Output** for the logs above
6. **Report findings** with relevant log sections

---

## Quick Search Terms

In Debug Output window, use Ctrl+F to search for:
- `WM_ENABLESPOUTMIX` - Message reception
- `EnableSpoutMixing START` - Function entry point
- `Creating new spoutDX9` - Receiver initialization
- `SetReceiverName` - Sender configuration
- `Connected to:` - Successful connection
- `ERROR` - Any error conditions
- `WARNING` - Any warnings
- `[SPOUT` - All Spout-specific events

