# Code Issues

Remaining build warnings and known code issues deferred for later review.

---

## Build Warnings

### Project Settings

| Warning | Description | Action |
|---------|-------------|--------|
| **D9035** | Compiler option `Zc:forScope-` is deprecated and will be removed in a future MSVC release | Remove from `plugin.vcxproj` — but first resolve C4288 scoping issues in `milkdropfs.cpp` that this option currently masks |

---

### `plugin.cpp`

| Warning | Line(s) | Description | Action |
|---------|---------|-------------|--------|
| **C4995** | 5778, 5786 | `DwmEnableComposition` is deprecated | Replace with `DwmSetWindowAttribute(hwnd, DWMWA_TRANSITIONS_FORCEDISABLED, ...)` or remove DWM toggle entirely if no longer needed on modern Windows |

---

### `milkdropfs.cpp`

| Warning | Line(s) | Description | Action |
|---------|---------|-------------|--------|
| **C4288** | 926–928 | Nonstandard extension: loop control variable `i` declared in `for` is used outside the loop scope, conflicts with outer declaration at line 836 | Rename the inner loop variable or add a scoping block; blocked on D9035 removal |
| **C4101** | 836 | Unreferenced local variable `i` (outer declaration shadowed by inner for-loop variable) | Remove or fold into the block once C4288 is resolved |
| **C4553** | 3862, 3863 | `==` result not used — possible logic bug where `=` was intended | Review carefully before changing; could be a silent no-op assignment or a real bug |
| **C4244** | 3446, 3522, 3613, 3681, 3690, 3750, 3882–3887, 3963, 4049, 4851–4869, 4899–4901 | `double` literal constants assigned/passed to `float` fields | Change double literals to `float` literals (append `f`) or add `static_cast<float>()` |
| **C4305** | 3613, 3690 | Truncation from `double` to `float` | Same as C4244 above |
| **C4244** | 5705, 5707 | `float` to `int` conversion | Add `static_cast<int>()` |

---

### `pluginshell.cpp`

| Warning | Line(s) | Description | Action |
|---------|---------|-------------|--------|
| **C4244** | 215, 273, 600, 1462, 2022, 2031, 2718, 2719, 2729, 2750, 2752, 2754 | Mixed `double`/`float`/`int` implicit conversions | Add explicit casts or change literal suffixes |
| **C4305** | 2032 | Truncation from `double` to `float` | Add `f` suffix to the literal |
| **C4554** | 1973 | Operator precedence: `&` in complex expression — parentheses missing | Add parentheses to clarify intent |
| **C4715** | `GetHeight` function | Not all control paths return a value | Add a fallback `return` at end of function |

---

### `Milkdrop2PcmVisualizer.cpp`

| Warning | Line(s) | Description | Action |
|---------|---------|-------------|--------|
| **C4996** | 1207–1208, 1484–1485, 1805 | `std::wstring_convert`, `std::codecvt_utf8`, and `std::locale::empty` are deprecated in C++17 | Replace with `MultiByteToWideChar` / `WideCharToMultiByte` from `<windows.h>`. Alternatively suppress with `_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING` if deferring |
| **C4101** | 2018 | Unreferenced local variable `fullPath` | Remove the variable |

---

## C# Warnings — `MilkwaveRemote`

### `Helper/DarkModeCS.cs` (third-party)

Most warnings in this file originate from a third-party dark-mode helper. The simplest fix is to add `#nullable disable` at the top of the file. Individual issues are:

- **CS8618** — Non-nullable fields/properties not initialized in constructor (`newWndProcDelegate`, `Components`, `OScolors`, `MyColors`, `ownerFormControlAdded`, `controlHandleCreated`, `controlControlAdded`)
- **CS8602/CS8600/CS8604/CS8605/CS8603/CS8625** — Numerous possible-null-dereference and null-conversion warnings throughout the file
- **CS8622** — Nullability mismatch on event handler delegates (`Tsdd_Opening`, `Tsmi_DropDownOpening`)
- **CS0184** — `is ComboBox` check is always false
- **CS0169** — Unused field `formHandle`
- **CS8600 / CS8602** at lines 1251–1288 — Null-unsafe string operations

---

### `MilkwaveRemoteForm.cs`

| Warning | Line(s) | Description | Action |
|---------|---------|-------------|--------|
| **CS8618** | 984 | Several non-nullable fields not initialized in constructor (`spoutRefreshTimer`, `MidiHelper`, `RemoteHelper`, `ofd`, `ofdShader`, `ofdShaderHLSL`, `CancellationTokenFilterPresetList`) | Initialize in constructor or mark fields as nullable |
| **CS0168** | 1016, 1267, 1383, 1480, 2953, 2976, 5442 | Declared but unused catch variables (`ex`) | Remove variable name: `catch (Exception)` |
| **CS0168** | 3115, 3487 | Declared but unused catch variable `e` | Remove variable name: `catch (Exception)` |
| **CS0219** | 1106 | Variable `doOpen` assigned but never used | Remove assignment or the variable |
| **CS8622** | 339, 1281–1283, 1429, 1431 | Nullability mismatch on event handler delegates | Change parameter type to `object?` |
| **CS8602** | 1285–1286, 2721, 2736, 2752, 2761, 2771, 5125 | Possible null dereference | Add null checks or use null-conditional operator |
| **CS8625** | 2763 | Cannot convert `null` to non-nullable reference type | Mark parameter or field as nullable |
| **CS8600** | 6331 | Converting possible null to non-nullable type | Add null check or use null-forgiving operator |
| **CS4986** | 4986 | Unused catch variable `ex` | Remove variable name |

---

### `Helper/ShaderHelper.cs`

| Warning | Line(s) | Description | Action |
|---------|---------|-------------|--------|
| **CS0168** | 148, 250, 316 | Declared but unused catch variable `e` | Remove variable name: `catch (Exception)` |

---

### `Helper/MidiHelper.cs`

| Warning | Line(s) | Description | Action |
|---------|---------|-------------|--------|
| **CS0168** | 37, 48 | Declared but unused catch variable `ex` | Remove variable name: `catch (Exception)` |

---

### `Helper/MonitorHelper.cs`

| Warning | Line(s) | Description | Action |
|---------|---------|-------------|--------|
| **CS0168** | 54, 68 | Declared but unused catch variable `e` | Remove variable name: `catch (Exception)` |

---

### `Helper/FlatTabControl.cs`

| Warning | Line(s) | Description | Action |
|---------|---------|-------------|--------|
| **CS8618** | 73 | Non-nullable field `PreRemoveTabPage` never initialized | Mark as nullable: `Action? PreRemoveTabPage` |
| **CS0649** | 73 | Field `PreRemoveTabPage` is never assigned | Same as above |

---

### `MilkwaveRemoteForm.Designer.cs`

| Warning | Line(s) | Description | Action |
|---------|---------|-------------|--------|
| **CS0649** | 5250 | Field `txtAutoplay` never assigned | Check if this control is wired up in the designer; may be a leftover |
| **CS0169** | 5229, 5291, 5443, 5555 | Unused fields: `statusSupporters`, `toolStripMenuItemVisualizerPanel`, `btnShaderError`, `btnVideoInputSet` | Remove if the controls are not used |
| **CS0169** | 5228 | Unused field `statusHelp` | Remove if not used |
