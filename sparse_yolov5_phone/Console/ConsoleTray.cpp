#include "StdAfx.h"
#include "ConsoleTray.h"
#include "ConsoleMain.h"

#include "wpp.h"
#include "ConsoleTray.tmh"

ConsoleTray::ConsoleTray(QObject *parent) : QObject(parent)
{
	m_pTrayMenu = new QMenu();

	m_pExitAction = m_pTrayMenu->addAction(
		QIcon(QPixmap(Resource_Image::GetImagePath(Module_Console::ICON_EXIT))),
		Resource_Lang::GetText(Module_Console::TEXT_MENU_LABEL_EXIT));

	m_pTray = new QSystemTrayIcon(this);
	m_pTray->setIcon(QIcon(QPixmap(Resource_Image::GetImagePath(Module_Console::ICON_TRAY))));
	m_pTray->setToolTip(GET_TEXT(Module_Common::PRODUCT_NAME));

	connect(m_pTray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(OnTrayActivated(QSystemTrayIcon::ActivationReason)));
	connect(m_pExitAction, SIGNAL(triggered()), this, SLOT(OnExitAction()));

	m_pTimer = new QTimer(this);
	connect(m_pTimer, SIGNAL(Timeout()), this, SLOT(OnTimerCallbackFunc()));
	m_pTimer->start(1000);

	m_pTray->show();
}

ConsoleTray::~ConsoleTray(void)
{
	if (NULL != m_pTimer)
	{
		m_pTimer->stop();
		delete m_pTimer;
		m_pTimer = NULL;
	}
}

void ConsoleTray::OnTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
	_TRACE_I(Application, L"ConsoleTray::OnTrayActivated() - reason=%d", reason);

	switch (reason)
	{
	case QSystemTrayIcon::Context:
	case QSystemTrayIcon::MiddleClick:
		// #1737
		m_pTrayMenu->exec(QCursor::pos());
		break;
	case QSystemTrayIcon::DoubleClick:
	case QSystemTrayIcon::Trigger:
        break;
	default:
		break;
	}
}

void ConsoleTray::OnExitAction()
{
	AppAPI::RaiseEventExisting(ABG_EVENT_CONSOLE_EXIT);
}

void ConsoleTray::OnTimerCallbackFunc()
{
	try{
		if (NULL != m_pTray)
		{
			if (!m_pTray->isVisible())
			{
				_TRACE_I(Application, L"SHOW TRAY ICON AGAIN");
				m_pTray->setVisible(true);
			}
		}
	}
	catch (Exception *pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}

}