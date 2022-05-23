#pragma once

class EventWaitThread : public QThread
{
	Q_OBJECT
public:
	EventWaitThread(void);

	void run();
	void stop();

signals:
	void ExitEvent();

private:
	EventWaitHandle *m_pStopEvent;
};