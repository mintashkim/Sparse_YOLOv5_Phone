#include "StdAfx.h"
#include <Library/API/AppAPI.h>
#include <Library/deelx.h>
#include <NoAD/EHALS.h>
#include <NoAD/ipc/all.h>

#include <process.h>

#include <shlobj.h>
#include <iphlpapi.h>
#include <Psapi.h>
#include <Shlwapi.h>
#include <Winsock2.h>

#pragma comment (lib, "psapi")
#pragma comment (lib, "Shlwapi")
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "iphlpapi.lib")

#include "wpp.h"
#include "AppAPI.tmh"

typedef DWORD (WINAPI *GETADAPTERSINFO)( PIP_ADAPTER_INFO, PULONG );
typedef DWORD (WINAPI *GETBESTINTERFACE)( IPAddr, PDWORD );

#define MIN_PASSWORD_LENGTH			8
#define MAX_PASSWORD_REPETITION		3

namespace Library
{
namespace API
{

	bool AppAPI::IsSupportedWindows()
	{
		static bool fChecked = false;
		static bool fIsSupport = false;
		if (!fChecked)
		{
			OSName osName = Environment::GetOSName();
			_TRACE_I(Application, L"%ws() - Current windows version: %d", __FUNCTIONW__, osName);

			if (SUPPORTED_WIN_VERSION_MIN > osName ||
				SUPPORTED_WIN_VERSION_MAX < osName)
			{
				_TRACE_W(Application, L"%ws() - Unsupported windows version. min:%d, max:%d", __FUNCTIONW__, SUPPORTED_WIN_VERSION_MIN, SUPPORTED_WIN_VERSION_MAX);
				fIsSupport = false;
			}
			else
			{
				fIsSupport = true;
			}
			fChecked = true;
		}
		return fIsSupport;
	}

	string AppAPI::GetPCName()
	{
		string str;
		array<wchar_t> awc(256);
		DWORD dwLen = awc.GetLength();
		if (::GetComputerNameW(awc.GetBuffer(), &dwLen))
			str = (const wchar_t*)awc.GetBuffer();
		return str;
	}

	string AppAPI::GetUserName()
	{
		string str;
		array<wchar_t> awc(256);
		DWORD dwLen = awc.GetLength();
		if (::GetUserNameW(awc.GetBuffer(), &dwLen))
			str = (const wchar_t*)awc.GetBuffer();
		return str;
	}

	string AppAPI::GetLocalIPWithAdapterName()
	{
		PIP_ADAPTER_INFO pAdapterInfo;
		PIP_ADAPTER_INFO pAdapter = NULL;
		DWORD dwRetVal = 0;

		string strMyIP = L"0.0.0.0";

		ULONG ulOutBufLen = sizeof (IP_ADAPTER_INFO);
		pAdapterInfo = (IP_ADAPTER_INFO *) malloc(sizeof (IP_ADAPTER_INFO));

		if ( pAdapterInfo != NULL )
		{
			memset( pAdapterInfo, 0, sizeof (IP_ADAPTER_INFO) );
			try
			{
				if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
				{
					free(pAdapterInfo);
					pAdapterInfo = (IP_ADAPTER_INFO *) malloc(ulOutBufLen);
					if (pAdapterInfo == NULL)
					{
						throw new OutOfMemoryException();
					}
				}

				if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR)
				{
					pAdapter = pAdapterInfo;

					while (pAdapter)
					{
						string strNICName(pAdapter->Description);
						string strIP(pAdapter->IpAddressList.IpAddress.String);

						if (!strNICName.ToLower().Contains(L"mirageworks") && 
							!strIP.IsEmpty() && !strIP.Equals(L"0.0.0.0"))
						{
							strMyIP = pAdapter->IpAddressList.IpAddress.String;
							break;
						}
						pAdapter = pAdapter->Next;
					}
				}
			}
			catch ( Exception* pe )
			{
				_TRACE_EX(pe);
				delete pe;
			}
		}

		if (pAdapterInfo) 
			free(pAdapterInfo);

		return strMyIP;
	}

	string AppAPI::GetLocalIP()
	{
		string strMyIP = L"127.0.0.1";

		DWORD dwErr, dwAdapterInfoSize = 0;
		PIP_ADAPTER_INFO	pAdapterInfo, pAdapt;
		PIP_ADDR_STRING	pAddrStr;

		if( ( dwErr = ::GetAdaptersInfo( NULL, &dwAdapterInfoSize ) ) != 0 )
		{
			if( dwErr != ERROR_BUFFER_OVERFLOW )
			{
				strMyIP = GetLocalIPWithAdapterName();
				return strMyIP;
			}
		}

		// Allocate memory from sizing information
		if( ( pAdapterInfo = (PIP_ADAPTER_INFO) GlobalAlloc( GPTR, dwAdapterInfoSize )) == NULL )
		{
			strMyIP = GetLocalIPWithAdapterName();
			return strMyIP;
		}

		// Get actual adapter information
		if( ( dwErr = ::GetAdaptersInfo( pAdapterInfo, &dwAdapterInfoSize ) ) != 0 )
		{
			strMyIP = GetLocalIPWithAdapterName();
			return strMyIP;
		}

		for( pAdapt = pAdapterInfo; pAdapt; pAdapt = pAdapt->Next )
		{
			switch ( pAdapt->Type )
			{
			case MIB_IF_TYPE_ETHERNET:
			case MIB_IF_TYPE_PPP:
				if( strlen( pAdapt->GatewayList.IpAddress.String ) > 0 )
				{
					DWORD	dwGwIp, dwMask, dwIp, dwGwNetwork, dwNetwork;

					dwGwIp = inet_addr( pAdapt->GatewayList.IpAddress.String );
					for( pAddrStr = &(pAdapt->IpAddressList); pAddrStr; pAddrStr = pAddrStr->Next )
					{
						if( strlen(pAddrStr->IpAddress.String) > 0 )
						{
							dwIp = inet_addr( pAddrStr->IpAddress.String );
							dwMask = inet_addr( pAddrStr->IpMask.String );
							dwNetwork = dwIp & dwMask;
							dwGwNetwork = dwGwIp & dwMask;

							if( dwGwNetwork == dwNetwork )
							{
								strMyIP = pAddrStr->IpAddress.String;
								if(!strMyIP.Equals(L"0.0.0.0"))
								{
									GlobalFree( pAdapterInfo );
									//Trace_WriteLine(L"Get Local IP : %ws", (const wchar_t*)strMyIP);
									return strMyIP;
								}
								break;
							}
						}
					}
				}
				break;
			default:
				break;
			}
		}

		if(strMyIP.Equals(L"127.0.0.1") || strMyIP.Equals(L"0.0.0.0"))
			strMyIP = GetLocalIPWithAdapterName();

		GlobalFree( pAdapterInfo );

		return strMyIP;
	}

	string AppAPI::GetDeviceID()
	{
		// For 1.9 하위호환성
		// 	string strDeviceId;
		// 	Application::GetCommonAppDataRegistry()->GetValue(L"DeviceId", strDeviceId);
		// 	if (!strDeviceId.IsEmpty())
		// 		return strDeviceId;
		// 	else
		//return NoAD::EHALS::GetPCId0();

		return NoAD::EHALS::GetDeviceId_Version3();
	}

	string AppAPI::GetDeviceIDVersion2()
	{
		return NoAD::EHALS::GetDeviceId_Version1();
	}

	string AppAPI::GetDeviceIDForMigration()
	{
		string str_1 = NoAD::EHALS::GetPCId0();
		string str_2 = NoAD::EHALS::GetDeviceId_Version1();

		return str_1 + L";" + str_2;
	}

	string AppAPI::ExpandSpecialFolder( string strPath )
	{
		if (strPath.StartsWith(L"$("))
		{
			if ( !strPath.EndsWith(L"\\") ) 
				strPath += L"\\";
			int nIndex = strPath.IndexOf(L'\\');
			string strSpecialFolder = strPath.Substring(0, nIndex);
			string strRealPath;

			if (strSpecialFolder == L"$(ApplicationData)")
				strRealPath = Environment::GetFolderPath(ESpecialFolder:: ApplicationData);
			else if (strSpecialFolder == L"$(BitBucket)")
				strRealPath = Environment::GetFolderPath(ESpecialFolder:: BitBucket);
			else if (strSpecialFolder == L"$(CommonApplicationData)")
				strRealPath = Environment::GetFolderPath(ESpecialFolder:: CommonApplicationData);
			else if (strSpecialFolder == L"$(CommonDeskTopDirectory)")
				strRealPath = Environment::GetFolderPath(ESpecialFolder:: CommonDeskTopDirectory);
			else if (strSpecialFolder == L"$(CommonFavorites)")
				strRealPath = Environment::GetFolderPath(ESpecialFolder:: CommonFavorites);
			else if (strSpecialFolder == L"$(CommonProgramFiles)")
				strRealPath = Environment::GetFolderPath(ESpecialFolder:: CommonProgramFiles);
			else if (strSpecialFolder == L"$(CommonPrograms)")
				strRealPath = Environment::GetFolderPath(ESpecialFolder:: CommonPrograms);
			else if (strSpecialFolder == L"$(CommonStartMenu)")
				strRealPath = Environment::GetFolderPath(ESpecialFolder:: CommonStartMenu);
			else if (strSpecialFolder == L"$(CommonStartup)")
				strRealPath = Environment::GetFolderPath(ESpecialFolder:: CommonStartup);
			else if (strSpecialFolder == L"$(Cookies)")
				strRealPath = Environment::GetFolderPath(ESpecialFolder:: Cookies);
			else if (strSpecialFolder == L"$(Desktop)")
				strRealPath = Environment::GetFolderPath(ESpecialFolder:: Desktop);
			else if (strSpecialFolder == L"$(DesktopDirectory)")
				strRealPath = Environment::GetFolderPath(ESpecialFolder:: DesktopDirectory);
			else if (strSpecialFolder == L"$(Favorites)")
				strRealPath = Environment::GetFolderPath(ESpecialFolder:: Favorites);
			else if (strSpecialFolder == L"$(History)")
				strRealPath = Environment::GetFolderPath(ESpecialFolder:: History);
			else if (strSpecialFolder == L"$(InternetCache)")
				strRealPath = Environment::GetFolderPath(ESpecialFolder:: InternetCache);
			else if (strSpecialFolder == L"$(LocalApplicationData)")
				strRealPath = Environment::GetFolderPath(ESpecialFolder:: LocalApplicationData);
			else if (strSpecialFolder == L"$(MyComputer)")
				strRealPath = Environment::GetFolderPath(ESpecialFolder:: MyComputer);
			else if (strSpecialFolder == L"$(MyDocuments)")
				strRealPath = Environment::GetFolderPath(ESpecialFolder:: MyDocuments);
			else if (strSpecialFolder == L"$(MyPictures)")
				strRealPath = Environment::GetFolderPath(ESpecialFolder:: MyPictures);
			else if (strSpecialFolder == L"$(Personal)")
				strRealPath = Environment::GetFolderPath(ESpecialFolder:: Personal);
			else if (strSpecialFolder == L"$(ProgramFiles)")
				strRealPath = Environment::GetFolderPath(ESpecialFolder:: ProgramFiles);
			else if (strSpecialFolder == L"$(Programs)")
				strRealPath = Environment::GetFolderPath(ESpecialFolder:: Programs);
			else if (strSpecialFolder == L"$(Recent)")
				strRealPath = Environment::GetFolderPath(ESpecialFolder:: Recent);
			else if (strSpecialFolder == L"$(SendTo)")
				strRealPath = Environment::GetFolderPath(ESpecialFolder:: SendTo);
			else if (strSpecialFolder == L"$(StartMenu)")
				strRealPath = Environment::GetFolderPath(ESpecialFolder:: StartMenu);
			else if (strSpecialFolder == L"$(Startup)")
				strRealPath = Environment::GetFolderPath(ESpecialFolder:: Startup);
			else if (strSpecialFolder == L"$(System)")
				strRealPath = Environment::GetFolderPath(ESpecialFolder:: System);
			else if (strSpecialFolder == L"$(Templates)")
				strRealPath = Environment::GetFolderPath(ESpecialFolder:: Templates);
			else if (strSpecialFolder == L"$(Windows)")
				strRealPath = Environment::GetFolderPath(ESpecialFolder:: Windows);
			else if (strSpecialFolder == L"$(AllUsersProfile)")
				strRealPath = Environment::GetEnvironmentVariable(L"ALLUSERSPROFILE");
			else if (strSpecialFolder == L"$(UserProfile)")
				strRealPath = Environment::GetEnvironmentVariable(L"USERPROFILE");
			else if (strSpecialFolder == L"$(SystemDrive)")
				strRealPath = Environment::GetEnvironmentVariable(L"SystemDrive");
			else if (strSpecialFolder == L"$(ProgramFilesX86)")
				strRealPath = Environment::GetEnvironmentVariable(L"ProgramFiles(x86)");
			else if (strSpecialFolder == L"$(MirageWorksPath)")
				strRealPath = Application::GetStartupPath();
			else
				strRealPath = L"";

			if (!strRealPath.IsEmpty())
				strPath = strPath.Replace(strSpecialFolder, strRealPath);
		}
		if (strPath.EndsWith(L"\\"))
			strPath = strPath.Substring(0, strPath.GetLength() - 1);
		return strPath;
	}

	bool AppAPI::ContainsPendingFileRenameOperations(const string& str)
	{
		bool fResult = false;

		RegistryKey *pKey = NULL;
		try
		{
			pKey = Registry::GetLocalMachine()->OpenSubKey(L"SYSTEM\\CurrentControlSet\\Control\\Session Manager");
			if (pKey != NULL)
			{
				array<string> astr;
				pKey->GetValue(L"PendingFileRenameOperations", astr);

				for (int i = 0 ; i < astr.GetLength() ; i++)
				{
					if (astr[i].ToLower().Contains(str.ToLower()))
					{
						fResult = true;
						break;
					}
				}
			}
		}
		catch (Exception *pe)
		{
			_TRACE_EX(pe);
			delete pe;
		}
		if (pKey != NULL)
			delete pKey;

		return fResult;
	}

	void AppAPI::RaiseEvent(const string& strName, bool fOnlyExists, bool fThrowOnFailed)
	{
		try
		{
			EventWaitHandle *pHandle = NULL;
			if (fOnlyExists)
				pHandle = EventWaitHandle::OpenExisting(strName);
			else
				pHandle = new EventWaitHandle(false, EEventResetMode::AutoReset, strName);

			if (pHandle != NULL)
			{
				pHandle->Set();
				delete pHandle;
				pHandle = NULL;
			}
		}
		catch (Exception *pe)
		{
			_TRACE_EX(pe);
			if (fThrowOnFailed)
				throw pe;
			else
				delete pe;
		}
	}

	void AppAPI::WaitEvent(const string& strName, int nTimeout, bool fOnlyExists, bool fThrowOnFailed)
	{
		try
		{
			EventWaitHandle *pHandle = NULL;
			if (fOnlyExists)
				pHandle = EventWaitHandle::OpenExisting(strName);
			else
				pHandle = new EventWaitHandle(false, EEventResetMode::AutoReset, strName);

			if (pHandle != NULL)
			{
				pHandle->WaitOne(nTimeout);
				delete pHandle;
				pHandle = NULL;
			}
		}
		catch (Exception *pe)
		{
			_TRACE_EX(pe);
			if (fThrowOnFailed)
				throw pe;
			else
				delete pe;
		}
	}

	bool AppAPI::RaiseEventExisting(const string& strName)
	{
		bool fRet = false;
		EventWaitHandle *pHandle = null;
		try
		{
			pHandle = EventWaitHandle::OpenExisting(strName);
			if (pHandle != null)
			{
				pHandle->Set();
				fRet = true;
			}
		}
		catch (Exception *pe)
		{
			delete pe;
		}
		if (pHandle != null)
			delete pHandle;
		return fRet;
	}

	bool AppAPI::ExistsEvent(const string& strName)
	{
		bool fRet = false;
		EventWaitHandle *pHandle = null;
		try
		{
			pHandle = EventWaitHandle::OpenExisting(strName);
			if (pHandle != null)
				fRet = true;
		}
		catch (Exception *pe)
		{
			delete pe;
		}
		if (pHandle != null)
			delete pHandle;
		return fRet;
	}

	bool AppAPI::IsSamsungSSOPC(bool fCheckEpTrayRunning)
	{
		bool bSamsungSsoExist = true;

		wchar_t szSystem32Path[MAX_PATH] = {0,};
#ifdef _M_X64
		SHGetSpecialFolderPath( NULL, szSystem32Path, CSIDL_SYSTEMX86, FALSE );
#else
		SHGetSpecialFolderPath( NULL, szSystem32Path, CSIDL_SYSTEM, FALSE );
#endif
		string strSystem32 = string(szSystem32Path);
		string strSsoOcxModulePath = strSystem32 + L"\\EpAdm2.ocx";
		string strSsoDllModulePath = strSystem32 + L"\\EpTrayUtil.dll";

		_TRACE_I(Application, L"sso ocx path = %ws", (const wchar_t*)strSsoOcxModulePath);
		_TRACE_I(Application, L"sso dll path = %ws", (const wchar_t*)strSsoDllModulePath);
		if ( !PathFileExists(strSsoOcxModulePath) )
		{
			_TRACE_I(Application, L"EpAdm2.ocx is not found.");
			bSamsungSsoExist = false;
		}

		if ( !PathFileExists(strSsoDllModulePath) )
		{
			_TRACE_I(Application, L"eptrayutil.dll is not found.");
			bSamsungSsoExist = false;
		}

		if (bSamsungSsoExist && fCheckEpTrayRunning)
			bSamsungSsoExist = _IsSamSungSsoEpTrayRunning();

		return bSamsungSsoExist;
	}

	bool AppAPI::_IsSamSungSsoEpTrayRunning()
	{
		bool fRet = false;

		string strSsoEptrayPath = string("mySingle\\Component\\eptray.exe").ToLower();
		array<Process*> arProcesses = Process::GetProcesses();
		int nCount = arProcesses.GetLength();

		for (int n = 0; n < nCount; n++)
		{
			string strProcessPath = _GetProcessPathWithPID(arProcesses[n]->GetId()).ToLower();
			if (strProcessPath.Contains(strSsoEptrayPath)) 
			{
				fRet = true;
				break;
			}
		}

		for (int n = 0 ; n < nCount ; n++)
			delete arProcesses[n];

		return fRet;
	}

	string AppAPI::_GetProcessPathWithPID(int nPID)
	{
		string str;

		HANDLE hProcess = NULL;
		HMODULE hPSAPI = NULL;
		try
		{
			hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, nPID);
			if (NULL == hProcess)
				throw new Win32Exception(::GetLastError());

			array<wchar_t> awc(MAX_PATH);
			DWORD dwLength = 0;
			if (Version(5, 1) <= Environment::GetOSVersion()->GetVersion())
			{
				//	Windows XP 이상 
				hPSAPI = ::LoadLibraryW(L"psapi.dll");
				if (NULL == hPSAPI)
					throw new Win32Exception(::GetLastError());

				typedef DWORD (WINAPI *PFGetProcessImageFileNameW)(HANDLE, LPWSTR, DWORD);
				PFGetProcessImageFileNameW pfGetProcessImageFileNameW = (PFGetProcessImageFileNameW)::GetProcAddress(hPSAPI, "GetProcessImageFileNameW");
				if (NULL == pfGetProcessImageFileNameW)
					throw new Win32Exception(::GetLastError());

				dwLength = pfGetProcessImageFileNameW(hProcess, awc.GetBuffer(), awc.GetLength());
				if (0 == dwLength)
					throw new Win32Exception(::GetLastError());
			}
			else
			{
				dwLength = ::GetModuleFileNameExW(hProcess, NULL, awc.GetBuffer(), awc.GetLength());
				if (0 == dwLength)
					throw new Win32Exception(::GetLastError());
			}

			string strKernelPath = string(awc, 0, (int)dwLength);
			str = _FromKernelPath(strKernelPath);
		}
		catch (Exception* pe)
		{
			_TRACE_EX(pe);
			delete pe;
		}

		if (NULL != hProcess)
			::CloseHandle(hProcess);
		if (NULL != hPSAPI)
			::FreeLibrary(hPSAPI);

		return str;
	}

	string AppAPI::_FromKernelPath(string strKernelPath)
	{
		Dictionary<string, string> m_dicDeviceName;
		{
			string str = L"A:";
			DWORD dw = 0;
			array<wchar_t> awc(MAX_PATH);
			for (int n = 0; n < 26; n++)
			{
				str.GetChars()[0] = L'A' + n;
				dw = ::QueryDosDeviceW(str, awc.GetBuffer(), awc.GetLength());
				if (0 != dw)
				{
					string strDeviceName = string(awc, 0, (int)dw);
					m_dicDeviceName.Add(strDeviceName, L"\\??\\" + str);
				}
			}
		}

		{
			string str = strKernelPath;
			if (str.StartsWith(L"\\Device\\"))
			{
				int nCount = 3;
				int nIndex = -1;
				for (int n = 0; n < str.GetLength(); n++)
				{
					if (L'\\' == str.GetChars()[n] && 0 == --nCount)
					{
						nIndex = n;
						break;
					}
				}
				if (0 <= nIndex)
				{
					string strHead = str.Substring(0, nIndex);
					if (m_dicDeviceName.TryGetValue(strHead, strHead))
						str = strHead + str.Substring(nIndex);
				}
			}
			if (str.StartsWith(L"\\??\\") && 4 < str.GetLength())
				str = str.Substring(4);
			if (str.StartsWith(L"\\DosDevices\\") && 12 < str.GetLength())
				str = str.Substring(12);
			if (str.Contains(L'~'))
				str = _ToLongPath(str);
			if (str.EndsWith(L"\\"))
				str = str.Substring(0, str.GetLength() - 1);

			return str;
		}
	}

	string AppAPI::_ToLongPath(string strPath)
	{
		string str = strPath;

		array<wchar_t> awc(4096);
		DWORD dwLength = ::GetLongPathNameW(strPath, awc.GetBuffer(), awc.GetLength());
		if (0 < dwLength)
			str = string(awc, 0, dwLength);
		else
		{
			string strParentPath;
			string strTail = strPath;
			int nIndex = strPath.LastIndexOf(L'\\');
			if (0 <= nIndex)
			{
				strParentPath = strPath.Substring(0, nIndex);
				strTail = strPath.Substring(nIndex);
			}

			if (!strParentPath.IsEmpty())
				str = _ToLongPath(strParentPath) + strTail;
			else
				str = strTail;
		}

		return str;
	}

	string AppAPI::MakeCommonExtensionUrl(string strExtension, string strServerHost, string strSessionId)
	{
		string strResult;

		if (strExtension.ToLower().StartsWith(L"https://") || strExtension.ToLower().StartsWith(L"http://"))
			strResult = strExtension + strSessionId;
		else if (strServerHost.ToLower().StartsWith(L"https://") || strServerHost.ToLower().StartsWith(L"http://"))
			strResult = strServerHost + strExtension + strSessionId;
		else
			strResult = L"https://" + strServerHost + strExtension + strSessionId;

		return strResult;
	}

	bool AppAPI::IsComplexPassword(string strPassword, string strValidReg, string strInvalidReg)
	{
		if (strValidReg.GetLength() > 0 && strInvalidReg.GetLength() > 0)
		{
			wchar_t * text = (wchar_t*)strPassword;

			CRegexpT <wchar_t> regexpForValid((const wchar_t*)strValidReg, IGNORECASE | MULTILINE);
			if ( !regexpForValid.Match(text).IsMatched() )  return false;

			CRegexpT <wchar_t> regexpForInvalid((const wchar_t*)strInvalidReg, IGNORECASE | MULTILINE);
			if ( regexpForInvalid.Match(text).IsMatched() ) return false;

			return true;
		}
		else
		{
			// Password는 8자 이상이어야함.
			if ( strPassword.GetLength() < MIN_PASSWORD_LENGTH ) return false;

			bool bResult = true;

			DWORD dwNumericCount = 0;
			DWORD dwAlphabetCount = 0;
			DWORD dwSpCharacterCount = 0;
			array<byte> abPassword = Encoding::GetUTF8()->GetBytes(strPassword);

			DWORD dwRepetition = 0;
			char cBefore = 0;

			for ( int n = 0; n < abPassword.GetLength(); n++ )
			{
				if ( abPassword[n] >= '0' && abPassword[n] <='9' ) dwNumericCount++;
				else if ( abPassword[n] >= 'a' && abPassword[n] <='z' ) dwAlphabetCount++;
				else if ( abPassword[n] >= 'A' && abPassword[n] <='Z' ) dwAlphabetCount++;
				else if ( abPassword[n] >= '!' && abPassword[n] <='/' ) dwSpCharacterCount++;
				else if ( abPassword[n] >= ':' && abPassword[n] <='@' ) dwSpCharacterCount++;
				else if ( abPassword[n] >= '[' && abPassword[n] <='`' ) dwSpCharacterCount++;
				else if ( abPassword[n] >= '{' && abPassword[n] <='~' ) dwSpCharacterCount++;

				// 같은 문자가  3번이상 반복되지 않아야함.
				if ( cBefore == abPassword[n] )
				{
					dwRepetition++;
					if ( dwRepetition >= MAX_PASSWORD_REPETITION ) return false;
				}
				else
				{
					cBefore = abPassword[n];
					dwRepetition = 1;
				}
			}

			// 알파벳과 숫자가 있어야함.
			if ( dwNumericCount == 0 || dwAlphabetCount == 0 )
				bResult = false;

			return bResult;
		}
	}

}
}