#include "StdAfx.h"
#include <Library/API/ShellAPI.h>
#include <Library/API/FSAPI.h>
#include <sddl.h>
#include <shlobj.h>
#include <ExDisp.h>
#include <TlHelp32.h>
#include <CommCtrl.h>
#include <PowrProf.h>

#pragma comment (lib, "Comctl32.lib")
#pragma comment (lib, "powrprof.lib")

#include <wpp.h>
#include <ShellAPI.tmh>

const IID IID_IImageList = {0x46EB5926, 0x582E, 0x4017, 0x9F, 0xDF, 0xE8, 0x99, 0x8D, 0xAA, 0x09, 0x50};

typedef struct TRAYDATA
{
	HWND hwnd;
	UINT uID;
	UINT uCallbackMessage;
	DWORD Reserved[2];
	HICON hIcon;
} TRAYDATA;

typedef DWORD (WINAPI *_PowerWriteACValueIndex) (
	HKEY RootPowerKey,
	CONST GUID *SchemeGuid,
	CONST GUID *SubGroupOfPowerSettingsGuid,
	CONST GUID *PowerSettingGuid,
	DWORD AcValueIndex
	);

typedef DWORD(WINAPI *_PowerWriteDCValueIndex) (
	HKEY RootPowerKey,
	CONST GUID *SchemeGuid,
	CONST GUID *SubGroupOfPowerSettingsGuid,
	CONST GUID *PowerSettingGuid,
	DWORD DcValueIndex
	);

typedef DWORD (WINAPI *_PowerGetActiveScheme) (
	HKEY UserRootPowerKey,
	GUID **ActivePolicyGuid
	);

typedef DWORD (WINAPI *_PowerSetActiveScheme) (
	HKEY UserRootPowerKey,
	const GUID *SchemeGuid
	);

HICON Library::API::ShellAPI::GetIcon(int nIndex, int nType /*= SHIL_EXTRALARGE*/)
{
	void* pv = NULL;
	HRESULT hr = ::SHGetImageList(
		nType,
		IID_IImageList,
		&pv
		);
	if (!SUCCEEDED(hr))
		throw new COMException(hr);
	HIMAGELIST hsil = (HIMAGELIST)pv;
	return ImageList_GetIcon(hsil, nIndex, ILD_NORMAL);
}

HICON Library::API::ShellAPI::GetFileIcon(string& strPath, int nType /*= SHIL_EXTRALARGE*/)
{
	SHFILEINFOW fi = { 0 };
	DWORD_PTR dwResult = ::SHGetFileInfoW(
		strPath,
		0,
		&fi,
		sizeof(fi),
		SHGFI_SYSICONINDEX
		);
	if (0 == dwResult)
		throw new Win32Exception(::GetLastError());

	return GetIcon(fi.iIcon, nType);
}

HICON Library::API::ShellAPI::GetFileIcon(string& strPath, int nIndex, int nType)
{
	HICON hLarge, hSmall;
	ExtractIconEx((const wchar_t*)strPath, nIndex, &hLarge, &hSmall, 1);
	return hLarge;
}

void Library::API::ShellAPI::CloseRemovableExplorerWindow()
{
	try
	{
		List<HWND> aHwnd;
		IShellWindows *psw;
		if (SUCCEEDED(CoCreateInstance(CLSID_ShellWindows, NULL, CLSCTX_ALL, IID_IShellWindows, (void**)&psw)))
		{
			VARIANT v;
			V_VT(&v) = VT_I4;
			IDispatch  *pdisp;
			for (V_I4(&v) = 0; psw->Item(v, &pdisp) == S_OK; V_I4(&v)++)
			{
				BOOL fSuccess = TRUE;
				IWebBrowserApp *pwba = NULL;
				IServiceProvider *psp = NULL;
				IShellBrowser *psb = NULL;
				IShellView *psv = NULL;
				IFolderView *pfv = NULL;
				IPersistFolder2 *ppf2 = NULL;
				LPITEMIDLIST pidlFolder = NULL;
				TCHAR szPath[MAX_PATH] = { 0, };
				HWND hShellWnd = NULL;

				if (fSuccess && !SUCCEEDED(pdisp->QueryInterface(IID_IWebBrowserApp, (void**)&pwba)))
					fSuccess = FALSE;
				if (fSuccess && !SUCCEEDED(pwba->get_HWND((LONG_PTR*)&hShellWnd)))
					fSuccess = FALSE;
				if (fSuccess && !SUCCEEDED(pwba->QueryInterface(IID_IServiceProvider, (void**)&psp)))
					fSuccess = FALSE;
				if (fSuccess && !SUCCEEDED(psp->QueryService(SID_STopLevelBrowser, IID_IShellBrowser, (void**)&psb)))
					fSuccess = FALSE;
				if (fSuccess && !SUCCEEDED(psb->QueryActiveShellView(&psv)))
					fSuccess = FALSE;
				if (fSuccess && !SUCCEEDED(psv->QueryInterface(IID_IFolderView, (void**)&pfv)))
					fSuccess = FALSE;
				if (fSuccess && !SUCCEEDED(pfv->GetFolder(IID_IPersistFolder2, (void**)&ppf2)))
					fSuccess = FALSE;
				if (fSuccess && !SUCCEEDED(ppf2->GetCurFolder(&pidlFolder)))
					fSuccess = FALSE;

				if (fSuccess && SHGetPathFromIDList(pidlFolder, szPath))
				{
					string strPath = String(szPath);
					if (!strPath.IsEmpty() && Library::API::FSAPI::IsRemovableDrive(strPath.Substring(0, 1)))
						aHwnd.Add(hShellWnd);
					CoTaskMemFree(pidlFolder);
				}
				if (ppf2 != NULL)
					ppf2->Release();
				if (pfv != NULL)
					pfv->Release();
				if (psv != NULL)
					psv->Release();
				if (psb != NULL)
					psb->Release();
				if (psp != NULL)
					psp->Release();
				if (pwba != NULL)
					pwba->Release();
				if (pdisp != NULL)
					pdisp->Release();
			}
			psw->Release();
		}

		for (int i = 0; i < aHwnd.GetCount(); i++)
		{
			Sleep(100);
			::PostMessage(aHwnd[i], WM_CLOSE, 0, 0);
		}
	}
	catch (Exception *pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}
	catch (...)
	{
	}
}

void Library::API::ShellAPI::BringWindowToTopLevel(HWND hWnd)
{
	if (IsIconic(hWnd))
		ShowWindowAsync(hWnd, SW_RESTORE);

	ShowWindowAsync(hWnd, SW_SHOW);
	SetForegroundWindow(hWnd);

	HWND foregroundWindow = GetForegroundWindow();
	DWORD Dummy = 0;

	UINT forgroundThreadId = GetWindowThreadProcessId(foregroundWindow, &Dummy);
	UINT thisThreadId = GetWindowThreadProcessId(hWnd, &Dummy);

	if (AttachThreadInput(thisThreadId, forgroundThreadId, true))
	{
		BringWindowToTop(hWnd);
		SetForegroundWindow(hWnd);
		AttachThreadInput(thisThreadId, forgroundThreadId, false);
	}

	if (GetForegroundWindow() != hWnd)
	{
		DWORD Timeout = 0;
		SystemParametersInfo(SPI_GETFOREGROUNDLOCKTIMEOUT, 0, &Timeout, 0);
		SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, &Dummy, SPIF_SENDCHANGE);
		BringWindowToTop(hWnd);
		SetForegroundWindow(hWnd);
		SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, &Timeout, SPIF_SENDCHANGE);
	}
}

HWND Library::API::ShellAPI::FindMyTopMostWindow()
{
	DWORD dwProcID = GetCurrentProcessId();
	HWND hWnd = GetTopWindow(GetDesktopWindow());
	while (hWnd)
	{
		DWORD dwWndProcID = 0;
		GetWindowThreadProcessId(hWnd, &dwWndProcID);
		if (dwWndProcID == dwProcID)
			return hWnd;
		hWnd = GetNextWindow(hWnd, GW_HWNDNEXT);
	}

	return NULL;
}

HWND Library::API::ShellAPI::FindTrayToolbarWindow()
{
	HWND hWnd_ToolbarWindow32 = NULL;
	HWND hWnd_ShellTrayWnd = ::FindWindow(L"Shell_TrayWnd", NULL);
	if (hWnd_ShellTrayWnd)
	{
		HWND hWnd_TrayNotifyWnd = ::FindWindowEx(hWnd_ShellTrayWnd, NULL, L"TrayNotifyWnd", NULL);
		if (hWnd_TrayNotifyWnd)
		{
			HWND hWnd_SysPager = ::FindWindowEx(hWnd_TrayNotifyWnd, NULL, L"SysPager", NULL);        // WinXP

			// WinXP 에서는 SysPager 까지 추적
			// Win2000 일 경우에는 SysPager 가 없이 TrayNotifyWnd -&gt; ToolbarWindow32 로 넘어간다
			if (hWnd_SysPager)
				hWnd_ToolbarWindow32 = ::FindWindowEx(hWnd_SysPager, NULL, L"ToolbarWindow32", NULL);
			else
				hWnd_ToolbarWindow32 = ::FindWindowEx(hWnd_TrayNotifyWnd, NULL, L"ToolbarWindow32", NULL);
		}
	}
	return hWnd_ToolbarWindow32;
}

void Library::API::ShellAPI::ClearDeadTrayIcon()
{
	HWND hTrayWnd = FindTrayToolbarWindow();
	HWND hOverflow = ::FindWindowEx(::FindWindow(L"NotifyIconOverflowWindow", NULL), NULL, L"ToolbarWindow32", NULL);
	if (hTrayWnd)
	{
		RECT rect;
		GetClientRect(hTrayWnd, &rect);
		for (int i = 0; i < rect.right; i += 5)
			for (int j = 0; j < rect.bottom; j += 5)
				SendMessage(hTrayWnd, WM_MOUSEMOVE, 0, (j << 16) + i);
	}
	if (hOverflow)
	{
		RECT rect;
		GetClientRect(hOverflow, &rect);
		for (int i = 0; i < rect.right; i += 5)
			for (int j = 0; j < rect.bottom; j += 5)
				SendMessage(hOverflow, WM_MOUSEMOVE, 0, (j << 16) + i);
	}
}

void Library::API::ShellAPI::ClearDeadSystemTrayIcon()
{
	HWND hTrayWnd = FindTrayToolbarWindow();
	HWND hOverflow = ::FindWindowEx(::FindWindow(L"NotifyIconOverflowWindow", NULL), NULL, L"ToolbarWindow32", NULL);

	if (hTrayWnd)
		ClearDeadSystemTrayIcon(hTrayWnd);
	if (hOverflow)
		ClearDeadSystemTrayIcon(hOverflow);
}

void Library::API::ShellAPI::ClearDeadSystemTrayIcon(HWND hWndTray)
{
	LPVOID pData = NULL;
	int i = 0, nCount = 0;
	TBBUTTON tb;
	TRAYDATA tray;
	DWORD dwTrayPid = 0, dwProcId = 0;
	HANDLE hProcTray = NULL;
	try
	{
		nCount = (int)SendMessage(hWndTray, TB_BUTTONCOUNT, 0, 0); /* 시스템 트레이 툴바안에 등록된 버튼의 수 조희 */
		GetWindowThreadProcessId(hWndTray, &dwTrayPid);
		hProcTray = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, dwTrayPid);
		if (hProcTray)
		{
			pData = VirtualAllocEx(hProcTray, NULL, sizeof(TBBUTTON), MEM_COMMIT, PAGE_READWRITE);

			HANDLE hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
			if (INVALID_HANDLE_VALUE == hSnapshot) throw new Win32Exception(::GetLastError());

			do
			{
				if (pData == NULL)
				{
					_TRACE_E(Application, L"VirtualAllocEx() return value is null | Code : %d ", ::GetLastError());
					break;
				}

				/* 시스템 트레이에 등록된 툴바 버튼의 수 많큼 루프를 돌면서, 등록된 응용프로그램의 정보 조회 */
				for (i = 0; i < nCount; i++)
				{
					dwProcId = 0;
					SendMessage(hWndTray, TB_GETBUTTON, i, (LPARAM)pData);
					ReadProcessMemory(hProcTray, (LPCVOID)pData, (LPVOID)&tb, sizeof(TBBUTTON), NULL);
					ReadProcessMemory(hProcTray, (LPCVOID)tb.dwData, (LPVOID)&tray, sizeof(tray), NULL);
					GetWindowThreadProcessId(tray.hwnd, &dwProcId);

					bool fRefresh = (dwProcId == 0) || !IsProcessExists(hSnapshot, dwProcId);
					if (fRefresh)
					{
						_TRACE_I(Application, "Refresh Tray | PID : %d", dwProcId);
						/* 해당 아이콘의 부모 프로세스가 존재하지 않는 경우에, 트레이에서 제거 */

						RECT rect;
						SendMessage(hWndTray, TB_GETITEMRECT, i, (LPARAM)pData);
						ReadProcessMemory(hProcTray, pData, (LPVOID)&rect, sizeof(RECT), NULL);
						int cx = (rect.left + rect.right) / 2;
						int cy = (rect.top + rect.bottom) / 2;
						SendMessage(hWndTray, WM_MOUSEMOVE, 0, (cy << 16) + cx);
					}
				}
			} while (FALSE);

			::CloseHandle(hSnapshot);
		}
	}
	catch (Exception* pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}
	catch (...)
	{
		_TRACE_EX_UNK();
	}

	if (NULL != pData)
		VirtualFreeEx(hProcTray, pData, 0, MEM_RELEASE);
	if (NULL != hProcTray)
		CloseHandle(hProcTray);
}

bool Library::API::ShellAPI::IsProcessExists(HANDLE hSnapshot, DWORD pid)
{
	PROCESSENTRY32 entry;
	memset(&entry, 0, sizeof(PROCESSENTRY32));
	entry.dwSize = sizeof(PROCESSENTRY32);
	BOOL b = ::Process32First(hSnapshot, &entry);
	while (b)
	{
		if (pid == entry.th32ProcessID)
			return true;
		entry.dwSize = sizeof(PROCESSENTRY32);
		b = ::Process32Next(hSnapshot, &entry);
	}
	return false;
}

void Library::API::ShellAPI::CheckIdleTimeout()
{
	bool fRet = false;
	bool fChangeIdleTimeoutAC = false;
	bool fChangeIdleTimeoutDC = false;

	HMODULE hModule = NULL;
	POWER_POLICY powerPolicy;
	GLOBAL_POWER_POLICY globalPowerPolicy;
	UINT puiID;
	GUID* pCurrentActivePowerScheme = NULL;

	do
	{
		if (!GetActivePwrScheme(&puiID))
			break;
		if (!GetCurrentPowerPolicies(&globalPowerPolicy, &powerPolicy))
			break;

		// 절전모드인지 확인, 21600은 escort일 때
		if (powerPolicy.user.IdleTimeoutAc > 0 && powerPolicy.user.IdleTimeoutAc != 21600)
			fChangeIdleTimeoutAC = true;
		if (powerPolicy.user.IdleTimeoutDc > 0 && powerPolicy.user.IdleTimeoutDc != 21600)
			fChangeIdleTimeoutDC = true;

		if (!fChangeIdleTimeoutAC && !fChangeIdleTimeoutDC)
			break;

		if (Version(6, 0) <= Environment::GetOSVersion()->GetVersion())
		{
			hModule = LoadLibraryW(L"PowrProf.dll");
			if (NULL == hModule)
				break;

			_PowerGetActiveScheme pGetActiveScheme = (_PowerGetActiveScheme)::GetProcAddress(hModule, "PowerGetActiveScheme");
			if (NULL == pGetActiveScheme)
				break;

			if (ERROR_SUCCESS != pGetActiveScheme(NULL, &pCurrentActivePowerScheme))
				break;

			_PowerWriteACValueIndex pPowerWriteACValueIndex = (_PowerWriteACValueIndex)::GetProcAddress(hModule, "PowerWriteACValueIndex");
			_PowerWriteDCValueIndex pPowerWriteDCValueIndex = (_PowerWriteDCValueIndex)::GetProcAddress(hModule, "PowerWriteDCValueIndex");
			_PowerSetActiveScheme pPowerSetActiveScheme = (_PowerSetActiveScheme)::GetProcAddress(hModule, "PowerSetActiveScheme");

			if ((fChangeIdleTimeoutAC && NULL == pPowerWriteACValueIndex) ||
				(fChangeIdleTimeoutDC && NULL == pPowerWriteDCValueIndex) ||
				NULL == pPowerSetActiveScheme)
				break;

			if (fChangeIdleTimeoutAC)
				pPowerWriteACValueIndex(NULL, pCurrentActivePowerScheme, &GUID_SLEEP_SUBGROUP, &GUID_STANDBY_TIMEOUT, 0);
			if (fChangeIdleTimeoutDC)
				pPowerWriteDCValueIndex(NULL, pCurrentActivePowerScheme, &GUID_SLEEP_SUBGROUP, &GUID_STANDBY_TIMEOUT, 0);

			if (ERROR_SUCCESS == pPowerSetActiveScheme(NULL, pCurrentActivePowerScheme))
				fRet = true;
		}
		else
		{
			// xp의 경우 powercfg.exe의 매개 변수가 다름
			if (fChangeIdleTimeoutAC)
				powerPolicy.user.IdleTimeoutAc = 0;
			if (fChangeIdleTimeoutDC)
				powerPolicy.user.IdleTimeoutDc = 0;

			if (SetActivePwrScheme(puiID, &globalPowerPolicy, &powerPolicy)) // 관리자 권한이 필요하여 xp에서만 사용
				fRet = true;
		}

	} while (false);

	if (NULL != hModule)
		FreeLibrary(hModule);
	if (NULL != pCurrentActivePowerScheme)
		LocalFree(pCurrentActivePowerScheme);

	if (fRet)
		_TRACE_I(Application, L"IdleTimeout changed successfully.");
}

void Library::API::ShellAPI::RegisterShellFolder(
	const string& strClsid,
	const string& strName,
	const string& strFolderPath,
	const string& strIconPath,
	bool fWow64)
{
	// Integrate a Cloud Storage Provider
	// https://docs.microsoft.com/en-us/windows/win32/shell/integrate-cloud-storage

	RegistryKey* pKey = null;
	RegistryKey* pSubDIKey = null;
	RegistryKey* pSubIPSKey = null;
	RegistryKey* pSubIKey = null;
	RegistryKey* pSubIPBKey = null;
	RegistryKey* pSubSFKey = null;

	try
	{
		string strClsidKey;
		if (fWow64)
			strClsidKey = L"Software\\Classes\\WOW6432Node\\CLSID";
		else
			strClsidKey = L"Software\\Classes\\CLSID";

		pKey = Registry::GetLocalMachine()->CreateSubKey(strClsidKey + L"\\" + strClsid);
		if (pKey)
		{
			// Add your CLSID and name your extension
			pKey->SetValue(L"", strName);

			// Add your extension to the Navigation Pane and make it visible
			pKey->SetValue(L"System.IsPinnedToNamespaceTree", (uint32)1);

			// Set the location for your extension in the Navigation Pane
			pKey->SetValue(L"SortOrderIndex", (uint32)0x42);

			// Set the image for your icon
			pSubDIKey = pKey->CreateSubKey(L"DefaultIcon");
			if (pSubDIKey)
			{
				string strIconEnvPath;

				string strProgramFiles = Environment::GetFolderPath(ESpecialFolder::ProgramFiles);
				if (strIconPath.StartsWith(strProgramFiles, true) &&
					strProgramFiles.GetLength() < strIconPath.GetLength())
				{
					strIconEnvPath = L"%ProgramW6432%" + strIconPath.Substring(strProgramFiles.GetLength());
				}

				if (!strIconEnvPath.IsEmpty())
					pSubDIKey->SetValue(L"", strIconEnvPath + L",0", ERegistryValueKind::ExpandString);
			}

			// Provide the dll that hosts your extension
			pSubIPSKey = pKey->CreateSubKey(L"InProcServer32");
			if (pSubIPSKey)
				pSubIPSKey->SetValue(L"", L"%SYSTEMROOT%\\system32\\shell32.dll", ERegistryValueKind::ExpandString);

			pSubIKey = pKey->CreateSubKey(L"Instance");
			if (pSubIKey)
			{
				// Define the instance object
				pSubIKey->SetValue(L"CLSID", L"{0E5AAE11-A475-4c5b-AB00-C66DE400274E}");

				pSubIPBKey = pSubIKey->CreateSubKey(L"InitPropertyBag");
				if (pSubIPBKey)
				{
					// Provide the file system attributes of the target folder
					uint32 uAttributes = FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_READONLY;
					pSubIPBKey->SetValue(L"Attributes", uAttributes);

					// Set the path for the sync root
					pSubIPBKey->SetValue(L"TargetFolderPath", strFolderPath, ERegistryValueKind::ExpandString);
				}
			}

			pSubSFKey = pKey->CreateSubKey(L"ShellFolder");
			if (pSubSFKey)
			{
				// Set appropriate shell flags
				pSubSFKey->SetValue(L"FolderValueFlags", (uint32)0x28);

				// Set the appropriate flags to control your shell behavior
				uint32 uAttributes =
					SFGAO_CANCOPY | SFGAO_CANLINK | SFGAO_STORAGE | SFGAO_HASPROPSHEET |
					SFGAO_STORAGEANCESTOR | SFGAO_FILESYSANCESTOR | SFGAO_FOLDER |
					SFGAO_FILESYSTEM | SFGAO_HASSUBFOLDER;

				pSubSFKey->SetValue(L"Attributes", uAttributes);
			}
		}
	}
	catch (Exception* px)
	{
		_TRACE_EX(px);
		delete px;
	}
	catch (...)
	{
		_TRACE_EX_UNK();
	}

	if (pSubSFKey)
		delete pSubSFKey;
	if (pSubIPBKey)
		delete pSubIPBKey;
	if (pSubIKey)
		delete pSubIKey;
	if (pSubIPSKey)
		delete pSubIPSKey;
	if (pSubDIKey)
		delete pSubDIKey;
	if (pKey)
		delete pKey;
}

void Library::API::ShellAPI::UnregisterShellFolder(
	const string& strClsid,
	bool fWow64)
{
	try
	{
		string strClsidKey;
		if (fWow64)
			strClsidKey = L"Software\\Classes\\WOW6432Node\\CLSID";
		else
			strClsidKey = L"Software\\Classes\\CLSID";

		Registry::GetLocalMachine()->DeleteSubKeyTree(strClsidKey + L"\\" + strClsid, false);
	}
	catch (Exception* px)
	{
		_TRACE_EX(px);
		delete px;
	}
	catch (...)
	{
		_TRACE_EX_UNK();
	}
}

void Library::API::ShellAPI::ShowShellFolderOnExplorer(
	const string& strClsid, 
	const string& strName,
	RegistryKey* pHKLMKey,
	bool fImport,
	bool fWow64)
{
	RegistryKey* pExplorerKey = null;
	RegistryKey* pEDNKey = null;
	RegistryKey* pEHNKey = null;
	RegistryKey* pEHNSrcKey = null;

	try
	{
		string strSoftwareKey;
		if (fWow64)
			strSoftwareKey = L"Software\\WOW6432Node";
		else
			strSoftwareKey = L"Software";

		pExplorerKey = pHKLMKey->CreateSubKey(strSoftwareKey + L"\\Microsoft\\Windows\\CurrentVersion\\Explorer");
		if (pExplorerKey)
		{
			// Register your extension in the namespace root
			pEDNKey = pExplorerKey->CreateSubKey(L"Desktop\\NameSpace\\" + strClsid);
			if (pEDNKey)
				pEDNKey->SetValue(L"", strName);

			// Hide your extension from the Desktop
			pEHNKey = pExplorerKey->CreateSubKey(L"HideDesktopIcons\\NewStartPanel");
			if (pEHNKey)
			{
				if (fImport)
				{
					pEHNSrcKey = Registry::GetLocalMachine()->OpenSubKey(
						strSoftwareKey + L"\\Microsoft\\Windows\\CurrentVersion\\Explorer\\HideDesktopIcons\\NewStartPanel");
					if (pEHNSrcKey)
						pEHNKey->CopyTree(pEHNSrcKey);
				}

				pEHNKey->SetValue(strClsid, (uint32)1);
			}
		}

	}
	catch (Exception* px)
	{
		_TRACE_EX(px);
		delete px;
	}
	catch (...)
	{
		_TRACE_EX_UNK();
	}

	if (pEDNKey)
		delete pEDNKey;
	if (pEHNKey)
		delete pEHNKey;
	if (pEHNSrcKey)
		delete pEHNSrcKey;
	if (pExplorerKey)
		delete pExplorerKey;
}

void Library::API::ShellAPI::HideShellFolderOnExplorer(
	const string& strClsid,
	RegistryKey* pHKLMKey,
	bool fWow64)
{
	RegistryKey* pExplorerKey = null;
	RegistryKey* pEHNKey = null;

	try
	{
		string strSoftwareKey;
		if (fWow64)
			strSoftwareKey = L"Software\\WOW6432Node";
		else
			strSoftwareKey = L"Software";

		pExplorerKey = pHKLMKey->OpenSubKey(strSoftwareKey + L"\\Microsoft\\Windows\\CurrentVersion\\Explorer", true);
		if (pExplorerKey)
		{
			pExplorerKey->DeleteSubKey(L"Desktop\\NameSpace\\" + strClsid, false);

			pEHNKey = pExplorerKey->OpenSubKey(L"HideDesktopIcons\\NewStartPanel", true);
			if (pEHNKey)
				pEHNKey->DeleteValue(strClsid, false);
		}

	}
	catch (Exception* px)
	{
		_TRACE_EX(px);
		delete px;
	}
	catch (...)
	{
		_TRACE_EX_UNK();
	}

	if (pEHNKey)
		delete pEHNKey;
	if (pExplorerKey)
		delete pExplorerKey;
}
