@echo off
rem -- Configure path section --
@set OSGEO4W_ROOT=C:\OSGeo4W64
@set MATISSE_ROOT=C:\Matisse\main\qt
@set MATLAB_RUNTIME_ROOT=C:\Program Files\MATLAB\MATLAB Compiler Runtime\v81
rem @set DLL_DIR=C:\Matisse\extern_dll
rem ----------------------------
@set GRASS_PREFIX=%OSGEO4W_ROOT%/apps/grass/grass-6.4.3
call "%OSGEO4W_ROOT%\bin\o4w_env.bat"
@set INCLUDE=%INCLUDE%;%OSGEO4W_ROOT%\include
@set LIB=%LIB%;%OSGEO4W_ROOT%\lib;%OSGEO4W_ROOT%\lib
rem @set PATH=%OSGEO4W_ROOT%\apps\qgis\bin;%PATH%
@set PATH=%OSGEO4W_ROOT%\apps\qgis\bin;%OSGEO4W_ROOT%\bin;%MATLAB_RUNTIME_ROOT%\runtime\win64;%MATLAB_RUNTIME_ROOT%\bin\win64;
rem @set PATH=%DLL_DIR%;%PATH%
rem cmd 
start MatisseServer.exe

