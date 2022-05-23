#include "LInstallHelper.h"
#include <Light/System/Security/Cryptography/all.h>
#include <Light/System/Collections/Generic/all.h>
#include <Light/System/Diagnostics/all.h>
#include <Light/System/Threading/all.h>
#include <Light/System/Text/all.h>
#include <Light/System/io/all.h>
#include <TlHelp32.h>

#include "../Base/Defines.h"
#include "../Base/VersionInfo.h"

#pragma comment(lib,"imagehlp")

#include <shlobj.h>
#pragma comment(lib, "shlwapi.lib")

LMutex *g_pMutex = NULL;

bool LInstallHelper::AddPendingFileRenameOperations( lstring strArg1, lstring strArg2 )
{
	LRegistryKey *pKey = NULL;
	bool fRet = false;
	try
	{
		pKey = LRegistry::GetLocalMachine()->OpenSubKey(L"SYSTEM\\CurrentControlSet\\Control\\Session Manager", true);
		if (pKey == NULL)
			return false;

		HKEY hkey = pKey->GetHandle();
		if (INVALID_HANDLE_VALUE == hkey)
			throw new LObjectDisposedException;

		larray<lstring> astrValues;
		RegGetMultiStringValue(hkey, L"PendingFileRenameOperations", astrValues);

		larray<lstring> astrNewValues(astrValues.GetLength() + 2);
		for (int i = 0 ; i < astrValues.GetLength() ; i++)
			astrNewValues[i] = astrValues[i];

		astrNewValues[astrValues.GetLength()] = strArg1;
		astrNewValues[astrValues.GetLength() + 1] = strArg2;

		RegSetMultiStringValue(hkey, L"PendingFileRenameOperations", astrNewValues);

		fRet = true;
	}
	catch (LException *pe)
	{
		OutputDebugStringW((PWCHAR)pe->ToString());
		delete pe;
	}
	if (NULL != pKey)
		delete pKey;

	return fRet;
}

void LInstallHelper::ClearRegistryKey(lstring strPath)
{
	LRegistryKey *pUserBaseKey = NULL;
	LRegistryKey *pMachinBaseKey = NULL;

	LRegistryKey *pUserProfileKey = NULL;

	// Machine Key
	do 
	{
		if (!strPath.Contains(L"\\"))
			break;

		try
		{
			pMachinBaseKey = LRegistryKey::OpenBaseKey(
				ERegistryHive::LocalMachine,
				Is64bitPlatform() ? ERegistryView::Registry64 : ERegistryView::Registry32);
		}
		catch (LException* pe)
		{
			delete pe;
		}

		if (NULL == pMachinBaseKey)
			break;

		if (!IsExistValueInRegistry(pMachinBaseKey, strPath))
		{
			LRegistryKey* pKey = NULL;
			try
			{
				lstring strKey = strPath.Substring(0, strPath.LastIndexOf(L"\\"));
				lstring strSubKey = strPath.Substring(strPath.LastIndexOf(L"\\") + 1);
				pKey = pMachinBaseKey->OpenSubKey(strKey, true);

				if (NULL != pKey)
					pKey->DeleteSubKeyTree(strSubKey, false);
			}
			catch (LException* pe)
			{
				delete pe;
			}

			if (NULL != pKey)
				delete pKey;
		}
	} while (false);

	// User Key
	do 
	{
		if (NULL == pMachinBaseKey)
			break;

		try
		{
			pUserBaseKey = LRegistryKey::OpenBaseKey(
				ERegistryHive::Users,
				Is64bitPlatform() ? ERegistryView::Registry64 : ERegistryView::Registry32);

			pUserProfileKey = pMachinBaseKey->OpenSubKey(L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList");
		}
		catch (LException* pe)
		{
			delete pe;
		}

		if (NULL == pUserProfileKey || NULL == pUserBaseKey)
			break;

		larray<lstring> astrSubKeys = pUserProfileKey->GetSubKeyNames();
		for (int i = 0; i < astrSubKeys.GetLength(); i++)
		{
			if (astrSubKeys[i].GetLength() <= LString(L"S-1-5-18").GetLength())
				continue;

			lstring strUserPath = astrSubKeys[i] + L"\\" + strPath;
			if (!IsExistValueInRegistry(pUserProfileKey, strUserPath))
			{
				LRegistryKey* pKey = NULL;
				try
				{
					lstring strKey = strUserPath.Substring(0, strUserPath.LastIndexOf(L"\\"));
					lstring strSubKey = strUserPath.Substring(strUserPath.LastIndexOf(L"\\") + 1);
			
					pKey = pUserBaseKey->OpenSubKey(strKey, true);

					if (NULL != pKey)
						pKey->DeleteSubKeyTree(strSubKey, false);
				}
				catch (LException* pe)
				{
					delete pe;
				}

				if (NULL != pKey)
					delete pKey;
			}
		}
	} while (false);

	if (NULL != pUserProfileKey)
		delete pUserProfileKey;
	if (NULL != pUserBaseKey)
		delete pUserBaseKey;
	if (NULL != pMachinBaseKey)
		delete pMachinBaseKey;
}

bool LInstallHelper::IsExistValueInRegistry(LRegistryKey* pKey, lstring strPath)
{
	bool fExistValue = false;

	LRegistryKey* pSubKey = NULL;

	do 
	{
		try
		{
			pSubKey = pKey->OpenSubKey(strPath);

			if (NULL == pKey)
				throw new LInvalidOperationException;

		}
		catch (LException* pe)
		{
			delete pe;
		}

		if (NULL == pSubKey)
			break;

		if (0 < pSubKey->GetValueCount())
		{
			fExistValue = true;
			break;
		}

		if (0 < pSubKey->GetSubKeyCount())
		{
			larray<lstring> astrSubKey = pSubKey->GetSubKeyNames();
			for (int i = 0; i < astrSubKey.GetLength(); i++)
			{
				fExistValue = LInstallHelper::IsExistValueInRegistry(pSubKey, astrSubKey[i]);

				if (fExistValue)
					break;
			}
		}

	} while (false);

	if (NULL != pSubKey)
	{
		OutputDebugStringW(LString::Format(L"LInstallHelper::IsExistValueInRegistry() - Path = %ws, Exist = %d", (const wchar_t*)pSubKey->GetName(), fExistValue));
		delete pSubKey;
	}

	return fExistValue;
}

void LInstallHelper::EnablePrivilege(LPCWSTR lpPrivilege, bool fEnable)
{
	HANDLE hToken = NULL;

	try
	{
		if (!::OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
			throw new LWin32Exception(::GetLastError());

		LUID luid;
		if (!::LookupPrivilegeValue(NULL, lpPrivilege, &luid))
			throw new LWin32Exception(::GetLastError());

		TOKEN_PRIVILEGES NewState;
		NewState.PrivilegeCount = 1;
		NewState.Privileges[0].Luid = luid;
		NewState.Privileges[0].Attributes = (fEnable ? SE_PRIVILEGE_ENABLED : 0);

		if (!::AdjustTokenPrivileges(hToken, FALSE, &NewState, 0, (PTOKEN_PRIVILEGES)NULL, 0))
			throw new LWin32Exception(::GetLastError());

		CloseHandle(hToken);
	}
	catch (...)
	{
		CloseHandle(hToken);

		throw;
	}
}

void LInstallHelper::TerminateExe( TCHAR *szExe )
{
	try
	{
		EnablePrivilege(SE_DEBUG_NAME, true);

		HANDLE handle = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0) ;
		if (INVALID_HANDLE_VALUE == handle)
			throw (::GetLastError());

		PROCESSENTRY32 entry;
		memset(&entry, 0, sizeof(PROCESSENTRY32));
		entry.dwSize = sizeof(PROCESSENTRY32) ;
		BOOL b = ::Process32First(handle, &entry);
		while(b)
		{
			if (0 == wcscmp(entry.szExeFile, szExe))
			{
				HANDLE handle = ::OpenProcess(PROCESS_TERMINATE, FALSE, entry.th32ProcessID);
				if (NULL != handle)
				{
					::TerminateProcess(handle, 0);
					::CloseHandle(handle);
				}
			}
			entry.dwSize = sizeof(PROCESSENTRY32) ;
			b = ::Process32Next(handle, &entry);
		}
		::CloseHandle(handle);

		EnablePrivilege(SE_DEBUG_NAME, false);
	}
	catch (DWORD /*dwError*/)
	{
	}
}

bool LInstallHelper::IsExeRunning(TCHAR *szExe)
{
	bool fRunning = false;

	try
	{
		EnablePrivilege(SE_DEBUG_NAME, true);

		HANDLE handle = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (INVALID_HANDLE_VALUE == handle)
			throw (::GetLastError());

		PROCESSENTRY32 entry;
		memset(&entry, 0, sizeof(PROCESSENTRY32));
		entry.dwSize = sizeof(PROCESSENTRY32);
		BOOL b = ::Process32First(handle, &entry);
		while (b)
		{
			if (0 == wcscmp(entry.szExeFile, szExe))
			{
				fRunning = true;
				break;
			}
			entry.dwSize = sizeof(PROCESSENTRY32);
			b = ::Process32Next(handle, &entry);
		}
		::CloseHandle(handle);

		EnablePrivilege(SE_DEBUG_NAME, false);
	}
	catch (DWORD /*dwError*/)
	{
	}

	return fRunning;
}

bool LInstallHelper::SystemReboot()
{
	bool fRet = false;
	HANDLE hToken = NULL;
	TOKEN_PRIVILEGES tkp;

	try
	{
		if (!::OpenProcessToken(::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
			throw new LWin32Exception(::GetLastError());

		if (!::LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid))
			throw new LWin32Exception(::GetLastError());

		tkp.PrivilegeCount = 1;
		tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

		if (!::AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0))
			throw new LWin32Exception(::GetLastError());

		if (!::ExitWindowsEx(EWX_REBOOT | EWX_FORCE, SHTDN_REASON_MAJOR_OPERATINGSYSTEM | SHTDN_REASON_MINOR_UPGRADE | SHTDN_REASON_FLAG_PLANNED))
			throw new LWin32Exception(::GetLastError());

		fRet = true;
	}
	catch (...)
	{
	}

	if (NULL != hToken && INVALID_HANDLE_VALUE != hToken)
		::CloseHandle(hToken);
	return fRet;
}

void LInstallHelper::SetServerHost(lstring str)
{
	LRegistryKey *pKey = NULL;
	for (int i = 0 ; i < 20 ; i++)
	{
		try
		{
			pKey = LRegistry::GetLocalMachine()->CreateSubKey(lstring(STRREGAPPDATA) + L"\\Server");
			if (pKey != NULL)
			{
				lstring strValue;
				if (pKey->GetValue(L"Server", strValue) &&
					pKey->GetValue(L"UpdateServer", strValue) &&
					pKey->GetValue(L"ServerList", strValue) &&
					pKey->GetValue(L"RegServer", strValue))
					break;
			}
		}
		catch (...)
		{
		}
		if (pKey != NULL)
		{
			delete pKey;
			pKey = NULL;
		}

		try
		{
			LRegistryKey *pKey = LRegistry::GetLocalMachine()->CreateSubKey(lstring(STRREGAPPDATA) + L"\\Server");
			if (pKey != NULL)
			{
				pKey->SetValue(L"Server", str);
				pKey->SetValue(L"UpdateServer", str);
				pKey->SetValue(L"ServerList", str);
				pKey->SetValue(L"RegServer", str);
			}
		}
		catch (...)
		{
		}
		if (pKey != NULL)
		{
			delete pKey;
			pKey = NULL;
		}
	}
}

bool LInstallHelper::CheckRebootAfterUninstall()
{
	LRegistryKey *pKey = NULL;
	bool fRet = false;
	try
	{
		pKey = LRegistry::GetLocalMachine()->OpenSubKey(L"SYSTEM\\CurrentControlSet\\Control\\Session Manager");
		if (pKey != NULL)
		{
			larray<lstring> astr;
			pKey->GetValue(L"PendingFileRenameOperations", astr);

			for (int i = 0 ; i < astr.GetLength() ; i++)
			{
				if (astr[i].ToLower().Contains(APPLICATION_LOWER_NAME))
				{
					fRet = true;
					break;
				}
			}
		}
	}
	catch (...)
	{
	}
	if (NULL != pKey)
		delete pKey;
	return fRet;
}

bool LInstallHelper::IsInstalledProduct()
{
	lstring strUninstallString;
	LRegistryKey *pBaseKey = NULL;
	LRegistryKey *pUninstallKey = NULL;

	try
	{
		pBaseKey = LRegistryKey::OpenBaseKey(
			ERegistryHive::LocalMachine, 
			Is64bitPlatform() ? ERegistryView::Registry64 : ERegistryView::Registry32
			);
		if (pBaseKey == NULL)
			throw new LApplicationException();

		pUninstallKey = pBaseKey->OpenSubKey(STRREGUNINSTALL);
		if (NULL != pUninstallKey)
			pUninstallKey->GetValue(L"UninstallString", strUninstallString);
	}
	catch (LException* pe)
	{
		LTrace::WriteLine(pe->ToString());
		delete pe;
	}

	if (NULL != pUninstallKey)
		delete pUninstallKey;
	if (NULL != pBaseKey)
		delete pBaseKey;

	return (!strUninstallString.IsEmpty());
}

void LInstallHelper::SetupServiceRecoveryOption(lstring strService)
{
	SC_HANDLE schSCManager = NULL;
	SC_HANDLE schService = NULL;

	SERVICE_FAILURE_ACTIONS sfa = {0};
	SC_ACTION sa[3];

	do
	{
		schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if (schSCManager == NULL)
			break;

		schService = OpenService(schSCManager, strService, SERVICE_ALL_ACCESS);
		if (schService == NULL)
			break;

		sa[0].Delay = 1000;
		sa[0].Type = SC_ACTION_RESTART;
		sa[1].Delay = 1000;
		sa[1].Type = SC_ACTION_RESTART;
		sa[2].Delay = 1000;
		sa[2].Type = SC_ACTION_RESTART;

		sfa.dwResetPeriod = 1 * 60 * 60 * 1000;
		sfa.lpRebootMsg = L"";
		sfa.lpCommand = L"";
		sfa.cActions = 3;
		sfa.lpsaActions = (SC_ACTION *)sa;

		ChangeServiceConfig2(schService, SERVICE_CONFIG_FAILURE_ACTIONS, &sfa);

	}
	while (false);

	if (schService != NULL)
		::CloseServiceHandle(schService);
	if (schSCManager != NULL)
		::CloseServiceHandle(schSCManager);
}

void LInstallHelper::ClearServiceRecoveryOption(lstring strService)
{
	SC_HANDLE schSCManager = NULL;
	SC_HANDLE schService = NULL;

	SERVICE_FAILURE_ACTIONS sfa = {0};
	SC_ACTION sa[3];

	do
	{
		schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if (schSCManager == NULL)
			break;

		schService = OpenService(schSCManager, strService, SERVICE_ALL_ACCESS);
		if (schService == NULL)
			break;

		sa[0].Delay = 1000 * 10;
		sa[0].Type = SC_ACTION_NONE;
		sa[1].Delay = 1000 * 10;
		sa[1].Type = SC_ACTION_NONE;
		sa[2].Delay = 1000 * 10;
		sa[2].Type = SC_ACTION_NONE;

		memset(sa, 0, sizeof(sa));

		sfa.dwResetPeriod = 1 * 60 * 60 * 1000;
		sfa.lpRebootMsg = L"";
		sfa.lpCommand = L"";
		sfa.cActions = 3;
		sfa.lpsaActions = (SC_ACTION *)sa;

		ChangeServiceConfig2(schService, SERVICE_CONFIG_FAILURE_ACTIONS, &sfa);

	}
	while (false);

	if (schService != NULL)
		::CloseServiceHandle(schService);
	if (schSCManager != NULL)
		::CloseServiceHandle(schSCManager);
}

bool LInstallHelper::Is64bitPlatform()
{
	bool f = false;

	BOOL fIsWow64;
	if (IsWow64Process(GetCurrentProcess(), &fIsWow64))
		f = fIsWow64;

	return f;
}

lstring LInstallHelper::GetPCId0()
{
	LStringBuilder sb;

	lstring str = LEnvironment::GetFolderPath(LESpecialFolder::System);
	str = str.Substring(0, str.IndexOf(L'\\') + 1);
	DWORD dwSerial = 0;
	if (!::GetVolumeInformationW(str, NULL, 0, &dwSerial, NULL, NULL, NULL, 0))
		throw new LWin32Exception(::GetLastError());
	sb.Append(LString::Format("%X", dwSerial));

	for (int n = sb.GetLength(); n < 56; n++)
		sb.Append(LConvert::ToString(n % 10));

	str = sb.ToString();
	if (56 < str.GetLength())
		str = str.Substring(0, 56);

	return str;
}

bool LInstallHelper::RegSetMultiStringValue( HKEY hKey, lstring strName, larray<lstring> astrValue )
{
	if (INVALID_HANDLE_VALUE == hKey)
		throw new LObjectDisposedException;

	DWORD dwType = REG_MULTI_SZ;
	DWORD cbData = 0;
	for (int n = 0; n < astrValue.GetLength(); n++)
	{
		cbData += (astrValue[n].GetLength() + 1) * sizeof(wchar_t);
	}
	cbData += sizeof(wchar_t);
	BYTE* pbData = new BYTE[cbData];
	DWORD dw = 0;
	memset(pbData, 0, cbData);
	for (int n = 0; n < astrValue.GetLength(); n++)
	{
		memcpy(&pbData[dw], (const wchar_t*)astrValue[n], (astrValue[n].GetLength()) * sizeof(wchar_t));
		dw += (astrValue[n].GetLength() + 1) * sizeof(wchar_t);
	}
	LONG lResult = RegSetValueExW(
		hKey,
		strName,
		NULL,
		dwType,
		pbData,
		cbData
		);
	if (ERROR_SUCCESS != lResult)
		throw new LWin32Exception(lResult);	

	delete []pbData;

	return true;
}

bool LInstallHelper::RegGetMultiStringValue( HKEY hKey, lstring strName, larray<lstring> &astrValue )
{
	if (INVALID_HANDLE_VALUE == hKey)
		throw new LObjectDisposedException;

	DWORD dwType;
	DWORD cbBuffer;
	LONG lResult = RegQueryValueExW(
		hKey,
		strName,
		NULL,
		&dwType,
		NULL,
		&cbBuffer
		);
	if (ERROR_SUCCESS != lResult)
		return false;

	if (REG_MULTI_SZ != dwType)
		throw new LArgumentException;

	BYTE* pb = new BYTE[cbBuffer];
	memset(pb, L'\0', cbBuffer);

	lResult = RegQueryValueExW(
		hKey,
		strName,
		NULL,
		NULL,
		pb,
		&cbBuffer
		);
	pb[cbBuffer - 1] = L'\0';

	if (ERROR_SUCCESS != lResult)
		throw new LWin32Exception(lResult);

	LList<lstring> lst;
	lstring str;

	DWORD dw = 0;
	while(dw < cbBuffer - 2)
	{
		str = (wchar_t*)&pb[dw];
		dw += (str.GetLength() + 1) * sizeof(wchar_t);
		lst.Add(str);
	}

	astrValue = larray<lstring>(lst.GetCount());
	lst.CopyTo(astrValue, 0);

	delete []pb;

	return true;
}

bool LInstallHelper::IsEmptyDirectory( lstring strDirectory, lstring strExceptFileExt )
{
	bool f = true;
	LDirectoryInfo di(strDirectory);
	if (di.Exists())
	{
		larray<LFileInfo*> apFI = di.GetFiles();
		if (apFI.GetLength() != 0)
		{
			f = false;
		}
		else
		{
			larray<LDirectoryInfo*> apDI = di.GetDirectories();
			for (int i = 0 ; i < apDI.GetLength() ; i++)
			{
				if (!IsEmptyDirectory(apDI[i]->GetFullName()))
				{
					f = false;
					break;
				}
			}
		}
	}
	return f;
}

/// ProductCode Sample
/// Visual C++ 2005 runtime files
/// SP1
/// x86 - {7299052b-02a4-4627-81f2-1818da5d550d}
/// x64 - {071c9b48-7c32-4621-a0ac-3f809523288f}
/// ATL UPDATE
/// x86 - {837b34e3-7c30-493c-8f6a-2b0f04e2912c}
/// x64 - {6ce5bae9-d3ca-4b99-891a-1dc6c118a5fc}
/// MFC SP1
/// x86 - {710f4c1c-cc18-4c49-8cbf-51240c89a1a2}
/// x64 - {ad8a2fa1-06e7-4b0d-927d-6e54b3d31028}
///
/// Visual C++ 2008 runtime files
/// SP1
/// x86 - {887868A2-D6DE-3255-AA92-AA0B5A59B874}
/// x64 - {92B8FD1F-C1AE-3750-8577-631B0AA85DF5}
/// SP1 EN
/// x86 - {9A25302D-30C0-39D9-BD6F-21E6EC160475}
/// x64 - {8220EEFE-38CD-377E-8595-13398D740ACE}
/// ATL UPDATE
/// x86 - {1F1C2DFC-2D24-3E06-BCB8-725134ADF989}
/// x64 - {4B6C7001-C7D6-3710-913E-5BC23FCE91E6}
/// MFC SP1
/// x86 - {9BE518E6-ECC6-35A9-88E4-87755C07200F}
/// x64 - {5FCE6D76-F5DC-37AB-B2B8-22AB8CEDB1D4}
///
/// Visual C++ 2010 runtime files
/// SP1
/// x86 - {F0C3E5D1-1ADE-321E-8167-68EF0DE699A5}, EstimatedSize 11394
/// x64 - {1D8E6291-B0D5-35EC-8441-6616F567A0F7}
/// MFC SP1
/// x86 - {F0C3E5D1-1ADE-321E-8167-68EF0DE699A5}, EstimatedSize 15363
/// x64 - {1D8E6291-B0D5-35EC-8441-6616F567A0F7}
///
/// Visual C++ 2013 runtime files
/// Normal
/// x86 - {13A4EE12-23EA-3371-91EE-EFB36DDFFF3E}
/// x64 - {A749D8E6-B613-3BE3-8F5F-045C84EBA29B}
///
/// Registry Path
/// x86 - HKLM \\ SOFTWARE \\ Microsoft \\ Windows \\ CurrentVersion \\ Uninstall 
/// x64 - HKLM \\ SOFTWARE \\ Wow6432 \\ Microsoft \\ Windows \\ CurrentVersion \\ Uninstall 
///		  HKLM \\ SOFTWARE \\ Microsoft \\ Windows \\ CurrentVersion \\ Uninstall 

bool LInstallHelper::IsInstalledRedist(lstring strProductName)
{
	bool fRet = false;

	LRegistryKey *pBaseKey = NULL;
	LRegistryKey *pUninstallKey = NULL;

	try
	{
		do 
		{
			pBaseKey = LRegistryKey::OpenBaseKey(
				ERegistryHive::LocalMachine,
				Is64bitPlatform() ? ERegistryView::Registry64 : ERegistryView::Registry32);

			if (NULL == pBaseKey)
				throw new LApplicationException(L"IsInstalledProduct() base key open failed.");

			lstring strKey = LString::Format(L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\%ws", (const wchar_t*)strProductName);
			pUninstallKey = pBaseKey->OpenSubKey(strKey);

			if (NULL != pUninstallKey)
			{
				fRet = true;
				break;
			}
			LTrace::WriteLine(LString::Format(L"IsInstalledProduct() Registry key open failed - %ws", (const wchar_t*)strKey));

			if (!Is64bitPlatform())
				break;

			strKey = LString::Format(L"Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\%ws", (const wchar_t*)strProductName);
			pUninstallKey = pBaseKey->OpenSubKey(strKey);

			if (NULL != pUninstallKey)
			{
				fRet = true;
				break;
			}
			LTrace::WriteLine(LString::Format(L"IsInstalledProduct() Registry key open failed - %ws", (const wchar_t*)strKey));

		} while (false);

	}
	catch (LException* pe)
	{
		LTrace::WriteLine(pe->ToString());
		delete pe;
	}
	
	if (NULL != pBaseKey)
		delete pBaseKey;
	if (NULL != pUninstallKey)
		delete pUninstallKey;

	LTrace::WriteLine(LString::Format(L"IsInstalledProduct() - %ws [%d]", (const wchar_t*)strProductName, fRet));
	return fRet;
}

bool LInstallHelper::IsinstalledDotnet35()
{
	bool fRet = false;

	LRegistryKey *pBaseKey = NULL;
	LRegistryKey *pSetupKey = NULL;

	try
	{
		pBaseKey = LRegistryKey::OpenBaseKey(
			ERegistryHive::LocalMachine,
			Is64bitPlatform() ? ERegistryView::Registry64 : ERegistryView::Registry32);

		if (NULL == pBaseKey)
			throw new LApplicationException(L"IsinstalledDotnet35() base key open failed.");

		lstring strKey = LString::Format(L"Software\\Microsoft\\NET Framework Setup\\NDP\\v3.5");
		pSetupKey = pBaseKey->OpenSubKey(strKey);

		if (NULL == pSetupKey)
			throw new LApplicationException(L"IsinstalledDotnet35() sub key open failed.");

		uint32 nValue = 0;
		pSetupKey->GetValue(L"Install", nValue);

		if (1 == nValue)
			fRet = true;
	}
	catch (LException* pe)
	{
		LTrace::WriteLine(pe->ToString());
		delete pe;
	}

	if (NULL != pBaseKey)
		delete pBaseKey;
	if (NULL != pSetupKey)
		delete pSetupKey;

	LTrace::WriteLine(LString::Format(L"IsinstalledDotnet35() - v3.5 [%d]", fRet));
	return fRet;
}

bool LInstallHelper::IsinstalledDotnet40()
{
	bool fRet = false;

	LRegistryKey *pBaseKey = NULL;
	LRegistryKey *pSetupKey = NULL;

	try
	{
		pBaseKey = LRegistryKey::OpenBaseKey(
			ERegistryHive::LocalMachine,
			Is64bitPlatform() ? ERegistryView::Registry64 : ERegistryView::Registry32);

		if (NULL == pBaseKey)
			throw new LApplicationException(L"IsinstalledDotnet40() base key open failed.");

		lstring strKey = LString::Format(L"Software\\Microsoft\\NET Framework Setup\\NDP\\v4\\Full");
		pSetupKey = pBaseKey->OpenSubKey(strKey);

		if (NULL == pSetupKey)
			throw new LApplicationException(L"IsinstalledDotnet40() sub key open failed.");

		uint32 nValue = 0;
		pSetupKey->GetValue(L"Install", nValue);

		if (1 == nValue)
			fRet = true;
	}
	catch (LException* pe)
	{
		LTrace::WriteLine(pe->ToString());
		delete pe;
	}

	if (NULL != pBaseKey)
		delete pBaseKey;
	if (NULL != pSetupKey)
		delete pSetupKey;

	LTrace::WriteLine(LString::Format(L"IsinstalledDotnet40() - v4 [%d]", fRet));
	return fRet;
}

lstring LInstallHelper::GetHash(lstring strPath)
{
	if (INVALID_FILE_ATTRIBUTES == ::GetFileAttributesW(strPath))
		return L"";

	lstring strHash;
	LHashAlgorithm* pHash = null;

	try
	{
		LFileStream fs(strPath, ELFileMode::Open, ELFileAccess::Read);
		
		pHash = LHashAlgorithm::Create(L"MD5");
		larray<byte> abHash = pHash->ComputeHash(&fs);
		strHash = LConvert::ToBase16String(abHash);
	}
	catch (LException* px)
	{
		LTrace::WriteLine(px->ToString());
		delete px;
	}

	if (pHash)
		delete pHash;

	return strHash;
}

void _DeleteFile(const lstring& strPath, bool fDirectory)
{
	lstring strDstPath = strPath + L".old";

	if (!::MoveFileW(strPath, strDstPath))
		LTrace::WriteLine(LString::Format(L"MoveFile Failed = %ws, %d", (const wchar_t*)strPath, ::GetLastError()));

	BOOL f = FALSE;
	if (fDirectory)
		f = ::RemoveDirectoryW(strDstPath);
	else
		f = ::DeleteFileW(strDstPath);

	if (FALSE == f)
	{
		if (!::MoveFileExW(strDstPath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT))
			LTrace::WriteLine(LString::Format(L"MoveFileEx Failed = %ws, %d", (const wchar_t*)strPath, ::GetLastError()));
	}
}

void LInstallHelper::DeleteAll(const lstring& strDirPath)
{
	DWORD dwAttr = ::GetFileAttributesW(strDirPath);
	if (INVALID_FILE_ATTRIBUTES != dwAttr && (FILE_ATTRIBUTE_DIRECTORY & dwAttr))
	{
		WIN32_FIND_DATAW find = { 0 };
		HANDLE hFind = ::FindFirstFileW(strDirPath + L"\\*", &find);
		if (INVALID_HANDLE_VALUE != hFind)
		{
			do
			{
				lstring strSubPath = strDirPath + L"\\" + find.cFileName;
				if (strSubPath.EndsWith(L".") || strSubPath.EndsWith(L".."))
					continue;

				bool fDirectory = (find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);

				if (fDirectory)
					DeleteAll(strSubPath);

				_DeleteFile(strSubPath, fDirectory);

			} while (::FindNextFileW(hFind, &find));

			::FindClose(hFind);
		}

		_DeleteFile(strDirPath, true);
	}
}
