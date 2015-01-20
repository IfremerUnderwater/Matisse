
; Inno Setup Configuration file for MatisseServer
#define OSGeo4W64Dir "C:\OSGeo4W64"
#define SevenZipDir "C:\Program Files\7-Zip"
#define ToolsDir "D:\Extern_libraries\50-Outils"
#define OpenCVDir "D:\Extern_libraries\opencv248\opencv\build\x64\vc10"
#define OpenCVVersion "248"


[Setup]
AppName=Matisse
AppVersion=3
AppPublisher=Ifremer
DefaultDirName={pf}\Matisse
DefaultGroupName=Ifremer Matisse
Compression=none
SolidCompression=no
OutputDir=InnoSetupOutput
OutputBaseFilename=MatisseSetup
; "ArchitecturesAllowed=x64" specifies that Setup cannot run on
; anything but x64.
ArchitecturesAllowed=x64
; "ArchitecturesInstallIn64BitMode=x64" requests that the install be
; done in "64-bit mode" on x64, meaning it should use the native
; 64-bit Program Files directory and the 64-bit view of the registry.
ArchitecturesInstallIn64BitMode=x64

[Components]
;Name: "OSGeo4W64"; Description: "OSGeo4W64 Bundle QGis"; Types: full
Name: "MatisseApplication"; Description: "Matisse Application"; Types: full compact
Name: "MatisseQGisPlugin"; Description: "Matisse Qgis Plugin";Types: full compact 

[Files]
Source: "qt\Modules\MatisseCppLib\distrib\*.dll"; DestDir: "{app}"; Components: MatisseApplication
Source: "qt\Setup\*.dll"; DestDir: "{app}"; Components: MatisseApplication
Source: "qt\MatisseServer\release\MatisseServer.exe"; DestDir: "{app}"; Components: MatisseApplication
Source: "qt\xml\*"; DestDir: "{app}\xml"; Flags: recursesubdirs; Components: MatisseApplication
Source: "qt\dll\*"; DestDir: "{app}\dll"; Flags: recursesubdirs; Components: MatisseApplication
Source: "qt\Setup\MatisseSettings.xml"; DestDir: "{app}"; Components: MatisseApplication
Source: "qt\Setup\Matisse.bat"; DestDir: "{app}"; Components: MatisseApplication
Source: "python\*"; DestDir: "{#OSGeo4W64Dir}\apps\qgis\python\plugins"; Flags: recursesubdirs; Components: MatisseQGisPlugin
Source: "{#OpenCVDir}\bin\opencv_core{#OpenCVVersion}.dll"; DestDir: "{app}"; Components: MatisseApplication
Source: "{#OpenCVDir}\bin\opencv_core{#OpenCVVersion}d.dll"; DestDir: "{app}"; Components: MatisseApplication
Source: "{#OpenCVDir}\bin\opencv_highgui{#OpenCVVersion}.dll"; DestDir: "{app}"; Components: MatisseApplication
Source: "{#OpenCVDir}\bin\opencv_highgui{#OpenCVVersion}d.dll"; DestDir: "{app}"; Components: MatisseApplication
Source: "{#OpenCVDir}\bin\opencv_imgproc{#OpenCVVersion}.dll"; DestDir: "{app}"; Components: MatisseApplication
Source: "{#OpenCVDir}\bin\opencv_imgproc{#OpenCVVersion}d.dll"; DestDir: "{app}"; Components: MatisseApplication
Source: "{#SevenZipDir}\*"; DestDir: "{tmp}"; Flags: deleteafterinstall; Components: MatisseApplication
; Pour bypasser la resolution automatique des DLL MSVC
Source: "{#ToolsDir}\msvcrt.zip"; DestDir: "{tmp}"; Flags: deleteafterinstall; Components: MatisseApplication

[Run]
Filename: "{tmp}\7z.exe"; Parameters: "x {tmp}\msvcrt.zip -o""{app}"" "; Components: MatisseApplication

[Icons]
Name: "{group}\Matisse"; Filename: "{app}\Matisse.bat"; IconFilename: "{app}\MatisseServer.exe"

[UninstallDelete]
Type: files; Name: "{app}\msvc*.dll"

