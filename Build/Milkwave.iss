; -- Milkwave.iss: Inno Setup script
; https://jrsoftware.org/isinfo.php

[Setup]
AppVersion=1.4
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
Source: "messages.ini"; DestDir: "{app}"
Source: "MilkwaveRemote.dll"; DestDir: "{app}"
Source: "MilkwaveRemote.exe"; DestDir: "{app}"
Source: "MilkwaveRemote.runtimeconfig.json"; DestDir: "{app}"
Source: "MilkwaveVisualizer.exe"; DestDir: "{app}"
Source: "NAudio.Wasapi.dll"; DestDir: "{app}"
Source: "README.txt"; DestDir: "{app}"; Flags: isreadme
Source: "script-default.txt"; DestDir: "{app}"
Source: "settings.ini"; DestDir: "{app}"
Source: "settings-milkwave.json"; DestDir: "{app}"
Source: "sprites.ini"; DestDir: "{app}"

[Icons]
Name: "{group}\Milkwave Remote"; Filename: "{app}\MilkwaveRemote.exe"
Name: "{group}\Milkwave Visualizer"; Filename: "{app}\MilkwaveVisualizer.exe"

[Run]
Filename: {app}\MilkwaveRemote.exe; Description: Run Milkwave now!; Flags: postinstall nowait skipifsilent