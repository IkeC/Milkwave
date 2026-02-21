# QUICK FIX - Spout Mixing Issue

## The Problem
Visualizer connects to wrong sender (Milkwave 2) instead of selected sender (OBS_Spout)

## The Fix

### File: Remote/MilkwaveRemoteForm.cs
**Location:** Around line 1599-1602 in the `SendToMilkwaveVisualizer()` method

**Current Code (BROKEN):**
```csharp
} else if (type == MessageType.SpoutInput) {
  string senderName = cboSputInput.Text;
  bool mixEnabled = chkSpoutMix.Checked;
  message = "SPOUTINPUT=" + (mixEnabled ? "1" : "0") + "|" + senderName;
  statusMessage = $"Spout mixing {(mixEnabled ? "enabled" : "disabled")}: {senderName}";
```

**New Code (FIXED):**
```csharp
} else if (type == MessageType.SpoutInput) {
  string senderName = cboSputInput.Text;
  bool mixEnabled = chkSpoutMix.Checked;
  statusMessage = $"Spout mixing {(mixEnabled ? "enabled" : "disabled")}: {senderName}";
  
  // Use direct window messages - send sender name FIRST, then enable
  if (foundWindow != IntPtr.Zero) {
    if (mixEnabled && senderName.Length > 0) {
      SendStringMessage(foundWindow, WM_SETSPOUTSENDER, senderName);
      System.Threading.Thread.Sleep(50);
    }
    PostMessage(foundWindow, WM_ENABLESPOUTMIX, (IntPtr)(mixEnabled ? 1 : 0), IntPtr.Zero);
    if (statusMessage.Length > 0) {
      SetStatusText($"{statusMessage} {foundWindowTitle}");
    }
  } else {
    SetStatusText(windowNotFound);
  }
  SendingMessage = false;
  return;
}
```

## Why This Works

1. **Removes the text message approach** - The old `SPOUTINPUT=...` message wasn't being properly parsed
2. **Sends sender name FIRST** - `WM_SETSPOUTSENDER` sets `m_szSpoutSenderName` in visualizer
3. **Then enables mixing** - `WM_ENABLESPOUTMIX` tells visualizer to enable, using the sender name that was just set
4. **Proper order matters** - If you enable before setting the name, visualizer defaults to active sender (Milkwave 2)

## Required Constants

These should already be defined in the form (check around line 1210-1220):
```csharp
private const int WM_SETSPOUTSENDER = 0x0400 + 108;    // WM_USER + 108
private const int WM_ENABLESPOUTMIX = 0x0400 + 109;    // WM_USER + 109
```

## Result

After this change, when you click "Mix":
1. Remote sends sender name to visualizer
2. Visualizer stores the sender name
3. Remote sends enable command
4. Visualizer connects to the CORRECT sender

✅ **Spout texture will appear in visualizer from the selected sender**

