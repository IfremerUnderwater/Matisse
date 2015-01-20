
; Inno Setup Configuration file for OSGeo4W64
#define OSGeo4W64Dir "C:\OSGeo4W64"
#define SevenZipDir "C:\Program Files\7-Zip"
#define ToolsDir "D:\Extern_libraries\50-Outils"



[Setup]
AppName=OSGeo4W64
AppVersion=2.0.1
AppPublisher=Ifremer
DefaultDirName={#OSGeo4W64Dir}
DisableDirPage=yes
DefaultGroupName=Ifremer Matisse
Compression=none
SolidCompression=no
OutputDir=InnoSetupOutput
OutputBaseFilename=OSGeo4W64Setup
; "ArchitecturesAllowed=x64" specifies that Setup cannot run on
; anything but x64.
ArchitecturesAllowed=x64
; "ArchitecturesInstallIn64BitMode=x64" requests that the install be
; done in "64-bit mode" on x64, meaning it should use the native
; 64-bit Program Files directory and the 64-bit view of the registry.
ArchitecturesInstallIn64BitMode=x64


[Files]
Source: "{#SevenZipDir}\*"; DestDir: "{tmp}"; Flags: deleteafterinstall
Source: "{#ToolsDir}\OSGeo4W64.zip"; DestDir: "{tmp}"; Flags: deleteafterinstall

[Run]
Filename: "{tmp}\7z.exe"; Parameters: "x {tmp}\OSGeo4W64.zip -o{#OSGeo4W64Dir}"

[UninstallDelete]
Type: filesandordirs; Name: "{#OSGeo4W64Dir}"


[Icons]
Name: "{group}\QGis"; Filename: "{#OSGeo4W64Dir}\bin\qgis.bat"; IconFilename: "{#OSGeo4W64Dir}\bin\qgis-bin.exe"


