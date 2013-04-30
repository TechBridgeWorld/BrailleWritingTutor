; -- BWTEmulator32.iss --
; This is the Inno Setup Windows Installer script
; for installing on 32-bit windows machines.
; If anyone at TechBridgeWorld would like to add
; anything to the installer, for instance a terms
; of use, this would be the place to do so 

[Setup]
AppName=BWT Emulator
AppVersion=1.0
DefaultDirName={pf}\BWTEmulator
DefaultGroupName=BWTEmulator
Compression=zip
SolidCompression=yes
OutputDir=C:/Users/dswen/My Documents/Documents/School/15/239/

[Files]
Source: "emulator.jar"; DestDir: "{app}"
Source: "README.txt"; DestDir: "{app}"; Flags: isreadme
Source: "com0com\i386\setupc.exe"; DestDir: "{app}/com0com";
Source: "com0com\i386\com0com.inf"; DestDir: "{app}/com0com";
Source: "com0com\i386\com0com.sys"; DestDir: "{app}/com0com";
Source: "com0com\i386\setup.dll"; DestDir: "{app}/com0com";

[Icons]
Name: "{group}\BWT Emulator"; Filename: "{app}\emulator.jar"

[Run]
Filename: "{app}\com0com\setupc.exe"; Parameters: "install PortName=COM1 PortName=COM7"

[UninstallRun]
Filename: "{app}\com0com\setupc.exe"; Parameters: "uninstall"

