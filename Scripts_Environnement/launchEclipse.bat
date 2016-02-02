call env.bat

set JAVA_HOME_SAVE=%JAVA_HOME%
@echo "Using OSGEO4W_ROOT: "%OSGEO4W_ROOT%
call "%OSGEO4W_ROOT%\bin\o4w_env.bat"
call "%OSGEO4W_ROOT%\apps\grass\grass-6.4.3\etc\env.bat"
   
set JAVA_HOME=%JAVA_HOME_SAVE%
path %JAVA_HOME%\jre\bin;%PATH%
 rem cmd
start %ECLIPSE_ROOT%\eclipse.exe                                                                                                 