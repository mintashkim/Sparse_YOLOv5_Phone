#pragma once
#include <System/all.h>
#include <Microsoft/Win32/all.h>

namespace Library
{
class WindowProfile : public Object
{
public:
	static void Initialize(string strSID) {
		_GetInstance()->_Initialize(strSID);
	}
	static string GetSID() {
		return _GetInstance()->_GetSID();
	}
	static RegistryKey* GetUserKey() {
		return _GetInstance()->_GetUserKey();
	}
	static RegistryKey* GetUserAppDataKey() {
		return _GetInstance()->_GetUserAppDataKey();
	}
	static HANDLE GetUserToken() {
		return _GetInstance()->_GetUserToken();
	}
	static string GetUserProfilePath() {
		return _GetInstance()->_GetUserProfilePath();
	}

private:
	WindowProfile(void);
	virtual ~WindowProfile(void);

	static WindowProfile* _GetInstance();

	void _Initialize(string strSID);

	string _GetSID();
	RegistryKey* _GetUserKey();
	RegistryKey* _GetUserAppDataKey();
	HANDLE _GetUserToken();
	string _GetUserProfilePath();

private:
	string m_strSID;
	RegistryKey *m_pUserKey;
	RegistryKey *m_pUserAppDataKey;
	HANDLE m_hUserToken;
	string m_strUserProfilePath;
};

}

