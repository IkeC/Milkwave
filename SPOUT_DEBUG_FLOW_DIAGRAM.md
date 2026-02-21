# Spout Mix Debug Flow Diagram

## Message Flow

```
Remote Form (C# .NET 8)
        ↓
   Click "Mix" button
        ↓
chkSpoutMix_CheckedChanged()
        ↓
   PostMessage(WM_ENABLESPOUTMIX, 1, 0)
        ↓ [IPC via Windows Messages]
        ↓
Visualizer (C++ DX9)
        ↓
Milkdrop2PcmVisualizer.cpp (Line ~1194)
        ↓
[MSG HANDLER] WM_ENABLESPOUTMIX received
        ↓
g_plugin.EnableSpoutMixing(true)
        ↓
plugin_inputmix.cpp:EnableSpoutMixing()
        ↓
Create spoutDX9 receiver
        ↓
SetReceiverName("OBS_Spout")
        ↓
Next frame: UpdateSpoutInputTexture()
        ↓
ReceiveDX9Texture()
        ↓
Update texture with sender data
        ↓
Render with Spout texture
```

## Logging Output Points

```
┌─ Message Handler (Lines 1194-1227) ────┐
│ [MSG HANDLER] WM_ENABLESPOUTMIX received │
│ [MSG HANDLER] wParam=0x1                │
│ [MSG HANDLER] Calling EnableSpoutMixing │
│ [MSG HANDLER] EnableSpoutMixing returned│
└─────────────────────────────────────────┘
           ↓
┌─ SetSpoutSender() (Lines 104-151) ─────┐
│ SetSpoutSender START                    │
│ Sender name length: 9 chars             │
│ Calling SetReceiverName(...)            │
│ SetSpoutSender END                      │
└─────────────────────────────────────────┘
           ↓
┌─ EnableSpoutMixing() (Lines 153-248) ──┐
│ EnableSpoutMixing START                 │
│ Parameter: enable=1                     │
│ Creating new spoutDX9 receiver object   │
│ Setting DX9 device: 0xXXXXXXXX          │
│ SetReceiverName('OBS_Spout')            │
│ m_pSpoutReceiver is: 0xYYYYYYYY         │
│ EnableSpoutMixing END                   │
└─────────────────────────────────────────┘
           ↓
┌─ UpdateSpoutInputTexture() (Every 30 frames) ┐
│ Frame 30: received=1, connected=1        │
│ [SENDER] Name: OBS_Spout (1920x1080)    │
│ texture=0x12345678                       │
└──────────────────────────────────────────┘
           ↓
         SUCCESS ✓
```

## Failure Points

```
Message Received?
    │
    ├─ NO → Check Remote form
    │       Check visualizer window
    │       Check message handling
    │
    └─ YES → Continue

EnableSpoutMixing called?
    │
    ├─ NO → Check if message forwarded correctly
    │
    └─ YES → Continue

DX9 Device Available?
    │
    ├─ NO ✗ FAILURE POINT #1
    │    DirectX not initialized
    │    m_lpDX->m_lpDevice = 0x0
    │
    └─ YES → Continue

Receiver Created?
    │
    ├─ NO ✗ FAILURE POINT #2
    │    m_pSpoutReceiver = 0x0
    │    Something wrong with spoutDX9
    │
    └─ YES → Continue

Receiver Name Set?
    │
    ├─ NO → Will try to connect to default/active
    │
    └─ YES → Connect to specified sender

Connected to Sender?
    │
    ├─ NO ✗ FAILURE POINT #3
    │    connected = 0
    │    Sender not found or offline
    │    Check sender name
    │    Check sender is running
    │
    └─ YES → Continue

Receiving Frames?
    │
    ├─ NO ✗ FAILURE POINT #4
    │    received = 0
    │    Sender connected but not sending
    │
    └─ YES → Continue

Texture Updated?
    │
    ├─ NO ✗ FAILURE POINT #5
    │    texture = 0x0
    │    Frame received but texture not created
    │
    └─ YES → Continue

Rendering with Spout?
    │
    └─ SHOULD BE VISIBLE ON SCREEN ✓
```

## State Variables to Monitor

```
Before Click:
  m_bSpoutInputEnabled = 0
  m_pSpoutReceiver = NULL
  m_pSpoutInputTexture = NULL

After Click (Should See):
  m_bSpoutInputEnabled = 1
  m_pSpoutReceiver = 0xNON_NULL
  m_szSpoutSenderName = "OBS_Spout" (or selected sender)

During Runtime (Should See):
  connected = 1 (within first few frames)
  received = 1 (within first few frames)
  m_pSpoutInputTexture = 0xNON_NULL
  texture dimensions = sender resolution (1920x1080, etc)
```

## Search Strategy in Debug Output

```
Ctrl+F in Debug Output window:

Step 1: Search "WM_ENABLESPOUTMIX"
   ├─ Found? → Message arrived ✓
   └─ Not found? → Message not sent or not received

Step 2: Search "EnableSpoutMixing START"
   ├─ Found? → Function called ✓
   └─ Not found? → Message handler issue

Step 3: Search "m_lpDX->m_lpDevice=0x"
   ├─ "0xDEAD..." → DX device OK ✓
   ├─ "0x0" → FAILURE: DX not initialized
   └─ "WARNING" → FAILURE: DX not available

Step 4: Search "m_pSpoutReceiver is: 0x"
   ├─ "0xCAFE..." → Receiver created ✓
   ├─ "0x0" → FAILURE: Receiver creation failed
   └─ "WARNING" → May not be initialized

Step 5: Search "connected="
   ├─ "connected=1" → Connected to sender ✓
   └─ "connected=0" → Not connected/unable to find

Step 6: Search "received="
   ├─ "received=1" → Receiving frames ✓
   └─ "received=0" → Not getting frame data

Step 7: Search "texture=0x"
   ├─ "0x12345..." → Texture exists ✓
   └─ "0x0" → Texture not created/updated
```

## Quick Issue Resolution

| Log Shows | Problem | Solution |
|-----------|---------|----------|
| No "WM_ENABLESPOUTMIX" | Message not received | Check Remote window title |
| No "EnableSpoutMixing" | Message not forwarded | Check visualizer focus |
| "m_lpDX->m_lpDevice=0x0" | DirectX not ready | Wait for visualizer to fully load |
| "m_pSpoutReceiver is: 0x0" | Receiver init failed | Restart visualizer |
| "connected=0" persistent | Can't find sender | Verify sender running/name |
| "received=0" persistent | Sender offline | Check sender application |
| "texture=0x0" | Texture creation failed | Check spoutDX9 library |

## Example: Good vs Bad

### ✓ GOOD - Everything Works

```
[MSG HANDLER] WM_ENABLESPOUTMIX received
  m_bSpoutInputEnabled=0
  Calling EnableSpoutMixing(1)...
EnableSpoutMixing START
  enable=1
  m_lpDX->m_lpDevice=0xDEADBEEF  ← DX OK
  m_pSpoutReceiver is: 0xCAFEBABE  ← Receiver created
EnableSpoutMixing END
UpdateSpoutInputTexture: Frame 30
  received=1, connected=1  ← Connected and receiving
  texture=0x12345678  ← Texture exists
[SPOUT CONNECTION] Connected to: OBS_Spout (1920x1080)
```

### ✗ BAD - DX Device Missing

```
[MSG HANDLER] WM_ENABLESPOUTMIX received
  m_bSpoutInputEnabled=0
  Calling EnableSpoutMixing(1)...
EnableSpoutMixing START
  enable=1
  WARNING - m_lpDX=0x0 or m_lpDX->m_lpDevice=NULL!  ← PROBLEM!
  m_pSpoutReceiver is: 0x0  ← Receiver not created
EnableSpoutMixing END
UpdateSpoutInputTexture: Frame 30
  connected=0  ← Can't connect
  texture=0x0  ← No texture
```

### ✗ BAD - Sender Not Found

```
[MSG HANDLER] WM_ENABLESPOUTMIX received
EnableSpoutMixing START
  m_lpDX->m_lpDevice=0xDEADBEEF  ← DX OK
  m_pSpoutReceiver is: 0xCAFEBABE  ← Receiver created
EnableSpoutMixing END
UpdateSpoutInputTexture: Frame 30
  received=1, connected=0  ← CONNECTED=0!
  texture=0x0
[... keeps saying connected=0 ...]  ← Can't find sender
```

