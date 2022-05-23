#pragma once
#include <windows.h>
#include "exdll.h"
#include <TlHelp32.h>
#include <Light/System/all.h>
#include <Light/Microsoft/Win32/all.h>
#include <Light/System/Collections/Generic/all.h>
#include <Light/System/io/all.h>
#include <Light/System/Text/all.h>
#include <Light/NoAD/ipc/all.h>

#define INSTALL_COMMANDLINE	L"ovti.exe install oemwin2k.inf tap0901"
#define SECUI_INSTALL_7_COMMANDLINE		L"QuickSecDriverSetup_Win7.exe /S /Install"
#define SECUI_INSTALL_7x64_COMMANDLINE	L"QuickSecDriverSetup_Win7_x64.exe /S /Install"
#define SECUI_INSTALL_XP_COMMANDLINE	L"QuickSecDriverSetup_WinXP.exe /S /Install"

void TerminateExe(TCHAR *szExe);
bool MoveDirectory(lstring strSource, lstring strDestination);
bool DeleteDirectory(lstring strDirectory);
bool AddPendingFileRenameOperations(lstring strArg1, lstring strArg2);
bool RegSetMultiStringValue(HKEY hKey, lstring strName, larray<lstring> astrValue);
bool RegGetMultiStringValue(HKEY hKey, lstring strName, larray<lstring> &astrValue);

// ret : Reboot required.
bool CheckIncops();
bool _SetErrorMode();

TCHAR* InstallVPNDriver(lstring strVPNCmdLine, lstring strVPNName);
void UninstallVPNDrivers(lstring strVPNCmdLine);