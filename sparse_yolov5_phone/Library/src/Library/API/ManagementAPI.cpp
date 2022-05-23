#include "StdAfx.h"
#include <Library/API/ManagementAPI.h>
#include <Library/API/DeviceAPI.h>
#include <Library/API/FSAPI.h>
#include <Library/API/RSAPI.h>
#include <Library/API/ProcessAPI.h>
#include <Library/XmlObject2.h>
#include <Library/WindowProfile.h>
#include <System/IO/all.h>
#include <shlobj.h>

#include "wpp.h"
#include "ManagementAPI.tmh"

namespace Library
{
namespace API
{

bool ManagementAPI::DisableAllControlPanels(array<string> astrExceptIDs)
{
	bool f = false;
	RegistryKey *pPoliciesKey = NULL;
	RegistryKey *pExplorerKey = NULL;
	RegistryKey *pDisallowCplKey = NULL;

	array<string> astrNames = GetAllControlPanelNames(astrExceptIDs);

	try
	{
		do 
		{			
			pPoliciesKey = Library::WindowProfile::GetUserKey()->OpenSubKey(L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies", true);
			if (pPoliciesKey == NULL)
				break;

			pExplorerKey = pPoliciesKey->CreateSubKey(L"Explorer");
			if (pExplorerKey == NULL)
				break;

			pDisallowCplKey = pExplorerKey->CreateSubKey(L"DisallowCpl");
			if (pDisallowCplKey == NULL)
				break;

			DWORD dwValue = 1;
			pExplorerKey->SetValue(L"DisallowCpl", dwValue);

			for (int i = 0 ; i < astrNames.GetLength() ; i++)
			{
				if (!astrNames[i].IsEmpty())
				{
					pDisallowCplKey->SetValue(Convert::ToString(i), astrNames[i]);
					_TRACE_I(Application, L"Hide Control Panel (%ws)", (const wchar_t*)astrNames[i]);
				}
			}

			f = true;
		} 
		while (false);
	}
	catch (Exception *pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}

	if (pDisallowCplKey != NULL)
		delete pDisallowCplKey;
	if (pExplorerKey != NULL)
		delete pExplorerKey;
	if (pPoliciesKey != NULL)
		delete pPoliciesKey;

	return f;
}

bool ManagementAPI::DisablecontrolPanelsXpStyle(array<string> astrCpls)
{
	bool f = false;
	RegistryKey *pControlPanelKey = NULL;
	RegistryKey *pDontLoadKey = NULL;

	try
	{
		do 
		{
			pControlPanelKey = Library::WindowProfile::GetUserKey()->OpenSubKey(L"Control Panel", true);
			if (pControlPanelKey == NULL)
				break;

			pDontLoadKey = pControlPanelKey->CreateSubKey(L"don't load", true);
			if (pDontLoadKey == NULL)
				break;

			string strValue = L"No";
			for (int i = 0 ; i < astrCpls.GetLength() ; i++)
			{
				pDontLoadKey->SetValue(astrCpls[i], strValue);
				_TRACE_I(Application, L"Hide Control Panel (%ws)", (const wchar_t*)astrCpls[i]);
			}
		} 
		while (false);
	}
	catch (Exception* pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}

	if (pDontLoadKey != NULL)
		delete pDontLoadKey;
	if (pControlPanelKey != NULL)
		delete pControlPanelKey;

	return f;
}

bool ManagementAPI::DisableDisplaySetting()
{
	bool f = false;
	RegistryKey *pShellKey = NULL;
	RegistryKey *pPoliciesKey = NULL;

	try
	{
		do 
		{
			if (Version(6, 0) <= Environment::GetOSVersion()->GetVersion())
			{
				pShellKey = Registry::GetClassesRoot()->OpenSubKey(L"DesktopBackground\\Shell", true);
				//pShellKey = Registry::GetLocalMachine()->OpenSubKey(L"SOFTWARE\\Classes\\DesktopBackground\\Shell", true);
				if (pShellKey == NULL)
					break;

				pShellKey->DeleteSubKey(L"Display");
				pShellKey->DeleteSubKey(L"Gadgets");
				//pShellKey->DeleteSubKey(L"Personalize");
			}
			else
			{
				pPoliciesKey = Library::WindowProfile::GetUserKey()->CreateSubKey(L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System");
				if (pPoliciesKey == NULL)
					break;
				
				pPoliciesKey->SetValue(L"NoDispAppearancePage", (DWORD)1);
				pPoliciesKey->SetValue(L"NoDispSettingPage", (DWORD)1);
			}
			_TRACE_I(Application, L"Hide Display Setting Panel");
			f = true;
		} 
		while (false);
	}
	catch (Exception* pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}

	if (pShellKey != NULL)
		delete pShellKey;
	if (pPoliciesKey != NULL)
		delete pPoliciesKey;

	return f;
}

array<string> ManagementAPI::GetAllControlPanelNames( array<string> astrExceptIDs )
{
	List<string> lstrControlPanelNames;
	RegistryKey *pCLSIDKey = NULL;
	RegistryKey *pNamespaceKey = NULL;

	try
	{
		do 
		{
			pCLSIDKey = Registry::GetClassesRoot()->OpenSubKey(L"CLSID");
			if (pCLSIDKey == NULL)
				break;

			pNamespaceKey = Registry::GetLocalMachine()->OpenSubKey(L"SoftWare\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ControlPanel\\NameSpace");
			if (pNamespaceKey == NULL)
				break;

			array<string> astrSubkeys = pNamespaceKey->GetSubKeyNames();
			for (int i = 0 ; i < astrSubkeys.GetLength() ; i++)
			{
				if (!astrSubkeys[i].StartsWith(L"{") || !astrSubkeys[i].EndsWith(L"}"))
					continue;

				bool fInclude = true;
				for (int j = 0 ; j < astrExceptIDs.GetLength() ; j++)
				{
					if (astrSubkeys[i].ToLower() == astrExceptIDs[j].ToLower())
					{
						fInclude = false;
						break;
					}
				}

				if (!fInclude)
					continue;

				string strDefaultName;
				RegistryKey *pChildKey = NULL;
				RegistryKey *pGUIDKey = NULL;

				try
				{
					pChildKey = pNamespaceKey->OpenSubKey(astrSubkeys[i]);
					if (pChildKey != NULL)
						pChildKey->GetValue(L"", strDefaultName);

					if (!strDefaultName.IsEmpty())
						lstrControlPanelNames.Add(strDefaultName);
						
					pGUIDKey = pCLSIDKey->OpenSubKey(astrSubkeys[i]);
					if (pGUIDKey != NULL)
					{
						string strValue;
						LoadMUIString(pGUIDKey->GetHandle(), L"LocalizedString", strValue, L"");
						if (!strValue.IsEmpty())
							lstrControlPanelNames.Add(strValue);
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

				if (pGUIDKey != NULL)
					delete pGUIDKey;
				if (pChildKey != NULL)
					delete pChildKey;
			}
		} 
		while (false);
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

	if (pCLSIDKey != NULL)
		delete pCLSIDKey;
	if (pNamespaceKey != NULL)
		delete pNamespaceKey;

	array<string> astrReturn(lstrControlPanelNames.GetCount());
	lstrControlPanelNames.CopyTo(astrReturn, 0);
	return astrReturn;
}

bool ManagementAPI::LoadMUIString(HKEY hKey, string strName, string& strValue, string strDirectory)
{
	bool fRet = false;
	HMODULE hModule = NULL;
	try
	{
		if (INVALID_HANDLE_VALUE == hKey)
			throw new ObjectDisposedException;

		typedef LONG (WINAPI* PFRegLoadMUIStringW)(
			__in       HKEY hKey,
			__in_opt   LPCWSTR pszValue,
			__out_opt  LPWSTR pszOutBuf,
			__in       DWORD cbOutBuf,
			__out_opt  LPDWORD pcbData,
			__in       DWORD Flags,
			__in_opt   LPCWSTR pszDirectory
			);

		hModule = ::LoadLibraryW(L"advapi32.dll");
		if (NULL == hModule)
			throw new Win32Exception(::GetLastError());

		PFRegLoadMUIStringW pfRegLoadMUIStringW = (PFRegLoadMUIStringW)::GetProcAddress(hModule, "RegLoadMUIStringW");
		if (NULL == pfRegLoadMUIStringW)
			throw new Win32Exception(::GetLastError());

		DWORD cbData = 0;
		LONG lResult = pfRegLoadMUIStringW(
			hKey,
			strName,
			NULL,
			0,
			&cbData,
			0,
			strDirectory.IsEmpty() ? NULL : (LPWSTR)strDirectory
			);

		if (ERROR_MORE_DATA == lResult)
		{
			array<wchar_t> awc(cbData / sizeof(wchar_t));
			lResult = pfRegLoadMUIStringW(
				hKey,
				strName,
				awc.GetBuffer(),
				cbData,
				&cbData,
				0,
				strDirectory.IsEmpty() ? NULL : (LPCWSTR)strDirectory
				);
			if (ERROR_SUCCESS != lResult)
				throw new Win32Exception(lResult);

			strValue = string(awc, 0, cbData / sizeof(wchar_t) - 1);
			fRet = true;
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
	
	if (NULL != hModule)
		::FreeLibrary(hModule);
	return fRet;
}

bool ManagementAPI::HideNetworkIcon()
{
	RegistryKey *pPoliciesKey = NULL;
	RegistryKey *pCommonPoliciesKey = NULL;

	try
	{
		pPoliciesKey = Library::WindowProfile::GetUserKey()->OpenSubKey(L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer", true);
		if (pPoliciesKey != NULL)
		{
			pPoliciesKey->SetValue(L"HideSCANetwork", (DWORD)0x00000001);
			_TRACE_I(Application, L"Hide User Network Icon");
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

	try
	{
		pCommonPoliciesKey = Registry::GetLocalMachine()->OpenSubKey(L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer", true);
		if (pCommonPoliciesKey != NULL)
		{
			pCommonPoliciesKey->SetValue(L"HideSCANetwork", (DWORD)0x00000001);
			_TRACE_I(Application, L"Hide Common Network Icon");
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
	
	if (pCommonPoliciesKey != NULL)
		delete pCommonPoliciesKey;
	if (pPoliciesKey != NULL)
		delete pPoliciesKey;

	return true;
}

bool ManagementAPI::SetupFolderHideOption(bool fHide)
{
	RegistryKey *pExplorerKey = NULL;

	try
	{
		pExplorerKey = Library::WindowProfile::GetUserKey()->OpenSubKey(L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced", true);
		if (pExplorerKey != NULL)
		{
			if (fHide)
			{
				pExplorerKey->SetValue(L"Hidden", (uint32)2);
				_TRACE_I(Application, L"Set Folder Hide Option, Hidden");
			}
			else
			{
				pExplorerKey->SetValue(L"Hidden", (uint32)1);
				_TRACE_I(Application, L"Set Folder Hide Option, Shown");
			}
		}
	}
	catch (Exception* pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}	

	if (pExplorerKey != NULL)
		delete pExplorerKey;
	return true;
}

void ManagementAPI::DeleteBluetoothIcon()
{
	RegistryKey *pDesktopNamespaceKey = NULL;
	try
	{
		pDesktopNamespaceKey = Registry::GetLocalMachine()->OpenSubKey(L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Desktop\\NameSpace", true);
		if (NULL != pDesktopNamespaceKey)
		{
			pDesktopNamespaceKey->DeleteSubKeyTree(L"{6af09ec9-b429-11d4-a1fb-0090960218cb}", false);
			pDesktopNamespaceKey->DeleteSubKeyTree(L"{32b4c379-4ac0-45f2-939c-d4e7ada56dc5}", false);
			pDesktopNamespaceKey->DeleteSubKeyTree(L"{B9AF185E-1276-4BC7-936B-692E16B1AD4A}", false);
			_TRACE_I(Application, L"Hide Bluetooth Icon");
		}
	}
	catch (Exception* pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}
	if (pDesktopNamespaceKey != NULL)
		delete pDesktopNamespaceKey;
}

// 휴지통 없애기
// 휴지통 지원이 정식적으로 되기 까지만
// Sean
void ManagementAPI::DisableRecycleBin()
{
	RegistryKey *pDesktopNamespaceKey = NULL;
	RegistryKey *pPoliciesNonEnumKey = NULL;
	RegistryKey *pExplorerBitBucketKey = NULL;
	RegistryKey *pExplorerBitBucketVolumeKey = NULL;
	RegistryKey *pCommonExplorerBitBucketKey = NULL;

	try
	{
		pDesktopNamespaceKey = Registry::GetLocalMachine()->OpenSubKey(L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Desktop\\NameSpace", true);
		if (pDesktopNamespaceKey != NULL)
			pDesktopNamespaceKey->DeleteSubKeyTree(L"{645FF040-5081-101B-9F08-00AA002F954E}", false);
	}
	catch (Exception* pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}

	try
	{
		pPoliciesNonEnumKey = Library::WindowProfile::GetUserKey()->CreateSubKey(L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\NonEnum");
		if (NULL != pPoliciesNonEnumKey)
			pPoliciesNonEnumKey->SetValue(L"{645FF040-5081-101B-9F08-00AA002F954E}", (uint32)1);
	}
	catch (Exception* pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}
	
	// 가상디스크가 마운트 매니저의 등록하는 방식으로 변경함에 따라
	// 휴지통을 사용하지 않으려면 추가 수정이 필요함
	// redmine #5458, SEAN
	try
	{
		pExplorerBitBucketKey = Library::WindowProfile::GetUserKey()->CreateSubKey(L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\BitBucket");
		if (NULL != pExplorerBitBucketKey)
		{
			pExplorerBitBucketKey->SetValue(L"NukeOnDelete", (uint32)1);
			pExplorerBitBucketKey->SetValue(L"UseGlobalSettings", (uint32)1);
		}
	}
	catch (Exception* pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}
	
	try
	{
		pExplorerBitBucketVolumeKey = Library::WindowProfile::GetUserKey()->OpenSubKey(L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\BitBucket\\Volume", true);
		if (NULL != pExplorerBitBucketVolumeKey)
		{
			array<string> arstrSubKeys = pExplorerBitBucketVolumeKey->GetSubKeyNames();
			int nCount = arstrSubKeys.GetLength();

			for (int n = 0; n < nCount; n++)
			{
				RegistryKey *pSubKey = pExplorerBitBucketVolumeKey->OpenSubKey(arstrSubKeys[n], true);
				if (NULL != pSubKey)
				{
					pSubKey->SetValue(L"NukeOnDelete", (uint32)1);
					delete pSubKey;
				}
			}
			_TRACE_I(Application, L"VDInit Force Delete [CurrentUser] Successed");
		}
	}
	catch (Exception* pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}

	try
	{
		pCommonExplorerBitBucketKey = Registry::GetLocalMachine()->CreateSubKey(L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\BitBucket");
		if (NULL != pCommonExplorerBitBucketKey)
		{
			pCommonExplorerBitBucketKey->SetValue(L"NukeOnDelete", (uint32)1);
			pCommonExplorerBitBucketKey->SetValue(L"UseGlobalSettings", (uint32)1);
			_TRACE_I(Application, L"VDInit Force Delete Successed");
		}
	}
	catch (Exception* pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}

	// 휴지통 사용 가능함.
	// 이전 버전에서 업데이트 한 경우 가상 영역의 휴지통을 한번은 초기화 시켜주어야 함.
	// Sean
	// Redmine #4669, #4104, #3039
	// vDesk 이슈가 처리되면 다시 주석 해제할 것

	//bool fInitRecycleBin = false;

	//try
	//{
	//	uint32 n = 0;
	//	Application::GetUserAppDataRegistry()->GetValue(L"InitRecycleBin", (uint32)n);
	//	if (1 != n)
	//		fInitRecycleBin = true;
	//}
	//catch(Exception* pe)
	//{
	//}

	//try
	//{
	//	if (fCreateNew || fInitRecycleBin)
	//	{
	//		SHQUERYRBINFO sqrb;
	//		memset(&sqrb, 0, sizeof(SHQUERYRBINFO));
	//		sqrb.cbSize = sizeof(SHQUERYRBINFO);

	//		array<DriveInfo*> apdi = DriveInfo::GetDrives();

	//		for (int n = 0; n < apdi.GetLength(); n++)
	//			HRESULT hr = ::SHEmptyRecycleBinW(NULL, apdi[n]->GetName(), SHERB_NOCONFIRMATION | SHERB_NOPROGRESSUI | SHERB_NOSOUND);

	//		Application::GetUserAppDataRegistry()->SetValue(L"InitRecycleBin", (uint32)1);
	//	}
	//}
	//catch(Exception* pe)
	//{
	//	delete pe;
	//}
	
	if (pCommonExplorerBitBucketKey != NULL)
		delete pCommonExplorerBitBucketKey;
	if (pExplorerBitBucketVolumeKey != NULL)
		delete pExplorerBitBucketVolumeKey;
	if (pExplorerBitBucketKey != NULL)
		delete pExplorerBitBucketKey;
	if (pPoliciesNonEnumKey != NULL)
		delete pPoliciesNonEnumKey;
	if (pDesktopNamespaceKey != NULL)
		delete pDesktopNamespaceKey;
}

void ManagementAPI::CreateDesktopUpdateKey()
{
	RegistryKey *pControlKey = NULL;
	RegistryKey *pUpdateKey = NULL;

	try
	{
		pControlKey = Registry::GetLocalMachine()->CreateSubKey(L"SYSTEM\\CurrentControlSet\\Control");
		if (pControlKey != NULL)
		{
			pUpdateKey = pControlKey->CreateSubKey(L"Update");
			if (pUpdateKey != NULL)
			{
				pUpdateKey->SetValue(L"UpdateMode", (uint32)0);
				_TRACE_I(Application, L"Created Desktop Update Key.");
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

	if (pUpdateKey != NULL)
		delete pUpdateKey;
	if (pControlKey != NULL)
		delete pControlKey;
}

void ManagementAPI::DisablePrintMenuOnStart()
{
	RegistryKey* pStartShowPrinters = NULL;
	RegistryKey* pShowPrinters = NULL;

	try
	{
		pStartShowPrinters = Library::WindowProfile::GetUserKey()->CreateSubKey(L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced");
		if (NULL != pStartShowPrinters)
		{
			pStartShowPrinters->SetValue(L"Start_ShowPrinters", (uint32)0);
			_TRACE_I(Application, L"Start_ShowPrinters Set Value (0) Successed");
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

	try
	{
		pShowPrinters = Registry::GetLocalMachine()->OpenSubKey(L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartMenu\\StartPanel",true);
		if (NULL != pShowPrinters)
		{
			pShowPrinters->DeleteSubKeyTree(L"ShowPrinters", false);
			_TRACE_I(Application, L"VDInit StartPanel-ShowPrinters Delete Successed");
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

	if (NULL != pStartShowPrinters)
		delete pStartShowPrinters;
	if (NULL != pShowPrinters)
		delete pShowPrinters;
}

void ManagementAPI::RemoveLinkFolderOfUser()
{
	if (Version(6, 0) > Environment::GetOSVersion()->GetVersion())
		return;

	RegistryKey *pKey = NULL;
	try
	{
		pKey = Library::WindowProfile::GetUserKey()->OpenSubKey(L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders");
		if (NULL != pKey)
		{
			string strValue;
			if (pKey->GetValue(L"{BFB9D5E0-C6A9-404C-B2B2-AE6DB6AF4968}", strValue))
			{
				DirectoryInfo di(strValue);
				if (di.Exists())
				{
					array<FileInfo*> arFiles = di.GetFiles();
					for (int n = 0; n < arFiles.GetLength() ; n++)
					{
						string strFileName = arFiles[n]->GetFullName();
						if (!::DeleteFileA(strFileName))
							_TRACE_I(Application, L"%ws Delete failed = %d", (const wchar_t*)strFileName, ::GetLastError());

						delete arFiles[n];
					}
				}
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
	if (pKey != NULL)
		delete pKey;
}

void ManagementAPI::DeleteLibrariesAndLinksOfUser()
{
	if (Version(6, 0) > Environment::GetOSVersion()->GetVersion())
		return;

	try
	{
		string str = Library::API::DeviceAPI::GetUserFolderPath(CSIDL_APPDATA) + L"\\Microsoft\\Windows\\Libraries";

		_TRACE_I(Application, L"DeleteLibrariesAndLinksOfUser");
		_TRACE_I(Application, L"%ws", (const wchar_t*)str);

		DirectoryInfo di(str);
		if (di.Exists())
		{
			array<FileInfo*> apFi = di.GetFiles();
			for (int i = 0 ; i < apFi.GetLength() ; i++)
			{
				if (apFi[i]->GetName().ToLower().EndsWith(L".library-ms"))
					::DeleteFileW(apFi[i]->GetFullName());
				delete apFi[i];
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

	try
	{
		string str = Library::WindowProfile::GetUserProfilePath() + L"\\Links";
		
		_TRACE_I(Application, L"DeleteLibrariesAndLinksOfUser");
		_TRACE_I(Application, L"%ws", (const wchar_t*)str);

		DirectoryInfo di(str);
		if (di.Exists())
		{
			array<FileInfo*> apFi = di.GetFiles();
			for (int i = 0 ; i < apFi.GetLength() ; i++)
			{
				if (apFi[i]->GetName().ToLower().EndsWith(L".lnk"))
					::DeleteFileW(apFi[i]->GetFullName());
				delete apFi[i];
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

void ManagementAPI::SetShowNetworkList(bool fShow)
{
	bool fRet = false;
	
	DWORD dwAttributes = fShow ? 0xb0040064 : 0xb0940064;
	string strNetworkListRegPath = L"CLSID\\{F02C1A0D-BE21-4350-88B0-7367FC96EF3C}\\ShellFolder";

	RegistryKey* pKey = NULL;
	try
	{
		Library::API::RSAPI::ChangeRegSecurityOption(ERegistryHive::ClassesRoot, strNetworkListRegPath, L"SYSTEM");
		pKey = Registry::GetClassesRoot()->OpenSubKey(strNetworkListRegPath, true);

		if (NULL != pKey)
		{
			pKey->SetValue(L"Attributes", dwAttributes);
			fRet = true;
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

	if (NULL != pKey)
		delete pKey;

#ifdef _M_X64
	pKey = NULL;
	strNetworkListRegPath = L"Wow6432Node\\CLSID\\{F02C1A0D-BE21-4350-88B0-7367FC96EF3C}\\ShellFolder";

	try
	{
		Library::API::RSAPI::ChangeRegSecurityOption(ERegistryHive::ClassesRoot, strNetworkListRegPath, L"SYSTEM");
		pKey = Registry::GetClassesRoot()->OpenSubKey(strNetworkListRegPath, true);

		if (NULL != pKey)
		{
			pKey->SetValue(L"Attributes", dwAttributes);
			fRet = true;
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

	if (NULL != pKey)
		delete pKey;
#endif

	_TRACE_I(Application, L"VDInit SetShowNetworkList [%d], Result = %d", fShow, fRet);
}

void ManagementAPI::SetShowHomegroupList(bool fShow)
{
	bool fRet = false;

	DWORD dwAttribute = fShow ? 0xb084010c : 0xb094010c;
	string strHomegroupRegPath = L"CLSID\\{B4FB3F98-C1EA-428d-A78A-D1F5659CBA93}\\ShellFolder";

	RegistryKey* pKey = NULL;
	try
	{
		Library::API::RSAPI::ChangeRegSecurityOption(ERegistryHive::ClassesRoot, strHomegroupRegPath, L"SYSTEM");
		pKey = Registry::GetClassesRoot()->OpenSubKey(strHomegroupRegPath, true);

		if (NULL != pKey)
		{
			pKey->SetValue(L"Attributes", dwAttribute);
			fRet = true;
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

	if (NULL != pKey)
		delete pKey;

#ifdef _M_X64
	pKey = NULL;
	strHomegroupRegPath = L"Wow6432Node\\CLSID\\{B4FB3F98-C1EA-428d-A78A-D1F5659CBA93}\\ShellFolder";

	try
	{
		Library::API::RSAPI::ChangeRegSecurityOption(ERegistryHive::ClassesRoot, strHomegroupRegPath, L"SYSTEM");
		pKey = Registry::GetClassesRoot()->OpenSubKey(strHomegroupRegPath, true);

		if (NULL != pKey)
		{
			pKey->SetValue(L"Attributes", dwAttribute);
			fRet = true;
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

	if (NULL != pKey)
		delete pKey;
#endif

	_TRACE_I(Application, L"VDInit SetHomegroup [%d], Result = %d", fShow, fRet);
}

void ManagementAPI::SetShowPersonalFolder(bool fShow)
{
	bool fRet = false;

	DWORD dwAttribute = fShow ? 0xf084012d : 0xf094012d;
	string strPersonalRegPath = L"CLSID\\{59031A47-3F72-44A7-89C5-5595FE6B30EE}\\ShellFolder";

	RegistryKey* pKey = NULL;
	try
	{
		Library::API::RSAPI::ChangeRegSecurityOption(ERegistryHive::ClassesRoot, strPersonalRegPath, L"SYSTEM");
		pKey = Registry::GetClassesRoot()->OpenSubKey(strPersonalRegPath, true);

		if (NULL != pKey)
		{
			pKey->SetValue(L"Attributes", dwAttribute);
			fRet = true;
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

	if (NULL != pKey)
		delete pKey;

#ifdef _M_X64
	pKey = NULL;
	strPersonalRegPath = L"Wow6432Node\\CLSID\\{59031A47-3F72-44A7-89C5-5595FE6B30EE}\\ShellFolder";

	try
	{
		Library::API::RSAPI::ChangeRegSecurityOption(ERegistryHive::ClassesRoot, strPersonalRegPath, L"SYSTEM");
		pKey = Registry::GetClassesRoot()->OpenSubKey(strPersonalRegPath, true);

		if (NULL != pKey)
		{
			pKey->SetValue(L"Attributes", dwAttribute);
			fRet = true;
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

	if (NULL != pKey)
		delete pKey;
#endif

	_TRACE_I(Application, L"VDInit SetHomegroup [%d], Result = %d", fShow, fRet);
}

void ManagementAPI::SetShowFavorites(bool fShow)
{
	bool fRet = false;

	DWORD dwAttribute = fShow ? 0xa0900100 : 0xa9400100;
	string strFavoritesRegPath = L"CLSID\\{323CA680-C24D-4099-B94D-446DD2D7249E}\\ShellFolder";

	RegistryKey* pKey = NULL;
	try
	{
		Library::API::RSAPI::ChangeRegSecurityOption(ERegistryHive::ClassesRoot, strFavoritesRegPath, L"SYSTEM");
		pKey = Registry::GetClassesRoot()->OpenSubKey(strFavoritesRegPath, true);

		if (NULL != pKey)
		{
			pKey->SetValue(L"Attributes", dwAttribute);
			fRet = true;
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

	if (NULL != pKey)
		delete pKey;

#ifdef _M_X64
	pKey = NULL;
	strFavoritesRegPath = L"Wow6432Node\\CLSID\\{323CA680-C24D-4099-B94D-446DD2D7249E}\\ShellFolder";

	try
	{
		Library::API::RSAPI::ChangeRegSecurityOption(ERegistryHive::ClassesRoot, strFavoritesRegPath, L"SYSTEM");
		pKey = Registry::GetClassesRoot()->OpenSubKey(strFavoritesRegPath, true);

		if (NULL != pKey)
		{
			pKey->SetValue(L"Attributes", dwAttribute);
			fRet = true;
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

	if (NULL != pKey)
		delete pKey;
#endif

	_TRACE_I(Application, L"VDInit SetFavorites [%d], Result = %d", fShow, fRet);
}

void ManagementAPI::SetShowLibraries(bool fShow)
{
	bool fRet = false;

	DWORD dwAttribute = fShow ? 0xb080010d : 0xb090010d;
	string strLibrariesRegPath = L"CLSID\\{031E4825-7B94-4dc3-B131-E946B44C8DD5}\\ShellFolder";

	RegistryKey* pKey = NULL;
	try
	{
		Library::API::RSAPI::ChangeRegSecurityOption(ERegistryHive::ClassesRoot, strLibrariesRegPath, L"SYSTEM");
		pKey = Registry::GetClassesRoot()->OpenSubKey(strLibrariesRegPath, true);

		if (NULL != pKey)
		{
			pKey->SetValue(L"Attributes", dwAttribute);
			fRet = true;
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

	if (NULL != pKey)
		delete pKey;

#ifdef _M_X64
	pKey = NULL;
	strLibrariesRegPath = L"Wow6432Node\\CLSID\\{031E4825-7B94-4dc3-B131-E946B44C8DD5}\\ShellFolder";

	try
	{
		Library::API::RSAPI::ChangeRegSecurityOption(ERegistryHive::ClassesRoot, strLibrariesRegPath, L"SYSTEM");
		pKey = Registry::GetClassesRoot()->OpenSubKey(strLibrariesRegPath, true);

		if (NULL != pKey)
		{
			pKey->SetValue(L"Attributes", dwAttribute);
			fRet = true;
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

	if (NULL != pKey)
		delete pKey;
#endif

	_TRACE_I(Application, L"VDInit SetLibraries [%d], Result = %d", fShow, fRet);
}

void ManagementAPI::SetEnableOneDrive(bool fEnable)
{
	if (Version(10, 0) > Environment::GetOSVersion()->GetVersion())
		return;

	bool fRet = false;
	RegistryKey* pKey_1 = NULL;
	RegistryKey* pKey_2 = NULL;
	try
	{
		pKey_1 = Library::WindowProfile::GetUserKey()->OpenSubKey(L"SOFTWARE\\Classes\\CLSID\\{018D5C66-4533-4307-9B53-224DE2ED1FE6}", true);
		if (NULL != pKey_1)
		{
			pKey_1->SetValue(L"System.IsPinnedToNameSpaceTree", (uint32)(fEnable ? 1 : 0));
			fRet = true;
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

#if _M_X64

	if (NULL != pKey_1)
	{
		delete pKey_1;
		pKey_1 = NULL;
	}

	try
	{
		pKey_1 = Library::WindowProfile::GetUserKey()->OpenSubKey(L"SOFTWARE\\Classes\\WOW6432Node\\CLSID\\{018D5C66-4533-4307-9B53-224DE2ED1FE6}", true);
		if (NULL != pKey_1)
		{
			pKey_1->SetValue(L"System.IsPinnedToNameSpaceTree", (uint32)(fEnable ? 1 : 0));
			fRet = true;
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

#endif

	try
	{
		pKey_2 = Registry::GetLocalMachine()->CreateSubKey(L"SOFTWARE\\Policies\\Microsoft\\Windows\\OneDrive");
		if (NULL != pKey_2)
			pKey_2->SetValue(L"DisableFileSyncNGSC", (uint32)(fEnable ? 0 : 1));
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

	if (NULL != pKey_1)
		delete pKey_1;
	if (NULL != pKey_2)
		delete pKey_2;

	_TRACE_I(Application, L"VDInit SetEnableOneDrive [%d], Result=%d", fEnable, fRet);
}

void ManagementAPI::SetShowQuickAccessList(bool fShow)
{
	if (Version(10, 0) > Environment::GetOSVersion()->GetVersion())
		return;

	bool fRet1 = false;
	bool fRet2 = true;
	bool fSetDefaultView = false;
	DWORD dwAttributes = fShow ? 0xA0100000 : 0xA0600000;
	string strQuickAccRegPath = L"CLSID\\{679F85CB-0220-4080-B29B-5540CC05AAB6}\\ShellFolder";
	string strQuickAccRegPath64 = L"WOW6432Node\\CLSID\\{679F85CB-0220-4080-B29B-5540CC05AAB6}\\ShellFolder";
	RegistryKey* pKey_1 = NULL;
	RegistryKey* pKey_2 = NULL;
	try
	{
		if (!fShow)
		{
			pKey_1 = Library::WindowProfile::GetUserKey()->OpenSubKey(L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced", true);
			if (NULL != pKey_1)
			{
				pKey_1->SetValue(L"LaunchTo", (uint32)1);	// 1 : This PC, 2 : QuickAccess
				fSetDefaultView = true;
			}
		}

		if (fSetDefaultView)
		{
			Library::API::RSAPI::ChangeRegSecurityOption(ERegistryHive::ClassesRoot, strQuickAccRegPath, L"SYSTEM");

			try
			{
				pKey_2 = Registry::GetClassesRoot()->OpenSubKey(strQuickAccRegPath, true);
				if (NULL != pKey_2)
				{
					pKey_2->SetValue(L"Attributes", dwAttributes);
					fRet1 = true;
				}
			}
			catch (Exception* pe)
			{
				_TRACE_EX(pe);
				delete pe;
			}
		}

#if _M_X64

		if (NULL != pKey_2)
		{
			delete pKey_2;
			pKey_2 = NULL;
		}

		if (fSetDefaultView)
		{
			Library::API::RSAPI::ChangeRegSecurityOption(ERegistryHive::ClassesRoot, strQuickAccRegPath64, L"SYSTEM");

			try
			{
				pKey_2 = Registry::GetClassesRoot()->OpenSubKey(strQuickAccRegPath64, true);
				if (NULL != pKey_2)
				{
					pKey_2->SetValue(L"Attributes", dwAttributes);
					fRet2 = true;
				}
			}
			catch (Exception* pe)
			{
				_TRACE_EX(pe);
				delete pe;
			}
		}

#endif
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
	if (NULL != pKey_1)
		delete pKey_1;
	if (NULL != pKey_2)
		delete pKey_2;

	_TRACE_I(Application, L"VDInit SetShowQuickAccessList [%d], Result=%d, %d", fShow, fRet1, fRet2);
}

void ManagementAPI::RemoveThisPCLinks(array<string> astrExceptLinks)
{
	if (Version(10, 0) > Environment::GetOSVersion()->GetVersion())
		return;

	string strRegPathPrefix = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\NameSpace\\";
#ifdef _M_X64
	string strRegPathPrefix64 = L"SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\NameSpace\\";
#endif

	for (int i = 0; i < astrExceptLinks.GetLength(); i++)
	{
		if (!astrExceptLinks[i].IsEmpty())
		{
			_TRACE_I(Application, L"ExceptThisPCLink = %ws", (const wchar_t*)astrExceptLinks[i]);
			Registry::GetLocalMachine()->DeleteSubKey(strRegPathPrefix + astrExceptLinks[i], false);
#ifdef _M_X64
			Registry::GetLocalMachine()->DeleteSubKey(strRegPathPrefix64 + astrExceptLinks[i], false);
#endif
		}
	}
}

void ManagementAPI::HideGameBarPanel()
{
	if (Version(10, 0) > Environment::GetOSVersion()->GetVersion())
		return;

	RegistryKey *pKey = NULL;
	try
	{
		pKey = Library::WindowProfile::GetUserKey()->OpenSubKey(L"Software\\Microsoft\\GameBar", true);
		if (NULL != pKey)
			pKey->SetValue(L"ShowStartupPanel", (DWORD)0);
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
	if (NULL != pKey)
		delete pKey;
}

void ManagementAPI::DisableExplorerRunAs()
{
	bool fRet = false;
	bool fFind = false;
	string strRunAs;
	string strExplorerRegPath = L"AppID\\{CDCBCFCA-3CDC-436f-A4E2-0E02075250C2}";
	RegistryKey* pKey_1 = NULL;
	RegistryKey* pKey_2 = NULL;

	try
	{
		pKey_1 = Registry::GetClassesRoot()->OpenSubKey(strExplorerRegPath);
		if (NULL != pKey_1)
		{
			array<string> astrValueNames = pKey_1->GetValueNames();
			for (int i = 0; i < astrValueNames.GetLength(); i++)
			{
				if (astrValueNames[i].Equals(L"runas", true))
				{
					fFind = true;
					break;
				}
			}

			if (!fFind)
				fRet = true;
		}
	}
	catch (Exception* pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}

	if (fFind)
	{
		try
		{
			Library::API::RSAPI::ChangeRegSecurityOption(ERegistryHive::ClassesRoot, strExplorerRegPath, L"Administrators");
			pKey_2 = Registry::GetClassesRoot()->OpenSubKey(strExplorerRegPath, true);
			if (NULL != pKey_2)
			{
				pKey_2->GetValue(L"RunAs", strRunAs);
				pKey_2->SetValue(L"RunAs_", strRunAs);
				pKey_2->DeleteValue(L"RunAs");
				fRet = true;
			}
		}
		catch (Exception* pe)
		{
			_TRACE_EX(pe);
			delete pe;
		}
	}

	if (NULL != pKey_1)
		delete pKey_1;
	if (NULL != pKey_2)
		delete pKey_2;

	_TRACE_I(Application, L"VDInit DisableExplorerRunAs Result=%d", fRet);
}

#define PACKAGEAPPS_GUID L"da6d82a1-3d9e-4895-ad58-db41dae8abb5"
void ManagementAPI::EnableBlockPackageApps()
{
	if (Version(6, 3) > Environment::GetOSVersion()->GetVersion())
		return;

	string strArg = L"";
	string strPolicyPath = Application::GetCommonAppDataPath() + L"\\ssGroupPolicy.xml";

	do 
	{
		if (IsBlockedPackageApps())
			break;
		
		ProcessAPI::StopServiceByName(L"AppIDSvc");

		_TRACE_I(Application, L"EnableBlockPackageApps() ENTRY");

		if (!SetGroupPolicy(strPolicyPath))
			break;

		_TRACE_I(Application, L"EnableBlockPackageApps() MARK 1");

		if (INVALID_FILE_ATTRIBUTES == ::GetFileAttributesW(strPolicyPath))
			break;

		_TRACE_I(Application, L"EnableBlockPackageApps() MARK 2");
		
		strArg = String::Format(L"Set-AppLockerPolicy -XMLPolicy %ws -merge", (const wchar_t*)strPolicyPath);

 		ProcessAPI::ExecutePowerShell(strArg);

		_TRACE_I(Application, L"EnableBlockPackageApps() MARK 3");

		ProcessAPI::ExecuteGroupUpdate();

		_TRACE_I(Application, L"EnableBlockPackageApps() EXIT");

	} while (false);

	ProcessAPI::StartServiceByName(L"AppIDSvc");
}

void ManagementAPI::DisableBlockPackageApps()
{
	if (Version(6, 3) > Environment::GetOSVersion()->GetVersion())
		return;
	
	string strArg = L"";
	string strPolicyPath = Application::GetCommonAppDataPath() + L"\\ssGroupPolicy.xml";
	string strLocalPolicyPath = Application::GetCommonAppDataPath() + L"\\LocalGroupPolicy.xml";

	do 
	{
		if (!IsBlockedPackageApps())
			break;

		ProcessAPI::StopServiceByName(L"AppIDSvc");

		_TRACE_I(Application, L"DisableBlockPackageApps() ENTRY");

		strArg = String::Format(L"Get-AppLockerPolicy -Local -XML > %ws", (const wchar_t*)strLocalPolicyPath);
		ProcessAPI::ExecutePowerShell(strArg);

		if (INVALID_FILE_ATTRIBUTES == ::GetFileAttributesW(strLocalPolicyPath))
			break;

		_TRACE_I(Application, L"DisableBlockPackageApps() MARK 1");

		ResetGroupPolicy(strPolicyPath, strLocalPolicyPath);

		_TRACE_I(Application, L"DisableBlockPackageApps() MARK 2");

		strArg = String::Format(L"Set-AppLockerPolicy -XMLPolicy %ws", (const wchar_t*)strPolicyPath);
 		if (INVALID_FILE_ATTRIBUTES == ::GetFileAttributesW(strPolicyPath))
 			break;

		_TRACE_I(Application, L"DisableBlockPackageApps() MARK 3");

		ProcessAPI::ExecutePowerShell(strArg);
		
		_TRACE_I(Application, L"DisableBlockPackageApps() MARK 4");

		ProcessAPI::ExecuteGroupUpdate();

		_TRACE_I(Application, L"DisableBlockPackageApps() EXIT");

	} while (false);

	ProcessAPI::StartServiceByName(L"AppIDSvc");
}

bool ManagementAPI::IsBlockedPackageApps()
{
	bool fRet = false;
	
	RegistryKey* pKey1 = NULL;
	RegistryKey* pKey2 = NULL;

	string strKey1 = L"SOFTWARE\\Policies\\Microsoft\\Windows\\SrpV2\\Appx";
	string strKey2 = String::Format(L"%ws\\%ws", (const wchar_t*)strKey1, (const wchar_t*)PACKAGEAPPS_GUID);

	DWORD dwValue = 0;
	string strValue = L"";

	try
	{
		do
		{
			pKey1 = Registry::GetLocalMachine()->OpenSubKey(strKey1);

			if (NULL == pKey1)
				break;
						
			pKey1->GetValue(L"EnforcementMode", dwValue);

			if (0 == dwValue)
				break;

			pKey2 = Registry::GetLocalMachine()->OpenSubKey(strKey2);

			if (NULL == pKey2)
				break;

			pKey2->GetValue(L"Value", strValue);

			if (!strValue.Contains(PACKAGEAPPS_GUID))
				break;

			fRet = true;

		} while (false);
	}
	catch (Exception* pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}

	if (NULL == pKey1)
		delete pKey1;

	if (NULL == pKey2)
		delete pKey2;

	return fRet;
}

void ManagementAPI::UninstallVirtualPrinter()
{
	if (Version(6, 3) > Environment::GetOSVersion()->GetVersion())
		return;

	bool fRet = false;
	bool fSetDefaultView = false;
	RegistryKey* pKey = NULL;

	try
	{
		string strPrintEnumRegPath = L"SYSTEM\\CurrentControlSet\\Enum\\SWD\\PRINTENUM";
		pKey = Registry::GetLocalMachine()->OpenSubKey(strPrintEnumRegPath, true);

		if (NULL != pKey)
		{
			array<string> astrKey = pKey->GetSubKeyNames();

			for (int i = 0; i < astrKey.GetLength(); i++)
			{
				if (astrKey[i].ToLower().EndsWith(L"_mwvp"))
				{
					string strKeyName1 = string::Format(L"%ws\\%ws\\Properties", (const wchar_t*)strPrintEnumRegPath, (const wchar_t*)astrKey[i]);
					string strKeyName2 = string::Format(L"%ws\\%ws", (const wchar_t*)strPrintEnumRegPath, (const wchar_t*)astrKey[i]);
					Library::API::RSAPI::ChangeRegSecurityOption(ERegistryHive::LocalMachine, strKeyName1, L"SYSTEM");
					Library::API::RSAPI::ChangeRegSecurityOption(ERegistryHive::LocalMachine, strKeyName2, L"SYSTEM");

					try
					{
						pKey->DeleteSubKeyTree(astrKey[i], true);
					}
					catch (Exception* pe)
					{
						_TRACE_EX(pe);
						delete pe;
					}
				}
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
	if (NULL != pKey)
		delete pKey;
}

void ManagementAPI::UninstallVirtualPrinterOldVersion()
{
	if (Version(6, 3) > Environment::GetOSVersion()->GetVersion())
		return;

	bool fRet = false;
	bool fSetDefaultView = false;
	RegistryKey* pKey = NULL;

	try
	{
		string strPrintEnumRegPath = L"SYSTEM\\CurrentControlSet\\Enum\\SWD\\PRINTENUM";
		pKey = Registry::GetLocalMachine()->OpenSubKey(strPrintEnumRegPath, true);

		if (NULL != pKey)
		{
			array<string> astrKey = pKey->GetSubKeyNames();

			for (int i = 0; i < astrKey.GetLength(); i++)
			{
				if (astrKey[i].ToLower().EndsWith(L"1"))
				{
					string strKeyName1 = string::Format(L"%ws\\%ws\\Properties", (const wchar_t*)strPrintEnumRegPath, (const wchar_t*)astrKey[i]);
					string strKeyName2 = string::Format(L"%ws\\%ws", (const wchar_t*)strPrintEnumRegPath, (const wchar_t*)astrKey[i]);
					Library::API::RSAPI::ChangeRegSecurityOption(ERegistryHive::LocalMachine, strKeyName1, L"SYSTEM");
					Library::API::RSAPI::ChangeRegSecurityOption(ERegistryHive::LocalMachine, strKeyName2, L"SYSTEM");

					try
					{
						pKey->DeleteSubKeyTree(astrKey[i], true);
					}
					catch (Exception* pe)
					{
						_TRACE_EX(pe);
						delete pe;
					}
				}
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
	if (NULL != pKey)
		delete pKey;
}

bool ManagementAPI::SetGroupPolicy(string strPolicyPath)
{
	bool fRet = false;

	FileStream *pStream = NULL;
	BinaryWriter *pWriter = NULL;
	array<byte> abResult;
	
	Library::XmlObject2* pXmlObejct = NULL;

	try
	{
		Library::XmlObject2* pBinaryVersionRange = new Library::XmlObject2(L"BinaryVersionRange", L"");
		pBinaryVersionRange->SetAttribute(L"LowSection", L"0.0.0.0");
		pBinaryVersionRange->SetAttribute(L"HighSection", L"*");

		Library::XmlObject2* pFilePublisherCondition = new Library::XmlObject2(L"FilePublisherCondition", L"");
		pFilePublisherCondition->SetAttribute(L"PublisherName", L"*");
		pFilePublisherCondition->SetAttribute(L"ProductName", L"*");
		pFilePublisherCondition->SetAttribute(L"BinaryName", L"*");
		pFilePublisherCondition->AddChild(pBinaryVersionRange);

		Library::XmlObject2* pContitions = new Library::XmlObject2(L"Conditions", L"");
		pContitions->AddChild(pFilePublisherCondition);


		Library::XmlObject2* pBinaryVersionRange2 = new Library::XmlObject2(L"BinaryVersionRange", L"");
		pBinaryVersionRange2->SetAttribute(L"LowSection", L"0.0.0.0");
		pBinaryVersionRange2->SetAttribute(L"HighSection", L"*");

		Library::XmlObject2* pFilePublisherCondition2 = new Library::XmlObject2(L"FilePublisherCondition", L"");
		pFilePublisherCondition2->SetAttribute(L"PublisherName", L"CN=Microsoft Windows, O=Microsoft Corporation, L=Redmond, S=Washington, C=US");
		pFilePublisherCondition2->SetAttribute(L"ProductName", L"*");
		pFilePublisherCondition2->SetAttribute(L"BinaryName", L"*");
		pFilePublisherCondition2->AddChild(pBinaryVersionRange2);

		Library::XmlObject2* pExceptions = new Library::XmlObject2(L"Exceptions", L"");
		pExceptions->AddChild(pFilePublisherCondition2);
		
		Library::XmlObject2* pFilePublisherRule = new Library::XmlObject2(L"FilePublisherRule", L"");
		pFilePublisherRule->SetAttribute(L"Id", PACKAGEAPPS_GUID);
		pFilePublisherRule->SetAttribute(L"Name", L"SecureSoft");
		pFilePublisherRule->SetAttribute(L"Description", L"SecureSoft Policy");
		pFilePublisherRule->SetAttribute(L"UserOrGroupSid", L"S-1-1-0");
		pFilePublisherRule->SetAttribute(L"Action", L"Deny");
		pFilePublisherRule->AddChild(pContitions);
		pFilePublisherRule->AddChild(pExceptions);

		Library::XmlObject2* pRuleCollection = new Library::XmlObject2(L"RuleCollection", L"");
		pRuleCollection->SetAttribute(L"Type", L"Appx");
		pRuleCollection->SetAttribute(L"EnforcementMode", L"Enabled");
		pRuleCollection->AddChild(pFilePublisherRule);

		pXmlObejct = new Library::XmlObject2(L"AppLockerPolicy", L"", L"Version", L"1");
		pXmlObejct->AddChild(pRuleCollection);

		abResult = Encoding::GetUTF8()->GetBytes(pXmlObejct->ToString());

		pStream = new FileStream(strPolicyPath, EFileMode::Create, EFileAccess::Write);
		pWriter = new BinaryWriter(pStream);
		pWriter->Write(abResult);

		fRet = true;
	}
	catch (Exception* pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}

	if (NULL != pXmlObejct)
		delete pXmlObejct;
	if (NULL != pWriter)
		delete pWriter;
	if (NULL != pStream)
		delete pStream;

	return fRet;
}

bool ManagementAPI::ResetGroupPolicy(string strPolicyPath, string strLocalPolicyPath)
{
	bool fRet = false;

	FileStream *pReadStream = NULL;
	FileStream *pWriteStream = NULL;

	BinaryReader *pReader = NULL;
	BinaryWriter *pWriter = NULL;
	array<byte> abResult;

	Library::XmlObject2* pXmlObejct = NULL;

	try
	{
		pReadStream = new FileStream(strLocalPolicyPath, EFileMode::Open, EFileAccess::Read);
		pReader = new BinaryReader(pReadStream);
		abResult = pReader->ReadBytes(pReadStream->GetLength());

		pXmlObejct = new Library::XmlObject2(abResult);
		string strLocalPolicyXml = pXmlObejct->ToString();

		int nRuleCollectionCount = pXmlObejct->GetChildCount();
		Library::XmlObject2* pRuleCollection = NULL;

		for (int i = 0; i < nRuleCollectionCount; i++)
		{
			Library::XmlObject2* pTemp = pXmlObejct->GetChild(i);
			string strTemp = pTemp->GetAttribute(L"Type", true);

			if (strTemp.Equals(L"Appx"))
				pRuleCollection = pTemp;
		}

		if (NULL == pRuleCollection)
			throw new NullReferenceException;

		int nFilePublisherRuleCount = pRuleCollection->GetChildCount();
		Library::XmlObject2* pFilePublisherRule = NULL;

		for (int i = 0; i < nFilePublisherRuleCount; i++)
		{
			Library::XmlObject2* pTemp = pRuleCollection->GetChild(i);
			string strTemp = pTemp->GetAttribute(L"Id", true);

			if (strTemp.Equals(PACKAGEAPPS_GUID))
				pFilePublisherRule = pTemp;
		}

		if (NULL == pFilePublisherRule)
			throw new NullReferenceException;

		pRuleCollection->RemoveChild(L"FilePublisherRule", L"Id", PACKAGEAPPS_GUID, true);
 
		abResult = Encoding::GetUTF8()->GetBytes(pXmlObejct->ToString(3));
 		pWriteStream = new FileStream(strPolicyPath, EFileMode::Create, EFileAccess::Write);
 		pWriter = new BinaryWriter(pWriteStream);
 		pWriter->Write(abResult);
		
		fRet = true;
	}
	catch (Exception* pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}

	if (NULL != pXmlObejct)
		delete pXmlObejct;
	if (NULL != pWriter)
		delete pWriter;
	if (NULL != pReader)
		delete pReader;
	if (NULL != pWriteStream)
		delete pWriteStream;
	if (NULL != pReadStream)
		delete pReadStream;

	return fRet;
}

string ManagementAPI::GetVolumeGUID(string strDrive)
{
	array<byte> abVolumeData;
	string strVolumeGUID;
	string strDriveName = L"\\DosDevices\\" + strDrive;
	if (!strDriveName.EndsWith(L':'))
		strDriveName += L':';

	bool fFind = false;
	RegistryKey* pKey = NULL;
	try
	{
		pKey = Registry::GetLocalMachine()->OpenSubKey(L"SYSTEM\\MountedDevices");
		if (NULL != pKey)
		{
			array<string> astrValues = pKey->GetValueNames();
			for (int i = 0; i < astrValues.GetLength(); i++)
			{
				if (strDriveName.Equals(astrValues[i], true))
				{
					pKey->GetValue(astrValues[i], abVolumeData);
					fFind = true;
					break;
				}
			}

			if (fFind)
			{
				for (int i = 0; i < astrValues.GetLength(); i++)
				{
					if (!strDriveName.Equals(astrValues[i], true))
					{
						array<byte> abTemp;
						pKey->GetValue(astrValues[i], abTemp);
						if (abVolumeData.Equals(abTemp))
						{
							int nIndex = astrValues[i].IndexOf(L'{');
							if (nIndex > 0)
							{
								strVolumeGUID = astrValues[i].Substring(nIndex);
								break;
							}
						}
					}
				}
			}
		}
	}
	catch (Exception* pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}
	if (NULL != pKey)
		delete pKey;

	_TRACE_I(Application, L"GetVolumeGUID() - %ws", (const wchar_t*)strVolumeGUID);
	return strVolumeGUID;
}

string ManagementAPI::GetSystemVolumeGUID()
{
	string strGUID;
	string strDrive = Environment::ExpandEnvironmentVariables(L"%SystemDrive%") + L"\\";

	array<wchar_t> awcVolumeMountPoint(1024);
	if (TRUE == ::GetVolumeNameForVolumeMountPoint(strDrive, awcVolumeMountPoint.GetBuffer(), awcVolumeMountPoint.GetLength()))
	{
		awcVolumeMountPoint.GetBuffer()[wcslen(awcVolumeMountPoint.GetBuffer()) - 1] = L'\0';
		string strVolumeName = string(awcVolumeMountPoint, 0, 48);
		
		strVolumeName = strVolumeName.Replace(L"\\\\?\\Volume", L"");
		strGUID = strVolumeName;
	}

	return strGUID;
}

void ManagementAPI::DisableRecycleBinProperty()
{
	RegistryKey* pKey = NULL;
	try
	{
		pKey = Library::WindowProfile::GetUserKey()->OpenSubKey(L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer", true);
		if (NULL != pKey)
			pKey->SetValue(L"NoPropertiesRecycleBin", (uint32)1);
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
	if (NULL != pKey)
		delete pKey;
}

void ManagementAPI::SetRecycleBinMaxCapacity(string strDrive, DWORD dwMaxCapacity, bool fSharedHDD /* = false */)
{
	// dwMaxCapacity가 0이라면 최대가능용량으로 설정.
	if (dwMaxCapacity == 0)
	{
		double dblTotal, dblUsed, dblRatio;
		Library::API::FSAPI::GetDriveCapacityInfo(strDrive, dblTotal, dblUsed, dblRatio);
		dwMaxCapacity = (dblTotal == 0) ? 512 : dblTotal;
	}
	
	string strVolumeGUID = GetVolumeGUID(strDrive);
	string strSystemVolumeGUID = GetSystemVolumeGUID();

	_TRACE_I(Application, L"strVolumeGUID : [%ws]%ws", (const wchar_t*)strDrive, (const wchar_t*)strVolumeGUID);
	_TRACE_I(Application, L"strSystemVolumeGUID : [%ws]%ws", (const wchar_t*)Environment::ExpandEnvironmentVariables(L"%SystemDrive%").Substring(0, 1), (const wchar_t*)strSystemVolumeGUID);

	RegistryKey* pKey = NULL;
	try
	{
		string strKeyPath = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\BitBucket\\Volume";
		pKey = Library::WindowProfile::GetUserKey()->CreateSubKey(strKeyPath);
		if (NULL != pKey)
		{
			array<string> arstrSubKeys = pKey->GetSubKeyNames();
			int nCount = arstrSubKeys.GetLength();

			for (int n = 0; n < nCount; n++)
			{
				if (strVolumeGUID.ToLower().Equals(arstrSubKeys[n].ToLower()))
					continue;

				if (strSystemVolumeGUID.ToLower().Equals(arstrSubKeys[n].ToLower()))
					continue;

				RegistryKey *pSubKey = pKey->OpenSubKey(arstrSubKeys[n], true);
				if (NULL != pSubKey)
				{
					pSubKey->SetValue(L"MaxCapacity", dwMaxCapacity);
					
					if (fSharedHDD)
						pSubKey->SetValue(L"NukeOnDelete", (uint32)0);
					else
						pSubKey->SetValue(L"NukeOnDelete", (uint32)1);

					delete pSubKey;
				}
			}
		}
	}
	catch (Exception* pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}

	if (NULL != pKey)
	{
		delete pKey;
		pKey = NULL;
	}

	try
	{
		if (!strVolumeGUID.IsEmpty())
		{
			string strKeyPath = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\BitBucket\\Volume\\" + strVolumeGUID;
			pKey = Library::WindowProfile::GetUserKey()->CreateSubKey(strKeyPath);
			if (NULL != pKey)
			{
				pKey->SetValue(L"NukeOnDelete", (uint32)0);
				pKey->SetValue(L"MaxCapacity", dwMaxCapacity);

				_TRACE_I(Application, L"SetRecycleBinMaxCapacity() Virtual Volume-> %d MB", dwMaxCapacity);
			}
		}
	}
	catch (Exception* pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}

	if (NULL != pKey)
	{
		delete pKey;
		pKey = NULL;
	}

	try
	{
		if (!strSystemVolumeGUID.IsEmpty())
		{
			string strKeyPath = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\BitBucket\\Volume\\" + strSystemVolumeGUID;
			pKey = Library::WindowProfile::GetUserKey()->CreateSubKey(strKeyPath);
			if (NULL != pKey)
			{
				pKey->SetValue(L"NukeOnDelete", (uint32)0);
				pKey->SetValue(L"MaxCapacity", dwMaxCapacity);

				_TRACE_I(Application, L"SetRecycleBinMaxCapacity() System Volume -> %d MB", dwMaxCapacity);
			}
		}
	}
	catch (Exception* pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}

	if (NULL != pKey)
		delete pKey;
}

bool ManagementAPI::IsSafeBootMode()
{
	return (0 < ::GetSystemMetrics(SM_CLEANBOOT));
}

}
}