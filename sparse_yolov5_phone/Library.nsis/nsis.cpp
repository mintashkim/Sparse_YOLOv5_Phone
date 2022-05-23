#include <windows.h>
#include "exdll.h"
#include <Light/System/all.h>
#include <Light/System/Diagnostics/all.h>
#include <Light/System/Threading/all.h>
#include <Light/Microsoft/Win32/all.h>
#include "LInstallHelper.h"

#include "../Base/Defines.h"

HINSTANCE g_hInstance;
HWND g_hwndParent;

#define NSIS_FUNCTION(x) extern "C" void __declspec(dllexport) x(\
	HWND hwndParent,\
	int string_size,\
	TCHAR *variables,\
	stack_t **stacktop,\
	extra_parameters *extra)

lstring PopString()
{
	TCHAR strSrc[1024];
	memset(strSrc, 0x00, sizeof(TCHAR) * 1024);
	popstring(strSrc);	
	return lstring(strSrc);
}

void PushResult(bool f)
{
	pushstring(f ? L"1" : L"0");
}

NSIS_FUNCTION(SystemReboot)
{
	g_hwndParent = hwndParent;
	EXDLL_INIT();
	LInstallHelper::SystemReboot();
}

NSIS_FUNCTION(IsSupportedWindows)
{
	g_hwndParent = hwndParent;
	EXDLL_INIT();

	LOSName osName = LEnvironment::GetOSName();
	LTrace::WriteLine(lstring::Format(L"%ws() - Current windows version: %d. min=[%d], max=[%d]", __FUNCTIONW__, osName, SUPPORTED_WIN_VERSION_MIN, SUPPORTED_WIN_VERSION_MAX));

	if (SUPPORTED_WIN_VERSION_MIN <= osName && SUPPORTED_WIN_VERSION_MAX >= osName)
		PushResult(true);
	else
		PushResult(false);
}

NSIS_FUNCTION(IsInstalledProduct)
{
	g_hwndParent = hwndParent;
	EXDLL_INIT();
	PushResult(LInstallHelper::IsInstalledProduct());
}

NSIS_FUNCTION(AddDummyValuePendingFileRenameOperations)
{
	g_hwndParent = hwndParent;
	EXDLL_INIT();
	LInstallHelper::AddPendingFileRenameOperations(APPLICATION_NAME, APPLICATION_NAME);
}

NSIS_FUNCTION(CheckRebootAfterUninstall)
{
	g_hwndParent = hwndParent;
	EXDLL_INIT();
	PushResult(LInstallHelper::CheckRebootAfterUninstall());
}

NSIS_FUNCTION(ProductUninstInit)
{
	g_hwndParent = hwndParent;
	EXDLL_INIT();
	LInstallHelper::TerminateExe(ABG_BINARY_CONSOLE);
}

NSIS_FUNCTION(ProductUpdateInit)
{
	g_hwndParent = hwndParent;
	EXDLL_INIT();
	LInstallHelper::TerminateExe(ABG_BINARY_CONSOLE);
}

NSIS_FUNCTION(SetupServiceRecoveryOption)
{
	g_hwndParent = hwndParent;
	EXDLL_INIT();

	lstring str = PopString();
	
	if (!str.IsEmpty())
		LInstallHelper::SetupServiceRecoveryOption(str);
}

NSIS_FUNCTION(ClearServiceRecoveryOption)
{
	g_hwndParent = hwndParent;
	EXDLL_INIT();
	lstring str = PopString();

	if (!str.IsEmpty())
		LInstallHelper::ClearServiceRecoveryOption(str);
}

NSIS_FUNCTION(SetFlagForUninstall)
{
	g_hwndParent = hwndParent;
	EXDLL_INIT();

	//LSharedSession::SetValue(ADU_UNINSTALL_KEY, ADU_UNINSTALL_UNINSTALLING, L"1");
}

NSIS_FUNCTION(ClearFlagForUninstall)
{
	g_hwndParent = hwndParent;
	EXDLL_INIT();

	//LSharedSession::SetValue(ADU_UNINSTALL_KEY, ADU_UNINSTALL_UNINSTALLING, L"0");
}

NSIS_FUNCTION(IsEmptyAppDataPath)
{
	g_hwndParent = hwndParent;
	EXDLL_INIT();

	bool fResult = false;
	lstring strAppDataPath = PopString();
	lstring strMiragePath = strAppDataPath + L"\\Ados";
	if (INVALID_FILE_ATTRIBUTES != ::GetFileAttributesW(strMiragePath))
	{
		fResult = LInstallHelper::IsEmptyDirectory(strMiragePath);
	}

	PushResult(fResult);
}

NSIS_FUNCTION(IsEmptyAppDataPath2)
{
	g_hwndParent = hwndParent;
	EXDLL_INIT();

	bool fResult = false;
	lstring strAppDataPath = PopString();
	lstring strMiragePath = strAppDataPath + L"\\NoAD Inc";
	if (INVALID_FILE_ATTRIBUTES != ::GetFileAttributesW(strMiragePath))
	{
		fResult = LInstallHelper::IsEmptyDirectory(strMiragePath);
	}

	PushResult(fResult);
}

NSIS_FUNCTION(GetProgramDataPath)
{
	g_hwndParent = hwndParent;
	EXDLL_INIT();

	lstring strPath = LEnvironment::GetFolderPath(LESpecialFolder::CommonApplicationData);
	pushstring(strPath);
}

void RenameDeleteFilesRecursive(lstring strPath)
{
	WIN32_FIND_DATAW find;
	HANDLE hFind = FindFirstFileW((const wchar_t*)(strPath + L"\\*"), &find);
	if (INVALID_HANDLE_VALUE != hFind)
	{
		do 
		{
			lstring strFileName_Src = strPath + L"\\" + find.cFileName;
			lstring strFileName_Dst = strFileName_Src + L".old";
			if (strFileName_Src.EndsWith(L".") || strFileName_Src.EndsWith(L".."))
				continue;

			if (find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (!lstring(find.cFileName).Equals(L"ext", true))
					RenameDeleteFilesRecursive(strFileName_Src);
			}
			else
			{
				if (!::MoveFileW(strFileName_Src, strFileName_Dst))
					LTrace::WriteLine(LString::Format(L"MoveFile Failed = %ws, %d", (const wchar_t*)strFileName_Src, ::GetLastError()));

				if (!::DeleteFileW(strFileName_Dst))
				{
					if (!::MoveFileExW(strFileName_Dst, NULL, MOVEFILE_DELAY_UNTIL_REBOOT))
						LTrace::WriteLine(LString::Format(L"MoveFileEx Failed = %ws, %d", (const wchar_t*)strFileName_Dst, ::GetLastError()));
				}
			}
		}
		while (FindNextFileW(hFind, &find));
		FindClose(hFind);
	}
}

NSIS_FUNCTION(ClearExistingFilesForUpdate)
{
	g_hwndParent = hwndParent;
	EXDLL_INIT();
	
	lstring str = PopString();
	RenameDeleteFilesRecursive(str);
}

NSIS_FUNCTION(ClearRegistryKey)
{
	g_hwndParent = hwndParent;
	EXDLL_INIT();

	lstring AppPath = PopString();
	LInstallHelper::ClearRegistryKey(AppPath);
}

NSIS_FUNCTION(ExitConsole)
{
	g_hwndParent = hwndParent;
	EXDLL_INIT();

	LEventWaitHandle(false, EEventResetMode::AutoReset, ABG_EVENT_CONSOLE_EXIT).Set();
}

BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	g_hInstance = (HINSTANCE)hInst;
	return TRUE;
}