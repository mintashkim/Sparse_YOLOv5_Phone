#include "StdAfx.h"
#include <ResourcePack/Module_Common.h>

QString Module_Common::GetText(TextType type)
{
	switch (type)
	{
	case PRODUCT_NAME:									return tr("COMMON_PRODUCT_NAME");

	case MSGBOX_BUTTON_ACCEPT:							return tr("COMMON_MSGBOX_BUTTON_ACCEPT");
	case MSGBOX_BUTTON_CANCEL:							return tr("COMMON_MSGBOX_BUTTON_CANCEL");
	}
	return QString("");
}

QString Module_Common::GetImagePath(ImageType type)
{
	switch (type)
	{
	case ICON_WINDOW:								return "images/Common/ico_window.ico";
	}
	return QString("");
}

QString Module_Common::GetStyleSheet(StyleSheetType type)
{
	switch (type)
	{
	case QSS_MESSAGEBOX:                          return "images/Common/MsgBox.qss";
	}
	return QString("");
}
