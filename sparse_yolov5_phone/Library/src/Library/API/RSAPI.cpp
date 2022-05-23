#include "StdAfx.h"
#include <Library/API/RSAPI.h>
#include <Library/API/SecurityAPI.h>
#include <Library/WindowProfile.h>
#include <AclAPI.h>

#include "wpp.h"
#include "RSAPI.tmh"

namespace Library
{
namespace API
{

string RSAPI::GetUserRegistryValueS(string strName, string strSubKey /*= L""*/)
{
	string strValue;
	RegistryKey *pKey = NULL;
	try
	{
		if (strSubKey.IsEmpty())
		{
			Application::GetUserAppDataRegistry()->GetValue(strName, strValue);
		}
		else 
		{
			pKey = Application::GetUserAppDataRegistry()->OpenSubKey(strSubKey);
			if (pKey != NULL)
				pKey->GetValue(strName, strValue);
		}
	}
	catch (Exception *pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}
	if (pKey != NULL)
		delete pKey;
	return strValue;
}

array<byte> RSAPI::GetUserRegistryValueB(string strName, string strSubKey /*= L""*/)
{
	array<byte> abValue;
	RegistryKey *pKey = NULL;
	try
	{
		if (strSubKey.IsEmpty())
		{
			Application::GetUserAppDataRegistry()->GetValue(strName, abValue);
		}
		else 
		{
			pKey = Application::GetUserAppDataRegistry()->OpenSubKey(strSubKey);
			if (pKey != NULL)
				pKey->GetValue(strName, abValue);
		}
	}
	catch (Exception *pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}
	if (pKey != NULL)
		delete pKey;
	return abValue;
}

DWORD RSAPI::GetUserRegistryValueD(string strName, string strSubKey /*= L""*/)
{
	DWORD dwValue = 0;
	RegistryKey *pKey = NULL;
	try
	{
		if (strSubKey.IsEmpty())
		{
			Application::GetUserAppDataRegistry()->GetValue(strName, dwValue);
		}
		else 
		{
			pKey = Application::GetUserAppDataRegistry()->OpenSubKey(strSubKey);
			if (pKey != NULL)
				pKey->GetValue(strName, dwValue);
		}
	}
	catch (Exception *pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}
	if (pKey != NULL)
		delete pKey;
	return dwValue;
}

void RSAPI::SetUserRegistryValue(string strName, string strValue, string strSubKey /*= L""*/)
{
	RegistryKey *pKey = NULL;
	try
	{
		if (strSubKey.IsEmpty())
		{
			Application::GetUserAppDataRegistry()->SetValue(strName, strValue);
		}
		else 
		{
			pKey = Application::GetUserAppDataRegistry()->CreateSubKey(strSubKey);
			if (pKey != NULL)
				pKey->SetValue(strName, strValue);
		}
	}
	catch (Exception *pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}
	if (pKey != NULL)
		delete pKey;
}

void RSAPI::SetUserRegistryValue(string strName, array<byte> abValue, string strSubKey /*= L""*/)
{
	RegistryKey *pKey = NULL;
	try
	{
		if (strSubKey.IsEmpty())
		{
			Application::GetUserAppDataRegistry()->SetValue(strName, abValue);
		}
		else 
		{
			pKey = Application::GetUserAppDataRegistry()->CreateSubKey(strSubKey);
			if (pKey != NULL)
				pKey->SetValue(strName, abValue);
		}
	}
	catch (Exception *pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}
	if (pKey != NULL)
		delete pKey;
}

void RSAPI::SetUserRegistryValue(string strName, DWORD dwValue, string strSubKey /*= L""*/)
{
	RegistryKey *pKey = NULL;
	try
	{
		if (strSubKey.IsEmpty())
		{
			Application::GetUserAppDataRegistry()->SetValue(strName, dwValue);
		}
		else 
		{
			pKey = Application::GetUserAppDataRegistry()->CreateSubKey(strSubKey);
			if (pKey != NULL)
				pKey->SetValue(strName, dwValue);
		}
	}
	catch (Exception *pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}
	if (pKey != NULL)
		delete pKey;
}

string RSAPI::GetCommonRegistryValueS(string strName, string strSubKey /*= L""*/)
{
	string strValue;
	RegistryKey *pKey = NULL;
	try
	{
		if (strSubKey.IsEmpty())
		{
			Application::GetCommonAppDataRegistry()->GetValue(strName, strValue);
		}
		else 
		{
			pKey = Application::GetCommonAppDataRegistry()->OpenSubKey(strSubKey);
			if (pKey != NULL)
				pKey->GetValue(strName, strValue);
		}
	}
	catch (Exception *pe)
	{
		_TRACE_EX(pe);
		delete pe;	
	}
	if (pKey != NULL)
		delete pKey;
	return strValue;
}

array<byte> RSAPI::GetCommonRegistryValueB(string strName, string strSubKey /*= L""*/)
{
	array<byte> abValue;
	RegistryKey *pKey = NULL;
	try
	{
		if (strSubKey.IsEmpty())
		{
			Application::GetCommonAppDataRegistry()->GetValue(strName, abValue);
		}
		else 
		{
			pKey = Application::GetCommonAppDataRegistry()->OpenSubKey(strSubKey);
			if (pKey != NULL)
				pKey->GetValue(strName, abValue);
		}
	}
	catch (Exception *pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}
	if (pKey != NULL)
		delete pKey;
	return abValue;
}

DWORD RSAPI::GetCommonRegistryValueD(string strName, string strSubKey /*= L""*/)
{
	DWORD dwValue = 0;
	RegistryKey *pKey = NULL;
	try
	{
		if (strSubKey.IsEmpty())
		{
			Application::GetCommonAppDataRegistry()->GetValue(strName, dwValue);
		}
		else 
		{
			pKey = Application::GetCommonAppDataRegistry()->OpenSubKey(strSubKey);
			if (pKey != NULL)
				pKey->GetValue(strName, dwValue);
		}
	}
	catch (Exception *pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}
	if (pKey != NULL)
		delete pKey;
	return dwValue;
}

void RSAPI::SetCommonRegistryValue(string strName, string strValue, string strSubKey /*= L""*/)
{
	RegistryKey *pKey = NULL;
	try
	{
		if (strSubKey.IsEmpty())
		{
			Application::GetCommonAppDataRegistry()->SetValue(strName, strValue);
		}
		else 
		{
			pKey = Application::GetCommonAppDataRegistry()->CreateSubKey(strSubKey);
			if (pKey != NULL)
				pKey->SetValue(strName, strValue);
		}
	}
	catch (Exception *pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}
	if (pKey != NULL)
		delete pKey;
}

void RSAPI::SetCommonRegistryValue(string strName, array<byte> abValue, string strSubKey /*= L""*/)
{
	RegistryKey *pKey = NULL;
	try
	{
		if (strSubKey.IsEmpty())
		{
			Application::GetCommonAppDataRegistry()->SetValue(strName, abValue);
		}
		else 
		{
			pKey = Application::GetCommonAppDataRegistry()->CreateSubKey(strSubKey);
			if (pKey != NULL)
				pKey->SetValue(strName, abValue);
		}
	}
	catch (Exception *pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}
	if (pKey != NULL)
		delete pKey;
}

void RSAPI::SetCommonRegistryValue(string strName, DWORD dwValue, string strSubKey /*= L""*/)
{
	RegistryKey *pKey = NULL;
	try
	{
		if (strSubKey.IsEmpty())
		{
			Application::GetCommonAppDataRegistry()->SetValue(strName, dwValue);
		}
		else 
		{
			pKey = Application::GetCommonAppDataRegistry()->CreateSubKey(strSubKey);
			if (pKey != NULL)
				pKey->SetValue(strName, dwValue);
		}
	}
	catch (Exception *pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}
	if (pKey != NULL)
		delete pKey;
}

bool RSAPI::IsMWBHORegisterd()
{
	bool fPathExist = false;
	bool fBHOPathExist = false;

#ifdef _M_X64
	string strPath = L"Wow6432Node\\CLSID\\{61E5816C-FDE8-4F9D-9C57-D7742620D76F}";
	string strBHOPath = L"SOFTWARE\\WoW6432Node\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Browser Helper Objects\\{61E5816C-FDE8-4F9D-9C57-D7742620D76F}";
#else
	string strPath = L"CLSID\\{61E5816C-FDE8-4F9D-9C57-D7742620D76F}";
	string strBHOPath = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Browser Helper Objects\\{61E5816C-FDE8-4F9D-9C57-D7742620D76F}";
#endif

	RegistryKey *pKey = null;
	try
	{
		pKey = Registry::GetClassesRoot()->OpenSubKey(strPath);

		if (null != pKey)
		{
			//_TRACE_I(Application, L"bho clsid exists");
			fPathExist = true;
		}
		else
			_TRACE_I(Application, L"bho clsid not exists");

	}
	catch (Exception *pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}
	if (null != pKey)
	{
		delete pKey;
		pKey = null;
	}

	try
	{
		pKey = Registry::GetLocalMachine()->OpenSubKey(strBHOPath);
		if (null != pKey)
		{
			//_TRACE_I(Application, L"BHO exists");
			fBHOPathExist = true;
		}
		else
			_TRACE_I(Application, L"BHO not exists");
	}
	catch (Exception *pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}

	if (null != pKey)
	{
		delete pKey;
		pKey = null;
	}

	return (fPathExist && fBHOPathExist);
}

bool RSAPI::IsMWActiveXRegisterd()
{
	bool fPathExist = false;
	bool fBHOPathExist = false;

#ifdef _M_X64
	string strPath = L"Wow6432Node\\CLSID\\{1646CD76-0F32-48E6-A6D0-25877690FECD}";
	string strBHOPath = L"SOFTWARE\\WoW6432Node\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Browser Helper Objects\\{1646CD76-0F32-48E6-A6D0-25877690FECD}";
#else
	string strPath = L"CLSID\\{1646CD76-0F32-48E6-A6D0-25877690FECD}";
	string strBHOPath = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Browser Helper Objects\\{1646CD76-0F32-48E6-A6D0-25877690FECD}";
#endif

	RegistryKey *pKey = null;
	try
	{
		pKey = Registry::GetClassesRoot()->OpenSubKey(strPath);

		if (null != pKey)
			fPathExist = true;
		else
			_TRACE_I(Application, L"bho clsid not exists");

	}
	catch (Exception *pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}
	if (null != pKey)
	{
		delete pKey;
		pKey = null;
	}

	try
	{
		pKey = Registry::GetLocalMachine()->OpenSubKey(strBHOPath);
		if (null != pKey)
			fBHOPathExist = true;
		else
			_TRACE_I(Application, L"BHO not exists");
	}
	catch (Exception *pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}

	if (null != pKey)
	{
		delete pKey;
		pKey = null;
	}

	return (fPathExist && fBHOPathExist);
}

void RSAPI::SetIEPopupWindowSetting(int nValue)
{
	string strExtPath = L"Software\\Microsoft\\Internet Explorer\\TabbedBrowsing";
	RegistryKey *pKey = null;

	try
	{
		pKey = Registry::GetCurrentUser()->OpenSubKey(strExtPath, true);

		if (null != pKey)
		{
			pKey->SetValue(L"PopupsUseNewWindow", (uint32)nValue);
		}
	}
	catch (Exception *pe)
	{
		delete pe;
	}

	if (null != pKey)
	{
		delete pKey;
		pKey = null;
	}
}

void RSAPI::SetEnableBrowserExtensions(bool fEnable)
{
	RegistryKey *pKey = NULL;
	try
	{
		pKey = Registry::GetCurrentUser()->OpenSubKey(L"Software\\Microsoft\\Internet Explorer\\Main", true);
		if (null != pKey)
		{
			if (fEnable)
				pKey->SetValue(L"Enable Browser Extensions", L"yes");
			else
				pKey->SetValue(L"Enable Browser Extensions", L"no");
		}
	}
	catch (Exception *pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}
	if (null != pKey)
		delete pKey;
}

bool RSAPI::IsIEProtectedMode()
{
	bool fRet = true;

	RegistryKey *pKey = null;
	try
	{
		pKey = Registry::GetCurrentUser()->OpenSubKey(L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\Zones\\3");
		if (NULL != pKey)
		{
			uint32 uValue;

			if (pKey->GetValue(L"2500", uValue))
			{
				if (3 == uValue)
					fRet = false;
			}
		}
	}
	catch (Exception *pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}
	if (null != pKey)
		delete pKey;

	return fRet;
}

void RSAPI::SetIEProtectedMode(bool fEnable)
{
	RegistryKey *pKey = null;
	RegistryKey *pKey2 = null;
	try
	{
		// 인터넷 익스플로러 보호모드 끄기.
		pKey = Registry::GetCurrentUser()->OpenSubKey(L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\Zones\\3", true);
		if (null != pKey)
		{
			if (fEnable)
				pKey->SetValue(L"2500", (DWORD)0x00000000);
			else
				pKey->SetValue(L"2500", (DWORD)0x00000003);
		}

		// 인터넷 익스플로러 보호모드 알림 끄기.
		pKey2 = Registry::GetCurrentUser()->OpenSubKey(L"SOFTWARE\\Microsoft\\Internet Explorer\\Main", true);
		if (null != pKey2)
		{
			if (fEnable)
				pKey2->SetValue(L"NoProtectedModeBanner", (DWORD)0x00000000);
			else
				pKey2->SetValue(L"NoProtectedModeBanner", (DWORD)0x00000001);
		}

	}
	catch (Exception *pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}

	if (null != pKey2)
		delete pKey2;
	if (null != pKey)
		delete pKey;
}

void RSAPI::SetIEIsolationValue(bool fPMIL /* = true */)
{
	RegistryKey *pKey = null;

	try
	{
		pKey = Registry::GetCurrentUser()->OpenSubKey(L"Software\\Microsoft\\Internet Explorer\\Main", true);

		if (null != pKey)
		{
			if (fPMIL)
				pKey->SetValue(L"Isolation", L"PMIL");
			else
				pKey->SetValue(L"Isolation", L"PMEM");
		}
	}
	catch (Exception* pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}

	if (null != pKey)
		delete pKey;
}

void RSAPI::DeleteRegistryValueSubKey_HKLM(const string& strSubKey)
{
	RegistryKey *pKey = NULL;
	try
	{
		pKey = Registry::GetLocalMachine()->OpenSubKey(strSubKey, true);
		if (NULL != pKey)
		{
			array<string> astr1 = pKey->GetSubKeyNames();
			for (int i = 0 ; i < astr1.GetLength() ; i++)
				pKey->DeleteSubKeyTree(astr1[i], false);

			array<string> astr2 = pKey->GetValueNames();
			for (int i = 0 ; i < astr2.GetLength() ; i++)
				pKey->DeleteValue(astr2[i], false);

			_TRACE_I(Application, L"[CHILDDELETED][HKLM\\%ws]", 
				(const wchar_t*)strSubKey
				);
		}
	}
	catch (Exception* pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}
	if (pKey != NULL)
		delete pKey;
}

void RSAPI::DeleteRegistryValueSubKey_HKCU(const string& strSubKey)
{
	RegistryKey *pKey = NULL;

	try
	{
		pKey = Library::WindowProfile::GetUserKey()->OpenSubKey(strSubKey, true);
		if (NULL != pKey)
		{
			array<string> astr1 = pKey->GetSubKeyNames();
			for (int i = 0 ; i < astr1.GetLength() ; i++)
				pKey->DeleteSubKeyTree(astr1[i], false);

			array<string> astr2 = pKey->GetValueNames();
			for (int i = 0 ; i < astr2.GetLength() ; i++)
				pKey->DeleteValue(astr2[i], false);

			_TRACE_I(Application, L"[CHILDDELETED][HKU\\%ws\\%ws]", 
				(const wchar_t*)Library::WindowProfile::GetSID(),
				(const wchar_t*)strSubKey
				);
		}
	}
	catch (Exception* pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}

	if (pKey != NULL)
		delete pKey;
}

void RSAPI::DeleteRegistryValueSubKey_AllUser(const string& strSubKey)
{
	RegistryKey *pKey = NULL;
	try
	{
		pKey = Registry::GetLocalMachine()->OpenSubKey(L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList");
		if (pKey != NULL)
		{
			array<string> astrSubKeys = pKey->GetSubKeyNames();
			for (int i = 0 ; i < astrSubKeys.GetLength() ; i++)
			{
				if (astrSubKeys[i].GetLength() <= String(L"S-1-5-18").GetLength())
					continue;

				RegistryKey *pKey2 = NULL;
				try
				{
					pKey2 = Registry::GetUsers()->OpenSubKey(astrSubKeys[i] + L"\\" + strSubKey, true);
					if (NULL != pKey2)
					{
						array<string> astr = pKey2->GetSubKeyNames();
						for (int n = 0 ; n < astr.GetLength() ; n++)
							pKey2->DeleteSubKeyTree(astr[n], false);

						array<string> astr2 = pKey2->GetValueNames();
						for (int n = 0 ; n < astr2.GetLength() ; n++)
							pKey->DeleteValue(astr2[n], false);

						_TRACE_I(Application, L"[CHILDDELETED][HKU\\%ws\\%ws]", 
							(const wchar_t*)astrSubKeys[i],
							(const wchar_t*)strSubKey
							);
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
				if (pKey2 != NULL)
					delete pKey2;
			}
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

	if (pKey != NULL)
		delete pKey;
}

void RSAPI::DeleteRegistryValue_HKLM(const string& strSubKey, const string& strValue)
{
	RegistryKey *pKey = NULL;
	
	try
	{
		pKey = Registry::GetLocalMachine()->OpenSubKey(strSubKey, true);
		if (NULL != pKey)
		{
			pKey->DeleteValue(strValue, false);
			_TRACE_I(Application, L"[VALUEDELETED][HKLM\\%ws][%ws]", 
				(const wchar_t*)strSubKey,
				(const wchar_t*)strValue
				);
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

	if (pKey != NULL)
		delete pKey;
}

void RSAPI::DeleteRegistryValue_HKCU(const string& strSubKey, const string& strValue)
{
	RegistryKey *pKey = NULL;
	try
	{
		pKey = Library::WindowProfile::GetUserKey()->OpenSubKey(strSubKey, true);
		if (NULL != pKey)
		{
			pKey->DeleteValue(strValue, false);
			_TRACE_I(Application, L"[VALUEDELETED][HKU\\%ws\\%ws][%ws]", 
				(const wchar_t*)Library::WindowProfile::GetSID(),
				(const wchar_t*)strSubKey,
				(const wchar_t*)strValue
				);
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
	if (pKey != NULL)
		delete pKey;
}

void RSAPI::DeleteRegistryValue_AllUser(const string& strSubKey, const string& strValue)
{
	RegistryKey *pKey = NULL;

	try
	{
		pKey = Registry::GetLocalMachine()->OpenSubKey(L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList");
		if (pKey == NULL)
			throw new Exception(L"Cannot open the ProfileList key.");
		
		array<string> astrSubKeys = pKey->GetSubKeyNames();
		for (int i = 0 ; i < astrSubKeys.GetLength() ; i++)
		{
			if (astrSubKeys[i].GetLength() <= String(L"S-1-5-18").GetLength())
				continue;

			RegistryKey *pKey2 = NULL;
			try
			{
				pKey2 = Registry::GetUsers()->OpenSubKey(astrSubKeys[i] + L"\\" + strSubKey, true);
				if (NULL != pKey2)
				{
					pKey2->DeleteValue(strValue, false);
					_TRACE_I(Application, L"[VALUEDELETED][HKU\\%ws\\%ws][%ws]", 
						(const wchar_t*)astrSubKeys[i],
						(const wchar_t*)strSubKey,
						(const wchar_t*)strValue
						);
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
			if (pKey2 != NULL)
				delete pKey2; 
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

	if (pKey != NULL)
		delete pKey;
}

void RSAPI::DeleteRegistrySubKey_HKLM(const string& strSubKey, const string& strKey)
{
	RegistryKey *pKey = NULL;
	
	try
	{
		pKey = Registry::GetLocalMachine()->OpenSubKey(strSubKey, true);
		if (NULL != pKey)
		{
			pKey->DeleteSubKey(strKey, false);
			_TRACE_I(Application, L"[KEYDELETED][HKLM\\%ws][%ws]", 
				(const wchar_t*)strSubKey,
				(const wchar_t*)strKey
				);
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

	if (pKey != NULL)
		delete pKey;
}

void RSAPI::DeleteRegistrySubKey_HKCU(const string& strSubKey, const string& strKey)
{
	RegistryKey *pKey = NULL;
	try
	{
		pKey = Library::WindowProfile::GetUserKey()->OpenSubKey(strSubKey, true);
		if (NULL != pKey)
		{
			pKey->DeleteSubKey(strKey, false);
			_TRACE_I(Application, L"[KEYDELETED][HKU\\%ws\\%ws][%ws]", 
				(const wchar_t*)Library::WindowProfile::GetSID(),
				(const wchar_t*)strSubKey,
				(const wchar_t*)strKey
				);
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
	if (pKey != NULL)
		delete pKey;
}

void RSAPI::DeleteRegistrySubKey_AllUser(const string& strSubKey, const string& strKey)
{
	RegistryKey *pKey = NULL;

	try
	{
		pKey = Registry::GetLocalMachine()->OpenSubKey(L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList");
		if (pKey == NULL)
			throw new Exception(L"Cannot open the ProfileList key.");
		
		array<string> astrSubKeys = pKey->GetSubKeyNames();
		for (int i = 0 ; i < astrSubKeys.GetLength() ; i++)
		{
			if (astrSubKeys[i].GetLength() <= String(L"S-1-5-18").GetLength())
				continue;

			RegistryKey *pKey2 = NULL;
			try
			{
				pKey2 = Registry::GetUsers()->OpenSubKey(astrSubKeys[i] + L"\\" + strSubKey, true);
				if (NULL != pKey2)
				{
					pKey2->DeleteSubKey(strKey, false);
					_TRACE_I(Application, L"[KEYDELETED][HKU\\%ws\\%ws][%ws]", 
						(const wchar_t*)astrSubKeys[i],
						(const wchar_t*)strSubKey,
						(const wchar_t*)strKey
						);
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
			if (pKey2 != NULL)
				delete pKey2; 
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

	if (pKey != NULL)
		delete pKey;
}

bool RSAPI::ChangeRegSecurityOption(RegistryHive hive, string _strRegPath, string strAccountName)
{
	bool fRet = false;
	string strMainKeyName;
	switch (hive)
	{
	case ERegistryHive::ClassesRoot:
		strMainKeyName = L"classes_root";
		break;
	case ERegistryHive::LocalMachine:
		strMainKeyName = L"machine";
		break;
	case ERegistryHive::CurrentUser:
		strMainKeyName = L"current_user";
		break;
	case ERegistryHive::Users:
		strMainKeyName = L"users";
		break;
	default:
		return false;
	}

	string strRegPath = strMainKeyName + L"\\" + _strRegPath;

	Library::API::SecurityAPI::EnablePrivilege(SE_TAKE_OWNERSHIP_NAME, true);
	_SetRegSecurityInfo(strRegPath, strAccountName, OWNER_SECURITY_INFORMATION);
	fRet = _SetRegSecurityInfo(strRegPath, strAccountName, DACL_SECURITY_INFORMATION);
	Library::API::SecurityAPI::EnablePrivilege(SE_TAKE_OWNERSHIP_NAME, false);

	_TRACE_I(Application, L"ChangeRegSecurityOption() Reg=%ws, Account=%ws, Result=%d", (const wchar_t*)strRegPath, (const wchar_t*)strAccountName, fRet);
	return fRet;
}

void RSAPI::AddFeatureBrowserEmulation(const string& strName)
{
	RegistryKey *pKey = NULL;
	try
	{
		pKey = Library::WindowProfile::GetUserKey()->CreateSubKey(L"Software\\Microsoft\\Internet Explorer\\Main\\FeatureControl\\FEATURE_BROWSER_EMULATION");
		if (NULL != pKey)
		{
			pKey->SetValue(strName, (DWORD)0);
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

	if (pKey != NULL)
	{
		delete pKey;
		pKey = NULL;
	}

#if _M_X64
	try
	{
		pKey = Library::WindowProfile::GetUserKey()->CreateSubKey(L"Software\\WOW6432Node\\Microsoft\\Internet Explorer\\Main\\FeatureControl\\FEATURE_BROWSER_EMULATION");
		if (NULL != pKey)
		{
			pKey->SetValue(strName, (DWORD)0);
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
#endif
}

void RSAPI::RemoveFeatureBrowserEmulation(const string& strName)
{
	RegistryKey *pKey = NULL;
	try
	{
		pKey = Library::WindowProfile::GetUserKey()->CreateSubKey(L"Software\\Microsoft\\Internet Explorer\\Main\\FeatureControl\\FEATURE_BROWSER_EMULATION");
		if (NULL != pKey)
		{
			pKey->DeleteValue(strName);
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

	if (pKey != NULL)
	{
		delete pKey;
		pKey = NULL;
	}

#if _M_X64
	try
	{
		pKey = Library::WindowProfile::GetUserKey()->CreateSubKey(L"Software\\WOW6432Node\\Microsoft\\Internet Explorer\\Main\\FeatureControl\\FEATURE_BROWSER_EMULATION");
		if (NULL != pKey)
		{
			pKey->DeleteValue(strName);
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
#endif
}

bool RSAPI::_SetRegSecurityInfo(string strRegPath, string strAccountName, SECURITY_INFORMATION securityInfo)
{
	bool fRet = false;
	PACL pAclDACL = NULL, pAclSACL = NULL, pAclDACLNew = NULL;
	PSID pSidOwner = NULL, pSidGroup = NULL;
	PSECURITY_DESCRIPTOR pSDSelfRel = NULL;
	PSECURITY_DESCRIPTOR pSDTemp = NULL;

	try
	{
		char szSid[MAX_PATH] = { 0 };
		wchar_t wszDn[MAX_PATH] = { 0 };
		DWORD dwSidLen = MAX_PATH;
		DWORD dwDnLen = MAX_PATH;
		DWORD dwRet = ERROR_SUCCESS;
		SID_NAME_USE SNU;

		if (!LookupAccountName(NULL, strAccountName, (PSID)szSid, &dwSidLen, wszDn, &dwDnLen, &SNU))
			throw new Win32Exception(::GetLastError());

		dwRet = GetNamedSecurityInfoW(strRegPath, SE_REGISTRY_KEY, securityInfo, NULL, NULL, NULL, NULL, &pSDSelfRel);
		if (ERROR_SUCCESS != dwRet)
			throw new ApplicationException(string::Format(L"GetNamedSecurityInfoW() failed with %d.", dwRet));

		DWORD nBufSD = 0, nBufDACL = 0, nBufSACL = 0, nBufOwner = 0, nBufGroup = 0;
		MakeAbsoluteSD(pSDSelfRel, NULL, &nBufSD, NULL, &nBufDACL, NULL, &nBufSACL, NULL, &nBufOwner, NULL, &nBufGroup);
		dwRet = GetLastError();
		if (ERROR_INSUFFICIENT_BUFFER != dwRet)
			throw new Win32Exception(dwRet);

		pSDTemp = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, nBufSD);
		if (nBufDACL) pAclDACL = (PACL)LocalAlloc(LPTR, nBufDACL);
		if (nBufSACL) pAclSACL = (PACL)LocalAlloc(LPTR, nBufSACL);
		if (nBufOwner) pSidOwner = (PACL)LocalAlloc(LPTR, nBufOwner);
		if (nBufGroup) pSidGroup = (PACL)LocalAlloc(LPTR, nBufGroup);

		if (!MakeAbsoluteSD(pSDSelfRel, pSDTemp, &nBufSD, pAclDACL, &nBufDACL, pAclSACL, &nBufSACL, pSidOwner, &nBufOwner, pSidGroup, &nBufGroup))
			throw new Win32Exception(::GetLastError());

		EXPLICIT_ACCESS ea = { 0 };
		ea.grfAccessPermissions = KEY_ALL_ACCESS;
		ea.grfAccessMode = SET_ACCESS;
		ea.grfInheritance = CONTAINER_INHERIT_ACE;
		ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
		ea.Trustee.ptstrName = (LPTSTR)(PSID)szSid;

		dwRet = SetEntriesInAcl(1, &ea, pAclDACL, &pAclDACLNew);
		if (ERROR_SUCCESS != dwRet)
			throw new ApplicationException(string::Format(L"SetEntriesInAcl() failed with %d.", dwRet));

		dwRet = SetNamedSecurityInfoW(strRegPath, SE_REGISTRY_KEY, securityInfo, (PSID)szSid, NULL, pAclDACLNew, NULL);
		if (ERROR_SUCCESS != dwRet)
			throw new ApplicationException(string::Format(L"SetNamedSecurityInfoW() failed with %d.", dwRet));

		fRet = true;
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

	if (NULL != pAclDACL) LocalFree(pAclDACL);
	if (NULL != pAclSACL) LocalFree(pAclSACL);
	if (NULL != pAclDACLNew) LocalFree(pAclDACLNew);
	if (NULL != pSidOwner) LocalFree(pSidOwner);
	if (NULL != pSidGroup) LocalFree(pSidGroup);
	if (NULL != pSDSelfRel) LocalFree(pSDSelfRel);
	if (NULL != pSDTemp) LocalFree(pSDTemp);

	_TRACE_I(Application, L"_SetRegSecurityInfo() SecurityInfo=%d, Result=%d", securityInfo, fRet);
	return fRet;
}

DWORD RSAPI::LoadRegistry(HKEY hRootKey, const string& strRegName, const string& strRegFilePath)
{
	DWORD dwError = LOAD_FAILED;
	try
	{
		UnloadRegistry(hRootKey, strRegName);

		SecurityAPI::EnablePrivilege(SE_RESTORE_NAME, true);
		SecurityAPI::EnablePrivilege(SE_BACKUP_NAME, true);

		for (int n = 0; n < 10; n++)
		{
			dwError = ::RegLoadKeyW(hRootKey, strRegName, strRegFilePath);
			if (ERROR_SUCCESS == dwError)
				break;
			else if (n == 9)
				throw new Win32Exception(dwError);
			else
				::Sleep(500);
		}

		SecurityAPI::SetLowRegSecurity(hRootKey, strRegName);
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
	return dwError;
}

DWORD RSAPI::UnloadRegistry(HKEY hRootKey, const string& strRegName)
{
	DWORD errorCode = UNLOAD_SUCESS;
	try
	{
		if (!IsRegistryMounted(hRootKey, strRegName))
			return REGISTRY_NOT_MOUNTED;

		SecurityAPI::EnablePrivilege(SE_RESTORE_NAME, true);
		SecurityAPI::EnablePrivilege(SE_BACKUP_NAME, true);

		for (int n = 0; n < 10; n++)
		{
			DWORD dwError = ::RegUnLoadKeyW(hRootKey, strRegName);
			if (ERROR_SUCCESS == dwError)
			{
				_TRACE_I(Others, "RSAPI::UnloadRegistry() SUCCEED at %d", n);
				errorCode = UNLOAD_SUCESS;
				break;
			}

			_TRACE_I(Others, "RSAPI::UnloadRegistry() FAILED WITH %d at %d", dwError, n);
			errorCode = UNLOADKEY_FAILED;

			::Sleep(500);
		}
	}
	catch (Exception* px)
	{
		errorCode = UNLOAD_EXCEPTION;
		_TRACE_EX(px);
		delete px;
	}
	catch (...)
	{
		errorCode = UNLOAD_EXCEPTION;
		_TRACE_EX_UNK();
	}
	return errorCode;
}

bool RSAPI::IsRegistryMounted(HKEY hRootKey, const string& strRegName)
{
	bool fMounted = false;

	HKEY hKey;
	if (ERROR_SUCCESS == ::RegOpenKeyExW(hRootKey, strRegName, 0, KEY_READ, &hKey))
		fMounted = true;
	::RegCloseKey(hKey);

	return fMounted;
}

}
}