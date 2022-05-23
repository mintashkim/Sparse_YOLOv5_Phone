#include "Functions.h"

void TerminateExe(TCHAR *szExe)
{
	try
	{
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
	}
	catch (DWORD /*dwError*/)
	{
	}
}

TCHAR* InstallVPNDriver(lstring strVPNCmdLine, lstring strVPNName)
{
	BOOL bResult = FALSE;
	STARTUPINFOW si = {0};
	PROCESS_INFORMATION pi = {0};
	memset(&si, 0, sizeof(si));
	memset(&pi, 0, sizeof(pi));
	si.cb = sizeof(si);

	TCHAR strarg[1024];
	memset(strarg, 0x00, sizeof(TCHAR) * 1024);
	popstring(strarg);

	TCHAR strCmdLine[1024];
	memset(strCmdLine, 0x00, sizeof(TCHAR) * 1024);

	wcscpy(strCmdLine, strVPNCmdLine);

	bResult = ::CreateProcessW(
		NULL,
		strCmdLine,
		NULL,
		NULL,
		FALSE,
		CREATE_NO_WINDOW,
		NULL,
		strarg,	// CurrentDirectory
		&si,
		&pi
		);

	TCHAR *result = L"0";
	if (WAIT_OBJECT_0 == ::WaitForSingleObject(pi.hProcess, INFINITE))
	{
		SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if (schSCManager != NULL)
		{
			SC_HANDLE hService = OpenService(schSCManager, strVPNName, SERVICE_QUERY_STATUS);
			if (hService != NULL)
				result = L"1";
		}
	}

	if (bResult)
	{
		::CloseHandle(pi.hProcess);
		::CloseHandle(pi.hThread);
	}
	return result;
}

void UninstallVPNDrivers(lstring strVPNCmdLine)
{
	BOOL bResult = FALSE;
	STARTUPINFOW si = {0};
	PROCESS_INFORMATION pi = {0};
	memset(&si, 0, sizeof(si));
	memset(&pi, 0, sizeof(pi));
	si.cb = sizeof(si);
	
	TCHAR strarg[1024];
	memset(strarg, 0x00, sizeof(TCHAR) * 1024);
	popstring(strarg);

	TCHAR strCmdLine[1024];
	memset(strCmdLine, 0x00, sizeof(TCHAR) * 1024);

	wcscpy(strCmdLine, strVPNCmdLine);

	bResult = ::CreateProcessW(
		NULL,
		strCmdLine,
		NULL,
		NULL,
		FALSE,
		CREATE_NO_WINDOW,
		NULL,
		strarg,	// CurrentDirectory
		&si,
		&pi
		);

	::WaitForSingleObject(pi.hProcess, INFINITE);

	if (bResult)
	{
		::CloseHandle(pi.hProcess);
		::CloseHandle(pi.hThread);
	}	
}

bool MoveDirectory(lstring strSource, lstring strDestination)
{
	WIN32_FIND_DATAW find;
	HANDLE hFind = FindFirstFileW((PWCHAR)(strSource + L"\\*"), &find);

	if (INVALID_HANDLE_VALUE == hFind )
		return false;

	lstring strSrc = strSource + L"\\" + find.cFileName;
	lstring strDst = strDestination + L"\\" + find.cFileName;

	if (find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{
		if (!LString(find.cFileName).Equals(L".") && !LString(find.cFileName).Equals(L".."))
		{
			if (!MoveDirectory(strSrc, strDst))
				return false;
		}
	}
	else
	{
		if (!AddPendingFileRenameOperations(L"\\??\\" + strSrc, L"\\??\\" + strDst))
			return false;
	}

	while (FindNextFileW(hFind, &find))
	{
		lstring strSrc = strSource + L"\\" + find.cFileName;
		lstring strDst = strDestination + L"\\" + find.cFileName;
		if (find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (!LString(find.cFileName).Equals(L".") && !LString(find.cFileName).Equals(L".."))
			{
				if (!MoveDirectory(strSrc, strDst))
					return false;
			}
		}
		else
		{
			if (!AddPendingFileRenameOperations(L"\\??\\" + strSrc, L"\\??\\" + strDst))
				return false;
		}
	}
	FindClose(hFind);

	return true;
}

bool DeleteDirectory(lstring strDirectory)
{
	WIN32_FIND_DATAW find;
	HANDLE hFind = FindFirstFileW((PWCHAR)(strDirectory + L"\\*"), &find);

	if (INVALID_HANDLE_VALUE == hFind )
		return false;

	lstring strSrc = strDirectory + L"\\" + find.cFileName;
	if (find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{
		if (!LString(find.cFileName).Equals(L".") && !LString(find.cFileName).Equals(L".."))
		{
			if (!DeleteDirectory(strSrc))
				return false;
		}
	}
	else
	{
		if (!strSrc.ToLower().Contains(L"\\uninst.exe"))
		{
			if (!AddPendingFileRenameOperations(L"\\??\\" + strSrc, LString()))
				return false;
		}
	}

	while (FindNextFileW(hFind, &find))
	{
		lstring strSrc = strDirectory + L"\\" + find.cFileName;
		if (find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (!LString(find.cFileName).Equals(L".") && !LString(find.cFileName).Equals(L".."))
			{
				if (!DeleteDirectory(strSrc))
					return false;
			}
		}
		else
		{
			if (!strSrc.ToLower().Contains(L"\\uninst.exe"))
			{
				if (!AddPendingFileRenameOperations(L"\\??\\" + strSrc, LString()))
					return false;
			}
		}
	}
	FindClose(hFind);

	return true;
}

bool AddPendingFileRenameOperations(lstring strArg1, lstring strArg2)
{
	try
	{
		LRegistryKey *pKey = LRegistry::GetLocalMachine()->OpenSubKey(L"SYSTEM\\CurrentControlSet\\Control\\Session Manager", true);
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

		pKey->Close();
		return true;
	}
	catch (LException *pe)
	{
		OutputDebugStringW((PWCHAR)pe->ToString());
		delete pe;
		return false;
	}
}

bool RegSetMultiStringValue(HKEY hKey, lstring strName, larray<lstring> astrValue)
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

bool RegGetMultiStringValue(HKEY hKey, lstring strName, larray<lstring> &astrValue)
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

bool CheckIncops()
{
	LRegistryKey *pKey = null;
	bool fRet = false;
	try
	{
		pKey = LRegistry::GetLocalMachine()->OpenSubKey(L"SYSTEM\\CurrentControlSet\\Services\\ICTrigger");

		if (null != pKey)
		{
			fRet = _SetErrorMode();
			delete pKey;
		}
	}
	catch (LException* pe)
	{
		delete pe;
	}

	return fRet;
}

bool _SetErrorMode()
{
	LRegistryKey *pKey = null;
	bool fRet = false;
	try
	{
		pKey = LRegistry::GetLocalMachine()->OpenSubKey(L"SYSTEM\\CurrentControlSet\\Control\\Windows", true);

		if (null != pKey)
		{
			pKey->SetValue(L"ErrorMode", (uint32)2);
			fRet = true;
			delete pKey;
		}
	}
	catch (LException* pe)
	{
		delete pe;
	}

	return fRet;
}

