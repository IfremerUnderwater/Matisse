@ECHO OFF

rem This script generates the matisse installer. It has to be executed in its directory

rem remove any residual data
rmdir /S /Q .\packages\Matisse3D\data

rem copy new data
xcopy /s /i ..\Run\Release .\packages\Matisse3D\data

rem generate installer
binarycreator.exe --offline-only -c config\config.xml -p packages matisse-setup.exe

rem remove data
rmdir /S /Q .\packages\Matisse3D\data

PAUSE