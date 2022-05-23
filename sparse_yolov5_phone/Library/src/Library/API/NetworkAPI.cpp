#include "StdAfx.h"
#include <Library/API/NetworkAPI.h>
#include <Library/API/ProcessAPI.h>
#include <Library/ntdll.h>

#include <iphlpapi.h>
#pragma comment (lib, "iphlpapi.lib")

#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment (lib, "Ws2_32.lib")

#include <wpp.h>
#include <NetworkAPI.tmh>

#define FIREWALL_ADD_COMMAND				L"netsh firewall add allowedprogram \"%ws\" \"%ws\" ENABLE"
#define FIREWALL_ADD_COMMAND_ADVANCED		L"netsh advfirewall firewall add rule name=\"%ws\" dir=in action=allow program=\"%ws\" enable=yes"
#define FIREWALL_ENABLE_COMMAND				L"netsh firewall set rule name=\"%ws\" new enable=yes"
#define FIREWALL_ENABLE_COMMAND_ADVANCED	L"netsh advfirewall firewall set rule name=\"%ws\" new enable=yes"

#define FIREWALL_DELETE_COMMAND				L"netsh firewall del allowedprogram \"%ws\""
#define FIREWALL_DELETE_COMMAND_ADVANCED	L"netsh advfirewall firewall delete rule name=\"%ws\""
#define FIREWALL_DISABLE_COMMAND			L"netsh firewall set rule name=\"%ws\" new enable=no"
#define FIREWALL_DISABLE_COMMAND_ADVANCED	L"netsh advfirewall firewall set rule name=\"%ws\" new enable=no"

typedef LONG(NTAPI* PFRtlIpv4StringToAddressW)(
	__in PCWSTR S,
	__in BOOLEAN Strict,
	__out LPCWSTR *Terminator,
	__out struct in_addr *Addr
	);
typedef PWSTR(NTAPI* PFRtlIpv4AddressToStringW)(
	_In_ const struct in_addr *Addr,
	_Out_writes_(16) PWSTR S
	);

namespace Library
{
namespace API
{
	FirewallWorker::FirewallWorker()
	{
		HRESULT hr = S_OK;
		comInit = E_FAIL;
		fwProfile = NULL;

		// Initialize COM.
		comInit = CoInitializeEx(
			0,
			COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE
			);

		if (comInit != RPC_E_CHANGED_MODE)
		{
			hr = comInit;
			if (FAILED(hr))
			{
				_TRACE_I(Application, L"CoInitializeEx failed: 0x%08lx\n", hr);
				_WidnowsFirewallClose();
			}
		}
	}

	FirewallWorker::~FirewallWorker(void)
	{
		_WidnowsFirewallClose();
	}

	HRESULT FirewallWorker::_WindowsFirewallOpen()
	{
		HRESULT hr = _WindowsFirewallInitialize(&fwProfile);
		if (FAILED(hr))
		{
			_TRACE_I(Application, L"WindowsFirewallInitialize failed: 0x%08lx\n", hr);
			_WidnowsFirewallClose();
		}
		return hr;
	}

	void FirewallWorker::_WidnowsFirewallClose()
	{
		// Release the firewall profile.
		_WindowsFirewallCleanup(fwProfile);

		// Uninitialize COM.
		if (SUCCEEDED(comInit))
		{
			CoUninitialize();
		}
	}

	HRESULT FirewallWorker::_WindowsFirewallInitialize(OUT INetFwProfile** fwProfile)
	{
		HRESULT hr = S_OK;
		INetFwMgr* fwMgr = NULL;
		INetFwPolicy* fwPolicy = NULL;

		_ASSERT(fwProfile != NULL);

		*fwProfile = NULL;

		// Create an instance of the firewall settings manager.
		hr = CoCreateInstance(
			__uuidof(NetFwMgr),
			NULL,
			CLSCTX_INPROC_SERVER,
			__uuidof(INetFwMgr),
			(void**)&fwMgr
			);
		if (FAILED(hr))
		{
			_TRACE_I(Application, L"CoCreateInstance failed: 0x%08lx\n", hr);
			goto error;
		}

		// Retrieve the local firewall policy.
		hr = fwMgr->get_LocalPolicy(&fwPolicy);
		if (FAILED(hr))
		{
			_TRACE_I(Application, L"get_LocalPolicy failed: 0x%08lx\n", hr);
			goto error;
		}

		// Retrieve the firewall profile currently in effect.
		hr = fwPolicy->get_CurrentProfile(fwProfile);
		if (FAILED(hr))
		{
			_TRACE_I(Application, L"get_CurrentProfile failed: 0x%08lx\n", hr);
			goto error;
		}

error:

		// Release the local firewall policy.
		if (fwPolicy != NULL)
		{
			fwPolicy->Release();
		}

		// Release the firewall settings manager.
		if (fwMgr != NULL)
		{
			fwMgr->Release();
		}

		return hr;
	}

	void FirewallWorker::_WindowsFirewallCleanup(IN INetFwProfile* fwProfile)
	{
		// Release the firewall profile.
		if (fwProfile != NULL)
		{
			fwProfile->Release();
		}
	}

	HRESULT FirewallWorker::_WindowsFirewallIsOn(IN INetFwProfile* fwProfile, OUT BOOL* fwOn)
	{
		HRESULT hr = S_OK;
		VARIANT_BOOL fwEnabled;

		_ASSERT(fwProfile != NULL);
		_ASSERT(fwOn != NULL);

		*fwOn = FALSE;

		// Get the current state of the firewall.
		hr = fwProfile->get_FirewallEnabled(&fwEnabled);
		if (FAILED(hr))
		{
			_TRACE_I(Application, L"get_FirewallEnabled failed: 0x%08lx\n", hr);
			goto error;
		}

		// Check to see if the firewall is on.
		if (fwEnabled != VARIANT_FALSE)
		{
			*fwOn = TRUE;
			_TRACE_I(Application, L"The firewall is on.\n");
		}
		else
		{
			_TRACE_I(Application, L"The firewall is off.\n");
		}

error:

		return hr;
	}

	HRESULT FirewallWorker::_WindowsFirewallTurnOn(IN INetFwProfile* fwProfile)
	{
		HRESULT hr = S_OK;
		BOOL fwOn;

		_ASSERT(fwProfile != NULL);

		// Check to see if the firewall is off.
		hr = _WindowsFirewallIsOn(fwProfile, &fwOn);
		if (FAILED(hr))
		{
			_TRACE_I(Application, L"WindowsFirewallIsOn failed: 0x%08lx\n", hr);
			goto error;
		}

		// If it is, turn it on.
		if (!fwOn)
		{
			// Turn the firewall on.
			hr = fwProfile->put_FirewallEnabled(VARIANT_TRUE);
			if (FAILED(hr))
			{
				_TRACE_I(Application, L"put_FirewallEnabled failed: 0x%08lx\n", hr);
				goto error;
			}

			_TRACE_I(Application, L"The firewall is now on.\n");
		}

error:

		return hr;
	}

	HRESULT FirewallWorker::_WindowsFirewallTurnOff(IN INetFwProfile* fwProfile)
	{
		HRESULT hr = S_OK;
		BOOL fwOn;

		_ASSERT(fwProfile != NULL);

		// Check to see if the firewall is on.
		hr = _WindowsFirewallIsOn(fwProfile, &fwOn);
		if (FAILED(hr))
		{
			_TRACE_I(Application, L"WindowsFirewallIsOn failed: 0x%08lx\n", hr);
			goto error;
		}

		// If it is, turn it off.
		if (fwOn)
		{
			// Turn the firewall off.
			hr = fwProfile->put_FirewallEnabled(VARIANT_FALSE);
			if (FAILED(hr))
			{
				_TRACE_I(Application, L"put_FirewallEnabled failed: 0x%08lx\n", hr);
				goto error;
			}

			_TRACE_I(Application, L"The firewall is now off.\n");
		}

error:

		return hr;
	}

	HRESULT FirewallWorker::_WindowsFirewallAppIsEnabled(
		IN INetFwProfile* fwProfile,
		IN const wchar_t* fwProcessImageFileName,
		OUT BOOL* fwAppEnabled
		)
	{
		HRESULT hr = S_OK;
		BSTR fwBstrProcessImageFileName = NULL;
		VARIANT_BOOL fwEnabled;
		INetFwAuthorizedApplication* fwApp = NULL;
		INetFwAuthorizedApplications* fwApps = NULL;

		_ASSERT(fwProfile != NULL);
		_ASSERT(fwProcessImageFileName != NULL);
		_ASSERT(fwAppEnabled != NULL);

		*fwAppEnabled = FALSE;

		// Retrieve the authorized application collection.
		hr = fwProfile->get_AuthorizedApplications(&fwApps);
		if (FAILED(hr))
		{
			_TRACE_I(Application, L"get_AuthorizedApplications failed: 0x%08lx\n", hr);
			goto error;
		}

		// Allocate a BSTR for the process image file name.
		fwBstrProcessImageFileName = SysAllocString(fwProcessImageFileName);
		if (SysStringLen(fwBstrProcessImageFileName) == 0)
		{
			hr = E_OUTOFMEMORY;
			_TRACE_I(Application, L"SysAllocString failed: 0x%08lx\n", hr);
			goto error;
		}

		// Attempt to retrieve the authorized application.
		hr = fwApps->Item(fwBstrProcessImageFileName, &fwApp);
		if (SUCCEEDED(hr))
		{
			// Find out if the authorized application is enabled.
			hr = fwApp->get_Enabled(&fwEnabled);
			if (FAILED(hr))
			{
				_TRACE_I(Application, L"get_Enabled failed: 0x%08lx\n", hr);
				goto error;
			}

			if (fwEnabled != VARIANT_FALSE)
			{
				// The authorized application is enabled.
				*fwAppEnabled = TRUE;

				_TRACE_I(Application, L"Authorized application %ws is enabled in the firewall.\n", fwProcessImageFileName);
			}
			else
			{
				_TRACE_I(Application, L"Authorized application %ws is disabled in the firewall.\n", fwProcessImageFileName);
			}
		}
		else
		{
			// The authorized application was not in the collection.
			hr = S_OK;

			_TRACE_I(Application, L"Authorized application %ws is disabled in the firewall.\n", fwProcessImageFileName);
		}

error:

		// Free the BSTR.
		SysFreeString(fwBstrProcessImageFileName);

		// Release the authorized application instance.
		if (fwApp != NULL)
		{
			fwApp->Release();
		}

		// Release the authorized application collection.
		if (fwApps != NULL)
		{
			fwApps->Release();
		}

		return hr;
	}

	HRESULT FirewallWorker::_WindowsFirewallAddApp(
		IN INetFwProfile* fwProfile,
		IN const wchar_t* fwProcessImageFileName,
		IN const wchar_t* fwName
		)
	{
		HRESULT hr = S_OK;
		BOOL fwAppEnabled;
		BSTR fwBstrName = NULL;
		BSTR fwBstrProcessImageFileName = NULL;
		INetFwAuthorizedApplication* fwApp = NULL;
		INetFwAuthorizedApplications* fwApps = NULL;

		_ASSERT(fwProfile != NULL);
		_ASSERT(fwProcessImageFileName != NULL);
		_ASSERT(fwName != NULL);

		// First check to see if the application is already authorized.
		hr = _WindowsFirewallAppIsEnabled(
			fwProfile,
			fwProcessImageFileName,
			&fwAppEnabled
			);
		if (FAILED(hr))
		{
			_TRACE_I(Application, L"WindowsFirewallAppIsEnabled failed: 0x%08lx\n", hr);
			goto error;
		}

		// Only add the application if it isn't already authorized.
		if (!fwAppEnabled)
		{
			// Retrieve the authorized application collection.
			hr = fwProfile->get_AuthorizedApplications(&fwApps);
			if (FAILED(hr))
			{
				_TRACE_I(Application, L"get_AuthorizedApplications failed: 0x%08lx\n", hr);
				goto error;
			}

			// Create an instance of an authorized application.
			hr = CoCreateInstance(
				__uuidof(NetFwAuthorizedApplication),
				NULL,
				CLSCTX_INPROC_SERVER,
				__uuidof(INetFwAuthorizedApplication),
				(void**)&fwApp
				);
			if (FAILED(hr))
			{
				_TRACE_I(Application, L"CoCreateInstance failed: 0x%08lx\n", hr);
				goto error;
			}

			// Allocate a BSTR for the process image file name.
			fwBstrProcessImageFileName = SysAllocString(fwProcessImageFileName);
			if (SysStringLen(fwBstrProcessImageFileName) == 0)
			{
				hr = E_OUTOFMEMORY;
				_TRACE_I(Application, L"SysAllocString failed: 0x%08lx\n", hr);
				goto error;
			}

			// Set the process image file name.
			hr = fwApp->put_ProcessImageFileName(fwBstrProcessImageFileName);
			if (FAILED(hr))
			{
				_TRACE_I(Application, L"put_ProcessImageFileName failed: 0x%08lx\n", hr);
				goto error;
			}

			// Allocate a BSTR for the application friendly name.
			fwBstrName = SysAllocString(fwName);
			if (SysStringLen(fwBstrName) == 0)
			{
				hr = E_OUTOFMEMORY;
				_TRACE_I(Application, L"SysAllocString failed: 0x%08lx\n", hr);
				goto error;
			}

			// Set the application friendly name.
			hr = fwApp->put_Name(fwBstrName);
			if (FAILED(hr))
			{
				_TRACE_I(Application, L"put_Name failed: 0x%08lx\n", hr);
				goto error;
			}

			// Add the application to the collection.
			hr = fwApps->Add(fwApp);
			if (FAILED(hr))
			{
				_TRACE_I(Application, L"Add failed: 0x%08lx\n", hr);
				goto error;
			}

			_TRACE_I(Application, L"Authorized application %ws is now enabled in the firewall.\n", fwProcessImageFileName);
		}

error:

		// Free the BSTRs.
		SysFreeString(fwBstrName);
		SysFreeString(fwBstrProcessImageFileName);

		// Release the authorized application instance.
		if (fwApp != NULL)
		{
			fwApp->Release();
		}

		// Release the authorized application collection.
		if (fwApps != NULL)
		{
			fwApps->Release();
		}

		return hr;
	}

	//////////////////////////////////////////////////////////////////////////
	HRESULT FirewallWorker::WindowsFirewallOpen()
	{
		HRESULT fResult = _WindowsFirewallOpen();
		return fResult;
	}
	HRESULT FirewallWorker::WindowsFirewallIsOn()
	{
		HRESULT fResult = true;
		BOOL fwOn;

		fResult = _WindowsFirewallIsOn(fwProfile,&fwOn);
		return fResult;
	}
	HRESULT FirewallWorker::WindowsFirewallTurnOn()
	{
		HRESULT fResult = true;

		try
		{
			fResult = _WindowsFirewallTurnOn(fwProfile);
		}
		catch (Exception* px)
		{
			fResult = false;
			_TRACE_EX(px);
			delete px;
		}

		return fResult;
	}
	HRESULT FirewallWorker::WindowsFirewallTurnOff()
	{
		HRESULT fResult = true;

		try
		{
			fResult = _WindowsFirewallTurnOff(fwProfile);
		}
		catch (Exception* px)
		{
			fResult = false;
			_TRACE_EX(px);
			delete px;
		}
		return fResult;
	}
	HRESULT FirewallWorker::WindowsFirewallAppIsEnabled(string strAppPath)
	{
		BOOL fwAppEnabled = false;

		try
		{
			_WindowsFirewallAppIsEnabled(fwProfile, strAppPath, &fwAppEnabled);
		}
		catch (Exception* px)
		{
			_TRACE_EX(px);
			delete px;
		}
		return fwAppEnabled ? S_OK : S_FALSE;
	}
	HRESULT FirewallWorker::WindowsFirewallAddApp(string strAppPath, string strAppName)
	{
		HRESULT fResult = true;

		try
		{
			fResult = _WindowsFirewallAddApp(fwProfile, strAppPath, strAppName);
		}
		catch (Exception* px)
		{
			fResult = false;
			_TRACE_EX(px);
			delete px;
		}

		return fResult;
	}

	// Firewall
	void NetworkAPI::AddFirewall(string strProcessName, string strProcessPath)
	{
		HRESULT fRet = S_FALSE;
		FirewallWorker* m_FirewallWorker;

		m_FirewallWorker = new FirewallWorker;

		HRESULT fFirewall = m_FirewallWorker->WindowsFirewallOpen();
		if(SUCCEEDED(fFirewall))
		{
			string strFilePath = strProcessPath;
			string strFileName = strProcessName;

			fRet = m_FirewallWorker->WindowsFirewallAppIsEnabled(strProcessPath);
			if(S_FALSE == fRet)
				fRet = m_FirewallWorker->WindowsFirewallAddApp(strFilePath, strFileName);
		}

		if(null != m_FirewallWorker)
		{
			delete m_FirewallWorker;
			m_FirewallWorker = NULL;
		}

		if(S_FALSE == fRet)
		{
			string strCommand = L"";
			if(Environment::GetOSVersion()->GetVersion() >= Version(6, 1))
				strCommand = String::Format(FIREWALL_ADD_COMMAND_ADVANCED, (const wchar_t*)strProcessName, (const wchar_t*)strProcessPath);
			else
				strCommand = String::Format(FIREWALL_ADD_COMMAND, (const wchar_t*)strProcessPath, (const wchar_t*)strProcessName);
			Library::API::ProcessAPI::ExecuteCmdLine(strCommand, false);
		}
	}

	void NetworkAPI::DeleteFirewall(string strProcessName, string strProcessPath)
	{
		string strCommand = L"";
		if(Environment::GetOSVersion()->GetVersion() >= Version(6, 1))
			strCommand = String::Format(FIREWALL_DELETE_COMMAND_ADVANCED, (const wchar_t*)strProcessPath);
		else
			strCommand = String::Format(FIREWALL_DELETE_COMMAND, (const wchar_t*)strProcessPath);
		Library::API::ProcessAPI::ExecuteCmdLine(strCommand);
	}

	void NetworkAPI::EnableFirewall(string strName)
	{
		string strCommand = L"";
		if (Environment::GetOSVersion()->GetVersion() >= Version(6, 1))
			strCommand = String::Format(FIREWALL_ENABLE_COMMAND_ADVANCED, (const wchar_t*)strName);
		else
			strCommand = String::Format(FIREWALL_ENABLE_COMMAND, (const wchar_t*)strName);
		Library::API::ProcessAPI::ExecuteCmdLine(strCommand, 1000);
	}

	void NetworkAPI::DisableFirewall(string strName)
	{
		string strCommand = L"";
		if (Environment::GetOSVersion()->GetVersion() >= Version(6, 1))
			strCommand = String::Format(FIREWALL_DISABLE_COMMAND_ADVANCED, (const wchar_t*)strName);
		else
			strCommand = String::Format(FIREWALL_DISABLE_COMMAND, (const wchar_t*)strName);
		Library::API::ProcessAPI::ExecuteCmdLine(strCommand, 1000);
	}

	string NetworkAPI::GetNetworkPnpInstanceID(string strAdapterGUID)
	{
		string strPnpInstanceID = L"";

		RegistryKey *pKey = null;
		RegistryKey *pSubKey = null;
		RegistryKey *pConnectionKey = null;

		try 
		{
			pKey = Registry::GetLocalMachine()->OpenSubKey(L"SYSTEM\\CurrentControlSet\\Control\\Network", false);

			if(null != pKey)
			{			
				array<string> astrSubKeyNames = pKey->GetSubKeyNames();

				for (int n = 0; n < astrSubKeyNames.GetLength(); n++)
				{
					try
					{
						pSubKey = pKey->OpenSubKey(astrSubKeyNames[n], false);
						if(null != pSubKey)
						{
							pConnectionKey = pSubKey->OpenSubKey(strAdapterGUID + L"\\Connection", false);
							if(null != pConnectionKey)
							{
								pConnectionKey->GetValue(L"PnpInstanceID", strPnpInstanceID);
							}
						}
					}
					catch (Exception* px)
					{
						_TRACE_EX(px);
						delete px;
					}
				}
			}
		}
		catch(Exception *pe)
		{
			_TRACE_EX(pe);
			delete pe;
		}

		if(null != pKey)
		{
			delete pKey;
			pKey = null;
		}
		if(null != pSubKey)
		{
			delete pSubKey;
			pSubKey = null;
		}
		if(null != pConnectionKey)
		{
			delete pConnectionKey;
			pConnectionKey = null;
		}

		return strPnpInstanceID;
	}

	bool NetworkAPI::ReleaseINetCfg(BOOL bHasWriteLock, INetCfg* pNetCfg)
	{
		// If write lock is present, unlock it
		if (SUCCEEDED(pNetCfg->Uninitialize()) && bHasWriteLock)
		{
			INetCfgLock* pNetCfgLock;

			// Get the locking interface
			if (SUCCEEDED(pNetCfg->QueryInterface(IID_INetCfgLock, (LPVOID *)&pNetCfgLock)))
			{
				pNetCfgLock->ReleaseWriteLock();
				ReleaseObj(pNetCfgLock);
			}
		}

		ReleaseObj(pNetCfg);
		CoUninitialize(); 

		return true;
	}

	bool NetworkAPI::NetworkChangeBindingOrder(string strPnpInstansName, bool fAfterBind /* = true */)
	{
		strPnpInstansName = String::Format(L"ms_tcpip->%ws", (const wchar_t*)strPnpInstansName);
		//Trace_WriteLine(strPnpInstansName);

		HRESULT hr = S_OK;

		INetCfg *pNetCfg = NULL; 
		INetCfgBindingPath *pNetCfgPath; 
		INetCfgComponent *pNetCfgComponent = NULL;
		INetCfgComponentBindings *pNetCfgBinding = NULL;
		INetCfgLock *pNetCfgLock = NULL;

		IEnumNetCfgBindingPath *pEnumNetCfgBindingPath = NULL;

		PWSTR szLockedBy;

		if (!SUCCEEDED(CoInitialize(NULL)))
		{
			return false;
		}

		if (S_OK != CoCreateInstance(CLSID_CNetCfg, NULL, CLSCTX_INPROC_SERVER, IID_INetCfg, (void**)&pNetCfg))
		{
			return false;
		}

		if (!SUCCEEDED(pNetCfg->QueryInterface(IID_INetCfgLock, (LPVOID *)&pNetCfgLock)))
		{
			ReleaseINetCfg(FALSE, pNetCfg);
			return false;
		}

		static const ULONG c_cmsTimeout = 5000;
		static const WCHAR c_szSampleNetcfgApp[] = L"TapRebinder (TapRebinder.exe)";

		if (!SUCCEEDED(pNetCfgLock->AcquireWriteLock(c_cmsTimeout, c_szSampleNetcfgApp, &szLockedBy)))
		{
			ReleaseObj(pNetCfgLock);
			ReleaseINetCfg(FALSE, pNetCfg);
			//Debug_WriteLine(L"Could not lock INetcfg, it is already locked by '%s'", szLockedBy);
			return false;
		}

		if (!SUCCEEDED(pNetCfg->Initialize(NULL)))
		{
			if (pNetCfgLock)
			{
				pNetCfgLock->ReleaseWriteLock();
			}

			ReleaseObj(pNetCfgLock);
			ReleaseINetCfg(FALSE, pNetCfg);
			return false;
		}
		ReleaseObj(pNetCfgLock);

		if (S_OK != pNetCfg->FindComponent(L"ms_tcpip", &pNetCfgComponent))
		{
			ReleaseINetCfg(TRUE, pNetCfg);
			return false;
		}
		if (S_OK != pNetCfgComponent->QueryInterface(IID_INetCfgComponentBindings, (LPVOID *)&pNetCfgBinding))
		{
			ReleaseObj(pNetCfgComponent);
			ReleaseINetCfg(TRUE, pNetCfg);
			return false;
		}
		if (S_OK != pNetCfgBinding->EnumBindingPaths(EBP_BELOW, &pEnumNetCfgBindingPath))
		{
			ReleaseObj(pNetCfgBinding);
			ReleaseObj(pNetCfgComponent);
			ReleaseINetCfg(TRUE, pNetCfg);
			return false;
		}
		while (S_OK == hr)
		{
			hr = pEnumNetCfgBindingPath->Next(1, &pNetCfgPath, NULL);
			LPWSTR pszwPathToken;
			pNetCfgPath->GetPathToken(&pszwPathToken);

			if (wcscmp(pszwPathToken, strPnpInstansName) == 0)
			{

				if(fAfterBind == true)
				{
					//Trace_WriteLine(L"Moving adapter to the last position: %s.\n", pszwPathToken);
					pNetCfgBinding->MoveAfter(pNetCfgPath, NULL);
				}
				else
				{
					//Trace_WriteLine(L"Moving adapter to the first position: %s.\n", pszwPathToken);
					pNetCfgBinding->MoveBefore(pNetCfgPath, NULL);
				}	
				pNetCfg->Apply();

				CoTaskMemFree(pszwPathToken);
				ReleaseObj(pNetCfgPath);

				break;
			}

			CoTaskMemFree(pszwPathToken);
			ReleaseObj(pNetCfgPath);
		}

		ReleaseObj(pEnumNetCfgBindingPath);

		ReleaseObj(pNetCfgBinding);
		ReleaseObj(pNetCfgComponent);

		ReleaseINetCfg(TRUE, pNetCfg);

		return true;
	}

	DWORD NetworkAPI::GetLocalDNS()
	{

		DWORD dwLocalDNS = 0;
		DWORD dwLocalNICIndex = 0;
		PMIB_IPFORWARDTABLE pIpRouteTable = NULL;

		DWORD status = NO_ERROR;
		DWORD dwActualSize = 0;

		int nLoop = 5;
		do 
		{
			nLoop--;
			status = ::GetIpForwardTable(pIpRouteTable, &dwActualSize, true);

			if(status == ERROR_INSUFFICIENT_BUFFER)
				pIpRouteTable = (PMIB_IPFORWARDTABLE) malloc(dwActualSize);

		} while (status != ERROR_SUCCESS && nLoop > 0);
		for (int n = 0; n < (int)pIpRouteTable->dwNumEntries; n++)
		{
			if (0 == pIpRouteTable->table[n].dwForwardDest)
			{
				dwLocalNICIndex = pIpRouteTable->table[n].dwForwardIfIndex;
				break;
			}
		}
		free(pIpRouteTable);

		PIP_ADAPTER_INFO pAdapterInfo;
		PIP_ADAPTER_INFO pAdapter = NULL;

		DWORD dwRetVal = 0;
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
						throw new OutOfMemoryException();
				}

				if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR)
				{
					pAdapter = pAdapterInfo;
					bool fFound = false;

					while (pAdapter)
					{
						if ( pAdapter->Index == dwLocalNICIndex )
						{
							fFound = true;
							break;
						}
						pAdapter = pAdapter->Next;
					}

					if (fFound)
					{
						// Get DNS
						PIP_PER_ADAPTER_INFO pDnsAdapterInfo;
						ULONG	uDnsBufLong = 0;
						pDnsAdapterInfo  = (PIP_PER_ADAPTER_INFO) malloc(sizeof(IP_PER_ADAPTER_INFO));
						uDnsBufLong = sizeof(IP_PER_ADAPTER_INFO);

						if (GetPerAdapterInfo(pAdapter->Index, pDnsAdapterInfo, &uDnsBufLong) != ERROR_SUCCESS) {
							free(pDnsAdapterInfo);
							pDnsAdapterInfo = (PIP_PER_ADAPTER_INFO)malloc(uDnsBufLong);
						}

						string strDns;
						if(NO_ERROR == GetPerAdapterInfo(pAdapter->Index, pDnsAdapterInfo, &uDnsBufLong))
							strDns =  string(pDnsAdapterInfo->DnsServerList.IpAddress.String);
						free(pDnsAdapterInfo);

						dwLocalDNS = inet_addr(strDns);
					}
				}
			}
			catch ( Exception* pe )
			{
				delete pe;
			}
		}

		if (pAdapterInfo) 
			free(pAdapterInfo);

		return dwLocalDNS;
	}

	bool NetworkAPI::validateIPv4Address(string& strIP)
	{
		if (4 != strIP.Split(String(L'.').GetChars()).GetLength())
			strIP = NetworkAPI::GetIPFromDomain(strIP);

		int len = strIP.GetLength();
		if (len < 7 || len>15)
		{
			_TRACE_I(Application, L"Invalid IP Address = %ws", (const wchar_t*)strIP);
			return false;
		}
		else
		{
			array<string> arAddress = strIP.Split(String(L'.').GetChars());
			if (4 == arAddress.GetLength())
			{
				for (int i = 0; i < arAddress.GetLength(); i++)
				{
					int num = Convert::ToInt32(arAddress[i]);

					if (num < 0 || num>255)
					{
						_TRACE_I(Application, L"Invalid IP Address = %ws", (const wchar_t*)strIP);
						return false;
					}

					if ((0 == i && num == 0) || (3 == i && num == 0))
					{
						_TRACE_I(Application, L"Invalid IP Address = %ws", (const wchar_t*)strIP);
						return false;
					}
				}
			}
			else
			{
				_TRACE_I(Application, L"Invalid IP Address = %ws", (const wchar_t*)strIP);
				return false;
			}
		}

		_TRACE_I(Application, L"valid IP Address = %ws", (const wchar_t*)strIP);
		return true;
	}

	bool NetworkAPI::strContainsAlpha(LPCWSTR lpwAddr)
	{
		bool fRet = false;

		int nLen = lstrlenW(lpwAddr);
		for(int i=0; i<nLen; i++)
		{
			if(iswalpha(*(lpwAddr+i)))
			{
				fRet = true;
				break;
			}
		}

		_TRACE_I(Application, L"strContainsAlpha() = %d [%ws]", fRet, (const wchar_t*)lpwAddr);
		return fRet;
	}

	string NetworkAPI::GetDomainFromUrl(const string& strUrl)
	{
		string strRet = L"";

		int nProtoIndex = strUrl.IndexOf(L"://");
		if (0 <= nProtoIndex)
		{
			if (nProtoIndex + 3 < strUrl.GetLength())
				strRet = strUrl.Substring(nProtoIndex + 3);
		}
		else
		{
			strRet = strUrl;
		}

		if (strRet.StartsWith(L"www.", true))
		{
			if (4 < strRet.GetLength())
				strRet = strRet.Substring(4);
			else
				strRet = L"";
		}

		int nDomainIndex = strRet.IndexOf(L'/');
		if (0 < nDomainIndex)
			strRet = strRet.Substring(0, nDomainIndex);

		int nPortIndex = strRet.IndexOf(L':');
		if (0 < nPortIndex)
			strRet = strRet.Substring(0, nPortIndex);

		//_TRACE_I(Application, "GetDomainFromUrl() = %ws [%ws]", (const wchar_t*)strRet, (const wchar_t*)strUrl);
		return strRet;
	}

	string NetworkAPI::GetIPFromDomain(string strDomain)
	{
		string strIP = L"0.0.0.0";

		hostent* pHostent;
		WSADATA wsaData;
		WORD wVersionRequested = MAKEWORD(2, 0);
		if(::WSAStartup(wVersionRequested, &wsaData) == 0)
		{
			pHostent = gethostbyname((const char*)strDomain);
			if (NULL != pHostent)
			{
				strIP = string((char*)inet_ntoa(*(struct in_addr*)*pHostent->h_addr_list));
				_TRACE_I(Application, L"GetIPFromDomain() - domain name = %ws, ip = %ws",
					(const wchar_t*)strDomain,
					(const wchar_t*)strIP);
			}
			else
				_TRACE_I(Application, L"GetIPFromDomain() - pHostname is null = %d", WSAGetLastError());

			::WSACleanup();
		}
		else
			_TRACE_I(Application, L"GetIPFromDomain() - WSAStartup failed = %d", WSAGetLastError());

		return strIP;

	}

	bool NetworkAPI::IsIPInNetworkRange(const string& strIP, const string& strNetwork, const string& strNetMask)
	{
		bool fRet = false;

		uint32 ip_addr = _IPToUInt32(strIP);
		uint32 network_addr = _IPToUInt32(strNetwork);
		uint32 mask_addr = _IPToUInt32(strNetMask);

		uint32 net_lower = (network_addr & mask_addr);
		uint32 net_upper = (net_lower | (~mask_addr));

		if(ip_addr >= net_lower && ip_addr <= net_upper)
			fRet = true;

		return fRet;
	}

	uint32 NetworkAPI::_IPToUInt32(const string& strIP)
	{
		uint32 uIP = 0;
		array<string> arAddress = strIP.Split(String(L'.').GetChars());

		if(4 == arAddress.GetLength())
		{
			uIP = Convert::ToInt32(arAddress[0]) << 24;
			uIP |= Convert::ToInt32(arAddress[1]) << 16;
			uIP |= Convert::ToInt32(arAddress[2]) << 8;
			uIP |= Convert::ToInt32(arAddress[3]);
		}
		else
		{
			if(!NetworkAPI::GetIPFromDomain(strIP).Equals(L"0.0.0.0"))
			{
				uIP = Convert::ToInt32(arAddress[0]) << 24;
				uIP |= Convert::ToInt32(arAddress[1]) << 16;
				uIP |= Convert::ToInt32(arAddress[2]) << 8;
				uIP |= Convert::ToInt32(arAddress[3]);
			}
		}

		return uIP;
	}

	bool NetworkAPI::IsNetworkConnected()
	{
		bool fSuccessInfo = false, fConnected = false;
		ULONG uBufferLength = 0;
		DWORD dwResult = ::GetAdaptersInfo(NULL, &uBufferLength);
		if (ERROR_BUFFER_OVERFLOW == dwResult)
		{
			PIP_ADAPTER_INFO pAdapterInfo = NULL;
			DWORD dwRetVal = 0;

			pAdapterInfo = (IP_ADAPTER_INFO *) new byte[uBufferLength];
			if ( pAdapterInfo != NULL )
			{
				memset( pAdapterInfo, 0, uBufferLength );

				if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &uBufferLength)) == NO_ERROR)
				{
					PIP_ADAPTER_INFO pAdapter = pAdapterInfo;
					while (NULL != pAdapter)
					{
						if (strcmp(pAdapter->IpAddressList.IpAddress.String, "0.0.0.0") != 0)
							fConnected = true;

						pAdapter = pAdapter->Next;
					}

					fSuccessInfo = true;
				}

				delete []pAdapterInfo;
				pAdapterInfo = NULL;
			}
		}

		if (false == fSuccessInfo)
			return true;

		return fConnected;
	}

	uint32 NetworkAPI::ToIPv4Addr(const string& strIP, bool fHostByteOrder)
	{
		uint32 uIP = 0;

		PFRtlIpv4StringToAddressW pfRtlIpv4StringToAddressW = (PFRtlIpv4StringToAddressW)::GetProcAddress(::GetModuleHandleW(L"ntdll.dll"), "RtlIpv4StringToAddressW");

		LPCWSTR wsTerminator;
		in_addr addr = { 0 };
		NTSTATUS status = pfRtlIpv4StringToAddressW(strIP, TRUE, &wsTerminator, &addr);
		if (NT_SUCCESS(status))
		{
			uIP = addr.S_un.S_addr;
			if (fHostByteOrder)
				uIP = H2N(uIP);
		}

		return uIP;
	}

	string NetworkAPI::ToIPv4String(uint32 uIP, bool fHostByteOrder)
	{
		wchar_t wsz[16];

		PFRtlIpv4AddressToStringW pfRtlIpv4AddressToStringW = (PFRtlIpv4AddressToStringW)::GetProcAddress(::GetModuleHandleW(L"ntdll.dll"), "RtlIpv4AddressToStringW");
		
		in_addr addr;
		addr.S_un.S_addr = (fHostByteOrder ? H2N(uIP) : uIP);
		PWSTR wsRet = pfRtlIpv4AddressToStringW(&addr, wsz);

		return string(wsz);
	}

	List<string> NetworkAPI::GetHostByDomain(string strDomain)
	{
		List<string> strIPs;

		hostent* pHostent;
		WSADATA wsaData;
		WORD wVersionRequested = MAKEWORD(2, 2);

		if (::WSAStartup(wVersionRequested, &wsaData) == 0)
		{
			pHostent = ::gethostbyname(strDomain);

			if (pHostent)
			{
				IN_ADDR addr;

				int i = 0;
				while (pHostent->h_addr_list[i])
				{
					memcpy(&addr, pHostent->h_addr_list[i++], pHostent->h_length);
					string NameIps = inet_ntoa(addr);
					strIPs.Add(NameIps);
				}
			}
			else
			{
				_TRACE_I(Application, L"gethostbyname failed");
			}
			::WSACleanup();
		}
		else
		{
			_TRACE_I(Application, L"GetIPFromDomain() - WSAStartup failed = %d", WSAGetLastError());
		}

		/* 참고 사이트
		http://ryuschool.tistory.com/entry/%EB%82%B4%EC%BB%B4%ED%93%A8%ED%84%B0-%EC%9D%B4%EB%A6%84-%EB%B0%8F-IP-%EB%8B%A4%EB%A5%B8-%ED%98%B8%EC%8A%A4%ED%8A%B8%EC%9D%98-IP-%EA%B5%AC%ED%95%98%EA%B8%B0
		*/

		return strIPs;
	}

	string _GetLocalIPWithAdapterName()
	{
		PIP_ADAPTER_INFO pAdapterInfo;
		PIP_ADAPTER_INFO pAdapter = NULL;
		DWORD dwRetVal = 0;

		string strMyIP = L"0.0.0.0";

		ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
		pAdapterInfo = (IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));

		if (pAdapterInfo != NULL)
		{
			memset(pAdapterInfo, 0, sizeof(IP_ADAPTER_INFO));
			try
			{
				if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
				{
					free(pAdapterInfo);
					pAdapterInfo = (IP_ADAPTER_INFO*)malloc(ulOutBufLen);
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
			catch (Exception* pe)
			{
				_TRACE_EX(pe);
				delete pe;
			}
		}

		if (pAdapterInfo)
			free(pAdapterInfo);

		return strMyIP;
	}

	string NetworkAPI::GetLocalIP()
	{
		string strMyIP = L"127.0.0.1";

		DWORD dwErr, dwAdapterInfoSize = 0;
		PIP_ADAPTER_INFO	pAdapterInfo, pAdapt;
		PIP_ADDR_STRING	pAddrStr;

		if ((dwErr = ::GetAdaptersInfo(NULL, &dwAdapterInfoSize)) != 0)
		{
			if (dwErr != ERROR_BUFFER_OVERFLOW)
			{
				strMyIP = _GetLocalIPWithAdapterName();
				return strMyIP;
			}
		}

		// Allocate memory from sizing information
		if ((pAdapterInfo = (PIP_ADAPTER_INFO)GlobalAlloc(GPTR, dwAdapterInfoSize)) == NULL)
		{
			strMyIP = _GetLocalIPWithAdapterName();
			return strMyIP;
		}

		// Get actual adapter information
		if ((dwErr = ::GetAdaptersInfo(pAdapterInfo, &dwAdapterInfoSize)) != 0)
		{
			strMyIP = _GetLocalIPWithAdapterName();
			return strMyIP;
		}

		for (pAdapt = pAdapterInfo; pAdapt; pAdapt = pAdapt->Next)
		{
			switch (pAdapt->Type)
			{
			case MIB_IF_TYPE_ETHERNET:
			case MIB_IF_TYPE_PPP:
				if (strlen(pAdapt->GatewayList.IpAddress.String) > 0)
				{
					DWORD	dwGwIp, dwMask, dwIp, dwGwNetwork, dwNetwork;

					dwGwIp = inet_addr(pAdapt->GatewayList.IpAddress.String);
					for (pAddrStr = &(pAdapt->IpAddressList); pAddrStr; pAddrStr = pAddrStr->Next)
					{
						if (strlen(pAddrStr->IpAddress.String) > 0)
						{
							dwIp = inet_addr(pAddrStr->IpAddress.String);
							dwMask = inet_addr(pAddrStr->IpMask.String);
							dwNetwork = dwIp & dwMask;
							dwGwNetwork = dwGwIp & dwMask;

							if (dwGwNetwork == dwNetwork)
							{
								strMyIP = pAddrStr->IpAddress.String;
								if (!strMyIP.Equals(L"0.0.0.0"))
								{
									GlobalFree(pAdapterInfo);
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

		if (strMyIP.Equals(L"127.0.0.1") || strMyIP.Equals(L"0.0.0.0"))
			strMyIP = _GetLocalIPWithAdapterName();

		GlobalFree(pAdapterInfo);

		return strMyIP;
	}

	void NetworkAPI::RegisterURIScheme(const string& strScheme, const string& strCommandLine)
	{
		RegistryKey* pKey = null;
		RegistryKey* pSubKey = null;
		RegistryKey* pIEKey = null;

		try
		{
			pKey = Registry::GetLocalMachine()->CreateSubKey(L"Software\\Classes\\" + strScheme);
			if (null == pKey)
				throw new ApplicationException;

			pKey->SetValue(L"URL Protocol", L"");

			pSubKey = pKey->CreateSubKey(L"Shell\\open\\command");
			if (null == pSubKey)
				throw new ApplicationException;

			pSubKey->SetValue(L"", strCommandLine);

			// Disable warning
			pIEKey = Registry::GetLocalMachine()->CreateSubKey(L"Software\\Microsoft\\Internet Explorer\\ProtocolExecute\\" + strScheme);
			if (null == pIEKey)
				throw new ApplicationException;

			pIEKey->SetValue(L"WarnOnOpen", (uint32)0);
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
		
		if (pSubKey)
			delete pSubKey;
		if (pKey)
			delete pKey;
	}

	void NetworkAPI::DeregisterURIScheme(const string& strScheme)
	{
		Registry::GetLocalMachine()->DeleteSubKeyTree(L"Software\\Classes\\" + strScheme, false);
		Registry::GetLocalMachine()->DeleteSubKeyTree(L"Software\\Microsoft\\Internet Explorer\\ProtocolExecute\\" + strScheme, false);
	}

}
}