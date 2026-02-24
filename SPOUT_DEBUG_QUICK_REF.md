# Spout Mix Debug - Quick Reference

## Steps to Debug

1. **Build**: F5 (Debug build)
2. **Open Debug Output**: `Debug → Windows → Output` in Visual Studio
3. **Test**: Click "Mix" on Spout sender in Remote form
4. **Search Debug Output** (Ctrl+F):

### What to Search For

| Search Term | What It Reveals |
|------------|-----------------|
| `WM_ENABLESPOUTMIX` | Message received by visualizer |
| `EnableSpoutMixing START` | Function called |
| `m_lpDX->m_lpDevice=0x` | DirectX device state |
| `WARNING` | Potential problem |
| `ERROR` | Definite problem |
| `m_pSpoutReceiver is:` | Receiver created? |
| `connected=` | Connection status |
| `[SPOUT CONNECTION]` | Just connected event |

## Expected Log Flow

```
[MSG HANDLER] ========== WM_ENABLESPOUTMIX received ==========
  ↓
================================ EnableSpoutMixing START ================================
  ↓
Setting DX9 device: m_lpDX->m_lpDevice=0x... (NOT 0x0)
  ↓
m_pSpoutReceiver is: 0x... (NOT 0x0)
  ↓
================================ EnableSpoutMixing END ================================
  ↓
UpdateSpoutInputTexture: Frame 30 - received=1, connected=1
  ↓
[SPOUT CONNECTION] Connected to: OBS_Spout (1920x1080)
```

## Red Flags

| Red Flag | Problem |
|----------|---------|
| `No logs after clicking Mix` | Message not reaching visualizer |
| `m_lpDX->m_lpDevice=0x0` | DirectX device NULL → no receiver |
| `m_pSpoutReceiver is: 0x0` | Receiver creation failed |
| `connected=0` (persistent) | Can't find sender or sender offline |
| `received=0` (persistent) | Not getting frame data |
| `WARNING - No DX9 device` | Major issue → DirectX not ready |
| `ERROR:` (any message) | Something failed |

## Most Likely Issues

### Issue 1: DX9 Device NULL
```
m_lpDX=0x0 or m_lpDX->m_lpDevice=NULL!
```
**Why**: DirectX not initialized when Mix clicked
**Solution**: Check if visualizer fully initialized before clicking Mix

### Issue 2: No Sender Connection
```
connected=0 (keeps saying this)
```
**Why**: Sender name wrong or sender not running
**Solution**: 
- Verify OBS Spout sender is running
- Check sender name matches exactly in Remote form

### Issue 3: Message Not Arriving
```
[No log output at all after clicking Mix]
```
**Why**: Remote not sending message or visualizer not receiving
**Solution**: Check Remote's SetStatusText shows "Spout mixing enabled"

## Quick Attach Debugger

1. Run visualizer without debugger
2. Click Mix in Remote (to see the problem)
3. In Visual Studio: `Debug → Attach to Process`
4. Find `MilkwaveVisualizer.exe`
5. Click "Attach"
6. Open Debug Output: `Debug → Windows → Output`
7. Click Mix again (should now log)

## Log Files

If not seeing Debug Output:
1. Check Milkwave log file location
2. May be in: Release folder or `%APPDATA%/Milkwave/`
3. Search for same terms in log file

## Minimal Test Case

1. Start OBS Studio
2. Add some video source to OBS
3. Enable Spout sender (OBS plugin)
4. Start Milkwave visualizer (Debug build)
5. Open Remote form
6. Go to "Input" tab
7. Verify Spout sender dropdown shows "OBS_Spout"
8. Click Mix
9. Check Debug Output in VS

## Key Files Modified

- `Visualizer/vis_milk2/Milkdrop2PcmVisualizer.cpp` - Message handler
- `Visualizer/vis_milk2/plugin_inputmix.cpp` - Enable/Set functions
- `Visualizer/vis_milk2/plugin_inputmix_render.cpp` - Frame updates

## Performance Impact

**Minimal** - logging occurs at:
- Every message (not frequent)
- Every 30 frames during Spout update (1-2 log lines per second max)
- No performance impact on visualization

