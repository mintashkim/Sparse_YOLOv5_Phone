#include "StdAfx.h"
#include <Library/XmlObject2.h>

#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>

#ifdef _DEBUG
#pragma comment(lib, "xerces-c_3D.lib")
#else
#pragma comment(lib, "xerces-c_3.lib")
#endif

using namespace xercesc;

namespace Library
{


XmlParseException::XmlParseException( string strMessage /*= L""*/, Exception* pInnerException /*= null*/ ) :
	Exception(strMessage, pInnerException)
{
}

XmlParseException::~XmlParseException()
{
}

string _Trim( string str )
{
	array<wchar_t> awcStr = str.GetChars();
	int nFirst = 0 , nLast = awcStr.GetLength() - 1;
	
	for (int i = nFirst ; i < awcStr.GetLength() ; i++)
	{
		if (awcStr[i] == L' ' || awcStr[i] == L'\t' || awcStr[i] == L'\r' || awcStr[i] == L'\n')
			nFirst ++;
		else
			break;
	}

	for (int i = nLast ; i >= 0 ; i--)
	{
		if (awcStr[i] == L' ' || awcStr[i] == L'\t' || awcStr[i] == L'\r' || awcStr[i] == L'\n')
			nLast --;
		else
			break;
	}

	if (nFirst > nLast)
		return L"";

	return str.Substring(nFirst, nLast - nFirst + 1);
}

void _BuildObject(DOMElement* pDOM, XmlObject2* pObject)
{
	if (null == pDOM || null == pObject)
		throw new ArgumentNullException();

	DOMElement* pChildDOM = pDOM->getFirstElementChild();
	while (null != pChildDOM)
	{
		string strName = pChildDOM->getTagName();
		string strValue = (0 < pChildDOM->getChildElementCount()) ? L"" : pChildDOM->getTextContent();
		if (strValue.Contains(L'\n'))
			strValue = strValue.Replace(L"\n", L"");
		XmlObject2* pChildObject = new XmlObject2(strName, strValue);

		DOMNamedNodeMap *attrs = pChildDOM->getAttributes();
		int len = attrs->getLength();
		for (int i = 0 ; i < len ; i++)
			pChildObject->SetAttribute(attrs->item(i)->getNodeName(), attrs->item(i)->getNodeValue());

		pObject->AddChild(pChildObject);
		_BuildObject(pChildDOM, pChildObject);
		pChildDOM = pChildDOM->getNextElementSibling();
	}
}

XmlObject2::XmlObject2( array<byte> abXml )
{
	XercesDOMParser* pDOMParser = new XercesDOMParser();
	pDOMParser->setValidationScheme(XercesDOMParser::Val_Always);
	pDOMParser->setDoNamespaces(true);

	ErrorHandler* pHandler = (ErrorHandler*)new HandlerBase();
	pDOMParser->setErrorHandler(pHandler);
	
	try
	{
		MemBufInputSource source(abXml.GetBuffer(), abXml.GetLength(), L"policy");
		pDOMParser->parse(source);
	}
	catch(const XMLException& x)
	{
		delete pDOMParser;
		delete pHandler;
		throw new XmlParseException(L"pParser->parse() throws XMLException" + string(x.getMessage()));
	}
	catch(const DOMException& x)
	{
		delete pDOMParser;
		delete pHandler;
		throw new XmlParseException(L"pParser->parse() throws DOMException" + string(x.getMessage()));
	}
	catch(const SAXException& x)
	{
		delete pDOMParser;
		delete pHandler;
		throw new XmlParseException(L"pParser->parse() throws SAXException:" + string(x.getMessage()));
	}

	DOMDocument* pDOMDocument = pDOMParser->getDocument();
	DOMElement* pDOMElement = pDOMDocument->getDocumentElement();
	if (null != pDOMElement)
	{
		string strName = pDOMElement->getTagName();
		string strValue = (0 < pDOMElement->getChildElementCount()) ? L"" : pDOMElement->getTextContent();
		m_strName = strName;
		m_strValue = strValue;

		DOMNamedNodeMap *attrs = pDOMElement->getAttributes();
		int len = attrs->getLength();
		for (int i = 0 ; i < len ; i++)
			m_dicAttribute.Add(attrs->item(i)->getNodeName(), attrs->item(i)->getNodeValue());

		_BuildObject(pDOMElement, this);
	}

	delete pDOMParser;
	delete pHandler;
}

XmlObject2::XmlObject2( const string& strName, const string& strValue )
{
	m_strName = strName;
	m_strValue = strValue;
}

XmlObject2::XmlObject2(const string& strName, const string& strValue, const string& strAttrName, const string& strAttrValue)
{
	m_strName = strName;
	m_strValue = strValue;
	m_dicAttribute.Add(strAttrName, strAttrValue);
}

XmlObject2::XmlObject2( XmlObject2 *pObject )
{
	m_strName = pObject->m_strName;
	m_strValue = pObject->m_strValue;

	IEnumerator<string> *pe = pObject->m_dicAttribute.GetKeys()->GetEnumerator();
	while (pe->MoveNext())
	{
		string strKey = pe->GetCurrent();
		string strValue = pObject->m_dicAttribute.GetItem(strKey);
		m_dicAttribute.Add(strKey, strValue);
	}
	delete pe;

	for (int i = 0 ; i < pObject->m_lstChild.GetCount() ; i++)
		m_lstChild.Add(new XmlObject2(pObject->m_lstChild[i]));
}

XmlObject2::~XmlObject2(void)
{
	for (int n = 0; n < m_lstChild.GetCount(); n++)
		delete m_lstChild[n];
	m_lstChild.Clear();
}

void XmlObject2::Initialize()
{
	Exception* pException = NULL;
	_GetInitMutex()->WaitOne();
	try
	{
		XMLPlatformUtils::Initialize();
	}
	catch (const XMLException& x)
	{
		pException = new ApplicationException(L"XMLPlatformUtils::Initialize() throws XMLException = ");
	}
	catch (...)
	{
		pException = new ApplicationException(L"XMLPlatformUtils::Initialize() throws unknown exception...");
	}
	_GetInitMutex()->ReleaseMutex();

	if (NULL != pException)
		throw pException;
}

void XmlObject2::Finalize()
{
	_GetInitMutex()->WaitOne();
	XMLPlatformUtils::Terminate();
	_GetInitMutex()->ReleaseMutex();
}

string XmlObject2::GetName()
{
	return m_strName;
}

void XmlObject2::SetValue( const string& strValue )
{
	m_strValue = strValue;
}

string XmlObject2::GetValue()
{
	if (m_strValue.Contains(L"&lt;"))
		m_strValue = m_strValue.Replace(L"&lt;", L"<");

	if (m_strValue.Contains(L"&gt;"))
		m_strValue = m_strValue.Replace(L"&gt;", L">");

	if (m_strValue.Contains(L"&quot;"))
		m_strValue = m_strValue.Replace(L"&quot;", L'"');

	if (m_strValue.Contains(L"&amp;"))
		m_strValue = m_strValue.Replace(L"&amp;", L"&");

	return _Trim(m_strValue);
}

void XmlObject2::SetValue( const string& strName, const string& strValue, bool fIgnoreCase /*= false*/ )
{
	XmlObject2* pChild = GetChild(strName, fIgnoreCase);
	if (pChild == NULL)
		pChild = CreateChild(strName, fIgnoreCase);
	pChild->SetValue(strValue);
}

string XmlObject2::GetValue( const string& strName, bool fIgnoreCase /*= false*/ )
{
	string strValue = L"";
	XmlObject2* pChild = GetChild(strName, fIgnoreCase);
	if (pChild != NULL)
		strValue = pChild->GetValue();

	if (strValue.Contains(L"&lt;"))
		strValue = strValue.Replace(L"&lt;", L"<");

	if (strValue.Contains(L"&gt;"))
		strValue = strValue.Replace(L"&gt;", L">");

	if (strValue.Contains(L"&quot;"))
		strValue = strValue.Replace(L"&quot;", L'"');

	if (strValue.Contains(L"&amp;"))
		strValue = strValue.Replace(L"&amp;", L"&");

	return strValue;
}

void XmlObject2::SetAttribute( const string& strName, const string& strValue, bool fIgnoreCase /*= false*/ )
{
	if (strName.IsEmpty())
		throw new ArgumentException();

	string strHead, strTail;
	int index = strName.LastIndexOf(L'.');
	if (0 <= index)
	{
		strHead = strName.Substring(0, index);
		strTail = strName.Substring(index + 1);
	}
	else
	{
		strHead = L"";
		strTail = strName;
	}

	if (!strHead.IsEmpty())
	{
		XmlObject2* pChild = GetChild(strHead, fIgnoreCase);
		if (pChild != NULL)
			pChild->SetAttribute(strTail, strValue, fIgnoreCase);
	}
	else
	{
		if (m_dicAttribute.ContainsKey(strTail))
			m_dicAttribute.Remove(strTail);
		m_dicAttribute.Add(strTail, strValue);
	}
}

string XmlObject2::GetAttribute( const string& strName, bool fIgnoreCase /*= false*/ )
{
	if (strName.IsEmpty())
		throw new ArgumentException();

	string strHead, strTail;
	int index = strName.LastIndexOf(L'.');
	if (0 <= index)
	{
		strHead = strName.Substring(0, index);
		strTail = strName.Substring(index + 1);
	}
	else
	{
		strHead = L"";
		strTail = strName;
	}

	string strValue = L"";
	if (!strHead.IsEmpty())
	{
		XmlObject2* pChild = GetChild(strHead, fIgnoreCase);
		if (pChild != NULL)
			strValue = pChild->GetAttribute(strTail, fIgnoreCase);
	}
	else
	{
		IEnumerator<string>* pe = m_dicAttribute.GetKeys()->GetEnumerator();
		while (pe->MoveNext())
		{
			string strKey = pe->GetCurrent();
			if (strKey.Equals(strTail, fIgnoreCase))
			{
				strValue = _Trim(m_dicAttribute.GetItem(strKey));
				break;
			}
		}
		delete pe;
	}

	return strValue;
}

Dictionary<string, string>* XmlObject2::GetAttributes()
{
	return &m_dicAttribute;
}

int XmlObject2::GetChildCount()
{
	return m_lstChild.GetCount();
}

void XmlObject2::AddChild( XmlObject2* pObject )
{
	if (NULL == pObject)
		throw new ArgumentNullException();
	m_lstChild.Add(pObject);
}

XmlObject2* XmlObject2::GetChild( int index )
{
	XmlObject2* pChild = NULL;
	if (0 <= index && index < m_lstChild.GetCount())
		pChild = m_lstChild[index];
	return pChild;
}

XmlObject2* XmlObject2::GetChild( const string& strName, bool fIgnoreCase /*= false*/ )
{
	if (strName.IsEmpty())
		throw new ArgumentException();

	string strHead, strTail;
	int index = strName.IndexOf(L'.');
	if (0 <= index)
	{
		strHead = strName.Substring(0, index);
		strTail = strName.Substring(index + 1);
	}
	else
	{
		strHead = strName;
		strTail = L"";
	}

	XmlObject2* pChild = NULL;
	for (int i = 0 ; i < m_lstChild.GetCount() ; i++)
	{
		if (strHead.Equals(m_lstChild[i]->GetName(), fIgnoreCase))
		{
			pChild = m_lstChild[i];
			break;
		}
	}

	if (pChild != NULL && !strTail.IsEmpty())
		pChild = pChild->GetChild(strTail, fIgnoreCase);

	return pChild;
}

XmlObject2* XmlObject2::RemoveChild( const string& strName, bool fIgnoreCase /*= false*/ )
{
	if (strName.IsEmpty())
		throw new ArgumentException();

	string strHead, strTail;
	int index = strName.IndexOf(L'.');
	if (0 <= index)
	{
		strHead = strName.Substring(0, index);
		strTail = strName.Substring(index + 1);
	}
	else
	{
		strHead = strName;
		strTail = L"";
	}

	XmlObject2* pChild = NULL;
	for (int i = 0 ; i < m_lstChild.GetCount() ; i++)
	{
		if (strHead.Equals(m_lstChild[i]->GetName(), fIgnoreCase))
		{
			pChild = m_lstChild[i];
			break;
		}
	}

	if (pChild != NULL && !strTail.IsEmpty())
		pChild = pChild->RemoveChild(strTail, fIgnoreCase);
	else if (pChild != NULL && strTail.IsEmpty())
		m_lstChild.Remove(pChild);

	return pChild;
}

XmlObject2* XmlObject2::RemoveChild(const string& strName, const string& strAttrName, const string& strAttrValue, bool fIgnoreCase /* = false */)
{
	if (strName.IsEmpty())
		throw new ArgumentException();
	if (strAttrName.IsEmpty())
		throw new ArgumentException();
	if (strAttrValue.IsEmpty())
		throw new ArgumentException();

	string strHead, strTail;
	int index = strName.IndexOf(L'.');
	if (0 <= index)
	{
		strHead = strName.Substring(0, index);
		strTail = strName.Substring(index + 1);
	}
	else
	{
		strHead = strName;
		strTail = L"";
	}

	XmlObject2* pChild = NULL;
	for (int i = 0; i < m_lstChild.GetCount(); i++)
	{
		if (strHead.Equals(m_lstChild[i]->GetName(), fIgnoreCase))
		{
			string strTemp;
			XmlObject2* pTemp = m_lstChild[i];
			
			strTemp = pTemp->GetAttribute(strAttrName);

			if (!strTemp.IsEmpty() && strTemp.Equals(strAttrValue))
			{
				pChild = m_lstChild[i];
				break;
			}
		}
	}

	if (pChild != NULL && !strTail.IsEmpty())
		pChild = pChild->RemoveChild(strTail, fIgnoreCase);
	else if (pChild != NULL && strTail.IsEmpty())
		m_lstChild.Remove(pChild);

	return pChild;
}

XmlObject2* XmlObject2::CreateChild( const string& strName, bool fIgnoreCase /*= false*/ )
{
	if (strName.IsEmpty())
		throw new ArgumentException();

	string strHead, strTail;
	int index = strName.IndexOf(L'.');
	if (0 <= index)
	{
		strHead = strName.Substring(0, index);
		strTail = strName.Substring(index + 1);
	}
	else
	{
		strHead = strName;
		strTail = L"";
	}

	XmlObject2* pChild = NULL;
	for (int i = 0 ; i < m_lstChild.GetCount() ; i++)
	{
		if (strHead.Equals(m_lstChild[i]->GetName(), fIgnoreCase))
		{
			pChild = m_lstChild[i];
			break;
		}
	}

	if (pChild == NULL)
	{
		pChild = new XmlObject2(strHead, L"");
		m_lstChild.Add(pChild);
	}

	if (!strTail.IsEmpty())
		pChild = pChild->CreateChild(strTail, fIgnoreCase);

	return pChild;
}

string XmlObject2::ToString()
{
	string strAttribute = L"";
	IEnumerator<string>* pe = m_dicAttribute.GetKeys()->GetEnumerator();
	while (pe->MoveNext())
	{
		string name = pe->GetCurrent();
		string value = m_dicAttribute.GetItem(name);
		strAttribute += (L" " + name + L"=\"" + value + L"\"");
	}
	delete pe;

	// Tag
	string strResult = L"<" + m_strName + strAttribute + L">";

	// Value
	strResult += m_strValue;

	// Child
	for (int i = 0 ; i < m_lstChild.GetCount() ; i++)
	{
		if (m_lstChild[i] != null)
			strResult += m_lstChild[i]->ToString();
	}

	// TagEnd
	strResult += (L"</" + m_strName + L">");

	return strResult;
}

string XmlObject2::ToString( int margin )
{
	return _ToString(0, margin);
}

string XmlObject2::_ToString( int depth, int margin )
{
	string strAttribute = L"";
	IEnumerator<string>* pe = m_dicAttribute.GetKeys()->GetEnumerator();
	while (pe->MoveNext())
	{
		string name = pe->GetCurrent();
		string value = m_dicAttribute.GetItem(name);
		strAttribute += (L" " + name + L"=\"" + value + L"\"");
	}
	delete pe;

	string strResult;
	if (m_lstChild.GetCount() == 0)
	{
		for (int i = 0 ; i < depth * margin ; i++)
			strResult += L" ";

		strResult += L"<" + m_strName + strAttribute + L">" + m_strValue + L"</" + m_strName + L">";
	}
	else
	{
		for (int i = 0 ; i < depth * margin ; i++)
			strResult += L" ";
		strResult += L"<" + m_strName + strAttribute + L">" + m_strValue + L"\n";

		for (int i = 0 ; i < m_lstChild.GetCount() ; i++)
		{
			if (m_lstChild[i] == NULL) continue;
			
			strResult += m_lstChild[i]->_ToString(depth + 1, margin);
			strResult += L"\n";
		}

		for (int i = 0 ; i < depth * margin ; i++)
			strResult += L" ";
		strResult += L"</" + m_strName + L">";
	}

	return strResult;
}

Mutex* XmlObject2::_GetInitMutex()
{
	static Mutex mutex;
	return &mutex;
}

}