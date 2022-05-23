#pragma once
class Module_Common
{
	Q_DECLARE_TR_FUNCTIONS(Module_Common)
public:
	enum TextType
	{
		PRODUCT_NAME,

		MSGBOX_BUTTON_ACCEPT,
		MSGBOX_BUTTON_CANCEL,
	};
	static QString GetText(TextType type);

	enum ImageType
	{
		ICON_WINDOW,
	};
	static QString GetImagePath(ImageType type);

	enum StyleSheetType
	{
		QSS_MESSAGEBOX,
	};
	static QString GetStyleSheet(StyleSheetType type);

};

