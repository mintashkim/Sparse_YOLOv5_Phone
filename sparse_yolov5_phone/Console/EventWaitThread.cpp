#include "stdafx.h"
#include "EventWaitThread.h"

#include <wpp.h>
#include "EventWaitThread.tmh"

EventWaitThread::EventWaitThread()
{
	m_pStopEvent = new EventWaitHandle(false, EEventResetMode::ManualReset);
}

void EventWaitThread::run()
{
	array<WaitHandle*> apEvents(2);
	apEvents[0] = m_pStopEvent;
	apEvents[1] = new EventWaitHandle(false, EEventResetMode::AutoReset, ABG_EVENT_CONSOLE_EXIT);

	_TRACE_I(Application, L"Start waiting for events!");
	
	while (true)
	{
		int n = WaitHandle::WaitAny(apEvents);
		if (n == 0) break;

		//_TRACE_I(Application, L"Event Raised, (Event Number = %d)", n);
		switch (n)
		{
		case 1:
			emit ExitEvent();
			break;
		}
	}

	for (int i = 1 ; i < apEvents.GetLength() ; i++)
		delete apEvents[i];
}

void EventWaitThread::stop()
{
	m_pStopEvent->Set();
}