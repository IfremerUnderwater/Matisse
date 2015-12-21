rem @echo off
call env.bat

call "%OSGEO4W_ROOT%\bin\o4w_env.bat"

@set INCLUDE=%INCLUDE%;%OSGEO4W_ROOT%\include
@set LIB=%LIB%;%OSGEO4W_ROOT%\lib;%OSGEO4W_ROOT%\lib
@set MATLAB_LIB_DIR=%MATLAB_RUNTIME_ROOT%\extern

@set WORKSPACE_ROOT=%CD%\..

set PATH=%OSGEO4W_ROOT%\apps\qgis\bin;%OPENCV_DIR%\bin;%PATH%
set PATH=%MATLAB_RUNTIME_ROOT%\bin\win64;%PATH%
set PATH=%MATLAB_RUNTIME_ROOT%\runtime\win64;%PATH%
set PATH=%SVN_ROOT%;%PATH%

rem Ajout des paths pour les DLL des modules
set MATISSE_LIB_ROOT=%WORKSPACE_ROOT%\Build\Debug\Libraries
set MATISSE_LIB_PATH=%MATISSE_LIB_ROOT%\MatisseTools;%MATISSE_LIB_ROOT%\MatisseCommon;%MATISSE_LIB_ROOT%\OpticalMapping
set PATH=%MATISSE_LIB_PATH%;%PATH%

echo before
echo %WORKSPACE_ROOT%\Run\Debug\MatisseServer.exe
echo after