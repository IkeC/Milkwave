# Debug Logging Implementation - Summary

## What Was Done

Added comprehensive logging throughout the Spout mixing pipeline to identify why clicking "Mix" in the Remote form doesn't produce visible results in the visualizer.

## Files Modified

### 1. `Visualizer/vis_milk2/Milkdrop2PcmVisualizer.cpp`
- **Lines 1160-1227**: Enhanced message handlers
  - `WM_SETSPOUTSENDER` (0x0400 + 108)
  - `WM_ENABLESPOUTMIX` (0x0400 + 109)
  - Added detailed parameter logging
  - Added state tracking and validation
  - OutputDebugString to VS debug window

### 2. `Visualizer/vis_milk2/plugin_inputmix.cpp`
- **Lines 104-151**: `SetSpoutSender()` function
  - Validates sender name input
  - Logs string conversion (wide to narrow/ANSI)
  - Shows receiver state
  - Logs `SetReceiverName()` calls

- **Lines 153-248**: `EnableSpoutMixing()` function
  - Logs enable/disable transitions
  - **Critical**: Logs DX9 device availability
  - Logs receiver creation
  - Logs sender configuration
  - Detailed state tracking
  - Boxed log headers for visual clarity

### 3. `Visualizer/vis_milk2/plugin_inputmix_render.cpp`
- **Lines 18-100**: `UpdateSpoutInputTexture()` function
  - First-call diagnostics
  - Per-30-frame status updates
  - Connection status changes
  - Texture dimensions
  - Sender metadata
  - Frame count tracking

## Logging Output Destinations

1. **Visual Studio Debug Window**
   - `Debug → Windows → Output` (when debugging)
   - Most convenient for live debugging

2. **OutputDebugString**
   - Windows debugger tools can capture
   - DbgView.exe can show

3. **Milkwave Log System**
   - Via `milkwave->LogInfo()`
   - Logged to Milkwave log file

## What Gets Logged

### Message Reception (MSGHandler)
```
[MSG HANDLER] ========== WM_ENABLESPOUTMIX received ==========
[MSG HANDLER]   wParam=0x1 (enable=1), lParam=0x0
[MSG HANDLER]   Current state: m_bSpoutInputEnabled=0
[MSG HANDLER]   Calling EnableSpoutMixing(1)...
```

### Spout Initialization
```
================================ EnableSpoutMixing START ================================
  Parameter: enable=1
  Current state: m_bSpoutInputEnabled=0
  State changed to: m_bSpoutInputEnabled=1
  *** ENABLING SPOUT MIXING ***
    Setting DX9 device: m_lpDX->m_lpDevice=0xDEADBEEF
    SetDX9device() called successfully
    m_pSpoutReceiver is: 0xCAFEBABE
================================ EnableSpoutMixing END ================================
```

### Runtime Status (Every 30 Frames)
```
UpdateSpoutInputTexture: Frame 30 - received=1, connected=1, texture=0x12345678
  [SENDER] Name: OBS_Spout, Size: 1920x1080, NewFrame: 1
```

### Connection Events
```
[SPOUT CONNECTION] Connected to: OBS_Spout (1920x1080)
[SPOUT DISCONNECT] Sender disconnected
[SPOUT RESIZE] Texture size changed: 0x0 -> 1920x1080
```

## Key Diagnostic Information

### Critical Values to Check
- `m_lpDX->m_lpDevice` - DirectX device (must NOT be 0x0)
- `m_pSpoutReceiver` - Receiver object (must NOT be 0x0)
- `connected` - Connection status (should be 1)
- `received` - Frame reception (should be 1)
- `texture` - Texture pointer (should NOT be 0x0)

### Error Indicators
- `WARNING - No DX9 device available!`
- `ERROR: (any message)`
- `m_pSpoutReceiver is: 0x0`
- `connected=0` (persistent)
- `received=0` (persistent)

## How to Use for Debugging

1. **Build visualizer** (F5 Debug build)
2. **Attach VS Debugger** to running visualizer
3. **Open Debug Output**: `Debug → Windows → Output`
4. **Click "Mix"** in Remote form
5. **Search logs** for key terms:
   - `WM_ENABLESPOUTMIX`
   - `EnableSpoutMixing`
   - `WARNING`
   - `ERROR`
   - `m_lpDX`
   - `connected`

6. **Analyze** the flow to find where it breaks

## Troubleshooting Guide

| Symptom | What to Check |
|---------|--------------|
| No log output | Message handler logs - is message arriving? |
| DX9 device NULL | Why is DX not initialized? |
| Receiver is NULL | Why did receiver creation fail? |
| connected=0 | Is Spout sender running? Name correct? |
| received=0 | Is sender actually sending? |
| texture=0x0 | Is spoutDX9 working properly? |

## Performance Impact

- **Minimal** - only logs on:
  - Message reception (rare)
  - Every 30 frames during Spout update (very small overhead)
  - Connection status changes (rare)
- **No impact on visualization performance**

## Next Steps

1. **Test** with the new logging enabled
2. **Identify** where the flow breaks
3. **Focus debugging** on that specific area
4. **Potentially add more detailed logging** based on findings

## Documentation Created

1. `SPOUT_MIXING_DEBUG_LOGGING.md` - Comprehensive guide
2. `SPOUT_LOGGING_SUMMARY.md` - Implementation details
3. `SPOUT_DEBUG_QUICK_REF.md` - Quick reference card

## Build Status

✅ **Compilation**: Successful - no errors or warnings
✅ **Project builds**: Cleanly without issues

