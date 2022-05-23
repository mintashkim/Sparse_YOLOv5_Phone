#pragma once
#include <System/all.h>
#include <Library/ErrorCode.h>

namespace Library
{

class ProductException : public Exception
{
public:
	ProductException(ErrorCode code, string strMessage = L"", string strExtData = L"") : Exception(strMessage, null)
	{
		m_code = code;
		m_ExtData = strExtData;
		m_pObject = null;
	}

	ProductException(ErrorCode code, Object* pObject) : Exception(L"", null)
	{
		m_code = code;
		m_pObject = pObject;
	}


	ErrorCode GetErrorCode()
	{
		return m_code;
	}

	Object* GetObject()
	{
		return m_pObject;
	}

	string ToString()
	{
		return String::Format(L"[%d]\n%ws", m_code, (const wchar_t*)Exception::ToString());
	}

	string ToShortString()
	{
		return String::Format(L"[%d]\n%ws", m_code, (const wchar_t*)Exception::ToShortString());
	}

	string GetExtData()
	{
		return m_ExtData;
	}

private:
	ErrorCode m_code;
	string m_ExtData;
	Object *m_pObject;
};

class LibcurlException : public Exception
{
public:
	LibcurlException(string strMessage = L"", ProductException *pInnerException = NULL)
		: Exception(strMessage, pInnerException)
	{
	}
	virtual ~LibcurlException(void)
	{
	}
};

}