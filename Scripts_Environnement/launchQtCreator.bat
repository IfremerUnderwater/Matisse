rem @echo off

call env.bat
call "%OSGEO4W_ROOT%\bin\o4w_env.bat"

@set INCLUDE=%INCLUDE%;%OSGEO4W_ROOT%\include
rem Correction du PATH des libs ?
rem @set LIB=%LIB%;%OSGEO4W_ROOT%\lib;%OSGEO4W_ROOT%\lib
@set LIB=%LIB%;%OSGEO4W_ROOT%\lib

@set WORKSPACE_ROOT=%CD%\..

set PATH=%OSGEO4W_ROOT%\apps\qgis\bin;%OPENCV_DIR%\bin;%OSG_DIR%\bin;%PROTOBUF_DIR%\bin;%MATISSE_DEV_WORKSPACE%\Run\Debug\Libraries;%MATISSE_DEV_WORKSPACE%\Run\Release\Libraries;%PATH%


start %QT_CREATOR_ROOT%\bin\qtcreator.exe