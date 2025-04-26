; -- Milkwave.iss: Inno Setup script
; https://jrsoftware.org/isinfo.php
; Deprecated: Using NSIS now (Milkwave.nsi)

[Setup]
AppVersion=1.5
SetupIconFile=..\Remote\MilkwaveRemote.ico
SourceDir=..\Release\
AppName=Milkwave
OutputBaseFilename=Milkwave-{#SetupSetting("AppVersion")}-Installer
WizardStyle=modern
DefaultDirName=C:\Tools\Milkwave
DefaultGroupName=Milkwave
UninstallDisplayIcon={app}\MilkwaveRemote.exe
Compression=lzma2
SolidCompression=yes
UsePreviousAppDir=false
DisableWelcomePage=yes

[Code]
procedure InitializeWizard();
var
  InfoLabel: TNewStaticText;
  InfoLabel2: TNewStaticText;
begin
  InfoLabel := TNewStaticText.Create(WizardForm);
  InfoLabel.Parent := WizardForm.SelectDirPage;
  InfoLabel.Left := 0;
  InfoLabel.Top := WizardForm.DirEdit.Top + WizardForm.DirEdit.Height + ScaleY(16);
  InfoLabel.Font.Style := [fsBold];
  InfoLabel.Caption := 'Please note that Milkwave needs full write access to its directory!'

  InfoLabel2 := TNewStaticText.Create(WizardForm);
  InfoLabel2.Parent := WizardForm.SelectDirPage;
  InfoLabel2.Left := 0;
  InfoLabel2.Top := InfoLabel.Top + InfoLabel.Height + 5;
  InfoLabel2.Caption := 'Do not install into "Program Files" or a similar protected location.'
end;

[Files]

Source: "resources\data\*"; DestDir: "{app}\resources\data"; Flags: recursesubdirs
Source: "resources\docs\*"; DestDir: "{app}\resources\docs"; Flags: recursesubdirs
Source: "resources\presets\*"; DestDir: "{app}\resources\presets"; Flags: recursesubdirs
Source: "resources\sprites\*"; DestDir: "{app}\resources\sprites"; Flags: recursesubdirs
Source: "resources\textures\*"; DestDir: "{app}\resources\textures"; Flags: recursesubdirs
Source: "messages.ini"; DestDir: "{app}"; Flags: onlyifdoesntexist
Source: "MilkwaveRemote.dll"; DestDir: "{app}"
Source: "MilkwaveRemote.exe"; DestDir: "{app}"
Source: "MilkwaveRemote.runtimeconfig.json"; DestDir: "{app}"
Source: "MilkwaveVisualizer.exe"; DestDir: "{app}"
Source: "NAudio.Wasapi.dll"; DestDir: "{app}"
Source: "README.txt"; DestDir: "{app}"
Source: "script-default.txt"; DestDir: "{app}"
Source: "settings.ini"; DestDir: "{app}"; Flags: onlyifdoesntexist
Source: "settings-remote.json"; DestDir: "{app}"; Flags: onlyifdoesntexist
Source: "sprites.ini"; DestDir: "{app}"; Flags: onlyifdoesntexist

[Icons]
Name: "{group}\Milkwave Visualizer"; Filename: "{app}\MilkwaveVisualizer.exe"
Name: "{group}\Milkwave Remote"; Filename: "{app}\MilkwaveRemote.exe"

[Run]
Filename: "{app}\README.txt"; WorkingDir: "{app}"; Description: "View README.txt"; Flags: postinstall shellexec
Filename: "{app}\MilkwaveRemote.exe"; WorkingDir: "{app}"; Description: "Run Milkwave now!"; Flags: postinstall shellexec nowait