#----------------------------------------------------------------------
/*
	Build from the command prompt:
		makensis.exe /DgVerInstaller=1.20.37 Timekeeper.nsi

	Documentation:

	http://nsis.sourceforge.net/Sample_installation_script_for_an_application
	http://nsis.sourceforge.net/Docs/Modern%20UI%202/Readme.html

	http://nsis.sourceforge.net/Examples/one-section.nsi
	http://nsis.sourceforge.net/Docs/Chapter4.html#4.9.1
	http://nsis.sourceforge.net/Uninstall_only_installed_files

	http://nsis.sourceforge.net/Add_uninstall_information_to_Add/Remove_Programs
	http://nsis.sourceforge.net/Add/Remove_Functionality
*/
#----------------------------------------------------------------------

!include MUI2.nsh

!include x64.nsh
!include Library.nsh				# InstallLib http://nsis.sourceforge.net/Docs/AppendixB.html

!define gProductName				"Timekeeper"
!define gProductTarget			"Timekeeper.dll"

!define gCompanyName				"12noon"
!define gCompanyURL				"http://12noon.com"

!define gRegistryKeySuite		"Software\${gCompanyName}"
!define gRegistryKeyApp			"${gRegistryKeySuite}\${gProductName}"
!define gUninstallKey			"Software\Microsoft\Windows\CurrentVersion\Uninstall\${gCompanyName} ${gProductName}"
!define gUninstallExe			"Uninstall.exe"

!define /date gCopyrightYear	"2006-%Y"
!ifndef gVerInstaller
	!define gVerInstaller		"0.0.0.0"			# Pass on the command line
!endif

Unicode true

#	64-bit support
#Var b64bit
Var MyProgramFiles
# http://nsis.sourceforge.net/Docs/Chapter4.html#4.2.3
Function .onInit
	# http://forums.winamp.com/showthread.php?s=&threadid=237297
	# http://forums.winamp.com/showthread.php?s=&threadid=284081
	# http://forums.winamp.com/showthread.php?s=&threadid=292670
	${If} ${RunningX64}
#		StrCpy $b64bit 1
		StrCpy $MyProgramFiles $PROGRAMFILES64
		SetRegView 64
	${Else}
#		StrCpy $b64bit 0
		StrCpy $MyProgramFiles $PROGRAMFILES
	${EndIf}
	StrCpy $INSTDIR "$MyProgramFiles\${gCompanyName} ${gProductName}"
FunctionEnd

Name "${gProductName}"
OutFile timekeeper-setup.exe

# Adding a trailing backslash prevents directory from being appended to a directory the user selects.
InstallDir "$MyProgramFiles\${gCompanyName} ${gProductName}"
InstallDirRegKey HKLM "${gUninstallKey}" "UninstallString"

# http://nsis.sourceforge.net/UAC_plug-in
RequestExecutionLevel admin

BrandingText "${gCompanyName}"
Caption "${gCompanyName} ${gProductName}"

ShowUninstDetails show
XPStyle on


#----------------------------------------------------------------------
#
# Modern UI 2 settings
#
!define MUI_ABORTWARNING

!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\orange-install.ico"			# "${NSISDIR}\Contrib\Graphics\Icons\pixel-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\orange-uninstall.ico"	# "${NSISDIR}\Contrib\Graphics\Icons\pixel-uninstall.ico"

!define MUI_WELCOMEFINISHPAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Wizard\orange.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Wizard\orange-uninstall.bmp"

!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Header\orange.bmp"
!define MUI_HEADERIMAGE_UNBITMAP "${NSISDIR}\Contrib\Graphics\Header\orange-uninstall.bmp"


#----------------------------------------------------------------------
#
# Installer pages
#
!insertmacro MUI_PAGE_WELCOME
###!insertmacro MUI_PAGE_LICENSE "${NSISDIR}\Docs\Modern UI\License.txt"
#No Components page because they're all required
#!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
#!insertmacro MUI_PAGE_STARTMENU pageStartMenu strStartMenuFolder
!insertmacro MUI_PAGE_INSTFILES
#	!define MUI_FINISHPAGE_RUN_TEXT "Start ${gProductName}"
#	!define MUI_FINISHPAGE_RUN "$INSTDIR\timekeeper.exe"
#	!define MUI_FINISHPAGE_RUN_NOTCHECKED
###Comment out Finish page so we can see the Show Details:
###!insertmacro MUI_PAGE_FINISH

#----------------------------------------------------------------------
#
# Uninstaller pages
#
!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
#(So we can see the Show Details) !insertmacro MUI_UNPAGE_FINISH


#----------------------------------------------------------------------
#
# Language support
#
!insertmacro MUI_LANGUAGE English


#----------------------------------------------------------------------
#
# Setup file's version information
#
VIProductVersion "${gVerInstaller}"
VIAddVersionKey ProductVersion	"${gVerInstaller}"
VIAddVersionKey FileVersion		"${gVerInstaller}"
VIAddVersionKey FileDescription	"${gProductName}"
VIAddVersionKey ProductName		"${gProductName}"
VIAddVersionKey CompanyName		"${gCompanyName}"
VIAddVersionKey LegalCopyright	"© ${gCopyrightYear} ${gCompanyName}"
VIAddVersionKey LegalTrademarks	"${gCompanyName} ${gSuiteName}"


#----------------------------------------------------------------------
#
#	Installer Section
#
Section "${gProductName}" sectionMain
	SectionIn 1 RO		# Make this section read-only (user cannot clear it)

	#
	#	Create all files in install directory
	#
	SetOutPath "$INSTDIR"

	#
	#	Documents for all products
	#
	File /nonfatal "files\LICENSE"

	#
	# Application files
	#
	File /r /x "LICENSE" /x "Desktop.ini" /x "thumbs.db" /x *.dll "files\*.*"
	${If} ${RunningX64}
		File "/oname=Timekeeper.dll" "files\Timekeeper64.dll"
	${Else}
		File "files\Timekeeper.dll"
	${EndIf}


	#
	# Register the DLL
	#
	${If} ${RunningX64}
#		ExecWait 'regsvr32.exe /s "$INSTDIR\${gProductTarget}"'
		!define LIBRARY_X64
		!InsertMacro InstallLib REGDLL NOTSHARED REBOOT_NOTPROTECTED "files\Timekeeper64.dll" "$INSTDIR\${gProductTarget}" "$INSTDIR"
		!undef LIBRARY_X64
	${Else}
#		RegDLL "$INSTDIR\${gProductTarget}"
		!InsertMacro InstallLib REGDLL NOTSHARED REBOOT_NOTPROTECTED "files\Timekeeper.dll" "$INSTDIR\${gProductTarget}" "$INSTDIR"
	${EndIf}


	# Create any Registry keys we need
#?	WriteRegStr HKCU "${gRegistryKeyApp}" "Example Key" "Sample Data"


	# Create shortcut for All Users
#	SetShellVarContext all
#	CreateShortCut "$SMPROGRAMS\${gProductName}.lnk" "$INSTDIR\${gProductTarget}" "" "$INSTDIR\icon-file.dll" 0
 
 
	# Save Uninstall info for Add/Remove Programs
	# http://nsis.sourceforge.net/Add_uninstall_information_to_Add/Remove_Programs
	WriteRegStr		HKLM "${gUninstallKey}" "DisplayName"		"${gCompanyName} ${gProductName}"
	WriteRegStr		HKLM "${gUninstallKey}" "DisplayIcon"		"$INSTDIR\${gProductTarget},0"
	WriteRegStr		HKLM "${gUninstallKey}" "DisplayVersion"	"${gVerInstaller}"
	WriteRegDWORD	HKLM "${gUninstallKey}" "NoModify"			1
	WriteRegDWORD	HKLM "${gUninstallKey}" "NoRepair"			1
	WriteRegStr		HKLM "${gUninstallKey}" "UninstallString"	"$INSTDIR\${gUninstallExe}"

	# Write default values for custom keys
	WriteRegStr		HKLM "${gUninstallKey}" "Contact"			"support-request@12noon.com"
	WriteRegStr		HKLM "${gUninstallKey}" "HelpLink"			"${gCompanyURL}"
	WriteRegStr		HKLM "${gUninstallKey}" "Publisher"			"${gCompanyName}"
	WriteRegStr		HKLM "${gUninstallKey}" "URLInfoAbout"		"${gCompanyURL}"
	WriteRegStr		HKLM "${gUninstallKey}" "URLUpdateInfo"	"${gCompanyURL}"
	#ProductID
	#Readme
	
	# Create uninstaller
	WriteUninstaller "$INSTDIR\${gUninstallExe}"

SectionEnd


#----------------------------------------------------------------------
#
#	Descriptions
#
#	NOTE: If there is no Components page, we don't need to do this.
#
#LangString DESC_sectionMain ${LANG_ENGLISH} "Installs required application files, shortcuts, and uninstall information."


# Assign language strings to sections
#!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
#	!insertmacro MUI_DESCRIPTION_TEXT ${sectionMain} $(DESC_sectionMain)
#!insertmacro MUI_FUNCTION_DESCRIPTION_END


#----------------------------------------------------------------------
#
#	Uninstaller Section
#
Section "Uninstall"
	#
	# Unregister the DLL
	#
	${If} ${RunningX64}
#		ExecWait 'regsvr32.exe /s /u "${gProductTarget}"'
		!define LIBRARY_X64
		!InsertMacro UnInstallLib REGDLL NOTSHARED REBOOT_NOTPROTECTED "$INSTDIR\${gProductTarget}"
		!undef LIBRARY_X64
	${Else}
#		UnRegDLL "${gProductTarget}"
		!InsertMacro UnInstallLib REGDLL NOTSHARED REBOOT_NOTPROTECTED "$INSTDIR\${gProductTarget}"
	${EndIf}


	#
	#	Delete what we installed
	#
	Delete /rebootok "$INSTDIR\LICENSE"
	#
	Delete /rebootok "$INSTDIR\Timekeeper.htm"
	Delete /rebootok "$INSTDIR\*.png"
	Delete /rebootok "$INSTDIR\12noon-89x97.gif"
	Delete /rebootok "$INSTDIR\Timekeeper.dll"
	#
	Delete /rebootok "$INSTDIR\${gUninstallExe}"

	# Delete install directory if it's empty
	RMDir /rebootok "$INSTDIR"


	# Delete shortcut
#	SetShellVarContext all
#	Delete "$SMPROGRAMS\${gProductName}.lnk"


	# Delete Registry entries
	DeleteRegKey HKCU "${gRegistryKeyApp}"
	DeleteRegKey /ifempty HKCU "${gRegistryKeySuite}"


	# Delete Registry Uninstall info from Add/Remove Programs
	DeleteRegKey HKLM "${gUninstallKey}"

SectionEnd
