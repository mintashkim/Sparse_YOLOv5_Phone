!ifdef WRITE_UNINST
OutFile "./_bin/_writer-uninst.exe"
RequestExecutionLevel user
SetCompress off
!else
RequestExecutionLevel admin
#SetCompressor /SOLID lzma
!endif

!include "MUI.nsh"
!include nsDialogs.nsh
!include LogicLib.nsh
!include 'FileFunc.nsh'
!insertmacro Locate
!include "x64.nsh"
!include "_bin\nsis\inc\CommonFunctions.nsi"
!include "_bin\nsis\inc\DirManage.nsi"
!include "_bin\nsis\inc\GetCommonAppData.nsi"
!include "_bin\nsis\inc\UAC.nsh"

!define SETUP_MIRAGEWORKS_PRODUCT_NAME "BossGo"

!define PRODUCT_UNINST_KEY		"Software\Microsoft\Windows\CurrentVersion\Uninstall\BossGo"
!define PRODUCT_APP_DATA		"Ados\BossGo"
!define PRODUCT_APP_KEY			"Software\Ados\BossGo"
!define PRODUCT_RUN_KEY			"Software\Microsoft\Windows\CurrentVersion\Run"
!define PRODUCT_RUNONCE_KEY		"Software\Microsoft\Windows\CurrentVersion\RunOnce"

!define PRODUCT_ENGINE_KEY		"Software\NoAD Inc."
!define PRODUCT_MIRAGEWORKS_KEY	"Software\Ados"

!define MUI_ABORTWARNING
!define MUI_UNABORTWARNING
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_RIGHT
!define MUI_HEADERIMAGE_BITMAP			"res\header.bmp"
!define MUI_WELCOMEFINISHPAGE_BITMAP	"res\welcomefinish.bmp"
!define MUI_ICON						"res\inst.ico"
!define MUI_UNICON						"res\inst.ico"

!define MUI_CUSTOMFUNCTION_ABORT OnAbort

!define INSTOPT_SKIP_VIRTUAL_DISK_PAGE

!define MUI_PAGE_CUSTOMFUNCTION_PRE PreWelcomePage
!insertmacro MUI_PAGE_WELCOME
!define MUI_PAGE_CUSTOMFUNCTION_PRE PreLicensePage
!insertmacro MUI_PAGE_LICENSE $(license)
!define MUI_PAGE_CUSTOMFUNCTION_PRE PreComponentPage
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_INSTFILES
!define MUI_PAGE_CUSTOMFUNCTION_PRE PreFinishPage
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "Korean"
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

!include "product-setup-lang.nsh"

LicenseLangString license ${LANG_ENGLISH}		res\License-english.txt
LicenseLangString license ${LANG_KOREAN}		res\License.txt
LicenseData $(license)

!insertmacro GetVersionLocal "_bin\bin_x64\abgConsole.exe" MyVer_
VIProductVersion "${MyVer_1}.${MyVer_2}.${MyVer_3}.${MyVer_4}"
!define PRODUCT_VERSION "${MyVer_1}.${MyVer_2}.${MyVer_3}.${MyVer_4}"

Name "$(PRODUCT_NAME)"
ShowInstDetails nevershow
ShowUninstDetails nevershow
XPStyle on
BrandingText /TRIMRIGHT "$(PRODUCT_NAME) ${PRODUCT_VERSION}"
InstallDir "$PROGRAMFILES\BossGo"

VIAddVersionKey ProductName "BossGo"
VIAddVersionKey CompanyName "Ados"
VIAddVersionKey LegalCopyright "Copyright 2022 Ados Co., Ltd. All rights reserved."
VIAddVersionKey FileDescription "BossGo Installer"
VIAddVersionKey FileVersion "${MyVer_1}.${MyVer_2}.${MyVer_3}.${MyVer_4}"
VIAddVersionKey ProductVersion "${MyVer_1}.${MyVer_2}.${MyVer_3}.${MyVer_4}"
VIAddVersionKey InternalName "bossgo-setup.exe"
VIAddVersionKey OriginalFilename "bossgo-setup.exe"

Var IsInstalledProduct
Var Variable_String_ServerUrl

Function .onInit

	# Uninstaller 작성
	!ifdef WRITE_UNINST
	SetSilent silent
	WriteUninstaller "$EXEDIR\uninst.exe"
	Quit
	!endif

	!addplugindir "_bin\nsis"
	
	!insertmacro UAC_RunElevated
	${Switch} $0
	${Case} 0
		${IfThen} $1 = 1 ${|} Abort ${|}
		${IfThen} $3 <> 0 ${|} ${Break} ${|}
		${If} $1 = 3
			MessageBox MB_OK|MB_ICONEXCLAMATION|MB_TOPMOST|MB_SETFOREGROUND "This installer requires admin privileges, aborting!"
			Abort
		${EndIf}
	${Case} 1223
		MessageBox MB_OK|MB_ICONEXCLAMATION|MB_TOPMOST|MB_SETFOREGROUND "This installer requires admin privileges, aborting!"
		Abort
	${Case} 1062
		MessageBox MB_OK|MB_ICONEXCLAMATION|MB_TOPMOST|MB_SETFOREGROUND "This installer requires admin privileges, aborting!"
		Abort
	${Default}
		MessageBox MB_OK|MB_ICONEXCLAMATION|MB_TOPMOST|MB_SETFOREGROUND "This installer requires admin privileges, aborting!"
		Abort
	${EndSwitch}

	# 중복실행을 확인합니다.
	
	System::Call 'kernel32::CreateMutexW(i 0, i 0, t "abgInstallerMutex") ?e'
	Pop $R0
	StrCmp $R0 0 +3 0
	MessageBox MB_OK|MB_ICONEXCLAMATION|MB_TOPMOST|MB_SETFOREGROUND "The installer is already running."
	Abort
	
	System::Call 'kernel32::CreateMutexW(i 0, i 0, t "Global\abgInstallerMutex2") ?e'
	Pop $R0
	StrCmp $R0 0 +3 0
	MessageBox MB_OK|MB_ICONEXCLAMATION|MB_TOPMOST|MB_SETFOREGROUND "The installer is already running."
	Abort
	
	# 설치, 삭제, 업데이트 후 재부팅 여부를 확인하여,
	# 재부팅이 이뤄지지 않으면 설치가 진행되지 않도록합니다.
	
	!addplugindir "_bin\nsis"
	abgnsis::CheckRebootAfterUninstall
	Pop $R0
	StrCmp $R0 "1" 0 +3
	MessageBox MB_ICONSTOP|MB_OK "$(MSGBOX_ALERT_REBOOT_REQUIRED_FOR_INST)"
	Abort
	
	# 기존 버전이 설치되어 있는지 확인합니다.
	
	abgnsis::IsInstalledProduct
	Pop $IsInstalledProduct
	
	# 언어 선택창
	
!ifndef INSTOPT_SKIP_LANGUAGE_SELECT_DIALOG
	${If} $IsInstalledProduct == "1"
		!insertmacro MUI_LANGDLL_DISPLAY
	${EndIf}
!endif


	# 64비트 환경에서의 Redirect 비활성화
	
	${If} ${RunningX64}
		StrCpy "$INSTDIR" "$PROGRAMFILES64\BossGo"
		SetRegView 64
	${EndIf}

	
	# 지원하는 Windows Version을 확인합니다.
	# 지원하지 않는 OS일 경우, 설치를 중단합니다.
	
	abgnsis::IsSupportedWindows
	Pop $0
	${If} $0 != "1"
		${If} $IsInstalledProduct == "1"
			WriteRegDWORD HKCU "${PRODUCT_APP_KEY}" "NoUpdate"	1
		${EndIf}
		
		MessageBox MB_ICONSTOP|MB_OK "$(MSGBOX_ALERT_NOT_SUPPORT_OS)"
		Abort
	${EndIf}
	
	!ifdef X64_ONLY
	${If} ${RunningX64}
	${Else}
		MessageBox MB_ICONSTOP|MB_OK "$(MSGBOX_ALERT_NOT_SUPPORT_OS)"
		Abort
	${EndIf}
	!endif
	
	StrCmp $IsInstalledProduct "1" 0 +2
	ReadRegStr $Variable_String_ServerUrl HKLM "${PRODUCT_APP_KEY}\Server" Server
		
	!addplugindir "tools"
	#abgnsis::AddPidToDiskFilter
		
done:

FunctionEnd

Function PreWelcomePage
	StrCmp $IsInstalledProduct "1" pass done
pass:
	Abort
done:
!ifdef INSTOPT_SKIP_WELCOME_PAGE
	Abort
!endif
	
FunctionEnd

Function PreLicensePage
	StrCmp $IsInstalledProduct "1" pass done
pass:
	Abort
done:
!ifdef INSTOPT_SKIP_LICENSE_PAGE
	Abort
!endif
FunctionEnd

Function PreComponentPage
	StrCmp $IsInstalledProduct "1" pass done
pass:
	Abort
done:
!ifdef INSTOPT_SKIP_COMPONENT_PAGE
	Abort
!endif
FunctionEnd

Function PreFinishPage
!ifdef INSTOPT_SKIP_FINISH_PAGE
	Abort
!endif
FunctionEnd

Function OnAbort

	# 임시 모듈 삭제
	SetOutPath "$TEMP"
	RMDir /r /REBOOTOK "$TEMP\abg"
	
FunctionEnd

; ===================================================================================================================================================
; INSTALL SECTION START
; ===================================================================================================================================================

Section "$(SECTIONNAME_PROGRAM)"
	SectionIn RO
	
	${If} ${RunningX64}
		${DisableX64FSRedirection}
		StrCpy "$INSTDIR" "$PROGRAMFILES64\BossGo"
	${EndIf}
	
	${If} $IsInstalledProduct == "1"
	
		# 업데이트인 경우, 모든 파일을 새롭게 복사하기 위해,
		# 기존 파일을 정리하는 루틴이 필요합니다. (기존 파일 Rename, 재부팅 후 Delete)
		
		!addplugindir "_bin\nsis"
		
		abgnsis::SetFlagForUninstall
		
		abgnsis::ProductUpdateInit
				
		abgnsis::ClearExistingFilesForUpdate $INSTDIR
		
		ReadRegStr $R0 HKLM "${PRODUCT_APP_KEY}" "PreviousDesktopShortcut"
		StrCmp $R0 "" +2 0
		Delete /REBOOTOK "$R0"
		
		ReadRegStr $R0 HKLM "${PRODUCT_APP_KEY}" "PreviousProgramsPath"
		StrCmp $R0 "" +2 0
		RMDir /r "$R0"
	
	${Else}

		# 서버 확인 임시 모듈 삭제
		SetOutPath "$TEMP"
		RMDir /r /REBOOTOK "$TEMP\abg"

	${EndIf}
	
	# 제품 Binaries 복사
	
	SetOverwrite try
	${If} ${RunningX64}

		SetOutPath "$INSTDIR"
		File /r /x .svn "_bin\bin_x64\*.*"
		File "_inst\info.xml"
		
	${Else}
		!ifndef X64_ONLY
		SetOutPath "$INSTDIR"
		#임시
		#File /r /x .svn "_bin\bin_x86\*.*"
		#File "_inst\info.xml"
		!endif
	${Endif}
	
	
	#
	# 파일복사가 끝난 뒤, 설치를 초기화 하고, 서비스 및 드라이버 설치합니다.
	#
	
	${If} $IsInstalledProduct == "1"

		ExecWait '"$INSTDIR\abgConsole.exe" update'
		
	${Else}

		ExecWait '"$INSTDIR\abgConsole.exe" install'
		
	${EndIf}
	
	WriteRegStr HKLM "${PRODUCT_APP_KEY}" "ProductName" "${SETUP_MIRAGEWORKS_PRODUCT_NAME}"
	
	#
	# 서비스의 복구옵션을 설정합니다. 파일 연결을 등록합니다.
	#
	
	abgnsis::ClearFlagForUninstall
	#abgnsis::SetupServiceRecoveryOption "aduWatcherSvc"
	
	Goto done
	
install_abort:
	SetOutPath "$INSTDIR\.."
	RMDir /r "$INSTDIR"
	Abort

done:
	
SectionEnd

Section -Post

	# 임시 모듈 삭제
	SetOutPath "$TEMP"
	RMDir /r /REBOOTOK "$TEMP\abg"

	# 바로가기 등의 시작위치는 모두 설치경로로 설정
	SetOutPath "$INSTDIR"
	
	# Uninstall Key 정보 입력
	WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayName" 	"$(PRODUCT_NAME)"
	WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
	WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayIcon" 	"$INSTDIR\abgConsole.exe"
	WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
	WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "URLInfoAbout" 	"http://www.adoscompany.com"
	WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "Publisher" 	"Ados"
	
	${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
	IntFmt $0 "0x%08X" $0
	WriteRegDWORD HKLM "${PRODUCT_UNINST_KEY}" "EstimatedSize" "$0"
	
	# 프로그램 바로가기 등록
	SetShellVarContext all
	WriteRegStr HKLM "${PRODUCT_APP_KEY}" "PreviousProgramsPath" "$SMPROGRAMS\$(SHORTCUTNAME_PROGRAMS_DIRECTORY)"
	CreateDirectory "$SMPROGRAMS\$(SHORTCUTNAME_PROGRAMS_DIRECTORY)"
	CreateShortCut "$SMPROGRAMS\$(SHORTCUTNAME_PROGRAMS_DIRECTORY)\$(SHORTCUTNAME_PROGRAMS_START).lnk" "$INSTDIR\abgConsole.exe"
	CreateShortCut "$SMPROGRAMS\$(SHORTCUTNAME_PROGRAMS_DIRECTORY)\$(SHORTCUTNAME_PROGRAMS_UNINSTALL).lnk" 		"$INSTDIR\uninst.exe"
	
	;redmine #13697
	Call GetWindowsVersion
	Pop $0
	StrCmp $0 "Win8_1" 0 +2
	CreateShortCut "$SMPROGRAMS\$(SHORTCUTNAME_PROGRAMS_DIRECTORY)\$(SHORTCUTNAME_PROGRAMS_UNINSTALL) .lnk" 		"$INSTDIR\uninst.exe"
	
	SetShellVarContext current
	
	Call RefreshShellIcons
	
done:
	
SectionEnd

Section "$(SECTIONNAME_SHORTCUT)"
	
	# 바탕화면 바로가기 등록
	
	SetOutPath "$INSTDIR"
	SetShellVarContext all
	CreateShortCut "$DESKTOP\$(SHORTCUTNAME_DESKTOP).lnk" "$INSTDIR\abgConsole.exe"
	WriteRegStr HKLM "${PRODUCT_APP_KEY}" "PreviousDesktopShortcut" "$DESKTOP\$(SHORTCUTNAME_DESKTOP).lnk"
	SetShellVarContext current
	
SectionEnd

Function .onInstSuccess

	#Exec '"$INSTDIR\aduWatcherSvc.exe" start'
	#Exec '"$INSTDIR\adlogsvc.exe" adu start'
	SetErrorLevel 0	
	
	#MessageBox MB_YESNO|MB_TOPMOST|MB_SETFOREGROUND "$(MSGBOX_ALERT_REBOOT_REQUIRED)" IDNO +4
	#abgnsis::SystemReboot
	#SetErrorLevel 0
	#Abort
	
FunctionEnd

Function LaunchLink
  ExecShell "" "$INSTDIR\abgConsole.exe"
FunctionEnd

; ===================================================================================================================================================
; INSTALL SECTION END
; ===================================================================================================================================================

Function un.onInit
	
	!addplugindir "_bin\nsis"
	!insertmacro UAC_RunElevated
	${Switch} $0
	${Case} 0
		${IfThen} $1 = 1 ${|} Abort ${|}
		${IfThen} $3 <> 0 ${|} ${Break} ${|}
		${If} $1 = 3
			MessageBox MB_OK|MB_ICONEXCLAMATION|MB_TOPMOST|MB_SETFOREGROUND "This installer requires admin privileges, aborting!"
			Abort
		${EndIf}
	${Case} 1223
		MessageBox MB_OK|MB_ICONEXCLAMATION|MB_TOPMOST|MB_SETFOREGROUND "This installer requires admin privileges, aborting!"
		Abort
	${Case} 1062
		MessageBox MB_OK|MB_ICONEXCLAMATION|MB_TOPMOST|MB_SETFOREGROUND "This installer requires admin privileges, aborting!"
		Abort
	${Default}
		MessageBox MB_OK|MB_ICONEXCLAMATION|MB_TOPMOST|MB_SETFOREGROUND "This installer requires admin privileges, aborting!"
		Abort
	${EndSwitch}
	
	# Uninstall을 시작합니다. 중복실행일 경우, 삭제를 중단합니다.
	
	System::Call 'kernel32::CreateMutexW(i 0, i 0, t "abgUnInstallerMutex") ?e'
	Pop $R0
	StrCmp $R0 0 +3
	MessageBox MB_OK|MB_ICONEXCLAMATION|MB_TOPMOST|MB_SETFOREGROUND "The Uninstaller is already running."	
	Abort
	
	System::Call 'kernel32::CreateMutexW(i 0, i 0, t "Global\abgUnInstallerMutex2") ?e'
	Pop $R0
	StrCmp $R0 0 +3
	MessageBox MB_OK|MB_ICONEXCLAMATION|MB_TOPMOST|MB_SETFOREGROUND "The Uninstaller is already running."	
	Abort
	
	# Uninstall을 시작합니다. 현재 구동중일 경우 종료합니다.
	
	#!addplugindir "_bin\nsis"
	#abgnsis::IsRunningProduct
	#Pop $0
	#StrCmp $0 "1" 0 +3
	#MessageBox MB_OK|MB_ICONEXCLAMATION|MB_TOPMOST|MB_SETFOREGROUND "$(MSGBOX_UNINST_ALERT_PRODUCT_RUNNING)"
	#Abort
	
	#abgnsis::IsAllowUninstall
	#Pop $0
	#StrCmp $0 "0" 0 +3
	#MessageBox MB_OK|MB_ICONEXCLAMATION|MB_TOPMOST|MB_SETFOREGROUND "$(MSGBOX_UNINST_ALERT_LOW_AUTHORITY)"
	#Abort
	
	MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "$(MSGBOX_UNINST_ASK_CONFIRM)" /SD IDYES IDYES +2
	Abort
		
	!addplugindir "tools"
	#abgnsis::AddPidToDiskFilter
	
FunctionEnd

Section Uninstall
	SetAutoClose false
	
	${If} ${RunningX64}
		StrCpy "$INSTDIR" "$PROGRAMFILES64\BossGo"
		SetRegView 64
	${EndIf}
	
	# 프로그램 추가 제거창 위로 끌어올리기 위해 항상 위로 옵션을 걸었다가 해제합니다.	
	StrCpy $3 $HWNDPARENT
	System::Call "user32::SetWindowPos(i r3, i -1, i 0, i 0, i 0, i 0, i 3)"
	System::Call "user32::SetWindowPos(i r3, i -2, i 0, i 0, i 0, i 0, i 3)"

	# Service의 복구 옵션을 초기화합니다.
	# 공유메모리에 현재 uninstall이 진행중임을 기록합니다.
	# (다른 프로세스는 그에 따라 동작할 수 있습니다.)
	
	!addplugindir "${PATH_PREFIX}Common2\nsis"
	abgnsis::SetFlagForUninstall
	#abgnsis::ClearServiceRecoveryOption "aduWatcherSvc"
	
	# Service 및 프로세스, dll들을 종료하고, 삭제를 진행합니다.
	
	SetOutPath $INSTDIR
	abgnsis::ExitConsole
	abgnsis::ProductUninstInit

	ExecWait '"$INSTDIR\abgConsole.exe" uninstall'

	# 바로가기 파일 및 설치폴더의 binary들을 삭제합니다.
	
	SetShellVarContext all
	Delete /REBOOTOK "$DESKTOP\$(SHORTCUTNAME_DESKTOP).lnk"
	RMDir /REBOOTOK /r "$SMPROGRAMS\$(SHORTCUTNAME_PROGRAMS_DIRECTORY)"
	SetShellVarContext current
	
	SetOutPath "$INSTDIR\.."
	RMDir /r /REBOOTOK "$INSTDIR"

	${If} ${RunningX64}
		!insertmacro DisableX64FSRedirection
	${EndIf}
	
	#Delete /REBOOTOK "$SYSDIR\drivers\aduflt.sys"
	#Delete /REBOOTOK "$SYSDIR\drivers\aduflt.ini"
	
	#Delete /REBOOTOK "$SYSDIR\drivers\adufs.sys"
	#Delete /REBOOTOK "$SYSDIR\drivers\aduff.sys"
	#Delete /REBOOTOK "$SYSDIR\drivers\adufn.sys"
	
	${If} ${RunningX64}
		!insertmacro EnableX64FSRedirection
	${EndIf}
	
	SetShellVarContext all
	RMDir /r /REBOOTOK "$APPDATA\${PRODUCT_APP_DATA}"
	SetShellVarContext current
	
	RMDir /r /REBOOTOK "$LOCALAPPDATA\${PRODUCT_APP_DATA}"
	Delete /REBOOTOK "$APPDATA\${PRODUCT_APP_DATA}\*.*"

	abgnsis::IsEmptyAppDataPath $LOCALAPPDATA
	Pop $R0
	${If} $R0 == "1"
		RMDir /r /REBOOTOK "$LOCALAPPDATA\Ados"
	${EndIf}
	
	abgnsis::IsEmptyAppDataPath $APPDATA
	Pop $R0
	${If} $R0 == "1"
		RMDir /r /REBOOTOK "$APPDATA\Ados"
	${EndIf}
	
	abgnsis::IsEmptyAppDataPath2 $APPDATA
	Pop $R0
	${If} $R0 == "1"
		RMDir /r /REBOOTOK "$APPDATA\NoAD Inc"
	${EndIf}
	SetShellVarContext current
		
	#모든 유저의 레지스트리를 찾아서 삭제합니다.
	StrCpy $0 0
	${Do}
		EnumRegKey $1 HKU "" $0
		${If} $1 != ""
			DeleteRegKey HKU "$1\${PRODUCT_APP_KEY}"
			IntOp $0 $0 + 1
		${EndIf}
	${LoopUntil} $1 == ""
	
	DeleteRegKey HKLM "${PRODUCT_APP_KEY}"
	
	abgnsis::ClearRegistryKey "${PRODUCT_ENGINE_KEY}"
	abgnsis::ClearRegistryKey "${PRODUCT_MIRAGEWORKS_KEY}"
	
	DeleteRegKey HKLM "${PRODUCT_UNINST_KEY}"
	
	abgnsis::AddDummyValuePendingFileRenameOperations
	abgnsis::ClearFlagForUninstall
	
SectionEnd

Function un.onUninstSuccess

	# 삭제 완료. 재부팅 유도 메시지 박스.
	
	#MessageBox MB_YESNO|MB_TOPMOST|MB_SETFOREGROUND "$(MSGBOX_UNINST_ALERT_REBOOT_REQUIRED)" IDNO +4 IDYES 0
	#abgnsis::SystemReboot
	SetErrorLevel 0
	#Abort
	
FunctionEnd