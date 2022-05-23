#pragma once
#include <ShTypes.h>

namespace Library
{
namespace API
{

/*
	Note : 
		DeviceAPI 클래스를 활용하기 위해서는 
		WindowProfile::Initialize(string)를 적절히 호출해 주어야합니다.
*/
class DeviceAPI
{
public:
	static void ShowDrive(string strDriveName, bool fNotify = true);
	static void HideDrive(string strDriveName, bool fNotify = true);
	static bool HideFolder(string strFilePath);
	static void MakeFolder(string strPath, bool fUseLowSecurity);
	static void SetupFixedDriveAppearance(bool fHide, string strForceShowDrive);
	static void SetupUnknownDriveAppearance(bool fHide, uint32 uUnknownDrives, string strExceptionDrive);
	static void SetupNetShareDriveAppearance(bool fHide, uint32 uNetShareDrives, string strExceptionDrive);
	
	static string GetUserFolderPath(int csidl);
	static void SetUserShellFolderKey(const string& strName, const string& strValue);
	static void SetUserKnownFolderPath(int csidl, string strPath, bool fUseLowSecurity = true);
	static void SetUserKnownFolderPath2(REFKNOWNFOLDERID rfid, string strPath, bool fUseLowSecurity = true);

	static string GetCommonFolderPath(int csidl);
	static void SetCommonShellFolderKey(const string& strName, const string& strValue);
	static void SetCommonKnownFolderPath(int csidl, string strPath, bool fUseLowSecurity = true);
	static void SetCommonKnownFolderPath2(REFKNOWNFOLDERID rfid, string strPath, bool fUseLowSecurity = true);
	
	static bool CreateJunctionPoint(string strMountPath, string strDestPath);
	static string GetTempPath();
	static void SetTempPath( string strPath , bool fUseLowSecurity = true);

	static array<string> GetNetworkPrinterAddresses();
};
}
}