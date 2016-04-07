;Fichier d'installation de Matisse
;---------------------
;Include Modern UI
	!include "MUI.nsh"

;---------------------
;Include text functions
	!include "TextFunc.nsh"
	!include "WordFunc.nsh"
	!insertmacro WordFind
	!insertmacro TrimNewLines

;--------------------------------
;General
	SetCompressor /FINAL /SOLID lzma
	SetCompressorDictSize 16

	!define APP "Matisse3"

    !define VERSION "3.2.4"

	!define INSTALL_DIR ".\"
	
	
	;Name and file
	Name "${APP}.${VERSION}"
    OutFile "${INSTALL_DIR}/Setup${APP}.${VERSION}.exe"

	;Default installation folder
	InstallDir "C:\${APP}\${APP}.${VERSION}"

;--------------------------------
;Interface Settings

	!define MUI_ABORTWARNING
	;!define MUI_ICON ".\resources\MatisseLogo.ico"
	;!define MUI_UNICON ".\resources\MatisseLogo.ico"
	
	;!define MUI_HEADERIMAGE
	;!define MUI_HEADERIMAGE_BITMAP "..\resources\Default\GUI\Bitmaps\MatisseInstallBar.bmp"
	;!define MUI_HEADERIMAGE_UNBITMAP "..\resources\Default\GUI\Bitmaps\MatisseInstallBar.bmp"

;--------------------------------
;Pages
	!insertmacro MUI_PAGE_COMPONENTS
	!insertmacro MUI_PAGE_DIRECTORY
	!insertmacro MUI_PAGE_INSTFILES
	!insertmacro MUI_PAGE_FINISH
	
	!insertmacro MUI_UNPAGE_CONFIRM
	!insertmacro MUI_UNPAGE_INSTFILES
	
;--------------------------------
;Languages
	!insertmacro MUI_LANGUAGE "English"
	!insertmacro MUI_LANGUAGE "French"
	
;--------------------------------
;Installer Sections

Section "${APP}" Sec${APP}
	;--- Section obligatoire
	SectionIn RO

	;--------------------------------
	
	SetOutPath "$INSTDIR\Modules"
	File /a /r ..\Run\Release\Modules\*.*
	
	SetOutPath "$INSTDIR\Libraries"
	File /a /r ..\Run\Release\Libraries\*.*

	SetOutPath "$INSTDIR\i18n"
	File /a /r ..\Run\Release\i18n\*.*

	SetOutPath "$INSTDIR\dll"
	File /a /r .\dll\*.*
	
	SetOutPath "$INSTDIR"
	File /a /r ..\Config\*.*
	
	SetOutPath "$INSTDIR"
	File /a ..\Run\Release\MatisseServer.exe
	
	SetOutPath "$INSTDIR"
	File /a /r .\Matisse.bat
	
	
	;--------------------------------	
	;Store installation folder
	WriteRegStr HKCU "Software\${APP}.${VERSION}" "" $INSTDIR
	
	;Create uninstaller
	WriteUninstaller "$INSTDIR\Uninstall.exe"
	
	;--------------------------------
	;Create shortcuts
	;SetOutPath "$INSTDIR\Matisse2Kernel\bin\Release"
	CreateDirectory "$SMPROGRAMS\${APP}"
	CreateShortCut "$SMPROGRAMS\${APP}\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
	CreateShortCut "$SMPROGRAMS\${APP}\Matisse.lnk" "$INSTDIR\Matisse.bat"
	
	CreateShortCut "$DESKTOP\Matisse.lnk" "$INSTDIR\Matisse.bat"

	

SectionEnd

Function .onInit

	;Extract InstallOptions INI files
	!insertmacro MUI_LANGDLL_DISPLAY

FunctionEnd

;Uninstaller Section
Section "Uninstall"
	;--------------------------------
	;ADD YOUR OWN FILES HERE...
	Delete "$DESKTOP\Matisse.lnk"
	;RMDir /r "$SMPROGRAMS\${APP}" 
	;--------------------------------

	Delete "$INSTDIR\Uninstall.exe"

	RMDir /r "$INSTDIR"

	DeleteRegKey /ifempty HKCU "Software\${APP}.${VERSION}"
SectionEnd

Function un.onInit
	!insertmacro MUI_UNGETLANGUAGE
FunctionEnd

