#include "StdAfx.h"
#include "file_deploy.h"


#define BUF_LEN 65536

file_deploy::file_deploy(void)
{
}

file_deploy::~file_deploy(void)
{
}

int file_deploy::deploy()
{
	int nRet = 1;
	LList<lstring> lstSection;
	LList<lstring> lstClearPath;

	try
	{
		if (!GetParameterInfo(m_strFilename, lstSection, lstClearPath))
			return nRet;

		if (!prepare(lstClearPath))
			return nRet;

		for (int i = 0 ; i < lstSection.GetCount() ; i++)
		{
			if (!work(lstSection[i]))
				return nRet;
		}

		nRet = 0;
	}
	catch (LException* pe)
	{
		LConsole::WriteLine(pe->ToString());
		delete pe;
	}

	return 0;
}

lstring file_deploy::Trim( lstring str )
{
	larray<wchar_t> awcStr = str.GetChars();

	int nFirst = 0 , nLast = awcStr.GetLength() - 1;
	for (int i = nFirst ; i < awcStr.GetLength() ; i++)
	{
		if (awcStr[i] == L' ' || awcStr[i] == L'\t' || awcStr[i] == L'\r' || awcStr[i] == L'\n' || awcStr[i] == L'\r\n')
			nFirst ++;
		else
			break;
	}

	for (int i = nLast ; i >= 0 ; i--)
	{
		if (awcStr[i] == L' ' || awcStr[i] == L'\t' || awcStr[i] == L'\r' || awcStr[i] == L'\n' || awcStr[i] == L'\r\n')
			nLast --;
		else
			break;
	}

	if (nFirst > nLast)
		return L"";

	return str.Substring(nFirst, nLast - nFirst + 1);
}

void file_deploy::CreateDirectoryRecursively( lstring strDirectory )
{
	lstring str;
	larray<lstring> astrDir = strDirectory.Split(lstring(L"\\").GetChars());
	for (int i = 0 ; i < astrDir.GetLength() ; i++)
	{
		if (str.IsEmpty())
			str += astrDir[i];
		else
			str += (L"\\" + astrDir[i]);

		if (INVALID_FILE_ATTRIBUTES == ::GetFileAttributesW(str))
			::CreateDirectoryW(str, NULL);
	}
}

lstring file_deploy::GetDataFilePath( lstring strFile )
{
	if (strFile.Contains(L":\\"))
	{
		return strFile;
	}
	else
	{
		wchar_t ac_Result[BUF_LEN];
		GetCurrentDirectoryW(BUF_LEN, ac_Result);

		lstring strFile_ = ac_Result;
		strFile_ += L"\\";
		strFile_ += strFile;
		return strFile_;
	}
}

bool file_deploy::RemoveDirectoryRecursively( lstring strDirectory )
{
	if (strDirectory.IsEmpty())
		return false;
	if (!strDirectory.EndsWith(L"\\"))
		strDirectory += L"\\";

	WIN32_FIND_DATAW fdata;
	HANDLE handle = ::FindFirstFileW((const wchar_t*)(strDirectory + L"*"), &fdata);
	if (handle != INVALID_HANDLE_VALUE)
	{
		do 
		{
			lstring path = strDirectory + fdata.cFileName;
			if (path.EndsWith(L"\\.") || path.EndsWith(L"\\.."))
				continue;

			if (fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (!RemoveDirectoryRecursively(path))
					return false;

				if (!::RemoveDirectory(path))
				{
					LConsole_WriteLine(L"failed to remove directory with %d : %ws", ::GetLastError(), (const wchar_t*)path);
					return false;
				}
				else
				{
					LConsole_WriteLine(L"remove directory : %ws", (const wchar_t*)path);
				}
			}
			else
			{
				if (!::DeleteFileW(path))
				{
					LConsole_WriteLine(L"failed to remove file with %d : %ws", ::GetLastError(), (const wchar_t*)path);
					return false;
				}
				else
				{
					LConsole_WriteLine(L"remove file : %ws", (const wchar_t*)path);
				}
			}
		}
		while (FindNextFileW(handle, &fdata));
		FindClose(handle);
	}

	return true;
}

bool file_deploy::GetParameterInfo( lstring& strDataFile, LList<lstring>& lstSections, LList<lstring>& lstClearPath )
{
	larray<lstring> astrArgs = LEnvironment::GetCommandLineArgs();
	if (astrArgs.GetLength() < 4)
	{
		LConsole_WriteLine(L"invalid argument");
		return false;
	}

	strDataFile = GetDataFilePath(astrArgs[2]);
	if (INVALID_FILE_ATTRIBUTES == ::GetFileAttributesW(strDataFile))
	{
		LConsole_WriteLine(L"the input data file do not exist.");
		return false;
	}

	for (int i = 3 ; i < astrArgs.GetLength() ; i++)
	{
		if (!GetSectionInfo(strDataFile, astrArgs[i], lstSections))
		{
			LConsole_WriteLine(L"failed to retrieve the section information.");
			return false;
		}

		if (!GetClearPathInfo(strDataFile, astrArgs[i], lstClearPath))
		{
			LConsole_WriteLine(L"failed to retrieve the clear path information.");
			return false;
		}
	}

	return true;
}

bool file_deploy::GetSectionInfo( lstring strDataFile, lstring strSection, LList<lstring>& lstSections )
{
	if (strDataFile.IsEmpty())
		return false;
	if (strSection.IsEmpty())
		return false;

	lstring str;
	wchar_t awc[1024];

	GetPrivateProfileString((const wchar_t*)strSection, L"section", L"", awc, 1024, (const wchar_t*)strDataFile);
	str = awc;

	if (!str.IsEmpty())
	{
		larray<lstring> astrSection = str.Split(lstring(L",").GetChars());
		for (int i = 0 ; i < astrSection.GetLength() ; i++)
		{
			if (astrSection[i].Equals(strSection, true))
			{
				LConsole_WriteLine(L"invalid file format. duplicate section : " + astrSection[i]);
				return false;
			}

			for (int j = 0 ; j < lstSections.GetCount() ; j++)
			{
				if (lstSections[j].Equals(astrSection[i], true))
				{
					LConsole_WriteLine(L"invalid file format. duplicate section : " + astrSection[i]);
					return false;
				}
			}

			GetSectionInfo(strDataFile, astrSection[i], lstSections);
		}
	}
	else
	{
		LConsole_WriteLine(L"prepare section : " + strSection);
		lstSections.Add(strSection);
	}

	return true;
}

bool file_deploy::GetClearPathInfo( lstring& strDataFile, lstring strSection, LList<lstring>& lstClearPath )
{
	if (strDataFile.IsEmpty())
		return false;
	if (strSection.IsEmpty())
		return false;

	lstring str;
	wchar_t awc[1024];

	GetPrivateProfileString((const wchar_t*)strSection, L"clear_path", L"", awc, 1024, (const wchar_t*)strDataFile);
	str = awc;

	if (!str.IsEmpty())
	{
		larray<lstring> astrPath = str.Split(lstring(L",").GetChars());
		for (int i = 0 ; i < astrPath.GetLength() ; i++)
		{
			for (int j = 0 ; j < lstClearPath.GetCount() ; j++)
			{
				if (lstClearPath[j].Equals(astrPath[i], true))
				{
					LConsole_WriteLine(L"invalid file format. duplicate clear_path : " + astrPath[i]);
					return false;
				}
			}

			lstClearPath.Add(astrPath[i]);
		}
	}

	return true;
}

void file_deploy::GetSectionData( lstring& strDataFile, lstring strSection, LList<lstring>& lstData )
{
	DWORD dwRet = 0;
	wchar_t ac_Result[BUF_LEN];
	lstring strSectionData = L"";

	dwRet = GetPrivateProfileSection((const wchar_t*)strSection, ac_Result, BUF_LEN, (const wchar_t*)strDataFile);
	for (int i = 0 ; i < dwRet ; i++)
	{
		if (ac_Result[i] != L'\0')
		{
			strSectionData = strSectionData + ac_Result[i];
		}
		else
		{
			strSectionData = Trim(strSectionData);
			if (!strSectionData.IsEmpty())
				lstData.Add(strSectionData);
			strSectionData = L"";
		}
	}
}

bool file_deploy::prepare(LList<lstring> lstClearPath)
{
	for (int i = 0 ; i < lstClearPath.GetCount() ; i++)
	{
		DWORD dwAttr = ::GetFileAttributesW(lstClearPath[i]);
		if (INVALID_FILE_ATTRIBUTES != dwAttr && FILE_ATTRIBUTE_DIRECTORY & dwAttr)
		{
			if (!RemoveDirectoryRecursively(lstClearPath[i]))
				return false;

			if (!::RemoveDirectory(lstClearPath[i]))
			{
				LConsole_WriteLine(L"failed to remove directory with %d : %ws", ::GetLastError(), (const wchar_t*)lstClearPath[i]);
				return false;
			}
			else
			{
				LConsole_WriteLine(L"remove directory : %ws", (const wchar_t*)lstClearPath[i]);
			}
		}
	}

	LList<lstring> lstVariables;
	GetSectionData(m_strFilename, L"VARIABLE", lstVariables);

	for (int i = 0 ; i < lstVariables.GetCount() ; i++)
	{
		lstring name = lstVariables[i].Substring(0, lstVariables[i].IndexOf(L'='));
		lstring value = lstVariables[i].Substring(lstVariables[i].IndexOf(L'=') + 1);

		if (name.IsEmpty() || value.IsEmpty())
		{
			LConsole_WriteLine(L"invalid file format : [VARIABLE] %ws", (const wchar_t*)lstVariables[i]);
			return false;
		}

		if (m_dicVariables.ContainsKey(name))
			m_dicVariables.Remove(name);

		LConsole_WriteLine(L"[variable] %ws : %ws", (const wchar_t*)name, (const wchar_t*)value);
		m_dicVariables.Add(name, value);
	}
	return true;
}

bool file_deploy::work( lstring strSection )
{
	LList<lstring> lstData;
	GetSectionData(m_strFilename, strSection, lstData);

	if (lstData.GetCount() == 0)
	{
		LConsole_WriteLine(L"invalid file format : no data [%ws]", (const wchar_t*)strSection);
		return false;
	}

	for (int i = 0 ; i < lstData.GetCount() ; i++)
	{
		ExpandData(lstData[i]);

		lstring source = Trim(lstData[i].Substring(0, lstData[i].IndexOf(L'=')));
		lstring target = Trim(lstData[i].Substring(lstData[i].IndexOf(L'=') + 1));
		if (source.IsEmpty() || target.IsEmpty())
		{
			LConsole_WriteLine(L"invalid file format : [%ws] %ws", (const wchar_t*)strSection, (const wchar_t*)lstData[i]);
			return false;
		}

		lstring filename = source.Contains(L"\\") ? source.Substring(source.LastIndexOf(L"\\") + 1) : source;
		CreateDirectoryRecursively(target);

		target = target.EndsWith(L"\\") ? (target + filename) : (target + L"\\" + filename);

		if (!::CopyFileW(source, target, FALSE))
		{
			LConsole_WriteLine(L"copy failed with %d : %ws -> %ws", ::GetLastError(), (const wchar_t*)source, (const wchar_t*)target);
			return false;
		}

		LConsole_WriteLine(L"copied : %ws -> %ws", (const wchar_t*)source, (const wchar_t*)target);
	}

	return true;
}

void file_deploy::ExpandData( lstring& str )
{
	LConsole_WriteLine(str);
	ILEnumerator<lstring> *pe = m_dicVariables.GetKeys()->GetEnumerator();
	while (pe->MoveNext())
	{
		lstring strKey = pe->GetCurrent();
		lstring strValue = m_dicVariables.GetItem(strKey);

		if (str.Contains(L"${" + strKey + L"}"))
			str = str.Replace(L"${" + strKey + L"}", strValue);
	}
	delete pe;
}