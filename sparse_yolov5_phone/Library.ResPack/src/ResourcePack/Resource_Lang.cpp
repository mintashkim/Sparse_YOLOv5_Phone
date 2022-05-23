#include "StdAfx.h"
#include <ResourcePack/Resource_Lang.h>

bool Resource_Lang::loadTranslator( QTranslator& translator )
{
	QString strIDext;
	LANGID langId = ::GetUserDefaultLangID();

	if (MAKELANGID(LANG_KOREAN, SUBLANG_KOREAN) == langId)
		strIDext = QString("ko.qm");
// 	else if (MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN) == langId)
// 		strIDext = QString("ja.qm");
// 	else if (MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED) == langId)
// 		strIDext = QString("zh.qm");
 	else
		strIDext = QString("en.qm");

	QString qstr = ":/lang/abgres_" + strIDext;
	translator.load(qstr);

	return true;
}

bool Resource_Lang::loadCustomTranslator( QTranslator& translator )
{
	QString strIDext;
	LANGID langId = ::GetUserDefaultLangID();

	if (MAKELANGID(LANG_KOREAN, SUBLANG_KOREAN) == langId)
		strIDext = QString("ko.qm");
// 	else if (MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN) == langId)
// 		strIDext = QString("ja.qm");
// 	else if (MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED) == langId)
// 		strIDext = QString("zh.qm");
 	else
		strIDext = QString("en.qm");

	QString qstr = ":/custom/lang/abgres_" + strIDext;
	if (QFile::exists(qstr))
	{
		translator.load(qstr);
		return true;
	}
	else
	{
		return false;
	}
}
