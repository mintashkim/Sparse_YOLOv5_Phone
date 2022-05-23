#pragma once
class file_deploy
{
public:
	file_deploy(void);
	~file_deploy(void);

	int deploy();

private:
	bool prepare(LList<lstring> lstClearPath);
	bool work(lstring strSection);
	void ExpandData(lstring& str);

private:
	lstring Trim(lstring str);
	void CreateDirectoryRecursively(lstring strDirectory);
	bool RemoveDirectoryRecursively(lstring strDirectory);
	bool GetParameterInfo(lstring& strDataFile, LList<lstring>& lstSections, LList<lstring>& lstClearPath);
	void GetSectionData(lstring& strDataFile, lstring strSection, LList<lstring>& lstData);

	lstring GetDataFilePath(lstring strFile);
	bool GetSectionInfo(lstring strDataFile, lstring strSection, LList<lstring>& lstSections);
	bool GetClearPathInfo(lstring& strDataFile, lstring strSection, LList<lstring>& lstClearPath);

private:
	lstring m_strFilename;
	LDictionary<lstring, lstring> m_dicVariables;
};