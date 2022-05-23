#include "StdAfx.h"
#include <Library/WindowProfile.h>
#include <Library/API/SecurityAPI.h>
#include <TlHelp32.h>

#include "wpp.h"
#include "WindowProfile.tmh"

namespace Library
{

WindowProfile::WindowProfile( void )
{
	m_strSID = L"";
	m_pUserKey = NULL;
	m_pUserAppDataKey = NULL;
	m_hUserToken = NULL;
}

WindowProfile::~WindowProfile( void )
{
	if (m_hUserToken != NULL)
		::CloseHandle(m_hUserToken);
	if (m_pUserKey != NULL)
		delete m_pUserKey;
	if (m_pUserAppDataKey != NULL)
		delete m_pUserAppDataKey;
}

WindowProfile* WindowProfile::_GetInstance()
{
	Singletons *ps = (Singletons*)GetSingletons();
	WindowProfile* pts = (WindowProfile*)ps->GetSingleton(L"Library::WindowProfile");
	if (pts == NULL)
	{
		pts = new WindowProfile();
		ps->SetSingleton(L"Library::WindowProfile", pts);
	}
	return pts;
}

void WindowProfile::_Initialize(string strSID)
{
	if (strSID.IsEmpty())
		throw new ArgumentException;

	m_strSID = strSID;

	// Current Sid's registry key.
	try
	{
		m_pUserKey = Registry::GetUsers()->OpenSubKey(m_strSID, true);
	}
	catch (Exception* pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}

	// Current Sid's appdata registry key.
	try
	{
		string strRegPath;
		string strCompanyName = Application::GetCompanyName();
		string strProductName = Application::GetProductName();

		if (!strCompanyName.IsEmpty())
			strRegPath = string::Format(L"Software\\%ws\\%ws", (const wchar_t*)strCompanyName, (const wchar_t*)strProductName);
		else
			strRegPath = string::Format(L"Software\\%ws", (const wchar_t*)strProductName);

		m_pUserAppDataKey = m_pUserKey->CreateSubKey(strRegPath);
	}
	catch (Exception* pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}

	// Current Sid's the highest token.
	HANDLE hSnapProcess = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (INVALID_HANDLE_VALUE != hSnapProcess)
	{
		PROCESSENTRY32 pe32 = { 0 };
		pe32.dwSize = sizeof(PROCESSENTRY32);

		for (BOOL f = ::Process32First(hSnapProcess, &pe32); f; f = ::Process32Next(hSnapProcess, &pe32))
		{
			HANDLE _hToken = NULL;
			try
			{
				_hToken = Library::API::SecurityAPI::GetProcessToken(pe32.th32ProcessID);
				if (m_strSID.ToLower() == Library::API::SecurityAPI::ToUserStringSID(_hToken).ToLower())
				{
// 					if (!Library::API::SecurityAPI::IsElevatedToken(_hToken))
// 					{
// 						DWORD dwLength = 0;
// 						TOKEN_LINKED_TOKEN tlt = {0};
// 						if (!::GetTokenInformation(_hToken, TokenLinkedToken, &tlt, sizeof(tlt), &dwLength))
// 							throw new Win32Exception(::GetLastError());
// 						m_hUserToken = tlt.LinkedToken;
// 					}
// 					else
// 					{
// 						m_hUserToken = _hToken;
// 					}
					m_hUserToken = _hToken;
					break;
				}
			}
			catch (Exception* px)
			{
				_TRACE_EX(px);
				delete px;
			}

			if (NULL != _hToken)
				::CloseHandle(_hToken);
		}
		::CloseHandle(hSnapProcess);
	}

	RegistryKey *pProfile = NULL;
	try
	{
		pProfile = Registry::GetLocalMachine()->OpenSubKey(L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList\\" + m_strSID);
		if (pProfile != NULL)
		{
			string strValue;
			pProfile->GetValue(L"ProfileImagePath", strValue);

			if (!strValue.IsEmpty())
				m_strUserProfilePath = Environment::ExpandEnvironmentVariables(strValue);
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
	if (pProfile != NULL)
		delete pProfile;

	if (m_pUserKey == NULL)
		throw new ArgumentException;
	if (m_pUserAppDataKey == NULL)
		throw new ArgumentException;
	if (m_hUserToken == NULL)
		throw new ArgumentException;
	if (m_strUserProfilePath.IsEmpty())
		throw new ArgumentException;

	_TRACE_I(Application, L"WindowProfile is initialized successfully.");
}

string WindowProfile::_GetSID()
{
	if (m_strSID.IsEmpty())
		throw new InvalidOperationException;

	return m_strSID;
}

RegistryKey* WindowProfile::_GetUserKey()
{
	if (m_pUserKey == NULL)
		throw new InvalidOperationException;

	return m_pUserKey;
}

RegistryKey* WindowProfile::_GetUserAppDataKey()
{
	if (m_pUserAppDataKey == NULL)
		throw new InvalidOperationException;

	return m_pUserAppDataKey;
}

HANDLE WindowProfile::_GetUserToken()
{
	if (m_hUserToken == NULL)
		throw new InvalidOperationException;

	return m_hUserToken;
}

string WindowProfile::_GetUserProfilePath()
{
	if (m_strUserProfilePath.IsEmpty())
		throw new InvalidOperationException;

	return m_strUserProfilePath;
}
}