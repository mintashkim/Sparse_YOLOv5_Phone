#pragma once

#include "EventWaitThread.h"
#include "Detector.h"

#include "ConsoleTray.h"
#include "BlockScreenWidget.h"

class ConsoleMain : public QObject, public Object
{
	Q_OBJECT

public:
	ConsoleMain(void);
	~ConsoleMain();

	void StartConsole(bool bShowCam);
	void ExitConsole();

public slots:
	void OnExitEvent();
	void OnShowBlockScreen(int nClassId, bool fShow);
	void OnShowCam(cv::Mat img);

protected:
	void ShowPhonePresence(bool fShow);
	void ShowPersonAbsence(bool fShow);

private:
	EventWaitThread* m_pEventWaitThread;
	Detector* m_pDetector;

	ConsoleTray* m_pTray;
	QList<BlockScreenWidget*> m_lstPhonePresenceWidgets;
	QList<BlockScreenWidget*> m_lstPersonAbsenceWidgets;
};

extern ConsoleMain *g_pConsoleMain;