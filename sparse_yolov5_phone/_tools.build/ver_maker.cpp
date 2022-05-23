#include "StdAfx.h"
#include "ver_maker.h"


ver_maker::ver_maker(void)
{
}


ver_maker::~ver_maker(void)
{
}

int ver_maker::make()
{
	try
	{
		lstring base_file = LApplication::GetExecutablePath();
		lstring input_file = LApplication::GetStartupPath() + L"\\info_.xml";
		lstring output_file = LApplication::GetStartupPath() + L"\\info.xml";

		larray<lstring> astrArgs = LEnvironment::GetCommandLineArgs();
		for (int i = 2; i < astrArgs.GetLength(); i++)
		{
			if (astrArgs[i].ToLower().Equals(L"-f"))
				base_file = astrArgs[i + 1];
			else if (astrArgs[i].ToLower().Equals(L"-i"))
				input_file = astrArgs[i + 1];
			else if (astrArgs[i].ToLower().Equals(L"-o"))
				output_file = astrArgs[i + 1];
		}

		LFileStream fileStream(input_file, ELFileMode::Open, ELFileAccess::Read);
		LFileStream fileStream_(output_file, ELFileMode::Create, ELFileAccess::Write);

		LBinaryReader reader(&fileStream);
		LBinaryWriter writer(&fileStream_);

		LFileVersionInfo* pInfo = LFileVersionInfo::GetVersionInfo(base_file);

		lstring strXml = LEncoding::GetUTF8()->GetString(reader.ReadBytes((int)fileStream.GetLength()));
		int nPre = strXml.IndexOf(L"<Version>");
		int nPost = strXml.IndexOf(L"</Version>") + 9;

		lstring strVersion = strXml.Substring(nPre, nPost - nPre + 1);

		lstring strVersion_ = L"<Version>" + pInfo->GetFileVersion().Replace(L", ", L".") + L"</Version>";
		lstring strXml_ = strXml.Replace(strVersion, strVersion_);

		writer.Write(LEncoding::GetUTF8()->GetBytes(strXml_));

		return 1;
	}
	catch (LException *pe)
	{
		delete pe;
	}

	return 0;
}
