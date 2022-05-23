#pragma once
#include <Light/System/all.h>
#include <Light/Microsoft/Win32/all.h>

class LInstallHelper
{
public:
	static bool AddPendingFileRenameOperations(lstring strArg1, lstring strArg2);
	static void EnablePrivilege(LPCWSTR lpPrivilege, bool fEnable);
	static void TerminateExe(TCHAR *szExe);
	static bool IsExeRunning(TCHAR *szExe);
	static bool SystemReboot();

	static void SetServerHost(lstring str);
	static bool CheckRebootAfterUninstall();

	static void DeleteAll(const lstring& strDirPath);

	// Registry
	static bool IsInstalledProduct();

	static void SetupServiceRecoveryOption(lstring strService);
	static void ClearServiceRecoveryOption(lstring strService);

	static lstring GetProductID(lstring AppPath);

	/// 주어진 directory에 파일이 존재하지 않으면 true, 있으면 false (Directory는 무시됨)
	static bool IsEmptyDirectory(lstring strDirectory, lstring strExceptFileExt = L".mwi");

	// Redistributable Package & Dotnet
	static bool IsInstalledRedist(lstring strProductName);
	static bool IsinstalledDotnet35();
	static bool IsinstalledDotnet40();

	static void ClearRegistryKey(lstring strPath);
	static bool IsExistValueInRegistry(LRegistryKey* pKey, lstring strPath);

	static lstring GetHash(lstring strPath);

private:
	static bool Is64bitPlatform();
	static lstring GetPCId0();
	static bool RegSetMultiStringValue(HKEY hKey, lstring strName, larray<lstring> astrValue);
	static bool RegGetMultiStringValue(HKEY hKey, lstring strName, larray<lstring> &astrValue);
};