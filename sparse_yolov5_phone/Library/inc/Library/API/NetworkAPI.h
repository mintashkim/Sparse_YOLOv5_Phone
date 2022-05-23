#pragma once
#include <System/Text/all.h>
#include <System/Threading/all.h>

#include <windows.h>
#include <crtdbg.h>
#include <netfw.h>
#include <objbase.h>
#include <oleauto.h>
#include <TlHelp32.h>

#include <Library/WinDDK/netcfgx.h>

#define H2N(x)	((((uint32)x >> 24) & 0xFF) | (((uint32)x >> 8) & 0xFF00) | (((uint32)x << 8) & 0xFF0000) | (((uint32)x << 24) & 0xFF000000))
#define H2N_16(x)	((((uint16)x >> 8) & 0xFF) | (((uint16)x << 8) & 0xFF00))

#define IP_1(x)	((uint32)x & 0xFF)
#define IP_2(x)	(((uint32)x >> 8) & 0xFF)
#define IP_3(x)	(((uint32)x >> 16) & 0xFF)
#define IP_4(x)	((uint32)x >> 24)

#define IP(a, b, c, d)	(((uint32)d << 24) | ((uint32)c << 16) | ((uint32)b << 8) | a)

namespace Library
{

namespace API
{

class FirewallWorker
{
public:
	FirewallWorker(void);
	~FirewallWorker(void);

private:
	HRESULT comInit;
	INetFwProfile* fwProfile;

	HRESULT _WindowsFirewallOpen();
	void _WidnowsFirewallClose();
	HRESULT _WindowsFirewallInitialize(OUT INetFwProfile** fwProfile);
	void _WindowsFirewallCleanup(IN INetFwProfile* fwProfile);
	HRESULT _WindowsFirewallIsOn(IN INetFwProfile* fwProfile, OUT BOOL* fwOn);
	HRESULT _WindowsFirewallTurnOn(IN INetFwProfile* fwProfile);
	HRESULT _WindowsFirewallTurnOff(IN INetFwProfile* fwProfile);

	HRESULT _WindowsFirewallAppIsEnabled(IN INetFwProfile* fwProfile,
		IN const wchar_t* fwProcessImageFileName,
		OUT BOOL* fwAppEnabled
		);
	HRESULT _WindowsFirewallAddApp(IN INetFwProfile* fwProfile,
		IN const wchar_t* fwProcessImageFileName,
		IN const wchar_t* fwName
		);

public:

	HRESULT WindowsFirewallOpen();
	HRESULT WindowsFirewallIsOn();
	HRESULT WindowsFirewallTurnOn();
	HRESULT WindowsFirewallTurnOff();
	HRESULT WindowsFirewallAppIsEnabled(string);
	HRESULT WindowsFirewallAddApp(string, string);
};

class NetworkAPI
{
public:
	// Firewall
	static void AddFirewall(string strProcessName, string strProcessPath);
	static void DeleteFirewall(string strProcessName, string strProcessPath);

	static void EnableFirewall(string strName);
	static void DisableFirewall(string strName);

	static string GetNetworkPnpInstanceID(string strAdapterGUID);
	static bool NetworkChangeBindingOrder(string strPnpInstansName, bool fAfterBind = true);

	static DWORD GetLocalDNS();

	static bool validateIPv4Address(string& strIP);
	static bool strContainsAlpha(LPCWSTR lpwAddr);
	static string GetDomainFromUrl(const string& strUrl);
	static string GetIPFromDomain(string strDomain);

	static bool IsIPInNetworkRange(const string& strIP, const string& strNetwork, const string& strNetMask);
	static bool IsNetworkConnected();

	static uint32 ToIPv4Addr(const string& strIP, bool fHostByteOrder);
	static string ToIPv4String(uint32 uIP, bool fHostByteOrder);

	static List<string> GetHostByDomain(string strDomain);

	static string GetLocalIP();

	static void RegisterURIScheme(const string& strScheme, const string& strCommandLine);
	static void DeregisterURIScheme(const string& strScheme);

private:
	static inline ULONG ReleaseObj(IUnknown* punk)
	{
		return punk ? punk->Release() : 0;
	}
	static bool ReleaseINetCfg(BOOL bHasWriteLock, INetCfg* pNetCfg);

	static uint32 _IPToUInt32(const string& strIP);
};

}
}