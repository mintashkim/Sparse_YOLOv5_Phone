#include "StdAfx.h"
#include <Library/API/FSAPI.h>
#include <System/IO/all.h>
#include <Microsoft/Win32/all.h>
#include <NoAD/Win32/all.h>
#include <ShlObj.h>
#include <WinIoCtl.h>
#include <Winnetwk.h>
#include <LM.h>
#include <Dbt.h>
#include <ntddscsi.h>
#include <imapi2.h>
#include <aclapi.h>

#pragma comment (lib, "mpr.lib")
#pragma comment (lib, "Netapi32.lib")

#include "wpp.h"
#include "FSAPI.tmh"

namespace Library
{
namespace API
{
#pragma pack(push, 1)
typedef struct _DeviceConfigHeader
{
	ULONG DataLength;
	USHORT Reserved;
	USHORT CurrentProfile;
	USHORT FeatureCode;
	UCHAR Version;
	UCHAR AdditionalLength;
	UCHAR OtherData[102];
} DeviceConfigHeader;
#pragma pack(pop)

#pragma pack(push, 4)
typedef struct _SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER {
	SCSI_PASS_THROUGH_DIRECT sptd;
	ULONG Filler;
	UCHAR SenseBuf[32];
} SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, *PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER;
#pragma pack(pop)

#define _DRIVE_CDRW	1000

enumeration class ECDRomDriveType
{
public:
	enum _ECDRomDriveType
	{
		NoCurrentProfile = 0,
		NonRemovableDisk,
		RemovableDisk,
		MagnetoOpticalErasable,
		OpticalWriteOnce,
		AS_NO,
		CD_ROM,
		CD_R,
		CD_RW,
		DVD_ROM,
		DVD_R_SequentialRecording,
		DVD_RAM,
		DVD_RW_RestrictedOverwrite,
		DVD_RW_SequentialRecording,
		DVD_PLUS_RW,
		DVD_PLUS_R,
		DDCD_ROM,
		DDCD_R,
		DDCD_RW,
		Unknown,
	};

	static bool IsWritable(_ECDRomDriveType type)
	{
		switch (type)
		{
		case CD_RW:
		case DVD_RW_RestrictedOverwrite:
		case DVD_RW_SequentialRecording:
		case DVD_PLUS_RW:
		case DDCD_RW:
			return true;
		}
		return false;
	}
};
typedef ECDRomDriveType::_ECDRomDriveType CDRomDriveType;

class DiscException : public Exception
{
public:
	DiscException(string strMessage = L"", Exception* pInnerException = null) : Exception(strMessage, pInnerException)
	{
	};
	virtual ~DiscException()
	{
	}
};

class DiscMaster2
{
public:
	DiscMaster2()
	{
		HRESULT hr = CoCreateInstance(CLSID_MsftDiscMaster2, NULL, CLSCTX_INPROC_SERVER, IID_IDiscMaster2, (LPVOID*)&m_pDiscMaster2);
		if (FAILED(hr))
			throw new DiscException(string::Format(L"IDiscMaster2 CoCreateInstance failed = %x", hr));
	}
	~DiscMaster2()
	{
		if (m_pDiscMaster2) m_pDiscMaster2->Release();
	}

	bool IsSupportedEnvironment()
	{
		if (NULL == m_pDiscMaster2)
			throw new DiscException(L"IDiscMaster2 instance is null.");

		VARIANT_BOOL isSupported = VARIANT_FALSE;
		HRESULT hr = m_pDiscMaster2->get_IsSupportedEnvironment(&isSupported);
		if (FAILED(hr))
			throw new DiscException(string::Format(L"IDiscMaster2::get_IsSupportedEnvironment() failed = %x", hr));

		return (isSupported == VARIANT_TRUE);
	}
	int GetCount()
	{
		if (NULL == m_pDiscMaster2)
			throw new DiscException(L"IDiscMaster2 instance is null.");

		long totalDevices = 0;
		HRESULT hr = m_pDiscMaster2->get_Count(&totalDevices);
		if (FAILED(hr))
			throw new DiscException(string::Format(L"IDiscMaster2::get_Count() failed! - Error:0x%08x", hr));

		return totalDevices;
	}
	string GetDeviceUniqueId(int index)
	{
		if (NULL == m_pDiscMaster2)
			throw new DiscException(L"IDiscMaster2 instance is null.");

		string strUniqueId;
		BSTR uniqueID = NULL;
		HRESULT hr = m_pDiscMaster2->get_Item(index, &uniqueID);
		if (FAILED(hr))
			throw new DiscException(string::Format(L"IDiscMaster2::get_Item(%d) failed! - Error:0x%08x", index, hr));

		strUniqueId = string(uniqueID);
		SysFreeString(uniqueID);
		return strUniqueId;
	}

private:
	IDiscMaster2* m_pDiscMaster2;
};

class DiscRecorder2
{
public:
	DiscRecorder2()
	{
		HRESULT hr = CoCreateInstance(CLSID_MsftDiscRecorder2, NULL, CLSCTX_INPROC_SERVER, IID_IDiscRecorder2, (LPVOID*)&m_pDiscRecorder2);
		if (FAILED(hr))
			throw new DiscException(string::Format(L"IDiscRecorder2::CoCreateInstance failed = %x", hr));
	}
	~DiscRecorder2()
	{
		if (m_pDiscRecorder2) m_pDiscRecorder2->Release();
	}

	IDiscRecorder2* GetInstance()
	{
		return m_pDiscRecorder2;
	}
	void InitializeDiscRecorder(string strDeviceUniqueId)
	{
		if (NULL == m_pDiscRecorder2)
			throw new DiscException(L"IDiscRecorder2 instance is null.");

		BSTR uniqueId = SysAllocString(strDeviceUniqueId);
		HRESULT hr = m_pDiscRecorder2->InitializeDiscRecorder(uniqueId);
		SysFreeString(uniqueId);

		if (FAILED(hr))
			throw new DiscException(string::Format(L"IDiscRecorder2::InitializeDiscRecorder() failed! - Error:0x%08x", hr));
	}
	array<string> GetVolumePathNames()
	{
		if (NULL == m_pDiscRecorder2)
			throw new DiscException(L"IDiscRecorder2 instance is null.");

		SAFEARRAY* volumePathNames;
		HRESULT hr = m_pDiscRecorder2->get_VolumePathNames(&volumePathNames);
		if (FAILED(hr))
			throw new DiscException(string::Format(L"IDiscRecorder2::get_VolumePathNames() failed! - Error:0x%08x", hr));

		int count = volumePathNames->rgsabound[0].cElements;
		array<string> astrVolumePath(count);
		for (int i = 0; i < count; i++)
			astrVolumePath[i] = string(((VARIANT*)volumePathNames->pvData)[i].bstrVal);	// "E:\\"

		SafeArrayDestroy(volumePathNames);
		return astrVolumePath;
	}

private:
	IDiscRecorder2* m_pDiscRecorder2;
};

class DiscFormat2Data
{
public:
	DiscFormat2Data()
	{
		HRESULT hr = CoCreateInstance(CLSID_MsftDiscFormat2Data, NULL, CLSCTX_INPROC_SERVER, IID_IDiscFormat2Data, (LPVOID*)&m_pDiscFormat2Data);
		if (FAILED(hr))
			throw new DiscException(string::Format(L"IDiscFormat2Data CoCreateInstance failed = %x", hr));
	}
	~DiscFormat2Data()
	{
		if (m_pDiscFormat2Data) m_pDiscFormat2Data->Release();
	}

	bool IsRecorderSupported(DiscRecorder2* pDiscRecorder2)
	{
		if (NULL == m_pDiscFormat2Data)
			throw new DiscException(L"IDiscFormat2Data instance is null.");
		if (NULL == pDiscRecorder2 || NULL == pDiscRecorder2->GetInstance())
			throw new DiscException(L"DiscRecorder2 instance is null.");

		VARIANT_BOOL isSupported = VARIANT_FALSE;
		HRESULT hr = m_pDiscFormat2Data->IsRecorderSupported(pDiscRecorder2->GetInstance(), &isSupported);
		if (FAILED(hr))
			throw new DiscException(string::Format(L"IDiscFormat2Data::IsRecorderSupported() failed! - Error:0x%08x", hr));

		return (isSupported == VARIANT_TRUE);
	}
	bool IsCurrentMediaSupported(DiscRecorder2* pDiscRecorder2)
	{
		if (NULL == m_pDiscFormat2Data)
			throw new DiscException(L"IDiscFormat2Data instance is null.");
		if (NULL == pDiscRecorder2 || NULL == pDiscRecorder2->GetInstance())
			throw new DiscException(L"DiscRecorder2 instance is null.");

		VARIANT_BOOL isSupported = VARIANT_FALSE;
		HRESULT hr = m_pDiscFormat2Data->IsCurrentMediaSupported(pDiscRecorder2->GetInstance(), &isSupported);
		if (FAILED(hr))
			throw new DiscException(string::Format(L"IDiscFormat2Data::IsCurrentMediaSupported() failed! - Error:0x%08x", hr));

		return (isSupported == VARIANT_TRUE);
	}

private:
	IDiscFormat2Data* m_pDiscFormat2Data;
};

bool FSAPI::ExistsFile( string strPath, string strFilename )
{
	bool f = false;
	DirectoryInfo info(strPath);
	if (info.Exists())
	{
		array<FileInfo*> apFileInfos = info.GetFiles();
		for (int i = 0 ; i < apFileInfos.GetLength() ; i++)
		{
			if (apFileInfos[i]->GetName().ToLower() == strFilename.ToLower())
				f = true;
			delete apFileInfos[i];
		}

		if (!f)
		{
			array<DirectoryInfo*> apDirInfos = info.GetDirectories();
			for (int i = 0 ; i < apDirInfos.GetLength() ; i++)
			{
				if (ExistsFile(apDirInfos[i]->GetFullName(), strFilename))
					f = true;
				delete apDirInfos[i];
			}
		}
	}
	return f;
}

void FSAPI::CreateDirectoryRecursively(string strDirectory)
{
	if (INVALID_FILE_ATTRIBUTES != ::GetFileAttributesW(strDirectory))
		return;

	if (!strDirectory.Contains(L":\\"))
		return;

	if (strDirectory.EndsWith(L"\\"))
		strDirectory = strDirectory.Substring(0, strDirectory.GetLength() - 1);

	string strOrginDirectory = strDirectory;
	string strParentDirectory = strOrginDirectory.Substring(0, 3);
	strOrginDirectory = strOrginDirectory.Substring(3);

	while (true)
	{
		int index = strOrginDirectory.IndexOf(L"\\");
		if (index <= 0)
			break;

		strParentDirectory += strOrginDirectory.Substring(0, index + 1);
		strOrginDirectory = strOrginDirectory.Substring(index + 1);

		if (!DirectoryInfo(strParentDirectory).Exists())
			::CreateDirectory((const wchar_t*)strParentDirectory, NULL);
	}

	if (!DirectoryInfo(strDirectory).Exists())
		::CreateDirectory((const wchar_t*)strDirectory, NULL);
}

void FSAPI::GetDriveCapacityInfo(string strDrive, double &total, double &used, double &ratio)
{
	// Get Capacity based on mega-byte;
	double _dblTotal = 0.0;
	double _dblUsed = 0.0;
	double _dblRatio = 0.0;

	try
	{
		DriveInfo drive(strDrive + L":\\");
		_dblTotal = drive.GetTotalSize() / 1024.0 / 1024.0;
		_dblUsed = _dblTotal - (drive.GetTotalFreeSpace() / 1024.0 / 1024.0);
		_dblRatio = _dblUsed / _dblTotal;
	}
	catch (Exception *pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}

	total = _dblTotal;
	used = _dblUsed;
	ratio = _dblRatio;
}

string FSAPI::GetInternetExplorePath()
{
#ifdef _M_X64
	string strIEPath;
	array<wchar_t> awc(MAX_PATH);
	HRESULT hr = ::SHGetFolderPathW(NULL, CSIDL_PROGRAM_FILESX86, NULL, NULL, awc.GetBuffer());
	if (S_OK == hr)
	{
		strIEPath = awc.GetBuffer();
		strIEPath += L"\\Internet Explorer\\iexplore.exe";
	}
#else
	string strIEPath = Environment::GetFolderPath(ESpecialFolder::ProgramFiles) + L"\\Internet Explorer\\iexplore.exe";
#endif
	return strIEPath;
}

void FSAPI::DeleteIEx64Shortcut(string strDir)
{
#ifdef _M_X64
	try
	{
		DirectoryInfo di(strDir);
		if (di.Exists())
		{
			string strIEx64Path = Environment::GetFolderPath(ESpecialFolder::ProgramFiles);
			strIEx64Path += L"\\Internet Explorer";

			array<FileInfo*> apFi = di.GetFiles();
			for (int i = 0 ; i < apFi.GetLength() ; i++)
			{
				string strFullname = apFi[i]->GetFullName();
				if (strFullname.ToLower().EndsWith(L".lnk"))
				{
					Shortcut shortcut;
					shortcut.Load(strFullname);
					if (shortcut.GetPath().Contains(strIEx64Path))
						::DeleteFile(strFullname);
				}
			}

			for (int i = 0; i < apFi.GetLength(); i++)
			{
				if (null != apFi[i])
				{
					delete apFi[i];
					apFi[i] = null;
				}
			}

			array<DirectoryInfo*> apDi = di.GetDirectories();
			for (int i = 0 ; i < apDi.GetLength() ; i++)
				FSAPI::DeleteIEx64Shortcut(apDi[i]->GetFullName());

			for (int i = 0; i < apDi.GetLength(); i++)
			{
				if (null != apDi[i])
				{
					delete apDi[i];
					apDi[i] = null;
				}
			}
		}
	}
	catch (Exception *pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}
#endif
}

int FSAPI::GetRootDiskSize(string strPath)
{
	ULARGE_INTEGER FreeByte;

	string strSystemRoot = strPath;
	int64 nSystemFreeSize = 0;

	if (FALSE == ::GetDiskFreeSpaceExW(string::Format(L"%ws:\\", (const wchar_t*)strSystemRoot.Substring(0, 1)), &FreeByte, NULL, NULL))
	{
		DWORD dwError = ::GetLastError();
	}
	else
	{
		nSystemFreeSize = FreeByte.QuadPart;
	}

	return (int)(nSystemFreeSize / 1024 / 1024);
}

bool FSAPI::IsRemovableDrive(string& strDriveLetter)
{
	if (strDriveLetter.IsEmpty())
		throw new ArgumentException;
	bool fRemovable = false;

	if (DRIVE_REMOVABLE == FSAPI::GetDriveType(strDriveLetter))
		fRemovable = true;

	return fRemovable;
}

//directoryname : 삭제랑 디렉토리 이름, flags : 1 = contents 2 = directory 3 = contents and directory
bool FSAPI::DeleteDirectory(string& directoryname, int flags)
{
	if(directoryname.Substring(directoryname.GetLength()-1,1) !=  '\\') directoryname += '\\';

	//contents
	if ((flags & 1) == 1)
	{
		WIN32_FIND_DATAW fdata;
		HANDLE dhandle;

		directoryname += "*";
		dhandle = ::FindFirstFileW((const wchar_t*)directoryname, &fdata);

		if( dhandle != INVALID_HANDLE_VALUE )
		{
			// Loop through all the files in the main directory and delete files & make a list of directories
			while(true)
			{
				if(::FindNextFileW(dhandle, &fdata))
				{
					string filename = fdata.cFileName;
					if(filename.CompareTo("..") != 0)
					{
						string filelocation = directoryname.Substring(0, directoryname.GetLength()-1) + filename;

						if( (fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)  
							::DeleteFileW(filelocation);
						else 
							FSAPI::DeleteDirectory(filelocation, 3);
					}
				} else if(GetLastError() == ERROR_NO_MORE_FILES)    break;
			}
			directoryname = directoryname.Substring(0, directoryname.GetLength()-2);
			::FindClose( dhandle );
		}
	}
	//directory
	if ((flags & 2) == 2)
	{
		HANDLE DirectoryHandle;
		DirectoryHandle = ::CreateFileW( (const wchar_t*) directoryname,
			FILE_LIST_DIRECTORY,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
			NULL);

		if( DirectoryHandle != INVALID_HANDLE_VALUE )
		{
			bool DeletionResult = (::RemoveDirectoryW((const wchar_t*) directoryname) != 0)?true:false;
			::CloseHandle(DirectoryHandle);
			return DeletionResult;
		}
		else
		{
			return true;
		}
	}
	return true;
}

void FSAPI::CopyDirectory(string strSourcePath, string strTargetPath)
{
	if (INVALID_FILE_ATTRIBUTES == ::GetFileAttributesW(strTargetPath))
		::CreateDirectoryW(strTargetPath, NULL);

	DirectoryInfo di(strSourcePath);

	// GetSubDirectories
	try
	{
		array<DirectoryInfo*> arDirs = di.GetDirectories();
		int nDirCount = arDirs.GetLength();

		for (int n = 0; n < nDirCount; n++)
		{
			DirectoryInfo *pInfo = arDirs[n];
			string strSubSource = string::Format(L"%ws\\%ws", 
				(const wchar_t*)strSourcePath,
				(const wchar_t*)pInfo->GetName());

			string strSubTarget = string::Format(L"%ws\\%ws", 
				(const wchar_t*)strTargetPath,
				(const wchar_t*)pInfo->GetName());

			if (INVALID_FILE_ATTRIBUTES == ::GetFileAttributesW(strSubTarget))
				::CreateDirectoryW(strSubTarget, NULL);
			FSAPI::CopyDirectory(strSubSource, strSubTarget);


			delete pInfo;
			pInfo = null;
		}
	}
	catch (Exception *pe)
	{
		delete pe;
	}

	// Copy Files
	try
	{
		array<FileInfo*> arFiles = di.GetFiles();
		int nFileCount = arFiles.GetLength();

		for (int n = 0; n < nFileCount; n++)
		{
			bool fCopy = true;

			FileInfo *pInfo = arFiles[n];
			string strSourceFullPath = string::Format(L"%ws\\%ws",
				(const wchar_t*)strSourcePath,
				(const wchar_t*)pInfo->GetName());
			string strTargetFullPath = string::Format(L"%ws\\%ws",
				(const wchar_t*)strTargetPath,
				(const wchar_t*)pInfo->GetName());

			if (INVALID_FILE_ATTRIBUTES != ::GetFileAttributesW(strTargetFullPath))
			{
				::SetFileAttributesW(strTargetFullPath, FILE_ATTRIBUTE_NORMAL);
				if (FALSE == ::DeleteFileW(strTargetFullPath))
					fCopy = false;
			}

			if (fCopy)
			{
				DWORD dwSource = ::GetFileAttributesW(strSourceFullPath);
				::SetFileAttributesW(strSourceFullPath, FILE_ATTRIBUTE_NORMAL);

				if (::CopyFileW(strSourceFullPath, strTargetFullPath, FALSE))
				{
					::SetFileAttributesW(strSourceFullPath, dwSource);
					::SetFileAttributesW(strTargetFullPath, dwSource);
				}
			}

			delete pInfo;
			pInfo = null;
		}
	}
	catch (Exception *pe)
	{
		delete pe;
	}
}

array<string> FSAPI::SplitString(string strString, wchar_t wch)
{
	array<string> astrResult;
	try
	{
		if (!strString.IsEmpty())
		{
			array<wchar_t> awc(1);
			awc[0] = wch;
			astrResult = strString.Split(awc);
		}
	}
	catch (Exception *pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}
	return astrResult;
}

string FSAPI::StringTrim(string str)
{
	string strResult = str;
	try
	{
		array<wchar_t> awcStr = str.GetChars();

		int nFirst = 0 , nLast = awcStr.GetLength() - 1;

		for (int i = nFirst ; i < awcStr.GetLength() ; i++)
		{
			if (awcStr[i] == L' ' || awcStr[i] == L'\t' || awcStr[i] == L'\r' || awcStr[i] == L'\n')
			{
				nFirst ++;
			}
			else
			{
				break;
			}
		}

		for (int i = nLast ; i >= 0 ; i--)
		{
			if (awcStr[i] == L' ' || awcStr[i] == L'\t' || awcStr[i] == L'\r' || awcStr[i] == L'\n')
			{
				nLast --;
			}
			else
			{
				break;
			}
		}

		if (nFirst > nLast)
		{
			strResult = L"";
		}
		else
		{
			strResult = str.Substring(nFirst, nLast - nFirst + 1);
		}
	}
	catch (Exception *pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}
	return strResult;
}

wchar_t FSAPI::GetEmptyDriveLetter(wchar_t wcStartLetter, wchar_t wcEndLetter, bool fGetLastDriveLetter)
{
	wchar_t wcDriveLetter;
	bool arrNetworkDrives[26] = {false,};
	bool fFound = false;
	int nStartLetter = fGetLastDriveLetter ? (towupper(wcEndLetter) - 'A') : (towupper(wcStartLetter) - 'A');
	int nEndLetter = fGetLastDriveLetter ? (towupper(wcStartLetter) - 'A') : (towupper(wcEndLetter) - 'A');
	int nInterval = fGetLastDriveLetter ? -1 : 1;
		
	Array<DriveInfo*> allDrives = DriveInfo::GetDrives();
	for(int i = 0 ; i < allDrives.GetLength() ; i++)
	{
		if (allDrives[i]->GetDriveTypeW() == EDriveType::Network)
		{
			wchar_t wcNetworkDriveLetter = ((wchar_t*)allDrives[i]->GetName())[0];
			arrNetworkDrives[wcNetworkDriveLetter - L'A'] = true;
		}
	}

	for (int i = nStartLetter; fGetLastDriveLetter ? (i >= nEndLetter) : (i <= nEndLetter); i += nInterval)
	{
		if(arrNetworkDrives[i] == true)
			continue;

		string strLocalName =  String((wchar_t)(L'A' + i)) + L":";
		string strDriveLetter = L"\\\\.\\" + strLocalName;

		HANDLE hFile = ::CreateFileW(
			(const wchar_t*)strDriveLetter,
			GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE, 
			NULL, 
			OPEN_EXISTING, 
			FILE_FLAG_NO_BUFFERING, 
			0
			);

		if (INVALID_HANDLE_VALUE == hFile)
		{
			DWORD dwError = ::GetLastError();

			if (2 == dwError)
			{
				wchar_t wszRemoteName[MAX_PATH];
				DWORD dwLen = MAX_PATH;
				// Check Network Drive

				DWORD dwRet = WNetGetConnectionW(strLocalName, wszRemoteName, &dwLen);

				if (ERROR_MORE_DATA != dwRet &&
					NO_ERROR != dwRet &&
					ERROR_CONNECTION_UNAVAIL != dwRet)
				{
					wcDriveLetter = char('A' + i);
					fFound = true;
				}
			}
		}
		else
		{
			::CloseHandle(hFile);

			DWORD dwType = ::GetDriveTypeW(string((wchar_t)(L'A'+i)) + L":\\");

			if (DRIVE_UNKNOWN == dwType)
			{
				wcDriveLetter = char('A' + i);
				fFound = true;
			}
		}
	
		if (fFound)
				break;
	}

	return wcDriveLetter;
}

UINT FSAPI::GetDriveType(string& strDriveLetter)
{
	UINT nDriveType = ::GetDriveTypeW(strDriveLetter + L":\\");

	if (nDriveType > 0)
	{
		if (DRIVE_FIXED == nDriveType)
		{
			string strDrive = string::Format(L"\\\\.\\%ws:", (const wchar_t*)strDriveLetter);
			HANDLE hFile = ::CreateFile(
				(const wchar_t*)strDrive,
				0,
				FILE_SHARE_READ | FILE_SHARE_WRITE, 
				NULL, 
				OPEN_EXISTING, 
				0, 
				0
				);
			if (INVALID_HANDLE_VALUE != hFile)
			{
				STORAGE_PROPERTY_QUERY query;
				PSTORAGE_DEVICE_DESCRIPTOR pDesc = (PSTORAGE_DEVICE_DESCRIPTOR)new BYTE[sizeof(STORAGE_DEVICE_DESCRIPTOR) + 1024];
				DWORD dwReturned;

				memset(&query, 0, sizeof(query));
				memset(pDesc, 0, sizeof(STORAGE_DEVICE_DESCRIPTOR) + 1024);
				pDesc->Size = sizeof(STORAGE_DEVICE_DESCRIPTOR) + 1024;
				if (::DeviceIoControl(hFile, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(query), pDesc, pDesc->Size, &dwReturned, NULL))
				{
					if (BusTypeUsb == pDesc->BusType ||
						BusTypeSd == pDesc->BusType ||
						BusTypeMmc == pDesc->BusType)
						nDriveType = DRIVE_REMOVABLE;
				}
				::CloseHandle(hFile);
				delete[] pDesc;
				pDesc = null;
			}
		}
	}

	return nDriveType;
}

array<string> FSAPI::GetFixedDrive()
{
	List<string> lst;

	for (int i = 0; i < 26; i++)
	{
		wchar_t ch = L'A' + i;

		UINT nDriveType = Library::API::FSAPI::GetDriveType(string(ch));
		if (nDriveType == DRIVE_FIXED)
			lst.Add(string::Format(L"%wc:", (wchar_t)ch));
	}

	array<string> astr(lst.GetCount());
	lst.CopyTo(astr, 0);
	return astr;
}

void FSAPI::GetMountDrives(uint32& uFixedDrives, uint32& uExtDrives, uint32& uCDRDrives, uint32& uCDRWDrives, uint32& uNetShareDrives, uint32& uUnknownDrives)
{
	bool fPrivilegeFail = false;
	FSAPI::GetMountDrives(uFixedDrives, uExtDrives, uCDRDrives, uCDRWDrives, uNetShareDrives, uUnknownDrives, fPrivilegeFail);
}

// #15159
void FSAPI::GetMountDrives(uint32& uFixedDrives, uint32& uExtDrives, uint32& uCDRDrives, uint32& uCDRWDrives, uint32& uNetShareDrives, uint32& uUnknownDrives, bool& fPrivilegeFail)
{
	fPrivilegeFail = false;

	for (int i = 0; i < 26; i++)
	{
		string strDrvLetter = string::Format(L"%wc:", (wchar_t)(L'A'+i));
		string strDeviceName = string::Format(L"\\\\.\\%wc:", (wchar_t)(L'A'+i));
		string strSystemDrv = Environment::GetFolderPath(ESpecialFolder::System).Substring(0, 2).ToLower();

		uint32 uDriveTypes = ::GetDriveTypeW(strDrvLetter + L"\\");

		if (DRIVE_FIXED == uDriveTypes)
		{
			HANDLE hFile = ::CreateFileW(strDeviceName, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, 0);

			if (INVALID_HANDLE_VALUE == hFile)
			{
				if (!strDrvLetter.ToLower().Equals(strSystemDrv))
					uUnknownDrives |= (1 << i);

				continue;
			}

			STORAGE_PROPERTY_QUERY query;
			STORAGE_DEVICE_DESCRIPTOR desc;
			DWORD dwReturned = 0;
			memset(&query, 0, sizeof(query));
			memset(&desc, 0, sizeof(desc));
			desc.Size = sizeof(desc);					

			::DeviceIoControl(hFile, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(query), &desc, desc.Size, &dwReturned, NULL);

			if (BusTypeUsb == desc.BusType ||
				BusTypeSd == desc.BusType ||
				BusTypeMmc == desc.BusType)
				uDriveTypes = DRIVE_REMOVABLE;

			::CloseHandle(hFile);
		}
		else if (DRIVE_CDROM == uDriveTypes)
		{
			CDRomDriveType type = (CDRomDriveType)_GetCDRomDriveType(strDeviceName, fPrivilegeFail);
			if (ECDRomDriveType::IsWritable(type))
				uDriveTypes = _DRIVE_CDRW;
			else
			{
				// redmine #14058
				// driveType이 CDR 일 때, CDR에서도 쓰기 가능한 Media가 존재 (공씨디)
				if (_IsRecordableMedia(strDrvLetter))
					uDriveTypes = _DRIVE_CDRW;
			}
		}

		if (strDrvLetter.ToLower().Equals(strSystemDrv))
			uDriveTypes = DRIVE_FIXED;

		if (1 >= uDriveTypes) continue;

		if (DRIVE_FIXED == uDriveTypes)
			uFixedDrives |= (1 << i);
		else if (DRIVE_CDROM == uDriveTypes)
			uCDRDrives |= (1 << i);
		else if (_DRIVE_CDRW == uDriveTypes)
			uCDRWDrives |= (1 << i);
		else
		{
			if (DRIVE_REMOTE == uDriveTypes)
				uNetShareDrives |= (1 << i);

			uExtDrives |= (1 << i);
		}
	}

	_TRACE_I(Application, L"FixedDrive = 0x%08X", uFixedDrives);
	_TRACE_I(Application, L"ExtDrive = 0x%08X", uExtDrives);
	_TRACE_I(Application, L"CDRDrive = 0x%08X", uCDRDrives);
	_TRACE_I(Application, L"CDRWDrive = 0x%08X", uCDRWDrives);
	_TRACE_I(Application, L"NetShareDrive = 0x%08X", uNetShareDrives);
	_TRACE_I(Application, L"UnknownDrive = 0x%08X", uUnknownDrives);
}

uint32 FSAPI::_GetCDRomDriveType(string strDeviceName, bool& fPrivilegeFail)
{
	CDRomDriveType type = ECDRomDriveType::Unknown;
	HANDLE hDevice = INVALID_HANDLE_VALUE;

	do 
	{
		hDevice = ::CreateFile(
			strDeviceName,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL
			);

		if (INVALID_HANDLE_VALUE == hDevice)
		{
			DWORD dwError = ::GetLastError();
			if (ERROR_ACCESS_DENIED == dwError)
				fPrivilegeFail = true;

			_TRACE_I(Application, L"[%ws] CreateFile failed... (error=%d)", (const wchar_t*)strDeviceName, dwError);
			break;
		}

		SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER sptdwb;
		DeviceConfigHeader dcHeader;
		DWORD dwSPTDWBSize = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);
		DWORD dwReturned;

		memset(&sptdwb, 0, sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER));
		memset(&dcHeader, 0, sizeof(DeviceConfigHeader));

		sptdwb.sptd.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
		sptdwb.sptd.CdbLength = 10;
		sptdwb.sptd.SenseInfoLength = 32;
		sptdwb.sptd.DataIn = 1;
		sptdwb.sptd.DataTransferLength = sizeof(DeviceConfigHeader);
		sptdwb.sptd.TimeOutValue = 5;	// second
		sptdwb.sptd.DataBuffer = &dcHeader;
		sptdwb.sptd.SenseInfoOffset = sizeof(SCSI_PASS_THROUGH_DIRECT) + sizeof(ULONG);

		sptdwb.sptd.Cdb[0] = 0x46;
		sptdwb.sptd.Cdb[1] = 0x02;
		sptdwb.sptd.Cdb[3] = 0x00;
		sptdwb.sptd.Cdb[7] = HIBYTE(sizeof(DeviceConfigHeader));
		sptdwb.sptd.Cdb[8] = LOBYTE(sizeof(DeviceConfigHeader));

		if (!::DeviceIoControl(
			hDevice,
			IOCTL_SCSI_PASS_THROUGH_DIRECT,
			&sptdwb,
			dwSPTDWBSize,
			&sptdwb,
			dwSPTDWBSize,
			&dwReturned,
			NULL
			))
		{
			_TRACE_I(Application, L"[%ws] DeviceIoControl failed... (error=%d)", (const wchar_t*)strDeviceName, ::GetLastError());
			break;
		}

		USHORT cp = ((dcHeader.CurrentProfile << 8) & 0xFF00) | ((dcHeader.CurrentProfile >> 8) & 0x00FF);
		switch (cp)
		{
		case 0x0000:	type = ECDRomDriveType::NoCurrentProfile; break;
		case 0x0001:	type = ECDRomDriveType::NonRemovableDisk; break;
		case 0x0002:	type = ECDRomDriveType::RemovableDisk; break;
		case 0x0003:	type = ECDRomDriveType::MagnetoOpticalErasable; break;
		case 0x0004:	type = ECDRomDriveType::OpticalWriteOnce; break;
		case 0x0005:	type = ECDRomDriveType::AS_NO; break;
		case 0x0008:	type = ECDRomDriveType::CD_ROM; break;
		case 0x0009:	type = ECDRomDriveType::CD_R; break;
		case 0x000A:	type = ECDRomDriveType::CD_RW; break;
		case 0x0010:	type = ECDRomDriveType::DVD_ROM; break;
		case 0x0011:	type = ECDRomDriveType::DVD_R_SequentialRecording; break;
		case 0x0012:	type = ECDRomDriveType::DVD_RAM; break;
		case 0x0013:	type = ECDRomDriveType::DVD_RW_RestrictedOverwrite; break;
		case 0x0014:	type = ECDRomDriveType::DVD_RW_SequentialRecording; break;
		case 0x001A:	type = ECDRomDriveType::DVD_PLUS_RW; break;
		case 0x001B:	type = ECDRomDriveType::DVD_PLUS_R; break;
		case 0x0020:	type = ECDRomDriveType::DDCD_ROM; break;
		case 0x0021:	type = ECDRomDriveType::DDCD_R; break;
		case 0x0022:	type = ECDRomDriveType::DDCD_RW; break;
		default:		type = ECDRomDriveType::Unknown; break;
		}

	} while (false);

	if (INVALID_HANDLE_VALUE != hDevice)
		::CloseHandle(hDevice);

	return type;
}

bool FSAPI::_IsRecordableMedia(string strDrvLetter)
{
	::CoInitialize(NULL);

	bool fRet = false;
	DiscMaster2* pDiscMaster2 = NULL;

	try
	{
		pDiscMaster2 = new DiscMaster2;
		if (!pDiscMaster2->IsSupportedEnvironment())
			throw new DiscException(L"not supported environment.");

		for (int i = 0; i < pDiscMaster2->GetCount(); i++)
		{
			try
			{
				string strUniqueId = pDiscMaster2->GetDeviceUniqueId(i);

				DiscRecorder2 recorder;
				recorder.InitializeDiscRecorder(strUniqueId);
				array<string> astrVolumePath = recorder.GetVolumePathNames();

				bool fFind = false;
				for (int j = 0; j < astrVolumePath.GetLength(); j++)
				{
					if (strDrvLetter.ToLower().GetChars()[0] == astrVolumePath[j].ToLower().GetChars()[0])
					{
						fFind = true;
						break;
					}
				}

				if (!fFind) continue;

				DiscFormat2Data data;
				if (!data.IsRecorderSupported(&recorder))
					continue;
				if (!data.IsCurrentMediaSupported(&recorder))
					continue;

				fRet = true;
				_TRACE_I(Application, L"[%ws] Media supported!", (const wchar_t*)strDrvLetter);
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

	if (pDiscMaster2)
		delete pDiscMaster2;

	::CoUninitialize();

	return fRet;
}

bool FSAPI::DisableNetworkShare(uint32 uDisableDrives)
{
	bool fRet = false;

	if (0 == uDisableDrives)
		return fRet;

	PSHARE_INFO_502 BufPtr,p;
	NET_API_STATUS res;
	DWORD er=0,tr=0,resume=0;
	do
	{
		res = NetShareEnum(NULL, 502, (LPBYTE *) &BufPtr, MAX_PREFERRED_LENGTH, &er, &tr, &resume);
		if (ERROR_SUCCESS == res || ERROR_MORE_DATA == res)
		{
			p = BufPtr;
			_TRACE_I(Application, L"er = %d, tr = %d", er, tr);

			for (int n = 1; n <= er; n++)
			{
				string strName = p->shi502_netname;
				string strPath = p->shi502_path;
				_TRACE_I(Application, L"Name = %ws, Path = %ws", (const wchar_t*)strName, (const wchar_t*)strPath);

				if (!strPath.IsEmpty())
				{
					int nIdx = strPath.ToUpper().GetChars()[0] - L'A';
					uint32 uValue = 1 << nIdx;

					if (uValue & uDisableDrives)
					{
						NET_API_STATUS res2;
						res2 = NetShareDel(NULL, strName, 0);
						_TRACE_I(Application, L"NetShareDel [%ws] Result = %x", (const wchar_t*)strName, res2);
					}
				}
				p++;
			}
			NetApiBufferFree(BufPtr);
		}
	} 
	while (ERROR_MORE_DATA == res);

	return fRet;
}

void FSAPI::BroadCastDriveMount(string strDrvLetter)
{
	if (strDrvLetter.IsEmpty())
		return;

	DEV_BROADCAST_VOLUME dbv;
	dbv.dbcv_size = sizeof (dbv); 
	dbv.dbcv_devicetype = DBT_DEVTYP_VOLUME; 
	dbv.dbcv_reserved = 0;
	dbv.dbcv_unitmask = (1 << strDrvLetter.GetChars()[0] -'A');
	dbv.dbcv_flags = 0; 

	UINT timeOut = 1000;
	DWORD_PTR dwResult;
	string strDirectory = strDrvLetter + L":\\MirageWorks2";

	_TRACE_I(Application, L"Drive Letter = %wc unitmask = %x", strDrvLetter.GetChars()[0], dbv.dbcv_unitmask);
	_TRACE_I(Application, L"Drive Letter = %ws", (const wchar_t*)strDrvLetter);
	_TRACE_I(Application, L"[SendMessageTimeout] Before CreateDirectory = %ws", (const wchar_t*)strDirectory);
	::CreateDirectory(strDirectory, NULL);
	_TRACE_I(Application, L"[SendMessageTimeout] After CreateDirectory Error = %d", ::GetLastError());
	_TRACE_I(Application, L"[SendMessageTimeout] Before SendMessageTimeout");
	SendMessageTimeout(HWND_BROADCAST, WM_DEVICECHANGE, DBT_DEVICEARRIVAL, (LPARAM)(&dbv), SMTO_ABORTIFHUNG, timeOut, &dwResult);

	if (ERROR_TIMEOUT == ::GetLastError())
		_TRACE_I(Application, L"Timeout exception");
	_TRACE_I(Application, L"[SendMessageTimeout] After SendMessageTimeout");

	DWORD dwValue = BSM_ALLCOMPONENTS;

	_TRACE_I(Application, L"[BroadcastSystemMessage] Before CreateDirectory = %ws", (const wchar_t*)strDirectory);
	::CreateDirectory(strDirectory, NULL);
	_TRACE_I(Application, L"[BroadcastSystemMessage] After CreateDirectory Error = %d", ::GetLastError());
	_TRACE_I(Application, L"[BroadcastSystemMessage] Before BroadcastSystemMessage");
	::BroadcastSystemMessage(BSF_FORCEIFHUNG | BSF_POSTMESSAGE, &dwValue, WM_DEVICECHANGE, DBT_DEVICEARRIVAL, (LPARAM)&dbv);
	_TRACE_I(Application, L"[BroadcastSystemMessage] After BroadcastSystemMessage Error = %d", ::GetLastError());
}

string FSAPI::GetFileHash(string strPath)
{
	string strHash = L"";

	do
	{
		if (INVALID_FILE_ATTRIBUTES == ::GetFileAttributesW(strPath))
			break;

		FileStream fs(strPath, EFileMode::Open, EFileAccess::Read);
		HashAlgorithm* pHash = NULL;

		try
		{
			pHash = HashAlgorithm::Create(L"SHA256");
			strHash = Convert::ToBase16String(pHash->ComputeHash(&fs));
		}
		catch (Exception* pe)
		{
			delete pe;
		}

		fs.Close();

		if (NULL != pHash)
			delete pHash;

	} while (false);

	return strHash;
}

string FSAPI::GetFileVersion(string strPath)
{
	string strVersion = L"0, 0, 0, 0";
	try
	{
		FileVersionInfo* pFVI = FileVersionInfo::GetVersionInfo(strPath);
		_TRACE_I(Application, L"GetFileVersion() %ws = %ws", (const wchar_t*)strPath, (const wchar_t*)pFVI->GetProductVersion());
		
		if (!pFVI->GetProductVersion().IsEmpty())
			strVersion = pFVI->GetProductVersion().Replace(L".", L", ");
	}
	catch (Exception* pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}
	
	return strVersion;
}

uint64 FSAPI::GetDiskFreeSizeOnByte(string strDrvLetter)
{
	uint64 nSystemFreeSize = 0;
	if(strDrvLetter.GetLength() > 1)
		strDrvLetter = strDrvLetter.Substring(0, 1) + L":\\";
	else
		strDrvLetter = strDrvLetter + L":\\";

	try
	{
		ULARGE_INTEGER FreeByte;
		if(FALSE != ::GetDiskFreeSpaceExW(strDrvLetter, &FreeByte, NULL, NULL))
		{
			nSystemFreeSize = FreeByte.QuadPart;
			_TRACE_I(Application, L"SystemFreeSize : %I64d", nSystemFreeSize);
		}
		else
		{
			DWORD dwError = ::GetLastError();
			_TRACE_E(Application, L"GetDiskFreeSpaceExW Error : %d", dwError);
		}
	}
	catch (Exception* pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}

	return nSystemFreeSize;
}

uint64 FSAPI::GetDirectorySizeOnByte(string strPath)
{
	int64 dirSize = 0;
	try
	{
		DirectoryInfo dir(strPath);
		if(dir.Exists())
		{
			array<FileInfo*> files = dir.GetFiles();

			for(int i = 0; i < files.GetLength(); i++)
			{
				dirSize += files[i]->GetLength();
				delete files[i];
			}

			array<DirectoryInfo*> dirs = dir.GetDirectories();
			for(int i = 0; i < dirs.GetLength(); i++)
			{
				dirSize += GetDirectorySizeOnByte(dirs[i]->GetFullName());	
				delete dirs[i];
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
	return dirSize;
}

int64 FSAPI::GetDirectoryCount(string strPath)
{
	int64 nDirCount = 0;
	try
	{
		DirectoryInfo dir(strPath);	
		if(dir.Exists())
		{
			nDirCount = dir.GetDirectoryCount();

			array<DirectoryInfo*> dirs = dir.GetDirectories();
			for(int i = 0; i < dirs.GetLength(); i++)
			{
				nDirCount += FSAPI::GetDirectoryCount(dirs[i]->GetFullName());
				delete dirs[i];
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

	return nDirCount;
}

int64 FSAPI::GetFileCount(string strPath)
{
	int64 nFileCount = 0;
	try
	{
		DirectoryInfo dir(strPath);	
		if(dir.Exists())
		{
			nFileCount = dir.GetFileCount();

			array<DirectoryInfo*> dirs = dir.GetDirectories();
			for(int i = 0; i < dirs.GetLength(); i++)
			{
				nFileCount += FSAPI::GetFileCount(dirs[i]->GetFullName());
				delete dirs[i];
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

	return nFileCount;
}

bool FSAPI::AddToACL(PACL& pACL, string AccountName, DWORD AccessOption)
{
	CHAR sid[MAX_PATH] = {0};
	DWORD sidlen = MAX_PATH;
	wchar_t dn[MAX_PATH] = {0};
	DWORD dnlen = MAX_PATH;
	SID_NAME_USE SNU;

	if(!LookupAccountName(NULL, AccountName, (PSID)sid, &sidlen, dn, &dnlen, &SNU))
		return FALSE;

	EXPLICIT_ACCESS ea = {0};
	ea.grfAccessPermissions = AccessOption;
	ea.grfAccessMode = SET_ACCESS;
	ea.grfInheritance= SUB_CONTAINERS_AND_OBJECTS_INHERIT;
	ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
	ea.Trustee.ptstrName = (LPTSTR) (PSID)sid;

	PACL temp = NULL;
	if(SetEntriesInAcl(1, &ea, pACL, &temp) != ERROR_SUCCESS)
		return FALSE;

	LocalFree(pACL);
	pACL = temp;
	return TRUE;
}

bool FSAPI::ChangeFileSecurityOption(string strPath)
{
	BYTE SDBuffer[4096] = {0};
	DWORD SDLength = 4096, RC;
	SECURITY_DESCRIPTOR* SD = (SECURITY_DESCRIPTOR*)SDBuffer;

	if(!InitializeSecurityDescriptor(SD, SECURITY_DESCRIPTOR_REVISION))
		return FALSE;

	PACL pACL = (PACL)LocalAlloc(LMEM_FIXED, sizeof(ACL));
	if(!InitializeAcl(pACL, MAX_PATH, ACL_REVISION))
	{
		LocalFree(pACL);
		return FALSE;
	}

	DWORD AccessOption = GENERIC_ALL | GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE;
	FSAPI::AddToACL(pACL, "Everyone", AccessOption);
	FSAPI::AddToACL(pACL, "Administrators", AccessOption);
	FSAPI::AddToACL(pACL, "Users", AccessOption);
	FSAPI::AddToACL(pACL, "SYSTEM", AccessOption);

	if(!SetSecurityDescriptorDacl(SD, TRUE, pACL, TRUE))
	{
		LocalFree(pACL);
		return FALSE;
	}

	RC = SetFileSecurity(strPath, DACL_SECURITY_INFORMATION, SD);
	LocalFree(pACL);
	return RC;
}

}
}