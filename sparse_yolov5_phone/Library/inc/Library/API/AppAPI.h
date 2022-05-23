#pragma once

#include <System/Threading/all.h>

namespace Library
{
namespace API
{

class AppAPI
{
public:
	static bool IsSupportedWindows();

	static string GetPCName();
	static string GetUserName();
	static string GetLocalIP();
	static string GetLocalIPWithAdapterName();
	static string GetDeviceID();
	static string GetDeviceIDVersion2();
	static string GetDeviceIDForMigration();
	
	static string ExpandSpecialFolder(string strPath);

	static bool ContainsPendingFileRenameOperations(const string& str);

	// Event API
	static void RaiseEvent(const string& strName, bool fOnlyExists = false, bool fThrowOnFailed = false);
	static void WaitEvent(const string& strName, int nTimeout = Timeout::Infinite, bool fOnlyExists = false, bool fThrowOnFailed = false);
	static bool RaiseEventExisting(const string& strName);
	static bool ExistsEvent(const string& strName);

	// MySingle API
	static bool IsSamsungSSOPC(bool fCheckEpTrayRunning = false);

	static string MakeCommonExtensionUrl(string strExtension, string strServerHost, string strSessionId);

	static bool IsComplexPassword(string strPassword, string strValidReg, string strInvalidReg);

private:
	static bool _IsSamSungSsoEpTrayRunning();
	static string _GetProcessPathWithPID(int nPid);
	static string _FromKernelPath(string strKernelPath);
	static string _ToLongPath(string strPath);
};

}
}
