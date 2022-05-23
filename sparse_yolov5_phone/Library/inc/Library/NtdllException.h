#pragma once

#include <Library/ntdll.h>

class NtdllException : public System::ExternalException
{
public:
	NtdllException(NTSTATUS status, string strCustomMessage = L"", Exception* pInnerException = null);
	virtual ~NtdllException(void);

	NTSTATUS GetStatus();
	string GetMessage();
	string ToString();

private:
	NTSTATUS m_status;
	string m_strCustomMessage;
};
