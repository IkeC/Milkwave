; Milkwave.nsi
; NSIS installer script

!include "MUI2.nsh"
!include "LogicLib.nsh"

!define MUI_ABORTWARNING

!define MUI_ICON "..\Resources\MilkwaveVisualizer.ico"
!define MUI_UNICON "..\Resources\MilkwaveVisualizer.ico"

!define VERSION "4.2"
!define VER_MAJOR 4
!define VER_MINOR 2

!define RELDIR "..\Release\"

!define REG_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\Milkwave"

; Per-user .milk association (RequestExecutionLevel user, so HKCU\Software\Classes)
!define REG_ASSOC_PROGID "Milkwave.Preset"
!define REG_ASSOC_EXT ".milk"

Name "Milkwave ${VERSION}"
OutFile "Milkwave-${VERSION}-Installer.exe"
InstallDir "C:\Tools\Milkwave"

RequestExecutionLevel user

; Page defines
!define MUI_COMPONENTSPAGE_NODESC

!define MUI_DIRECTORYPAGE_TEXT_TOP "Milkwave needs FULL WRITE ACCESS to its directory! Do NOT install into $\"Program Files$\" or a similar protected location."

!define MUI_FINISHPAGE_RUN "$INSTDIR\MilkwaveRemote.exe"
!define MUI_FINISHPAGE_RUN_TEXT "Run Milkwave now!"

; The MUI finish page has two checkbox slots. The README keeps its place as a link
; below them, and its former checkbox slot asks for the .milk file association.
; The box is ticked by default (MUI only clears it with *_NOTCHECKED).
!define MUI_FINISHPAGE_SHOWREADME ""
!define MUI_FINISHPAGE_SHOWREADME_TEXT "Associate .milk files with Milkwave Visualizer"
!define MUI_FINISHPAGE_SHOWREADME_FUNCTION AssocMilkFiles

!define MUI_FINISHPAGE_LINK "Show README.txt"
!define MUI_FINISHPAGE_LINK_LOCATION "$INSTDIR\README.txt"

; Installer Pages
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

; Unistaller Pages
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

; Languages
!insertmacro MUI_LANGUAGE "English"

; Installer Sections
Section "Milkwave" SecMilkwave
  
  SectionIn RO ;Make it read-only
    
  CopyFiles $INSTDIR\*.ini $INSTDIR\backup
  CopyFiles $INSTDIR\script-default.txt $INSTDIR\backup
  CopyFiles $INSTDIR\*.json $INSTDIR\backup
  
  SetOverwrite try
  
  SetOutPath "$INSTDIR\resources\buttons\"
  File /r "${RELDIR}resources\buttons\*" 

  SetOutPath "$INSTDIR\resources\data\"
  File /r "${RELDIR}resources\data\*" 
  
  SetOutPath "$INSTDIR\resources\docs\"
  File /r "${RELDIR}resources\docs\*"

  SetOutPath "$INSTDIR\resources\presets\Milkwave\"
  File "${RELDIR}resources\presets\Milkwave\*"

  SetOutPath "$INSTDIR\resources\presets\Milkwave\Shader\"
  File "${RELDIR}resources\presets\Milkwave\Shader\*"

  SetOutPath "$INSTDIR\resources\presets\Milkwave\Basic\"
  File "${RELDIR}resources\presets\Milkwave\Basic\*"

  SetOutPath "$INSTDIR\resources\sprites\"
  File /r "${RELDIR}resources\sprites\*"

  SetOutPath "$INSTDIR\resources\textures\"
  File /r "${RELDIR}resources\textures\*"
  
  SetOutPath "$INSTDIR\MDropDX12\"
  File /r "${RELDIR}MDropDX12\*"

  SetOutPath "$INSTDIR"
  File "${RELDIR}MilkwaveRemote.dll"
  File "${RELDIR}MilkwaveRemote.exe"
  File "${RELDIR}MilkwaveRemote.runtimeconfig.json"
  File "${RELDIR}MilkwaveVisualizer.exe"
  File "${RELDIR}NAudio.Core.dll"
  File "${RELDIR}NAudio.Midi.dll"
  File "${RELDIR}NAudio.Wasapi.dll"
  File "${RELDIR}README.txt"
  File "${RELDIR}System.Management.dll"
  File "${RELDIR}visualizer-keys.txt"

  SetOverwrite off
  File "${RELDIR}script-default.txt"
  File "${RELDIR}midi-default.txt"
  File "${RELDIR}settings-remote.json"
  File "${RELDIR}tags-remote.json"
  File "${RELDIR}controller-remote.json"
  File "${RELDIR}messages.ini"
  File "${RELDIR}settings.ini"
  File "${RELDIR}sprites.ini"
  File "${RELDIR}precompile.txt"
  File "${RELDIR}messages-editor.html"
  SetOverwrite on

  SetOutPath $INSTDIR

  ;Store installation folder
  WriteRegStr HKCU "Software\Milkwave" "" $INSTDIR

  WriteRegStr HKCU "${REG_UNINST_KEY}" "UninstallString" '"$INSTDIR\Uninstall.exe"'
  WriteRegStr HKCU "${REG_UNINST_KEY}" "QuietUninstallString" '"$INSTDIR\Uninstall.exe" /S'
  WriteRegStr HKCU "${REG_UNINST_KEY}" "InstallLocation" "$INSTDIR"
  
  WriteRegStr HKCU "${REG_UNINST_KEY}" "DisplayName" "Milkwave Visualizer"
  WriteRegStr HKCU "${REG_UNINST_KEY}" "DisplayIcon" "$INSTDIR\MilkwaveVisualizer.exe"
  
  WriteRegStr HKCU "${REG_UNINST_KEY}" "DisplayVersion" "${VERSION}"
  WriteRegDWORD HKCU "${REG_UNINST_KEY}" "VersionMajor" "${VER_MAJOR}"
  WriteRegDWORD HKCU "${REG_UNINST_KEY}" "VersionMinor" "${VER_MINOR}"

  WriteRegStr HKCU "${REG_UNINST_KEY}" "Publisher" "IkeC and Contributors"
  WriteRegStr HKCU "${REG_UNINST_KEY}" "URLInfoAbout" "https://github.com/IkeC/Milkwave"

  WriteRegDWORD HKCU "${REG_UNINST_KEY}" "NoModify" 1
  WriteRegDWORD HKCU "${REG_UNINST_KEY}" "NoRepair" 1
  ${MakeARPInstallDate} $1
  WriteRegStr HKCU "${REG_UNINST_KEY}" "InstallDate" $1
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"
SectionEnd

Section "Additional Presets" SecPresets
    
  SetOutPath "$INSTDIR"
  SetOverwrite try
  
  SetOutPath "$INSTDIR\resources\presets\BeatDrop"
  File /r "${RELDIR}resources\presets\BeatDrop\*.*"

  SetOutPath "$INSTDIR\resources\presets\Butterchurn"
  File /r "${RELDIR}resources\presets\Butterchurn\*.*"

  SetOutPath "$INSTDIR\resources\presets\Incubo_"
  File /r "${RELDIR}resources\presets\Incubo_\*.*"

  SetOutPath "$INSTDIR\resources\presets\Milkdrop2077"
  File /r "${RELDIR}resources\presets\Milkdrop2077\*.*"
  
  SetOutPath "$INSTDIR"
SectionEnd

Section "Start menu items"
  ;Create shortcuts
  CreateShortcut "$SMPROGRAMS\Milkwave Remote.lnk" "$INSTDIR\MilkwaveRemote.exe"
  CreateShortcut "$SMPROGRAMS\Milkwave Visualizer.lnk" "$INSTDIR\MilkwaveVisualizer.exe"
  CreateShortcut "$SMPROGRAMS\Milkwave Uninstall.lnk" "$INSTDIR\Uninstall.exe"
SectionEnd

Section "Desktop shortcuts"
  CreateShortcut "$Desktop\Milkwave Remote.lnk" "$INSTDIR\MilkwaveRemote.exe"
  CreateShortcut "$Desktop\Milkwave Visualizer.lnk" "$INSTDIR\MilkwaveVisualizer.exe"
SectionEnd

; Finish page: associate .milk files with the Visualizer (runs only when the
; checkbox is ticked). The Visualizer receives the preset file as its first
; argument and either loads it or forwards it to a running instance.
Function AssocMilkFiles
  WriteRegStr HKCU "Software\Classes\${REG_ASSOC_EXT}" "" "${REG_ASSOC_PROGID}"
  WriteRegStr HKCU "Software\Classes\${REG_ASSOC_PROGID}" "" "Milkwave Visualizer Preset"
  WriteRegStr HKCU "Software\Classes\${REG_ASSOC_PROGID}\DefaultIcon" "" "$INSTDIR\MilkwaveVisualizer.exe,0"
  WriteRegStr HKCU "Software\Classes\${REG_ASSOC_PROGID}\shell\open\command" "" '"$INSTDIR\MilkwaveVisualizer.exe" "%1"'

  ; "Open with" hand-picks the Applications\<exe> ProgID, which Windows builds
  ; without command line arguments. Register it so the preset path is passed on
  ; that route as well.
  WriteRegStr HKCU "Software\Classes\Applications\MilkwaveVisualizer.exe" "FriendlyAppName" "Milkwave Visualizer"
  WriteRegStr HKCU "Software\Classes\Applications\MilkwaveVisualizer.exe\DefaultIcon" "" "$INSTDIR\MilkwaveVisualizer.exe,0"
  WriteRegStr HKCU "Software\Classes\Applications\MilkwaveVisualizer.exe\shell\open\command" "" '"$INSTDIR\MilkwaveVisualizer.exe" "%1"'
  WriteRegStr HKCU "Software\Classes\Applications\MilkwaveVisualizer.exe\SupportedTypes" "${REG_ASSOC_EXT}" ""

  ; A leftover per-extension choice wins over the classes mapping above and can
  ; point at a handler that drops the file name, so clear it.
  DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\${REG_ASSOC_EXT}\UserChoice"
  DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\${REG_ASSOC_EXT}\UserChoiceLatest"

  WriteRegStr HKCU "Software\Milkwave" "PresetAssociation" "${REG_ASSOC_PROGID}"
  ; Tell Explorer about the new association instead of waiting for a logoff.
  System::Call 'shell32::SHChangeNotify(i 0x08000000, i 0x1000, i 0, i 0)'
FunctionEnd

; Uninstaller
Section Uninstall
    
  RMDir /r "$INSTDIR\resources"
  RMDir /r "$INSTDIR\backup"
  RMDir /r "$INSTDIR\log"
  RMDir /r "$INSTDIR\runtimes"
  RMDir /r "$INSTDIR\MDropDX12"

  Delete "$INSTDIR\MilkwaveRemote.dll"
  Delete "$INSTDIR\MilkwaveRemote.exe"
  Delete "$INSTDIR\MilkwaveRemote.runtimeconfig.json"
  Delete "$INSTDIR\MilkwaveVisualizer.exe"
  Delete "$INSTDIR\NAudio.Core.dll"
  Delete "$INSTDIR\NAudio.Midi.dll"
  Delete "$INSTDIR\NAudio.Wasapi.dll"
  Delete "$INSTDIR\README.txt"
  Delete "$INSTDIR\script-default.txt"
  Delete "$INSTDIR\midi-default.txt"
  Delete "$INSTDIR\settings-remote.json"
  Delete "$INSTDIR\tags-remote.json"
  Delete "$INSTDIR\controller-remote.json"
  Delete "$INSTDIR\messages.ini"
  Delete "$INSTDIR\settings.ini"
  Delete "$INSTDIR\sprites.ini"
  Delete "$INSTDIR\precompile.txt"
  Delete "$INSTDIR\messages-editor.html"
  
  Delete "$INSTDIR\Uninstall.exe"
  
  RMDir $INSTDIR
      
  Delete "$SMPROGRAMS\Milkwave*.lnk"
  Delete "$Desktop\Milkwave Remote.lnk"
  Delete "$Desktop\Milkwave Visualizer.lnk"
  
  
  DeleteRegKey HKCU "${REG_UNINST_KEY}"
  DeleteRegKey HKCU "Software\Milkwave"

  ; Drop the .milk association, but only while it still points at us - the user
  ; may have re-associated .milk files with another visualizer since.
  ReadRegStr $0 HKCU "Software\Classes\${REG_ASSOC_EXT}" ""
  ${If} $0 == "${REG_ASSOC_PROGID}"
    DeleteRegKey HKCU "Software\Classes\${REG_ASSOC_EXT}"
  ${EndIf}
  DeleteRegKey HKCU "Software\Classes\${REG_ASSOC_PROGID}"
  ReadRegStr $1 HKCU "Software\Classes\Applications\MilkwaveVisualizer.exe\shell\open\command" ""
  ${If} $1 == '"$INSTDIR\MilkwaveVisualizer.exe" "%1"'
    DeleteRegKey HKCU "Software\Classes\Applications\MilkwaveVisualizer.exe"
  ${EndIf}

  ; Without the classes mapping the stale choice would pop up "How do you want
  ; to open this file?" instead of falling back to another handler.
  DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\${REG_ASSOC_EXT}\UserChoice"
  DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\${REG_ASSOC_EXT}\UserChoiceLatest"
  System::Call 'shell32::SHChangeNotify(i 0x08000000, i 0x1000, i 0, i 0)'

SectionEnd