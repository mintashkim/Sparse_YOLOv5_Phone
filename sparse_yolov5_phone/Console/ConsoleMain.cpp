#include "stdafx.h"
#include "ConsoleMain.h"

#include "wpp.h"
#include "ConsoleMain.tmh"

ConsoleMain *g_pConsoleMain = NULL;

ConsoleMain::ConsoleMain(void)
{
	m_pEventWaitThread = new EventWaitThread;
	m_pDetector = null;

	m_pTray = new ConsoleTray(null);
}

ConsoleMain::~ConsoleMain()
{
	delete m_pEventWaitThread;

	if (m_pDetector)
		delete m_pDetector;

	delete m_pTray;

	foreach(BlockScreenWidget* pWidget, m_lstPhonePresenceWidgets)
	{
		pWidget->close();
		delete pWidget;
	}

	foreach(BlockScreenWidget* pWidget, m_lstPersonAbsenceWidgets)
	{
		pWidget->close();
		delete pWidget;
	}
}

void ConsoleMain::StartConsole(bool bShowCam)
{
	string strSID = SecurityAPI::GetProcessSID(::GetCurrentProcessId());
	WindowProfile::Initialize(strSID);

	string strModelPath = Application::GetStartupPath() + L"\\abg.onnx";

	string strCaptureDir = Application::GetUserAppDataPath() + L"\\cap";
	if (INVALID_FILE_ATTRIBUTES == ::GetFileAttributesW(strCaptureDir))
		::CreateDirectoryW(strCaptureDir, NULL);

	m_pDetector = new Detector(strModelPath, strCaptureDir, bShowCam);

	connect(m_pEventWaitThread, SIGNAL(ExitEvent()), this, SLOT(OnExitEvent()));
	connect(m_pDetector, SIGNAL(ShowBlockScreen(int, bool)), this, SLOT(OnShowBlockScreen(int, bool)));
	connect(m_pDetector, SIGNAL(ShowCam(cv::Mat)), this, SLOT(OnShowCam(cv::Mat)), Qt::BlockingQueuedConnection);

	m_pEventWaitThread->start();
	m_pDetector->start();
}

void ConsoleMain::ExitConsole()
{
	m_pDetector->stop();
	m_pEventWaitThread->stop();

	m_pDetector->wait(3000);
	m_pEventWaitThread->wait(3000);

	QTimer::singleShot(250, qApp, SLOT(quit()));
}

void ConsoleMain::OnExitEvent()
{
	ExitConsole();
}

void ConsoleMain::OnShowBlockScreen(int nClassId, bool fShow)
{
	if (0 == nClassId)
		ShowPhonePresence(fShow);
	else if (1 == nClassId)
		ShowPersonAbsence(fShow);
}

void ConsoleMain::OnShowCam(cv::Mat img)
{
	cv::imshow("bossGo", img);
}

void ConsoleMain::ShowPhonePresence(bool fShow)
{
	//_TRACE_I(Application, "ShowPhonePresence(%d)", fShow);

	if (fShow)
	{
		while (m_lstPhonePresenceWidgets.count() < QGuiApplication::screens().count())
		{
			BlockScreenWidget* pWidget = new BlockScreenWidget(1);
			m_lstPhonePresenceWidgets.append(pWidget);
		}

		for (int n = 0; n < QGuiApplication::screens().count(); n++)
		{
			m_lstPhonePresenceWidgets[n]->move(QGuiApplication::screens()[n]->geometry().topLeft());
			m_lstPhonePresenceWidgets[n]->showFullScreen();
		}
	}
	else
	{
		foreach(BlockScreenWidget* pWidget, m_lstPhonePresenceWidgets)
		{
			pWidget->hide();
		}
	}
}

void ConsoleMain::ShowPersonAbsence(bool fShow)
{
	//_TRACE_I(Application, "ShowPersonAbsence(%d)", fShow);

	if (fShow)
	{
		while (m_lstPersonAbsenceWidgets.count() < QGuiApplication::screens().count())
		{
			BlockScreenWidget* pWidget = new BlockScreenWidget(2);
			m_lstPersonAbsenceWidgets.append(pWidget);
		}

		for (int n = 0; n < QGuiApplication::screens().count(); n++)
		{
			m_lstPersonAbsenceWidgets[n]->move(QGuiApplication::screens()[n]->geometry().topLeft());
			m_lstPersonAbsenceWidgets[n]->showFullScreen();
		}
	}
	else
	{
		foreach(BlockScreenWidget* pWidget, m_lstPersonAbsenceWidgets)
		{
			pWidget->hide();
		}
	}
}
