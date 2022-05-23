#pragma once
#include <ResourcePack/ResourcePack.h>
#include <ResourcePack/Module_Common.h>
#include <ResourcePack/Module_Console.h>

class RESOURCEPACK_EXPORT Resource_Lang
{
public:
	static bool loadTranslator(QTranslator& translator);
	static bool loadCustomTranslator(QTranslator& translator);

	static QString GetText(Module_Common::TextType type)			{ return Module_Common::GetText(type); }
	static QString GetText(Module_Console::TextType type)			{ return Module_Console::GetText(type); }
};

#define GET_TEXT(x) Resource_Lang::GetText(x)
#define GET_ERRMSG(x, y) QString("%1 [%2]").arg(Resource_Lang::GetText(x)).arg(y)
