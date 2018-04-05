rem @echo off
cd %~dp0
call env.bat

call "%OSGEO4W_ROOT%\bin\o4w_env.bat"

@set WORKSPACE_ROOT=F:\Projects\MatisseWorkspaceWindows
echo %WORKSPACE_ROOT%

set PATH=%OSGEO4W_ROOT%\apps\qgis\bin;%PATH%

set PATH=%PATH%;%WORKSPACE_ROOT%\Build\Release\Libraries
set PATH=%PATH%;F:\ThirdPartyLibs\opencv331\opencv\build\x64\vc14\bin
set PATH=%PATH%;%OSGEO4W_ROOT%\apps\gdal-dev\bin
set PATH=%OSGEO4W_ROOT%\apps\Qt5\bin;%PATH%
set PATH=%PATH%;F:\ThirdPartyLibs\OpenSceneGraph-3.4.1\build-vs2017\bin
path

set QT_PLUGIN_PATH=%OSGEO4W_ROOT%\apps\qgis\bin\plugins

rem Ajout des paths pour les DLL des modules
rem set MATISSE_LIB_ROOT=%WORKSPACE_ROOT%\Run\Release\Libraries
rem set MATISSE_LIB_PATH=%MATISSE_LIB_ROOT%\MatisseTools;%MATISSE_LIB_ROOT%\MatisseCommon;%MATISSE_LIB_ROOT%\OpticalMapping
rem set PATH=%MATISSE_LIB_PATH%;%PATH%
set PATH=%WORKSPACE_ROOT%\Run\Release\Libraries;%PATH%

echo before
echo %WORKSPACE_ROOT%\Run\Release\MatisseServer.exe
echo after
pause
cd /d %WORKSPACE_ROOT%\Run\Release
start MatisseServer.exe