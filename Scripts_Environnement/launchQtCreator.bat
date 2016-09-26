rem @echo off
call env.bat

call "%OSGEO4W_ROOT%\bin\o4w_env.bat"

@set INCLUDE=%INCLUDE%;%OSGEO4W_ROOT%\include
rem Correction du PATH des libs ?
rem @set LIB=%LIB%;%OSGEO4W_ROOT%\lib;%OSGEO4W_ROOT%\lib
@set LIB=%LIB%;%OSGEO4W_ROOT%\lib

@set WORKSPACE_ROOT=%CD%\..

rem set PATH=%OSGEO4W_ROOT%\apps\qgis\bin;%OPENCV_DIR%\bin;%OSG_DIR%\bin;%PROTOBUF_DIR%\bin;%OSG_3RD_PARTIES%\x64\bin;%PATH%
set PATH=%QUAZIP_DIR%\bin;%OSGEO4W_ROOT%\apps\qgis\bin;%OPENCV_DIR%\bin;%OSG_DIR%\bin;%PROTOBUF_DIR%\bin;%MATISSE_DEV_WORKSPACE%\Run\Debug\Libraries;%MATISSE_DEV_WORKSPACE%\Run\Release\Libraries;%PATH%
echo %PATH%


start %QT_CREATOR_ROOT%\bin\qtcreator.exe







