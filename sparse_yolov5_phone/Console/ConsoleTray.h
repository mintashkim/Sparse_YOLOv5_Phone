#pragma once
#include <qtimer.h>

class ConsoleTray : public QObject
{
	Q_OBJECT

public:
	ConsoleTray(QObject *parent);
	~ConsoleTray(void);

private slots:
	void OnTrayActivated(QSystemTrayIcon::ActivationReason reason);

	void OnExitAction();

	void OnTimerCallbackFunc();

private:
	QSystemTrayIcon *m_pTray;

	QMenu *m_pTrayMenu;

	QAction* m_pExitAction;

	QTimer *m_pTimer;
};