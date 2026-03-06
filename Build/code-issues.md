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

*(No remaining warnings.)*

---

### `milkdropfs.cpp`

| Warning | Line(s) | Description | Action |
|---------|---------|-------------|--------|
| **C4288** | 926–928 | Nonstandard extension: loop control variable `i` declared in `for` is used outside the loop scope, conflicts with outer declaration at line 836 | Rename the inner loop variable or add a scoping block; blocked on D9035 removal |
| **C4101** | 836 | Unreferenced local variable `i` (outer declaration shadowed by inner for-loop variable) | Remove or fold into the block once C4288 is resolved |
| **C4553** | 3862, 3863 | `==` result not used — possible logic bug where `=` was intended | Review carefully before changing; could be a silent no-op assignment or a real bug |

---

## C# Warnings — `MilkwaveRemote`

*(No remaining warnings — all resolved.)*
