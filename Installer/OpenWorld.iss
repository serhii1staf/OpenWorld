#ifndef BuildRoot
  #error Supply /DBuildRoot=<absolute packaged Windows directory>
#endif
#ifndef AppVersion
  #define AppVersion "0.0.1"
#endif
[Setup]
AppId={{253A8DA0-367F-49D0-A46C-A922342D22D9}
AppName=OpenWorld
AppVersion={#AppVersion}
DefaultDirName={autopf}\OpenWorld
DefaultGroupName=OpenWorld
OutputDir=..\Artifacts
OutputBaseFilename=OpenWorld-Setup-{#AppVersion}
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
UninstallDisplayIcon={app}\OpenWorld.exe
[Files]
Source: "{#BuildRoot}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
[Icons]
Name: "{group}\OpenWorld"; Filename: "{app}\OpenWorld.exe"
Name: "{autodesktop}\OpenWorld"; Filename: "{app}\OpenWorld.exe"; Tasks: desktopicon
[Tasks]
Name: "desktopicon"; Description: "Create a desktop shortcut"; Flags: unchecked
[Run]
Filename: "{app}\Engine\Extras\Redist\en-us\UEPrereqSetup_x64.exe"; Parameters: "/quiet /norestart"; StatusMsg: "Installing runtime prerequisites..."; Check: FileExists(ExpandConstant('{app}\Engine\Extras\Redist\en-us\UEPrereqSetup_x64.exe'))
Filename: "{app}\OpenWorld.exe"; Description: "Launch OpenWorld"; Flags: nowait postinstall skipifsilent
