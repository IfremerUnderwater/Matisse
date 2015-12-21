rem Ce script doit être lancé dans une console ouverte en tant qu'Administrateur

call env.bat

rem Lien symbolique pour compatibilité Windows / Linux
rem RasterGeoreferencer.h pointe sur les headers gdal avec le path include\gdal
mklink /D %OSGEO4W_ROOT%\include\gdal %OSGEO4W_ROOT%\include\