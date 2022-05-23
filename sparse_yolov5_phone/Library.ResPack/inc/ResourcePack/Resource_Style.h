#pragma once
#include <ResourcePack/ResourcePack.h>
#include <ResourcePack/Module_Common.h>

class RESOURCEPACK_EXPORT Resource_Style
{
public:
	static QVariant GetStyle(const QString& strGroup, const QString& strValueName);

	static QString GetTransparentButtonStyle(QString strNormal, QString strHover = QString(""), QString strPress = QString(""), QString strDisable = QString(""));

	static QString GetStyleSheetFromFile(const QString& styleSheetPath);
	static QString GetQLabelTextColorStyle(const QColor& color);

	static QString GetStyleSheet(Module_Common::StyleSheetType type)		{ return GetStyleSheetFromFile(Module_Common::GetStyleSheet(type)); }
};

#define GET_STYLESHEET(x) Resource_Style::GetStyleSheet(x)