rem @echo off
cd %~dp0
call env.bat

call "%OSGEO4W_ROOT%\bin\o4w_env.bat"

@set INCLUDE=%INCLUDE%;%OSGEO4W_ROOT%\include
@set LIB=%LIB%;%OSGEO4W_ROOT%\lib

rem @set WORKSPACE_ROOT=%CD%\..

REM #correction bug ?
set OSG_DIR=F:\ThirdPartyLibs\OpenSceneGraph-3.4.1\build-vs2017

set PATH=%OSGEO4W_ROOT%\app\qgis\bin;%PATH%
set PATH=%PATH%;%OSGEO4W_ROOT%\apps\gdal-dev\bin
set PATH=%OSGEO4W_ROOT%\apps\Qt5\bin;%PATH%

set PATH=%PATH%:%QUAZIP_DIR%\bin;%OPENCV_DIR%\bin;%OSG_DIR%\bin;%PROTOBUF_DIR%\bin;%MATISSE_DEV_WORKSPACE%\Run\Release\Libraries
echo %PATH%

echo %OSG_DIR%

set QT_PLUGIN_PATH=%OSGEO4W_ROOT%\app\qgis\bin\plugins
rem set QT_PLUGIN_PATH=
echo QT_PLUGIN_PATH = %QT_PLUGIN_PATH%
echo %QT_CREATOR_ROOT%

pause
start %QT_CREATOR_ROOT%\bin\qtcreator.exe







