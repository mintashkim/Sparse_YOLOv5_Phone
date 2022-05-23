#pragma once

namespace Library
{
namespace API
{

class FSAPI
{
public:
	static bool ExistsFile(string strPath, string strFilename);
	static void CreateDirectoryRecursively(string strDirectory);
	static void GetDriveCapacityInfo(string strDrive, double &total, double &used, double &ratio);
	static string GetInternetExplorePath();
	static void DeleteIEx64Shortcut(string strDir);

	static int GetRootDiskSize(string strPath);
	static bool IsRemovableDrive(string& strDriveLetter);

	//directoryname : 삭제랑 디렉토리 이름, flags : 1 = contents 2 = directory 3 = contents and directory
	static bool DeleteDirectory(string& directoryname, int flags);
	static void CopyDirectory(string strSourcePath, string strTargetPath);

	// Strings
	static array<string> SplitString(string strString, wchar_t wch);
	static string StringTrim(string str);

	// Drives
	static wchar_t GetEmptyDriveLetter(wchar_t wcStartLetter = L'C', wchar_t wcEndLetter = L'V', bool fGetLastDriveLetter = true);
	static UINT GetDriveType(string& strDriveLetter);
	static void GetMountDrives(uint32& uFixedDrives, uint32& uExtDrives, uint32& uCDRDrives, uint32& uCDRWDrives, uint32& uNetShareDrives, uint32& uUnknownDrives);
	static void GetMountDrives(uint32& uFixedDrives, uint32& uExtDrives, uint32& uCDRDrives, uint32& uCDRWDrives, uint32& uNetShareDrives, uint32& uUnknownDrives, bool& fPrivilegeFail);
	static array<string> GetFixedDrive();

	static bool DisableNetworkShare(uint32 uDisableDrives);

	static void BroadCastDriveMount(string strDrvLetter);

	static string GetFileHash(string strPath);
	static string GetFileVersion(string strPath);

	// Directory
	static uint64 GetDiskFreeSizeOnByte(string strDrvLetter);
	static uint64 GetDirectorySizeOnByte(string strPath);
	
	static int64 GetFileCount(string strPath);
	static int64 GetDirectoryCount(string strPath);
	
	static bool ChangeFileSecurityOption(string strPath);
	static bool AddToACL(PACL& pACL, string AccountName, DWORD AccessOption);

private:
	static uint32 _GetCDRomDriveType(string strDeviceName, bool& fPrivilegeFail);
	static bool _IsRecordableMedia(string strDrvLetter);
};

}
}