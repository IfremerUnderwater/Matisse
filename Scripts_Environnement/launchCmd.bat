call env.bat

rem set JAVA_HOME_SAVE=%JAVA_HOME%
@echo "Using OSGEO4W_ROOT: "%OSGEO4W_ROOT%
call "%OSGEO4W_ROOT%\bin\o4w_env.bat"
rem call "%OSGEO4W_ROOT%\apps\grass\grass-6.4.3\etc\env.bat"

rem MSVC 2010 64bits
rem call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" amd64

rem @set JAVA_HOME=%JAVA_HOME_SAVE%
rem @set PATH=%OSGEO4W_ROOT%\bin;%PATH%   
rem @set PATH=%JAVA_HOME%\jre\bin;%PATH%
rem @set PATH=%M2_HOME%\bin;%PATH%
rem @set PATH=%OSGEO4W_ROOT%\apps\qgis\bin;%OPENCV_DIR%\bin;%PATH%
rem @set PATH=%MATLAB_RUNTIME_ROOT%\bin\win64;%PATH%
rem @set PATH=%MATLAB_RUNTIME_ROOT%\runtime\win64;%PATH%
rem @set PATH=%SVN_ROOT%;%PATH%
rem @set PATH=%QT_CREATOR_ROOT%\bin;%PATH%
rem @set PATH=%INNOSETUP_ROOT%\bin;%PATH%

rem @set MATLAB_LIB_DIR=%MATLAB_RUNTIME_ROOT%\extern

rem cd D:\Chrisar\Services\IFREMER\AF-130150_MATISSE_3.0\30-Systeme\02-Logiciel\trunk

set PATH=%QUAZIP_DIR%\bin;%OSGEO4W_ROOT%\apps\qgis\bin;%OPENCV_DIR%\bin;%OSG_DIR%\bin;%PROTOBUF_DIR%\bin;%MATISSE_DEV_WORKSPACE%\Run\Debug\Libraries;%MATISSE_DEV_WORKSPACE%\Run\Release\Libraries;%PATH%
set PATH=C:\Apps\Depends;%PATH%

cmd
                                                                                              
