#pragma once
#include <System/all.h>
#include <System/Collections/Generic/all.h>
#include <System/Threading/all.h>

namespace Library
{

class XmlParseException : public Exception
{
public:
	XmlParseException(string strMessage = L"", Exception* pInnerException = null);
	virtual ~XmlParseException();
};

class XmlObject2 : public Object
{
public:
	XmlObject2(array<byte> abXml);
	XmlObject2(const string& strName, const string& strValue);
	XmlObject2(const string& strName, const string& strValue, const string& strAttrName, const string& strAttrValue);
	XmlObject2(XmlObject2 *pObject);
	virtual ~XmlObject2(void);

	static void Initialize();
	static void Finalize();

	string GetName();

	void SetValue(const string& strValue);
	string GetValue();

	void SetValue(const string& strName, const string& strValue, bool fIgnoreCase = false);
	string GetValue(const string& strName, bool fIgnoreCase = false);

	void SetAttribute(const string& strName, const string& strValue, bool fIgnoreCase = false);
	string GetAttribute(const string& strName, bool fIgnoreCase = false);
	Dictionary<string, string>* GetAttributes();

	int GetChildCount();
	void AddChild(XmlObject2* pObject);
	XmlObject2* GetChild(int index);
	XmlObject2* GetChild(const string& strName, bool fIgnoreCase = false);
	XmlObject2* RemoveChild(const string& strName, bool fIgnoreCase = false);
	XmlObject2* RemoveChild(const string& strName, const string& strAttrName, const string& strAttrValue, bool fIgnoreCase = false);

	string ToString();
	string ToString(int margin);

private:
	XmlObject2* CreateChild(const string& strName, bool fIgnoreCase = false);
	string _ToString(int depth, int margin);
	static Mutex* _GetInitMutex();

private:
	string m_strName;
	string m_strValue;
	Dictionary<string, string> m_dicAttribute;
	List<XmlObject2*> m_lstChild;

};

}