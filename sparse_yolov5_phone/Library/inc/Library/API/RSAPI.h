#pragma once
namespace Library
{
namespace API
{

// Exception about UnloadRegistry
#define UNLOAD_SUCESS 0
#define REGISTRY_NOT_MOUNTED 1
#define UNLOADKEY_FAILED 2
#define UNLOAD_EXCEPTION 3

#define LOAD_FAILED 4

class RSAPI
{
public:
	static string GetUserRegistryValueS(string strName, string strSubKey = L"");
	static array<byte> GetUserRegistryValueB(string strName, string strSubKey = L"");
	static DWORD GetUserRegistryValueD(string strName, string strSubKey = L"");
	static void SetUserRegistryValue(string strName, string strValue, string strSubKey = L"");
	static void SetUserRegistryValue(string strName, array<byte> abValue, string strSubKey = L"");
	static void SetUserRegistryValue(string strName, DWORD dwValue, string strSubKey = L"");

	static string GetCommonRegistryValueS(string strName, string strSubKey = L"");
	static array<byte> GetCommonRegistryValueB(string strName, string strSubKey = L"");
	static DWORD GetCommonRegistryValueD(string strName, string strSubKey = L"");
	static void SetCommonRegistryValue(string strName, string strValue, string strSubKey = L"");
	static void SetCommonRegistryValue(string strName, array<byte> abValue, string strSubKey = L"");
	static void SetCommonRegistryValue(string strName, DWORD dwValue, string strSubKey = L"");

	static bool IsMWBHORegisterd();
	static bool IsMWActiveXRegisterd();
	static void SetIEPopupWindowSetting(int nValue);
	static void SetEnableBrowserExtensions(bool fEnable);
	static bool IsIEProtectedMode();
	static void SetIEProtectedMode(bool fEnable);
	static void SetIEIsolationValue(bool fPMIL = true);	// 향상된 보호 모드 사용*, PMIL = 체크해제, PMEM = 체크

	/*
		Note : 
			DeleteRegistryValueSubKey_HKCU, DeleteRegistryValue_HKCU, DeleteRegistrySubKey_HKCU 함수는
			반드시 mwWindowsProfile::Initialize(string) 함수를 적절히 호출해 준 뒤 사용해야 합니다.
	*/
	static void DeleteRegistryValueSubKey_HKLM(const string& strSubKey);
	static void DeleteRegistryValueSubKey_HKCU(const string& strSubKey);
	static void DeleteRegistryValueSubKey_AllUser(const string& strSubKey);
	static void DeleteRegistryValue_HKLM(const string& strSubKey, const string& strValue);
	static void DeleteRegistryValue_HKCU(const string& strSubKey, const string& strValue);
	static void DeleteRegistryValue_AllUser(const string& strSubKey, const string& strValue);
	static void DeleteRegistrySubKey_HKLM(const string& strSubKey, const string& strKey);
	static void DeleteRegistrySubKey_HKCU(const string& strSubKey, const string& strKey);
	static void DeleteRegistrySubKey_AllUser(const string& strSubKey, const string& strKey);

	static bool ChangeRegSecurityOption(RegistryHive hive, string strRegPath, string strAccountName);

	static void AddFeatureBrowserEmulation(const string& strName);
	static void RemoveFeatureBrowserEmulation(const string& strName);

	static DWORD LoadRegistry(HKEY hRootKey, const string& strRegName, const string& strRegFilePath);
	static DWORD UnloadRegistry(HKEY hRootKey, const string& strRegName);
	static bool IsRegistryMounted(HKEY hRootKey, const string& strRegName);

private:
	static bool _SetRegSecurityInfo(string strRegPath, string strAccountName, SECURITY_INFORMATION securityInfo);
};

}
}