#include "stdafx.h"
#include <Library/NtdllException.h>

NtdllException::NtdllException(NTSTATUS status, string strCustomMessage /* = L */, Exception* pInnerException /* = null */) :
	ExternalException(L"", pInnerException)
{
	m_status = status;
	m_strCustomMessage = strCustomMessage;
}

NtdllException::~NtdllException(void)
{
}

NTSTATUS NtdllException::GetStatus()
{
	return m_status;
}

string NtdllException::GetMessage()
{
	if (m_strMessage.IsEmpty())
	{
		StringBuilder sb;

		if (!m_strCustomMessage.IsEmpty())
		{
			sb.Append(m_strCustomMessage);
			sb.Append(L"(");
		}

		sb.Append(String::Format(L"[0x%08X]", m_status));

		HMODULE hModule = ::LoadLibraryW(L"NTDLL.DLL");
		if (NULL != hModule)
		{
			LPWSTR wsz = NULL;
			DWORD dwLength = ::FormatMessageW(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_FROM_HMODULE,
				hModule,
				m_status,
				0,
				(LPWSTR)&wsz,
				0,
				NULL
				);
			if (NULL != wsz)
			{
				// 2줄을 1줄로 바꾼다.
				for (int n = dwLength - 1; n >= 0; n--)
				{
					if (L'\n' == wsz[n])
					{
						wsz[n] = L'\0';
						break;
					}
					else if (L' ' != wsz[n])
					{
						break;
					}
				}
				sb.Append(wsz);
				::LocalFree(wsz);
			}

			::FreeLibrary(hModule);
		}

		if (!m_strMessage.IsEmpty())
			sb.Append(L")");

		m_strMessage = sb.ToString();
	}

	return m_strMessage;
}

string NtdllException::ToString()
{
	GetMessage();

	return __super::ToString();
}