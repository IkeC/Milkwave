// plugin_internals.h - Shared declarations for plugin_*.cpp translation units
// This header provides access to global variables, free functions, and macros
// defined in plugin.cpp that are needed across multiple plugin_*.cpp files.
#pragma once

#include "plugin.h"
#include <d3d9.h>
#include <cstdint>

// =====================================================================
// Macros
// =====================================================================
#define FRAND ((rand() % 7381) / 7380.0f)

#ifndef clamp
#define clamp(value, min, max) ((value) < (min) ? (min) : ((value) > (max) ? (max) : (value)))
#endif

#define IsAlphabetChar(x) ((x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z'))
#define IsAlphanumericChar(x) ((x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z') || (x >= '0' && x <= '9') || x == '.')
#define IsNumericChar(x) (x >= '0' && x <= '9')

// =====================================================================
// Extern declarations for global variables defined in plugin.cpp
// =====================================================================
extern int ToggleFPSNumPressed;
extern int HardcutMode;
extern float timetick;
extern float timetick2;
extern float TimeToAutoLockPreset;
extern int beatcount;
extern bool TranspaMode;
extern int NumTotalPresetsLoaded;
extern bool AutoLockedPreset;
extern uint64_t LastSentMilkwaveMessage;

extern CPlugin g_plugin;         // defined in main.cpp
extern bool g_bDebugOutput;      // defined in support.cpp
extern bool g_bDumpFileCleared;  // defined in support.cpp

// =====================================================================
// TCP network server control (defined in MilkwaveVisualizer.cpp)
// Used by the Ctrl+Shift+N / Ctrl+N hotkeys and by the Remote's
// "NETWORK_SERVER" / "NETWORK_SERVER_STATUS" button commands.
// =====================================================================
void ToggleNetworkServer(HWND hwnd);
void ShowNetworkServerStatus();

// =====================================================================
// Window helpers (defined in MilkwaveVisualizer.cpp, called from
// plugin_messages.cpp for the corresponding Remote button commands).
// =====================================================================
void ToggleStretch(HWND hwnd);
void ResetWindow(HWND hwnd);
void SetWindowFixedDimensions(HWND hwnd);
void ChangeWindowOpacityByStep(bool increase);
void SetDefaultAudioDevice();
void ShowAudioDeviceName();

// Locates MilkwaveRemote.exe next to the visualizer (see plugin.cpp)
bool MilkwaveRemoteFindExecutable(std::wstring& exePath);

// Preset blend transition types (settings.ini [Settings] Mixtype; -1 = random)
int GetTransitionTypeCount();
const wchar_t* GetTransitionTypeName(int mixType);

// Vertex declarations (defined in plugin.cpp)
extern D3DVERTEXELEMENT9 g_MyVertDecl[];
extern D3DVERTEXELEMENT9 g_WfVertDecl[];
extern D3DVERTEXELEMENT9 g_SpriteVertDecl[];

// Threading (defined in plugin.cpp)
extern volatile HANDLE g_hThread;
extern volatile bool g_bThreadAlive;
extern volatile int g_bThreadShouldQuit;

// Critical sections (defined in plugin.cpp)
extern CRITICAL_SECTION g_cs;
extern CRITICAL_SECTION g_csRemoteMessage;

// Help text (defined in plugin.cpp)
extern wchar_t* g_szHelp;
extern wchar_t* g_szHelp_Page2;
extern wchar_t* g_szHelp_Page3;
extern int g_szHelp_W;

// LC2UC table (defined in plugin.cpp)
extern const unsigned char LC2UC[256];

// =====================================================================
// Free function declarations (defined in plugin.cpp)
// =====================================================================

// Clipboard
void copyStringToClipboardA(const char* source);
void copyStringToClipboardW(const wchar_t* source);
char* getStringFromClipboardA();
wchar_t* getStringFromClipboardW();

// String conversion
void ConvertCRsToLFCA(const char* src, char* dst);
void ConvertCRsToLFCW(const wchar_t* src, wchar_t* dst);
void ConvertLFCToCRsA(const char* src, char* dst);
void ConvertLFCToCRsW(const wchar_t* src, wchar_t* dst);
int mystrcmpiW(const wchar_t* s1, const wchar_t* s2);

// File I/O
bool ReadFileToString(const wchar_t* szBaseFilename, char* szDestText, int nMaxBytes, bool bConvertLFsToSpecialChar);

// Shader utilities
void ConvertLLCto1310(char* d, const char* s);
void StripComments(char* str);

// Threading
void CancelThread(int max_wait_time_ms);

// Math utilities
float SquishToCenter(float x, float fExp);
int GetNearestPow2Size(int w, int h);
float fCubicInterpolate(float y0, float y1, float y2, float y3, float t);
DWORD dwCubicInterpolate(DWORD y0, DWORD y1, DWORD y2, DWORD y3, float t);

// Texture utilities
bool PickRandomTexture(const wchar_t* prefix, wchar_t* szRetTextureFilename);

// Shader params master list (defined in plugin_shaders.cpp)
extern CShaderParamsList global_CShaderParams_master_list;

// Editor callbacks (used by menu system)
void OnUserEditedPerFrame(LPARAM param1, LPARAM param2);
void OnUserEditedPerPixel(LPARAM param1, LPARAM param2);
void OnUserEditedPresetInit(LPARAM param1, LPARAM param2);
void OnUserEditedWavecode(LPARAM param1, LPARAM param2);
void OnUserEditedWavecodeInit(LPARAM param1, LPARAM param2);
void OnUserEditedShapecode(LPARAM param1, LPARAM param2);
void OnUserEditedShapecodeInit(LPARAM param1, LPARAM param2);
void OnUserEditedWarpShaders(LPARAM param1, LPARAM param2);
void OnUserEditedCompShaders(LPARAM param1, LPARAM param2);

// Input helpers (defined in plugin_input.cpp)
void ToggleTransparency(HWND hwnd);
void ToggleWindowOpacity(HWND hwnd, bool bDown);
void LoadPresetFilesViaDragAndDrop(WPARAM wParam);

// IPC helper
#define IPC_CB_VISRANDOM 628
