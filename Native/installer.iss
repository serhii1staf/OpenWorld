#ifndef PackageRoot
  #error Supply /DPackageRoot=<absolute packaged game directory>
#endif
[Setup]
AppId={{061748D4-8376-41D1-9A2D-951723840753}
AppName=OpenWorld - Port Meridian
AppVersion=0.2.0
AppPublisher=OpenWorld
AppPublisherURL=https://github.com/serhii1staf/OpenWorld
DefaultDirName={localappdata}\Programs\OpenWorld
DefaultGroupName=OpenWorld
PrivilegesRequired=lowest
OutputDir=..\Artifacts\Release
OutputBaseFilename=OpenWorld-Setup-0.2.0
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
Compression=lzma2/fast
SolidCompression=yes
WizardStyle=modern
UninstallDisplayIcon={app}\OpenWorld.exe
LicenseFile=licenses\OPENWORLD-LICENSE.txt
[Files]
Source: "{#PackageRoot}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
[Icons]
Name: "{group}\OpenWorld"; Filename: "{app}\OpenWorld.exe"
Name: "{autodesktop}\OpenWorld"; Filename: "{app}\OpenWorld.exe"; Tasks: desktopicon
[Tasks]
Name: "desktopicon"; Description: "Create a desktop shortcut"; Flags: unchecked
[Run]
Filename: "{app}\OpenWorld.exe"; Description: "Launch OpenWorld"; Flags: nowait postinstall skipifsilent
