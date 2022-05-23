#include "stdafx.h"
#include "ConsoleMain.h"

#include "wpp.h"
#include "main.tmh"

void _OnThreadException(Object* pSender, EventArgs* pEventArgs, Object* pObject)
{
	ThreadExceptionEventArgs* pArgs = dynamic_cast<ThreadExceptionEventArgs*>(pEventArgs);
    _TRACE_EX(pArgs->GetException());
    qApp->quit();
}

int main(int argc, char *argv[])
{
	int nReturn = 0;

	WPP_INIT_TRACING(NULL);
	::PoInitialize();

	array<string> astr = Environment::GetCommandLineArgs();
	if (2 == astr.GetLength() && astr[1].Equals(L"install", true))
	{
	}
	else if (2 == astr.GetLength() && astr[1].Equals(L"update", true))
	{
	}
	else if (2 == astr.GetLength() && astr[1].Equals(L"uninstall", true))
	{
	}
	else if (2 == astr.GetLength() && astr[1].Equals(L"stop", true))
	{
		AppAPI::RaiseEventExisting(ABG_EVENT_CONSOLE_EXIT);
	}
	else if (0 == ProcessAPI::GetProcessIDByName(L"explorer.exe"))
	{
		_TRACE_I(Application, L"explorer not found, exit");
	}
	else
	{
		XmlObject2::Initialize();
		//MCCAPIClient::Initialize();

		Application::ThreadException += _OnThreadException;
		Application::SetAppId(APPLICATION_ID);

		_TRACE_I(Application, L"Main Starting...");

		QApplication app(argc, argv);
		QTranslator translator_1;
		QTranslator translator_2;

		ResourcePack::Initialize();
		if (Resource_Lang::loadTranslator(translator_1))
			app.installTranslator(&translator_1);
		if (Resource_Lang::loadCustomTranslator(translator_2))
			app.installTranslator(&translator_2);
		app.setQuitOnLastWindowClosed(false);

		QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

		Mutex mutex(false, ABG_MUTEX_CONSOLE);
		if (!mutex.WaitOne(0))
		{
		}
		else if (!AppAPI::IsSupportedWindows())
		{
			string strCaption = ConvertFromQString(Resource_Lang::GetText(Module_Common::PRODUCT_NAME));
			string strText = ConvertFromQString(Resource_Lang::GetText(Module_Console::MSGBOX_ALERT_NOT_SUPPORT_OS));
			::MessageBoxW(NULL, strText, strCaption, MB_OK | MB_ICONWARNING);
		}
		else if (AppAPI::ContainsPendingFileRenameOperations(APPLICATION_NAME))
		{
			string strCaption = ConvertFromQString(Resource_Lang::GetText(Module_Common::PRODUCT_NAME));
			string strText = ConvertFromQString(Resource_Lang::GetText(Module_Console::MSGBOX_ALERT_REBOOT_REQUIRED));
			::MessageBoxW(NULL, strText, strCaption, MB_OK | MB_ICONWARNING);
		}
		else if (ManagementAPI::IsSafeBootMode())
		{
			// #1833
			string strCaption = ConvertFromQString(Resource_Lang::GetText(Module_Common::PRODUCT_NAME));
			string strText = ConvertFromQString(Resource_Lang::GetText(Module_Console::MSGBOX_ALERT_NOT_AVAILABLE_IN_SAFEMODE));
			::MessageBoxW(NULL, strText, strCaption, MB_OK | MB_ICONWARNING);
		}
		else
		{
			bool bShowCam = false;
			if (2 == astr.GetLength() && astr[1].Equals(L"cam", true))
				bShowCam = true;

			ConsoleMain VDMain;
			g_pConsoleMain = &VDMain;
			g_pConsoleMain->StartConsole(bShowCam);
			nReturn = app.exec();
		}

		//MCCAPIClient::Finalize();
		XmlObject2::Finalize();
	}

	::PoFinalize();
	WPP_CLEANUP();

	return nReturn;
}
