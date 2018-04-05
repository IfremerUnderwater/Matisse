@echo off 
rem set current directory
cd %~dp0

rem -- Configure path section --
set OSGEO4W_ROOT=F:\OSGeo4W64
rem ----------------------------

call "%OSGEO4W_ROOT%\bin\o4w_env.bat"
set PATH=%OSGEO4W_ROOT%\apps\qgis\bin;%PATH%
rem set PATH=C:\Program Files\qgis2.99.0\bin;%PATH%
set PATH=%PATH%;%~dp0\Libraries
set PATH=%PATH%;F:\ThirdPartyLibs\opencv331\opencv\build\x64\vc14\bin
set PATH=%PATH%;%OSGEO4W_ROOT%\apps\gdal-dev\bin
set PATH=%OSGEO4W_ROOT%\apps\Qt5\bin;%PATH%
set PATH=%PATH%;F:\ThirdPartyLibs\OpenSceneGraph-3.4.1\build-vs2017\bin
path

rem set QT_PLUGIN_PATH=C:\Program Files\qgis2.99.0\bin\plugins
rem set QT_PLUGIN_PATH=
set QT_PLUGIN_PATH=%OSGEO4W_ROOT%\apps\qgis\bin\plugins
pause

start MatisseServer.exe

