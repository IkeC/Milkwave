# Spout Mixing Debug Logging - Implementation Summary

## What Was Added

Comprehensive logging has been added throughout the Spout mixing pipeline to help diagnose why clicking "Mix" doesn't have a visible impact.

### Files Modified

1. **Visualizer/vis_milk2/Milkdrop2PcmVisualizer.cpp**
   - Enhanced message handler for `WM_SETSPOUTSENDER` (lines ~1160-1188)
   - Enhanced message handler for `WM_ENABLESPOUTMIX` (lines ~1190-1227)
   - Added detailed parameter logging
   - Added state tracking before/after calls

2. **Visualizer/vis_milk2/plugin_inputmix.cpp**
   - Enhanced `SetSpoutSender()` method (lines ~104-151)
     - Logs sender name validation
     - Logs string conversion (wide to narrow)
     - Logs receiver configuration
   
   - Enhanced `EnableSpoutMixing()` method (lines ~153-248)
     - Logs enable/disable flow with boxed headers
     - Logs DX9 device availability (critical!)
     - Logs receiver creation and configuration
     - Logs sender name setup
     - Logs state transitions

3. **Visualizer/vis_milk2/plugin_inputmix_render.cpp**
   - Enhanced `UpdateSpoutInputTexture()` method (lines ~18-100)
     - First-call initialization logging
     - Per-30-frame status logging
     - Connection status tracking
     - Frame statistics
     - Texture dimension tracking

## What the Logging Shows

### Entry Point (Message Received)
```
[MSG HANDLER] ========== WM_ENABLESPOUTMIX received ==========
[MSG HANDLER]   wParam=0x1 (enable=1), lParam=0x0
[MSG HANDLER]   Current state: m_bSpoutInputEnabled=0
```

### Main Flow
```
================================ EnableSpoutMixing START ================================
  Parameter: enable=1
  Current state: m_bSpoutInputEnabled=0
  State changed to: m_bSpoutInputEnabled=1
  *** ENABLING SPOUT MIXING ***
    Setting DX9 device: m_lpDX->m_lpDevice=0xABCD1234
    SetDX9device() called successfully
    Calling SetReceiverName('OBS_Spout')...
    SetReceiverName() returned
  m_pSpoutReceiver is: 0xCAFEBABE
================================ EnableSpoutMixing END ================================
```

### Runtime Status (Every 30 Frames)
```
UpdateSpoutInputTexture: Frame 30 - received=1, connected=1, texture=0x12345678
  [SENDER] Name: OBS_Spout, Size: 1920x1080, NewFrame: 1
```

## How to View Logs

### In Visual Studio
1. While debugging: **Debug → Windows → Output**
2. Filter for "Milkwave" or search for key terms
3. Look for:
   - `[MSG HANDLER]` - Message reception
   - `EnableSpoutMixing` - Main function
   - `[SPOUT CONNECTION]` - Connection events
   - `UpdateSpoutInputTexture` - Frame updates
   - `ERROR` - Problems
   - `WARNING` - Potential issues

### Key Fields to Monitor

| Field | What It Means |
|-------|--------------|
| `m_lpDX->m_lpDevice=0xXXXX` | DirectX device available ✅ |
| `m_lpDX->m_lpDevice=0x0` | DirectX device NULL ❌ |
| `m_pSpoutReceiver=0xXXXX` | Receiver created ✅ |
| `m_pSpoutReceiver=0x0` | Receiver NOT created ❌ |
| `connected=1` | Connected to sender ✅ |
| `connected=0` | Not connected ❌ |
| `received=1` | Receiving frames ✅ |
| `received=0` | Not receiving ❌ |

## Debugging Workflow

1. **Build the project** (should compile without errors)
2. **Open Debug Output window** in Visual Studio
3. **Start debugging** the visualizer
4. **Click "Mix"** in Remote form
5. **Check Debug Output** for logs
6. **Look for any ERROR or WARNING messages**
7. **Check the final state values** (m_pSpoutReceiver, connected, etc.)

## Common Issues to Look For

### Issue 1: No Logs at All
- Message handler not receiving WM_ENABLESPOUTMIX
- Visualizer window might not have focus
- Wrong build configuration (not Debug)

### Issue 2: DX9 Device is NULL
```
WARNING - m_lpDX=0x0 or m_lpDX->m_lpDevice=NULL!
```
- DirectX not initialized when message received
- This would prevent receiver creation

### Issue 3: Receiver Not Created
```
m_pSpoutReceiver is: 0x0
```
- Something wrong with spoutDX9 object creation
- Or receiver deleted unexpectedly

### Issue 4: Connected=0
```
UpdateSpoutInputTexture: Frame 30 - ... connected=0 ...
```
- Spout sender not found or not running
- Check Spout sender name matches exactly

### Issue 5: Connected=1 But No Texture
```
UpdateSpoutInputTexture: Frame 30 - received=1, connected=1, texture=0x0
```
- Receiver connected but not getting texture data
- May indicate spoutDX9 library issue

## Log Output Example

Here's what a successful flow looks like:

```
[MSG HANDLER] ========== WM_SETSPOUTSENDER received ==========
[MSG HANDLER]   Sender name: OBS_Spout
[MSG HANDLER] ========== WM_SETSPOUTSENDER complete ==========
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
  Notification added
  m_bSpoutInputEnabled is now: 1
  m_pSpoutReceiver is: 0xCAFEBABE
================================ EnableSpoutMixing END ================================
[MSG HANDLER]   EnableSpoutMixing returned
[MSG HANDLER]   New state: m_bSpoutInputEnabled=1
[MSG HANDLER] ========== WM_ENABLESPOUTMIX complete ==========
UpdateSpoutInputTexture: Frame 0 - received=0, connected=0, texture=0x0
UpdateSpoutInputTexture: Frame 30 - received=1, connected=1, texture=0x12345678
  [SENDER] Name: OBS_Spout, Size: 1920x1080, NewFrame: 1
[SPOUT CONNECTION] Connected to: OBS_Spout (1920x1080)
```

This shows the complete flow: message received → receiver created → connected to sender → receiving frames.

## Next Steps

1. **Rebuild** the visualizer with the new logging
2. **Run in Debug** mode with debugger attached
3. **Check Debug Output** window while testing Spout mix
4. **Look for ERROR messages** or NULL pointers
5. **Share the logs** if reporting an issue

