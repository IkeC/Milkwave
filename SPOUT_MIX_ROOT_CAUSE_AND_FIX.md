# SPOUT MIXING - ROOT CAUSE & FIX

## The Problem

When you click "Mix" for a Spout sender in the Remote form, the visualizer connects to **"Milkwave 2"** (the visualizer's own sender) instead of the selected sender (**"OBS_Spout"**).

## Root Cause

The message flow is broken:

```
Remote Form                                  Visualizer
┌──────────────┐                            ┌──────────────────┐
│ Click "Mix"  │                            │ Receive Message  │
└──────┬───────┘                            └──────────────────┘
       │
       ├─ Current: Sends SPOUTINPUT= message
       │           via TEXT MESSAGE system
       │           (not parsed properly)
       │
       └─ Should: Send WM_SETSPOUTSENDER first
                  with sender name
                  Then send WM_ENABLESPOUTMIX
                  to enable mixing
```

### Current Message Flow (BROKEN)

```
Remote: "SPOUTINPUT=1|OBS_Spout"
   ↓
Visualizer: Text message processing
   ↓
Not properly routed to SetSpoutSender() & EnableSpoutMixing()
   ↓
Visualizer: Calls EnableSpoutMixing(true)
   ↓
m_szSpoutSenderName is EMPTY
   ↓
Receiver connects to ACTIVE sender (Milkwave 2)
   ↓
WRONG SENDER!  ✗
```

### Correct Message Flow (NEEDED)

```
Remote: "OBS_Spout" selected in dropdown
   ↓
Click "Mix"
   ↓
Remote: WM_SETSPOUTSENDER → "OBS_Spout"
   ↓
Visualizer: SetSpoutSender("OBS_Spout")
   ↓
m_szSpoutSenderName = "OBS_Spout"
   ↓
Remote: WM_ENABLESPOUTMIX → 1
   ↓
Visualizer: EnableSpoutMixing(true)
   ↓
m_pSpoutReceiver->SetReceiverName("OBS_Spout")
   ↓
Receiver connects to OBS_Spout
   ↓
CORRECT SENDER! ✓
```

## Solution

### Step 1: Locate the Spout Mix Handler in Remote Form

Find and modify the handler that sends the Spout mix command. This is likely in a method that handles the checkbox or button click for Spout mixing.

**Where to look:**
- `chkSpoutMix_CheckedChanged()` method
- Or `btnSpoutMix_Click()` method in MilkwaveRemoteForm.cs

### Step 2: Fix the Message Sending

Replace the current `SPOUTINPUT=` message with direct window messages:

```csharp
// BEFORE (BROKEN):
// Sends text message that isn't properly routed
message = "SPOUTINPUT=" + (mixEnabled ? "1" : "0") + "|" + senderName;
SendToMilkwaveVisualizer(message, MessageType.SpoutInput);

// AFTER (CORRECT):
// Send sender name FIRST via WM_SETSPOUTSENDER
if (enabled && cboSpoutSender.SelectedIndex >= 0) {
    string senderName = cboSpoutSender.Text;
    SendStringMessage(foundWindow, WM_SETSPOUTSENDER, senderName);
    System.Threading.Thread.Sleep(50);  // Brief delay for message to be processed
}

// Then enable/disable mixing via WM_ENABLESPOUTMIX
PostMessage(foundWindow, WM_ENABLESPOUTMIX, (IntPtr)(enabled ? 1 : 0), IntPtr.Zero);
```

### Step 3: Verify the Message Constants

Ensure these constants are defined in the form:

```csharp
private const int WM_SETSPOUTSENDER = 0x0400 + 108;    // WM_USER + 108
private const int WM_ENABLESPOUTMIX = 0x0400 + 109;    // WM_USER + 109
```

### Step 4: Ensure SendStringMessage is Implemented

Check that `SendStringMessage()` method exists in the form (it likely does):

```csharp
private void SendStringMessage(IntPtr windowHandle, int messageId, string message) {
    byte[] messageBytes = Encoding.Unicode.GetBytes(message);
    IntPtr messagePtr = Marshal.AllocHGlobal(messageBytes.Length + 2);
    Marshal.Copy(messageBytes, 0, messagePtr, messageBytes.Length);
    Marshal.WriteInt16(messagePtr, messageBytes.Length, 0);

    COPYDATASTRUCT cds = new COPYDATASTRUCT {
        dwData = (IntPtr)messageId,
        cbData = messageBytes.Length + 2,
        lpData = messagePtr
    };

    SendMessageW(windowHandle, WM_COPYDATA, IntPtr.Zero, ref cds);
    Marshal.FreeHGlobal(messagePtr);
}
```

## Code Location

In **Remote/MilkwaveRemoteForm.cs**:

1. Find the Spout Mix checkbox or button handler
2. Find where `SPOUTINPUT=` message is built
3. Replace with the two-message approach:
   - First: WM_SETSPOUTSENDER with sender name
   - Then: WM_ENABLESPOUTMIX with enable flag

## Why This Works

1. **WM_SETSPOUTSENDER** tells visualizer which sender to use
   - Calls `g_plugin.SetSpoutSender(senderName)`
   - Sets `m_szSpoutSenderName = "OBS_Spout"`

2. **WM_ENABLESPOUTMIX** tells visualizer to enable mixing
   - Calls `g_plugin.EnableSpoutMixing(true)`
   - Creates receiver and calls `SetReceiverName(m_szSpoutSenderName)`
   - Receiver connects to the SPECIFIED sender, not the active one

3. **Order matters!**
   - Must set sender name FIRST
   - Then enable mixing
   - Otherwise mixing enables without a sender name set

## Testing the Fix

After implementing:

1. Start OBS with a Spout sender running
2. Open Milkwave Remote
3. Select "OBS_Spout" from dropdown
4. Click "Mix"
5. Check debug output:
   - Should see `WM_SETSPOUTSENDER received`
   - Should see sender name set
   - Should see `WM_ENABLESPOUTMIX received`
   - Should see `[SPOUT CONNECTION] Connected to: OBS_Spout`
   - NOT "Milkwave 2"!

## Key Debug Messages to Look For

✅ **Success:**
```
[MSG HANDLER] WM_SETSPOUTSENDER received
[MSG HANDLER] Sender name: OBS_Spout
SetSpoutSender START
  Sender name length: 8 chars
  Sender name (wide): 'OBS_Spout'
SetSpoutSender END
[SPOUT CONNECTION] Connected to: OBS_Spout (1920x1080)
```

❌ **Failure (Current):**
```
[MSG HANDLER] WM_ENABLESPOUTMIX received
  m_szSpoutSenderName is EMPTY!
[SPOUT CONNECTION] Connected to: Milkwave 2 (654x511)
```

## Files to Modify

- **Remote/MilkwaveRemoteForm.cs** - Fix the Spout mix message sending

## No Visualizer Changes Needed!

The visualizer already has:
- ✅ Message handlers for WM_SETSPOUTSENDER and WM_ENABLESPOUTMIX
- ✅ SetSpoutSender() method
- ✅ EnableSpoutMixing() method
- ✅ Comprehensive debug logging

Just fix the Remote form to send the messages correctly!

