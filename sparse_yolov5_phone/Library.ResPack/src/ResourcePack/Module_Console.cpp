#include "StdAfx.h"
#include <ResourcePack/Module_Console.h>

QString Module_Console::GetText(TextType type)
{
	switch (type)
	{
	case TEXT_MENU_LABEL_EXIT:							return tr("CONSOLE_TEXT_MENU_LABEL_EXIT");

	case MSGBOX_ALERT_REBOOT_REQUIRED:					return tr("CONSOLE_MSGBOX_ALERT_REBOOT_REQUIRED");
	case MSGBOX_ALERT_NOT_SUPPORT_OS:					return tr("CONSOLE_MSGBOX_ALERT_NOT_SUPPORT_OS");
	case MSGBOX_ALERT_NOT_AVAILABLE_IN_SAFEMODE:		return tr("CONSOLE_MSGBOX_ALERT_NOT_AVAILABLE_IN_SAFEMODE");
	}
	return QString("");
}

QString Module_Console::GetImagePath(ImageType type)
{
	switch (type)
	{
	case ICON_TRAY:											return "images/Console/ico_Tray.png";
	case ICON_EXIT:											return "images/Console/ico_Exit.png";

	case PHONE_PRESENCE:									return "images/Console/PhonePresence.png";
	case PERSON_ABSENCE:									return "images/Console/PersonAbsence.png";
	}
	return QString("");
}