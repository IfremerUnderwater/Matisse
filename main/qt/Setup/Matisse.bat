@echo off 

rem -- Configure path section --
set OSGEO4W_ROOT=C:\OSGeo4W64
set OPENCV_DIR=D:\Applis\OpenCV\build\x64\vc10
rem ----------------------------

call "%OSGEO4W_ROOT%\bin\o4w_env.bat"
set PATH=%OSGEO4W_ROOT%\apps\qgis\bin;%OPENCV_DIR%\bin;%PATH%


start MatisseServer.exe

