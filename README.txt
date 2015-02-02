
A) Fichier d'environnement
rem -- env.bat file --
rem -- Configure path section --
@set OSGEO4W_ROOT=C:\OSGeo4W64
@set JAVA_HOME=D:\Applis\jdk7u45x64
@set M2_HOME=D:\Applis\apache-maven-3.1.1
@set QT_CREATOR_ROOT=D:\Applis\Qt\Tools\QtCreator-3.1.1
@set ECLIPSE_ROOT=D:\Applis\eclipse-kepler-matisse-x64
@set OPENCV_DIR=D:\Applis\OpenCV\build\x64\vc10
@set MATLAB_RUNTIME_ROOT=C:\Program Files\MATLAB\MATLAB Compiler Runtime\v81
@set SVN_ROOT=C:\Program Files (x86)\CollabNet\Subversion Client
@set INNOSETUP_ROOT=C:\Program Files (x86)\Inno Setup 5
@set MSVC_ROOT=C:\Program Files (x86)\Microsoft Visual Studio 10.0
@set WINDOWS_SDK_ROOT=C:/Program Files/Microsoft SDKs/Windows/v7.1
rem ----------------------------

B) Fichier de configuration de CMD
rem -- launchCmd.bat --
call env.bat

set JAVA_HOME_SAVE=%JAVA_HOME%
@echo "Using OSGEO4W_ROOT: "%OSGEO4W_ROOT%
call "%OSGEO4W_ROOT%\bin\o4w_env.bat"
call "%OSGEO4W_ROOT%\apps\grass\grass-6.4.3\etc\env.bat"

rem MSVC 2010 64bits
call "%MSVC_ROOT%\VC\vcvarsall.bat" amd64

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

cmd
rem ----------------------------
               

C) Nettoyage
mvn clean

D) Compilation
mvn package

E) Déploiement sur Archiva
mvn deploy



