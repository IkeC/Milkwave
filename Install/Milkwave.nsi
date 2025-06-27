; Milkwave.nsi
; NSIS installer script

!include "MUI2.nsh"

!define MUI_ABORTWARNING

!define MUI_ICON "..\Resources\MilkwaveVisualizer.ico"
!define MUI_UNICON "..\Resources\MilkwaveVisualizer.ico"

!define VERSION "2.3"
!define VER_MAJOR 2
!define VER_MINOR 3

!define RELDIR "..\Release\"

!define REG_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\Milkwave"

Name "Milkwave ${VERSION}"
OutFile "Milkwave-${VERSION}-Installer.exe"
InstallDir "C:\Tools\Milkwave"

RequestExecutionLevel user

; Page defines
!define MUI_COMPONENTSPAGE_NODESC

!define MUI_DIRECTORYPAGE_TEXT_TOP "Milkwave needs FULL WRITE ACCESS to its directory! Do NOT install into $\"Program Files$\" or a similar protected location."

!define MUI_FINISHPAGE_RUN "$INSTDIR\MilkwaveVisualizer.exe"
!define MUI_FINISHPAGE_RUN_TEXT "Run Milkwave Visualizer now!"

!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\README.txt"
!define MUI_FINISHPAGE_SHOWREADME_TEXT "Show README.txt"

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
  CopyFiles $INSTDIR\settings-remote.json $INSTDIR\backup
  
  SetOverwrite try
  
  SetOutPath "$INSTDIR\resources\data\"
  File /r "${RELDIR}resources\data\*" 
  
  SetOutPath "$INSTDIR\resources\docs\"
  File /r "${RELDIR}resources\docs\*"

  SetOutPath "$INSTDIR\resources\presets\Milkwave\"
  File /r "${RELDIR}resources\presets\Milkwave\*"

  SetOutPath "$INSTDIR\resources\sprites\"
  File /r "${RELDIR}resources\sprites\*"

  SetOutPath "$INSTDIR\resources\textures\"
  File /r "${RELDIR}resources\textures\*"
  
  SetOutPath "$INSTDIR"
  File "${RELDIR}MilkwaveRemote.dll"
  File "${RELDIR}MilkwaveRemote.exe"
  File "${RELDIR}MilkwaveRemote.runtimeconfig.json"
  File "${RELDIR}MilkwaveVisualizer.exe"
  File "${RELDIR}NAudio.Wasapi.dll"
  File "${RELDIR}README.txt"
  SetOverwrite off
  File "${RELDIR}script-default.txt"
  File "${RELDIR}settings-remote.json"
  File "${RELDIR}tags-remote.json"
  File "${RELDIR}messages.ini"
  File "${RELDIR}settings.ini"
  File "${RELDIR}sprites.ini"
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

  SetOutPath "$INSTDIR\resources\presets\Incubo_ Picks"
  File /r "${RELDIR}resources\presets\Incubo_ Picks\*.*"

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

; Uninstaller
Section Uninstall
    
  RMDir /r "$INSTDIR\resources"
  RMDir /r "$INSTDIR\backup"
  RMDir /r "$INSTDIR\log"
  
  Delete "$INSTDIR\MilkwaveRemote.dll"
  Delete "$INSTDIR\MilkwaveRemote.exe"
  Delete "$INSTDIR\MilkwaveRemote.runtimeconfig.json"
  Delete "$INSTDIR\MilkwaveVisualizer.exe"
  Delete "$INSTDIR\NAudio.Wasapi.dll"
  Delete "$INSTDIR\README.txt"
  Delete "$INSTDIR\script-default.txt"
  Delete "$INSTDIR\settings-remote.json"
  Delete "$INSTDIR\tags-remote.json"
  Delete "$INSTDIR\messages.ini"
  Delete "$INSTDIR\settings.ini"
  Delete "$INSTDIR\sprites.ini"
  
  Delete "$INSTDIR\Uninstall.exe"
  
  RMDir $INSTDIR
      
  Delete "$SMPROGRAMS\Milkwave*.lnk"
  Delete "$Desktop\Milkwave Remote.lnk"
  Delete "$Desktop\Milkwave Visualizer.lnk"
  
  
  DeleteRegKey HKCU "${REG_UNINST_KEY}"
  DeleteRegKey HKCU "Software\Milkwave"
  
SectionEnd