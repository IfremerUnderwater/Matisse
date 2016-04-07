@echo off 

rem -- Configure path section --
set OSGEO4W_ROOT=C:\OSGeo4W64
rem ----------------------------

call "%OSGEO4W_ROOT%\bin\o4w_env.bat"
set PATH=%OSGEO4W_ROOT%\apps\qgis\bin;.\dll;%PATH%


start MatisseServer.exe

