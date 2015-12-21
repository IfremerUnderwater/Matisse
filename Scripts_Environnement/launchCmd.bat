call env.bat

set JAVA_HOME_SAVE=%JAVA_HOME%
@echo "Using OSGEO4W_ROOT: "%OSGEO4W_ROOT%
call "%OSGEO4W_ROOT%\bin\o4w_env.bat"
call "%OSGEO4W_ROOT%\apps\grass\grass-6.4.3\etc\env.bat"

rem MSVC 2010 64bits
call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" amd64

@set JAVA_HOME=%JAVA_HOME_SAVE%
@set PATH=%OSGEO4W_ROOT%\bin;%PATH%   
@set PATH=%JAVA_HOME%\jre\bin;%PATH%
@set PATH=%M2_HOME%\bin;%PATH%
@set PATH=%OSGEO4W_ROOT%\apps\qgis\bin;%OPENCV_DIR%\bin;%PATH%
@set PATH=%MATLAB_RUNTIME_ROOT%\bin\win64;%PATH%
@set PATH=%MATLAB_RUNTIME_ROOT%\runtime\win64;%PATH%
@set PATH=%SVN_ROOT%;%PATH%
@set PATH=%QT_CREATOR_ROOT%\bin;%PATH%
@set PATH=%INNOSETUP_ROOT%\bin;%PATH%

@set MATLAB_LIB_DIR=%MATLAB_RUNTIME_ROOT%\extern

cd D:\Chrisar\Services\IFREMER\AF-130150_MATISSE_3.0\30-Systeme\02-Logiciel\trunk

cmd
                                                                                              