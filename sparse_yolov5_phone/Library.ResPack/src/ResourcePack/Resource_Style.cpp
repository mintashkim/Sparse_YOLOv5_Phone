#include "stdafx.h"
#include <ResourcePack/Resource_Style.h>

QVariant Resource_Style::GetStyle(const QString& strGroup, const QString& strValueName)
{
	QString strPath = ":/default/UserInterfaceStyle.ini";
	if (QFile::exists(":/custom/UserInterfaceStyle.ini"))
		strPath = ":/custom/UserInterfaceStyle.ini";
	else if (QFile::exists(":/subres/UserInterfaceStyle.ini"))
		strPath = ":/subres/UserInterfaceStyle.ini";

	QVariant value;
	if (QFile::exists("./skins/customstyle.ini"))
	{
		QSettings settings("./skins/customstyle.ini", QSettings::IniFormat);
		settings.beginGroup(strGroup);
		if (settings.contains(strValueName))
			value = settings.value(strValueName);
		settings.endGroup();
	}

	if (value.isNull() && QFile::exists(strPath))
	{
		QSettings settings(strPath, QSettings::IniFormat);
		settings.beginGroup(strGroup);
		if (settings.contains(strValueName))
			value = settings.value(strValueName);
		settings.endGroup();
	}

	return value;
}

QString Resource_Style::GetTransparentButtonStyle(QString strNormal, QString strHover /*= QString("")*/, QString strPress /*= QString("")*/, QString strDisable /*= QString("")*/)
{
	QString strStyleSheet;
	strStyleSheet.append(QString::fromUtf8("QPushButton{\n"));
	strStyleSheet.append(QString::fromUtf8("	background:transparent;\n"));
	strStyleSheet.append(QString::fromUtf8("	background-image: url(%1); \n").arg(strNormal));	
	strStyleSheet.append(QString::fromUtf8(" }\n"));
	if (!strHover.isEmpty())
	{
		strStyleSheet.append(QString::fromUtf8("QPushButton:hover {\n"));
		strStyleSheet.append(QString::fromUtf8("	background:transparent;\n"));
		strStyleSheet.append(QString::fromUtf8("	background-image: url(%1); \n").arg(strHover));
		strStyleSheet.append(QString::fromUtf8(" }\n"));
	}
	if (!strPress.isEmpty())
	{
		strStyleSheet.append(QString::fromUtf8("QPushButton:pressed {\n"));
		strStyleSheet.append(QString::fromUtf8("	background:transparent;\n"));
		strStyleSheet.append(QString::fromUtf8("	background-image: url(%1); \n").arg(strPress));
		strStyleSheet.append(QString::fromUtf8(" }\n"));
	}
	if (!strDisable.isEmpty())
	{
		strStyleSheet.append(QString::fromUtf8("QPushButton:disabled {\n"));
		strStyleSheet.append(QString::fromUtf8("	background:transparent;\n"));
		strStyleSheet.append(QString::fromUtf8("	background-image: url(%1); \n").arg(strDisable));
		strStyleSheet.append(QString::fromUtf8(" }"));
	}
	return strStyleSheet;
}

QString Resource_Style::GetStyleSheetFromFile(const QString& styleSheetPath)
{
	QString styleSheet("");
	QFile* pQss_file = null;

	QString qstrPath_1 = ":/" + styleSheetPath;
	QString qstrPath_2 = ":/custom/" + styleSheetPath;
	if (QFile::exists(qstrPath_2))
		pQss_file = new QFile(qstrPath_2);
	else
		pQss_file = new QFile(qstrPath_1);

	if (pQss_file->open(QIODevice::ReadOnly))
	{
		try {
			QTextStream read_file(pQss_file);
			read_file.setCodec("UTF-8");
			styleSheet = read_file.readAll();
		}
		catch (QException& pe)
		{
			Q_UNUSED(pe);
		}
		pQss_file->close();
	}
	if (pQss_file)
	{
		delete pQss_file;
	}
	return styleSheet;
}

QString Resource_Style::GetQLabelTextColorStyle(const QColor& color)
{
	//QLabel {
	//color: rgba(30, 150, 100, 0.5);
	//}
	return QString("QLabel { color: rgba(%1,%2,%3,%4); }").arg(color.red(), color.green(), color.blue(), color.alphaF());
}
