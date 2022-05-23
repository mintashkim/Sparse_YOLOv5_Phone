#include "StdAfx.h"
#include <Library/API/DeviceAPI.h>
#include <Library/API/SecurityAPI.h>
#include <Library/API/FSAPI.h>
#include <Library/API/NetworkAPI.h>
#include <Library/WindowProfile.h>

#include <TlHelp32.h>
#include <WinIoCtl.h>
#include <ShTypes.h>

#include "wpp.h"
#include "DeviceAPI.tmh"

namespace Library
{
namespace API
{
struct TMN_REPARSE_DATA_BUFFER
{
	DWORD  ReparseTag;
	WORD   ReparseDataLength;
	WORD   Reserved;
	// IO_REPARSE_TAG_MOUNT_POINT specifics follow
	WORD   SubstituteNameOffset;
	WORD   SubstituteNameLength;
	WORD   PrintNameOffset;
	WORD   PrintNameLength;
	WCHAR  PathBuffer[1];
};

#define TMN_REPARSE_DATA_BUFFER_HEADER_SIZE \
	FIELD_OFFSET(TMN_REPARSE_DATA_BUFFER, SubstituteNameOffset)

#if !defined(FSCTL_SET_REPARSE_POINT) || \
	(FSCTL_SET_REPARSE_POINT != 0x900a4)
#undef FSCTL_SET_REPARSE_POINT
#define FSCTL_SET_REPARSE_POINT  CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 41, METHOD_BUFFERED, FILE_ANY_ACCESS)
#endif

#if !defined(FSCTL_DELETE_REPARSE_POINT) || \
		(FSCTL_DELETE_REPARSE_POINT != 0x900ac)
#undef FSCTL_DELETE_REPARSE_POINT
#define FSCTL_DELETE_REPARSE_POINT      CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 43, METHOD_BUFFERED, FILE_ANY_ACCESS)
#endif

void DeviceAPI::ShowDrive( string strDriveName, bool fNotify )
{
	RegistryKey* pKey = NULL;
	try
	{
		pKey = Registry::GetLocalMachine()->CreateSubKey(L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer");
		if (null != pKey)
		{
			uint32 uCurrentNoDrives = 0;
			pKey->GetValue(L"NoDrives", uCurrentNoDrives);
			if(0 == uCurrentNoDrives)
				pKey->GetValue(L"NoViewOnDrive", uCurrentNoDrives);

			uint32 uNoDrives = 1 << (uint32)(strDriveName.ToUpper().GetChars()[0] - L'A');
			pKey->SetValue(L"NoDrives", uCurrentNoDrives & ~uNoDrives);
			pKey->SetValue(L"NoViewOnDrive", uCurrentNoDrives & ~uNoDrives);

			pKey = WindowProfile::GetUserKey()->CreateSubKey(L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer");
			if (null != pKey)
			{
				pKey->SetValue(L"NoDrives", uCurrentNoDrives & ~uNoDrives);
				pKey->SetValue(L"NoViewOnDrive", uCurrentNoDrives & ~uNoDrives);
			}

			if (fNotify)
			{
				DWORD_PTR dwResult;
				SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0, 0, SMTO_ABORTIFHUNG, 100, &dwResult);

				string strLetter = strDriveName + L":\\";
				SHChangeNotify(SHCNE_DRIVEADD, SHCNF_PATH, (const wchar_t*)strLetter, NULL);
			}

			_TRACE_I(Application, L"Success to show drive = %ws", (const wchar_t*)strDriveName);
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
	if (pKey != NULL)
		delete pKey;
}

void DeviceAPI::HideDrive(string strDriveName, bool fNotify)
{
	RegistryKey *pKey = NULL;
	try
	{
		if (1 != strDriveName.GetLength())
			throw new ArgumentException();

		pKey = Registry::GetLocalMachine()->CreateSubKey(L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer");
		if (null != pKey)
		{
			uint32 uCurrentNoDrives = 0;
			pKey->GetValue(L"NoDrives", uCurrentNoDrives);
			if(0 == uCurrentNoDrives)
				pKey->GetValue(L"NoViewOnDrive", uCurrentNoDrives);

			uint32 uNoDrives = 1 << (uint32)(strDriveName.ToUpper().GetChars()[0] - L'A');
			pKey->SetValue(L"NoDrives", uCurrentNoDrives | uNoDrives);
			pKey->SetValue(L"NoViewOnDrive", uCurrentNoDrives | uNoDrives);

			pKey = WindowProfile::GetUserKey()->CreateSubKey(L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer");
			if (null != pKey)
			{
				pKey->SetValue(L"NoDrives", uCurrentNoDrives | uNoDrives);
				pKey->SetValue(L"NoViewOnDrive", uCurrentNoDrives | uNoDrives);
			}
			if (fNotify)
			{
				DWORD_PTR dwResult;
				SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0, 0, SMTO_ABORTIFHUNG, 100, &dwResult);

				string strLetter = strDriveName + L":\\";
				SHChangeNotify(SHCNE_DRIVEREMOVED, SHCNF_PATH | SHCNF_FLUSH, (const wchar_t*)strLetter, NULL);
			}

			_TRACE_I(Application, L"Success to hide drive = %ws", (const wchar_t*)strDriveName);
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
	if (pKey != NULL)
		delete pKey;
}

bool DeviceAPI::HideFolder(string strFilePath)
{
	DWORD dwAttr = ::GetFileAttributesW(strFilePath);
	BOOL bResult = FALSE;

	if (INVALID_FILE_ATTRIBUTES != dwAttr)
	{
		dwAttr = dwAttr | FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_NOT_CONTENT_INDEXED;
		bResult = !::SetFileAttributesW((const wchar_t*)strFilePath, dwAttr);

		if (!bResult)
			_TRACE_I(Application, L"SetFileAttributesW() Failed. | target : %ws | err : %d", (const wchar_t*)strFilePath, ::GetLastError());
	}
	if (bResult)
		return true;
	else
		return false;
}

void DeviceAPI::MakeFolder(string strPath, bool fUseLowSecurity)
{
	try 
	{
		if (INVALID_FILE_ATTRIBUTES == ::GetFileAttributesW(strPath))
		{
			if (!::CreateDirectoryW(strPath, NULL))
				throw new Win32Exception(::GetLastError());

			if(fUseLowSecurity)
			{
				Library::API::SecurityAPI::SetLowFileSecurity(strPath);
				Library::API::SecurityAPI::SetLowIntegrityFileLevel(strPath);
			}

			_TRACE_I(Application, L"Created Directory. path = %ws", (const wchar_t*)strPath);
		}
	}
	catch(Exception *pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}
	catch (...)
	{
		_TRACE_EX_UNK();
	}
}

void DeviceAPI::SetupFixedDriveAppearance(bool fHide, string strForceShowDrive)
{
	RegistryKey *pKey = NULL;
	try
	{
		pKey = Registry::GetLocalMachine()->CreateSubKey(L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer");
		if (NULL != pKey)
		{
			pKey->DeleteValue(L"NoDrives", false);
			pKey->DeleteValue(L"NoViewOnDrive", false);
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
	if (pKey != NULL)
		delete pKey;

	try
	{
		for (int i = 0 ; i < 26 ; i++)
		{
			wchar_t ch = L'A' + i;
			UINT nDriveType = Library::API::FSAPI::GetDriveType(string(ch));
			if (nDriveType == DRIVE_FIXED)
			{
				if (fHide && strForceShowDrive.GetChars()[0] != ch)
					HideDrive(ch);
			}
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
}

void DeviceAPI::SetupUnknownDriveAppearance(bool fHide, uint32 uUnknownDrives, string strExceptionDrive)
{
	_TRACE_I(Application, L"SetupUnknownDriveAppearance() - ENTRY");
	for (int i = 0; i < 26; i++)
	{
		uint32 driveIndex = (1 << i);

		if (driveIndex & uUnknownDrives)
		{
			wchar_t ch = L'A' + i;

			if (fHide && strExceptionDrive.GetChars()[0] != ch)
			{
				HideDrive(ch);
				_TRACE_I(Application, L"SetupUnknownDriveAppearance() - %ws:\\", (const wchar_t*)string(ch));
			}
		}
	}
	_TRACE_I(Application, L"SetupUnknownDriveAppearance() - EXIT");
}

void DeviceAPI::SetupNetShareDriveAppearance(bool fHide, uint32 uNetShareDrives, string strExceptionDrive)
{
	_TRACE_I(Application, L"SetupNetShareDriveAppearance() - ENTRY");
	for (int i = 0; i < 26; i++)
	{
		uint32 driveIndex = (1 << i);
		
		if (driveIndex & uNetShareDrives)
		{
			wchar_t ch = L'A' + i;

			if (fHide && strExceptionDrive.GetChars()[0] != ch)
			{
				HideDrive(ch);
				_TRACE_I(Application, L"SetupNetShareDriveAppearance() - %ws:\\", (const wchar_t*)string(ch));
			}
		}
	}
	_TRACE_I(Application, L"SetupNetShareDriveAppearance() - EXIT");
}

string DeviceAPI::GetUserFolderPath(int csidl)
{
	string strPath;
	try
	{
		array<wchar_t> awc(MAX_PATH);
		if (S_OK == ::SHGetFolderPathW(
			NULL, 
			csidl, 
			Library::WindowProfile::GetUserToken(), 
			NULL, 
			awc.GetBuffer()
			))
		{
			strPath = awc.GetBuffer();
		}
		else
		{
			_TRACE_I(Application, L"Failed to SHGetFolderPath");
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

	return strPath;
}

void DeviceAPI::SetUserShellFolderKey(const string& strName, const string& strValue)
{
	RegistryKey *pShellFolderKey = NULL;
	RegistryKey *pUserShellFolderKey = NULL;
	RegistryKey *pShellFolderWow6432Key = NULL;
	RegistryKey *pUserShellFolderWow6432Key = NULL;

	try
	{
		pShellFolderKey = Library::WindowProfile::GetUserKey()->CreateSubKey(L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders");
		if (pShellFolderKey != NULL)
		{
			pShellFolderKey->SetValue(strName, strValue);
			_TRACE_I(Application, L"User's ShellFolder, name = %ws", (const wchar_t*)strName);
			_TRACE_I(Application, L"User's ShellFolder, path = %ws", (const wchar_t*)strValue);
		}
	}
	catch (Exception* pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}

	try
	{
		pUserShellFolderKey = Library::WindowProfile::GetUserKey()->CreateSubKey(L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders");
		if (pUserShellFolderKey != NULL)
		{
			pUserShellFolderKey->SetValue(strName, strValue);
			_TRACE_I(Application, L"User's User ShellFolder, name = %ws", (const wchar_t*)strName);
			_TRACE_I(Application, L"User's User ShellFolder, path = %ws", (const wchar_t*)strValue);
		}
	}
	catch (Exception* pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}

#ifdef _M_X64
	try
	{
		pShellFolderWow6432Key = Library::WindowProfile::GetUserKey()->CreateSubKey(L"Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders");
		if (pShellFolderWow6432Key != NULL)
		{
			pShellFolderWow6432Key->SetValue(strName, strValue);
			_TRACE_I(Application, L"User's ShellFolder(Wow6432), name = %ws", (const wchar_t*)strName);
			_TRACE_I(Application, L"User's ShellFolder(Wow6432), path = %ws", (const wchar_t*)strValue);
		}
	}
	catch (Exception* pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}

	try
	{
		pUserShellFolderWow6432Key = Library::WindowProfile::GetUserKey()->CreateSubKey(L"Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders");
		if (pUserShellFolderWow6432Key != NULL)
		{
			pUserShellFolderWow6432Key->SetValue(strName, strValue);
			_TRACE_I(Application, L"User's User ShellFolder(Wow6432), name = %ws", (const wchar_t*)strName);
			_TRACE_I(Application, L"User's User ShellFolder(Wow6432), path = %ws", (const wchar_t*)strValue);
		}
	}
	catch (Exception* pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}
#endif
	
	if (pUserShellFolderWow6432Key != NULL)
		delete pUserShellFolderWow6432Key;
	if (pShellFolderWow6432Key != NULL)
		delete pShellFolderWow6432Key;
	if (pUserShellFolderKey != NULL)
		delete pUserShellFolderKey;
	if (pShellFolderKey != NULL)
		delete pShellFolderKey;
}

void DeviceAPI::SetUserKnownFolderPath(int csidl, string strPath, bool fUseLowSecurity /*= true*/)
{
	try
	{
		if (INVALID_FILE_ATTRIBUTES == ::GetFileAttributesW(strPath))
		{
			if (!::CreateDirectoryW(strPath, NULL))
				throw new Win32Exception(::GetLastError());
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

	if (fUseLowSecurity)
	{
		Library::API::SecurityAPI::SetLowFileSecurity(strPath);
		Library::API::SecurityAPI::SetLowIntegrityFileLevel(strPath);
	}
	
	HRESULT hr;
	if (S_OK != (hr = SHSetFolderPath(
		csidl, 
		Library::WindowProfile::GetUserToken(), 
		0, 
		strPath
		)))
	{
		_TRACE_I(Application, L"SHSetFolderPath failed. %d, %x", csidl, hr);
	}
	else
	{
		_TRACE_I(Application, L"SHSetFolderPath success. %d", csidl);
	}
}

void DeviceAPI::SetUserKnownFolderPath2(REFKNOWNFOLDERID rfid, string strPath, bool fUseLowSecurity /*= true*/)
{
	HMODULE hShell32 = LoadLibrary(L"shell32.dll");
	if(null == hShell32)
		return;

	typedef HRESULT (WINAPI *pSHSetKnownFolderPath) (REFKNOWNFOLDERID, DWORD, HANDLE, PCWSTR);
	pSHSetKnownFolderPath _SHSetKnownFolderPath = (pSHSetKnownFolderPath)::GetProcAddress(hShell32, "SHSetKnownFolderPath");
	if(null == _SHSetKnownFolderPath)
		return;

	try 
	{
		if (INVALID_FILE_ATTRIBUTES == ::GetFileAttributesW(strPath))
		{
			if (!::CreateDirectoryW(strPath, NULL))
				throw new Win32Exception(::GetLastError());
		}
	}
	catch(Exception *pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}
	catch (...)
	{
		_TRACE_EX_UNK();
	}

	if(fUseLowSecurity)
	{
		Library::API::SecurityAPI::SetLowFileSecurity(strPath);
		Library::API::SecurityAPI::SetLowIntegrityFileLevel(strPath);
	}
	
	HRESULT hr;
	if (S_OK != (hr = _SHSetKnownFolderPath(
		rfid,
		0,
		Library::WindowProfile::GetUserToken(),
		strPath
		)))
	{
		_TRACE_I(Application, L"SHSetKnownFolderPath failed. %x", hr);
	}
	else
	{
		_TRACE_I(Application, L"SHSetKnownFolderPath success.");
	}

	if (null != hShell32)
		::FreeLibrary(hShell32);
}

string DeviceAPI::GetCommonFolderPath(int csidl)
{
	string strPath;
	try
	{
		array<wchar_t> awc(MAX_PATH);
		if (S_OK == ::SHGetFolderPathW(
			NULL, 
			csidl, 
			NULL, 
			NULL, 
			awc.GetBuffer()
			))
		{
			strPath = awc.GetBuffer();
		}
		else
		{
			_TRACE_I(Application, L"Failed to SHGetFolderPath");
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

	return strPath;
}

void DeviceAPI::SetCommonShellFolderKey(const string& strName, const string& strValue)
{
	RegistryKey *pShellFolderKey = NULL;
	RegistryKey *pUserShellFolderKey = NULL;
	RegistryKey *pShellFolderWow6432Key = NULL;
	RegistryKey *pUserShellFolderWow6432Key = NULL;
	
	try
	{
		pShellFolderKey = Registry::GetLocalMachine()->CreateSubKey(L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders");
		if (pShellFolderKey != NULL)
		{
			pShellFolderKey->SetValue(strName, strValue);
			_TRACE_I(Application, L"Public ShellFolder, name = %ws", (const wchar_t*)strName);
			_TRACE_I(Application, L"Public ShellFolder, path = %ws", (const wchar_t*)strValue);
		}
	}
	catch (Exception* pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}

	try
	{
		pUserShellFolderKey = Registry::GetLocalMachine()->CreateSubKey(L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders");
		if (pUserShellFolderKey != NULL)
		{
			pUserShellFolderKey->SetValue(strName, strValue);
			_TRACE_I(Application, L"Public User ShellFolder, name = %ws", (const wchar_t*)strName);
			_TRACE_I(Application, L"Public User ShellFolder, path = %ws", (const wchar_t*)strValue);
		}
	}
	catch (Exception* pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}

#ifdef _M_X64
	try
	{
		pShellFolderWow6432Key = Registry::GetLocalMachine()->CreateSubKey(L"Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders");
		if (pShellFolderWow6432Key != NULL)
		{
			pShellFolderWow6432Key->SetValue(strName, strValue);
			_TRACE_I(Application, L"Public ShellFolder(Wow6432), name = %ws", (const wchar_t*)strName);
			_TRACE_I(Application, L"Public ShellFolder(Wow6432), path = %ws", (const wchar_t*)strValue);
		}
	}
	catch (Exception* pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}

	try
	{
		pUserShellFolderWow6432Key = Registry::GetLocalMachine()->CreateSubKey(L"Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders");
		if (pUserShellFolderWow6432Key != NULL)
		{
			pUserShellFolderWow6432Key->SetValue(strName, strValue);
			_TRACE_I(Application, L"Public User ShellFolder(Wow6432), name = %ws", (const wchar_t*)strName);
			_TRACE_I(Application, L"Public User ShellFolder(Wow6432), path = %ws", (const wchar_t*)strValue);
		}
	}
	catch (Exception* pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}
#endif
	
	if (pUserShellFolderWow6432Key != NULL)
		delete pUserShellFolderWow6432Key;
	if (pShellFolderWow6432Key != NULL)
		delete pShellFolderWow6432Key;
	if (pUserShellFolderKey != NULL)
		delete pUserShellFolderKey;
	if (pShellFolderKey != NULL)
		delete pShellFolderKey;
}

void DeviceAPI::SetCommonKnownFolderPath(int csidl, string strPath, bool fUseLowSecurity /*= true*/)
{
	try 
	{
		if (INVALID_FILE_ATTRIBUTES == ::GetFileAttributesW(strPath))
		{
			if (!::CreateDirectoryW(strPath, NULL))
				throw new Win32Exception(::GetLastError());
		}
	}
	catch(Exception *pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}
	catch (...)
	{
		_TRACE_EX_UNK();
	}

	if(fUseLowSecurity)
	{
		Library::API::SecurityAPI::SetLowFileSecurity(strPath);
		Library::API::SecurityAPI::SetLowIntegrityFileLevel(strPath);
	}

	HRESULT hr;
	if (S_OK != (hr = SHSetFolderPath(
		csidl,
		NULL,
		0,
		strPath
		)))
	{
		_TRACE_I(Application, L"Public SHSetFolderPath failed. %d, %x", csidl, hr);
	}
	else
	{
		_TRACE_I(Application, L"Public SHSetFolderPath success. %d", csidl);
	}
}

void DeviceAPI::SetCommonKnownFolderPath2(REFKNOWNFOLDERID rfid, string strPath, bool fUseLowSecurity /*= true*/)
{
	HMODULE hShell32 = LoadLibrary(L"shell32.dll");
	if(null == hShell32)
		return;

	typedef HRESULT (WINAPI *pSHSetKnownFolderPath) (REFKNOWNFOLDERID, DWORD, HANDLE, PCWSTR);
	pSHSetKnownFolderPath _SHSetKnownFolderPath = (pSHSetKnownFolderPath)::GetProcAddress(hShell32, "SHSetKnownFolderPath");
	if(null == _SHSetKnownFolderPath)
		return;

	try 
	{
		if (INVALID_FILE_ATTRIBUTES == ::GetFileAttributesW(strPath))
		{
			if (!::CreateDirectoryW(strPath, NULL))
				throw new Win32Exception(::GetLastError());
		}
	}
	catch(Exception *pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}
	catch (...)
	{
		_TRACE_EX_UNK();
	}

	if(fUseLowSecurity)
	{
		Library::API::SecurityAPI::SetLowFileSecurity(strPath);
		Library::API::SecurityAPI::SetLowIntegrityFileLevel(strPath);
	}

	HRESULT hr;
	if (S_OK != (hr = _SHSetKnownFolderPath(
		rfid,
		0,
		0,
		strPath
		)))
	{
		_TRACE_I(Application, L"Public SHSetKnownFolderPath failed. %x", hr);
	}
	else
	{
		_TRACE_I(Application, L"Public SHSetKnownFolderPath success.");
	}

	if (null != hShell32)
		::FreeLibrary(hShell32);
}

bool DeviceAPI::CreateJunctionPoint(string strMountPath, string strDestPath)
{
	if(strMountPath.IsEmpty() || strDestPath.IsEmpty())
		return false;
	
	try 
	{
		if(!strDestPath.StartsWith(L"\\??\\"))
		{
			array<wchar_t> awc(1024);
			LPWSTR pFilePart = 0;
			
			if (!::GetFullPathName(strDestPath, awc.GetLength(), awc.GetBuffer(), &pFilePart))
				return false;
			if (INVALID_FILE_ATTRIBUTES == ::GetFileAttributesW(awc.GetBuffer()))
				return false;

			strDestPath = string::Format(L"\\??\\%ws",(const wchar_t*) awc.GetBuffer());
			const size_t nDestMountPointBytes = strDestPath.GetLength() * 2;

			array <byte> awb(MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
			TMN_REPARSE_DATA_BUFFER *rdb = (TMN_REPARSE_DATA_BUFFER *)awb.GetBuffer();
			rdb->ReparseTag           = IO_REPARSE_TAG_MOUNT_POINT;
			rdb->ReparseDataLength    = nDestMountPointBytes + 12;
			rdb->Reserved             = 0;
			rdb->SubstituteNameOffset = 0;
			rdb->SubstituteNameLength = nDestMountPointBytes;
			rdb->PrintNameOffset      = nDestMountPointBytes + 2;
			rdb->PrintNameLength      = 0;

			lstrcpyW(rdb->PathBuffer, strDestPath);

			MakeFolder(strMountPath, true);

			HANDLE hMount = ::CreateFile(
				strMountPath,
				GENERIC_READ | GENERIC_WRITE,
				0,
				0,
				OPEN_EXISTING,
				FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
				0
				);

			DWORD dwBytes = 0;
			BOOL fResult = ::DeviceIoControl(
				hMount,
				FSCTL_SET_REPARSE_POINT,
				(LPVOID)rdb,
				rdb->ReparseDataLength + TMN_REPARSE_DATA_BUFFER_HEADER_SIZE,
				NULL,
				0,
				&dwBytes,
				0
				);

			if (INVALID_HANDLE_VALUE != hMount)
				::CloseHandle(hMount);
		}		
	}
	catch(Exception *pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}
	catch (...)
	{
		_TRACE_EX_UNK();
	}
	return true;
}

string DeviceAPI::GetTempPath()
{
	string strTempPath;
	DWORD dwRetVal = 0;

	wchar_t lpTempPathBuffer[MAX_PATH];
	dwRetVal = ::GetTempPathW(MAX_PATH,          // length of the buffer
		lpTempPathBuffer); // buffer for path 
	if (dwRetVal < MAX_PATH && dwRetVal > 0)
	{
		strTempPath = lpTempPathBuffer;
	}

	return strTempPath;
}

void DeviceAPI::SetTempPath(string strPath, bool fUseLowSecurity)
{
	try 
	{
		if (INVALID_FILE_ATTRIBUTES == ::GetFileAttributesW(strPath))
		{
			if (!::CreateDirectoryW(strPath, NULL))
				throw new Win32Exception(::GetLastError());
		}
	}
	catch(Exception *pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}
	catch (...)
	{
		_TRACE_EX_UNK();
	}

	if(fUseLowSecurity)
	{
		Library::API::SecurityAPI::SetLowFileSecurity(strPath);
		Library::API::SecurityAPI::SetLowIntegrityFileLevel(strPath);
	}

	RegistryKey *pEnvKey = NULL;
	try
	{
		pEnvKey = WindowProfile::GetUserKey()->CreateSubKey(L"Environment");
		if (pEnvKey != null)
		{
			pEnvKey->SetValue(L"TEMP", strPath);
			pEnvKey->SetValue(L"TMP", strPath);
		}
	}
	catch (Exception *pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}
	catch (...)
	{
		_TRACE_EX_UNK();
	}

	if(NULL != pEnvKey)
		delete pEnvKey;
}

array<string> DeviceAPI::GetNetworkPrinterAddresses()
{
	RegistryKey* pKey = null;
	RegistryKey* pMonitorKey = null;
	RegistryKey* pPrinterKey = null;
	List<string> lst;

	try
	{
		pKey = Registry::GetLocalMachine()->OpenSubKey(L"SYSTEM\\CurrentControlSet\\Control\\Print\\Monitors");
		if (null != pKey)
		{
			array<string> arstr = pKey->GetSubKeyNames();
			for (int n = 0; n < arstr.GetLength(); n++)
			{
				pMonitorKey = pKey->OpenSubKey(string::Format(L"%ws\\ports", (const wchar_t*)arstr[n]));
				if (null != pMonitorKey)
				{
					array<string> arstrsub = pMonitorKey->GetSubKeyNames();
					for (int m = 0; m < arstrsub.GetLength(); m++)
					{
						pPrinterKey = pMonitorKey->OpenSubKey(arstrsub[m]);
						if (null != pPrinterKey)
						{
							string strIP = L"";
							pPrinterKey->GetValue(L"IPAddress", strIP);
							_TRACE_I(Application, L"GetNetworkPrinterAddress() - IPAddress = %ws", strIP);

							if (strIP.IsEmpty() || !Library::API::NetworkAPI::validateIPv4Address(strIP))
							{
								pPrinterKey->GetValue(L"HostName", strIP);
								_TRACE_I(Application, L"GetNetworkPrinterAddress() - HostName = %ws", strIP);

								if (strIP.IsEmpty() || !Library::API::NetworkAPI::validateIPv4Address(strIP))
								{
									uint32 uIP = 0;
									pPrinterKey->GetValue(L"TargetAddress", uIP);

									if (0 != uIP)
									{
										strIP = String::Format(L"%d.%d.%d.%d",
											(uIP & 0x000000ff),
											(uIP & 0x0000ff00) >> 8,
											(uIP & 0x00ff0000) >> 16,
											(uIP & 0xff000000) >> 24);

										_TRACE_I(Application, L"GetNetworkPrinterAddress() - TargetAddress = %ws", strIP);
									}
								}
							}

							if(!strIP.IsEmpty() && Library::API::NetworkAPI::validateIPv4Address(strIP))
							{
								lst.Add(strIP);
								_TRACE_I(Application, L"AddNetworkPolicy : %ws", (const wchar_t*)strIP);
							}
							else
							{
								_TRACE_E(Application, L"Can't get IP address. PortName : %ws", (const wchar_t*)arstrsub[m]);
							}

							delete pPrinterKey;
							pPrinterKey = null;
						}
					}
					delete pMonitorKey;
					pMonitorKey = null;
				}
			}
			delete pKey;
			pKey = null;
		}
	}
	catch (Exception *pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}
	catch (...)
	{
		_TRACE_EX_UNK();
	}

	if (pPrinterKey)
		delete pPrinterKey;
	if (pMonitorKey)
		delete pMonitorKey;
	if (pKey)
		delete pKey;

	array<string> astr(lst.GetCount());
	lst.CopyTo(astr, 0);
	return astr;
}

}
}