#pragma once
class Module_Console
{
	Q_DECLARE_TR_FUNCTIONS(Module_Console)
public:
	enum TextType
	{
		TEXT_MENU_LABEL_EXIT,

		MSGBOX_ALERT_NOT_SUPPORT_OS,
		MSGBOX_ALERT_REBOOT_REQUIRED,
		MSGBOX_ALERT_NOT_AVAILABLE_IN_SAFEMODE,
	};
	static QString GetText(TextType type);

	enum ImageType
	{
		ICON_TRAY,
		ICON_EXIT,

		PHONE_PRESENCE,
		PERSON_ABSENCE,
	};
	static QString GetImagePath(ImageType type);
};

