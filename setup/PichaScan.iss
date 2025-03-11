[Setup]
AppName=PichaScan
AppVersion=1.0
DefaultDirName={autopf}\PichaScan
DefaultGroupName=PichaScan
OutputDir=.
OutputBaseFilename=PichaScan_Setup
Compression=lzma
SolidCompression=yes

[Files]
Source: "..\build\release\*"; DestDir: "{app}"; Flags: recursesubdirs

[Icons]
Name: "{commondesktop}\PichaScan"; Filename: "{app}\PichaScan.exe"; Check: IsNotSilent

[Run]
Filename: "{app}\PichaScan.exe"; Description: "Launch PichaScan"; Flags: nowait postinstall skipifsilent shellexec
