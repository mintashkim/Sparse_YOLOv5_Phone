#include "StdAfx.h"
#include <Library/API/SecurityAPI.h>
#include <Microsoft/Win32/all.h>
#include <TlHelp32.h>

#include <WinCrypt.h>
#include <WinTrust.h>
#include <SoftPub.h>
#include <wtsapi32.h>
#include <AclAPI.h>
#include <UserEnv.h>

#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "wintrust.lib")
#pragma comment(lib, "userenv.lib")

#include "wpp.h"
#include "SecurityAPI.tmh"

namespace Library
{
namespace API
{

bool SecurityAPI::SetLowFileSecurity(LPCWSTR lpPath)
{
	bool fHandled = true;
 	SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;  
 	SID_IDENTIFIER_AUTHORITY ntauth = SECURITY_WORLD_SID_AUTHORITY;
 	SID_IDENTIFIER_AUTHORITY ntcreator = {0,0,0,0,0,3}; //SECURITY_CREATOR_SID_AUTHORITY
 	PSID pInteractiveSid = NULL;   
 	PSID pAdministratorsSid = NULL;   
 	PSID pLocalSystemSid = NULL;   
 	PSID pAuthedUserSid = NULL;
 	PSID pRestrictedSid = NULL;
 	PSID pEveryOneSid = NULL;
 	PSID pCreatorOwnerSid = NULL;
 	PSID pUserSid = NULL;
	PSID pApplicationPackage = NULL;
	SECURITY_DESCRIPTOR sd;
 	PACL pDacl = NULL;   
 
 	try
 	{
		pInteractiveSid = SecurityAPI::_GetSpecificUserSID(SECURITY_INTERACTIVE_RID);
		pAdministratorsSid = SecurityAPI::_GetAliasAdministratorsSID();
		pLocalSystemSid = SecurityAPI::_GetLocalSystemSID();
		pAuthedUserSid = SecurityAPI::_GetAuthenticatedUsersSID();
		pEveryOneSid = SecurityAPI::_GetEveryoneSID();
		pCreatorOwnerSid = SecurityAPI::_GetCreatorOwnerSID();
		pRestrictedSid = SecurityAPI::_GetSpecificUserSID(SECURITY_RESTRICTED_CODE_RID);
		pUserSid = SecurityAPI::_GetAliasUsersSID();
		if (EOSName::Win8 <= Environment::GetOSName())
			pApplicationPackage = SecurityAPI::_GetApplicationPackageSID();

 		// compute size of new acl   
 		DWORD dwAclSize = sizeof(ACL) + 8 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)) + GetLengthSid(pInteractiveSid) + GetLengthSid(pAdministratorsSid) + GetLengthSid(pEveryOneSid) + GetLengthSid(pRestrictedSid) + GetLengthSid(pLocalSystemSid) + GetLengthSid(pCreatorOwnerSid) + GetLengthSid(pAuthedUserSid) + GetLengthSid(pUserSid);   
		if (NULL != pApplicationPackage)
			dwAclSize += 1 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)) + GetLengthSid(pApplicationPackage);

 		// allocate storage for Acl   
 		pDacl = (PACL)::HeapAlloc(GetProcessHeap(), 0, dwAclSize);   
 		if (NULL == pDacl)   
 			throw new Win32Exception(::GetLastError());
 
 		if (!::InitializeAcl(pDacl, dwAclSize, ACL_REVISION))   
 			throw new Win32Exception(::GetLastError());
 
		SecurityAPI::_AddAceToDacl(pDacl, pInteractiveSid, TRUE);
 		SecurityAPI::_AddAceToDacl(pDacl, pAdministratorsSid, TRUE);
 		SecurityAPI::_AddAceToDacl(pDacl, pAuthedUserSid, TRUE);
 		SecurityAPI::_AddAceToDacl(pDacl, pCreatorOwnerSid, TRUE);
 		SecurityAPI::_AddAceToDacl(pDacl, pEveryOneSid, TRUE);
 		SecurityAPI::_AddAceToDacl(pDacl, pLocalSystemSid, TRUE);
 		SecurityAPI::_AddAceToDacl(pDacl, pRestrictedSid, TRUE);
 		SecurityAPI::_AddAceToDacl(pDacl, pUserSid, TRUE);
		if (NULL != pApplicationPackage)
			SecurityAPI::_AddAceToDacl(pDacl, pApplicationPackage, TRUE);

 		if (!::InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION))   
 			throw new Win32Exception(::GetLastError());
 
 		if (!::SetSecurityDescriptorDacl(&sd, TRUE, pDacl, FALSE))   
 			throw new Win32Exception(::GetLastError());
 
 		if (!::SetFileSecurity(lpPath, (SECURITY_INFORMATION)DACL_SECURITY_INFORMATION, &sd))
 			throw new Win32Exception(::GetLastError());
 	}
 	catch(Exception* px)
 	{
// 		_TRACE_EX(px);
 		delete px;
		fHandled = false;
 	}
	catch (...)
	{
//		_TRACE_EX_UNK();
		fHandled = false;
	}
 
 	// free allocated resources   
 	if (NULL != pDacl)   
 		::HeapFree(::GetProcessHeap(), 0, pDacl);   
 	if (NULL != pInteractiveSid)   
 		::FreeSid(pInteractiveSid);   
 	if (NULL != pAdministratorsSid)   
 		::FreeSid(pAdministratorsSid);   
 	if (NULL != pAuthedUserSid)   
 		::FreeSid(pAuthedUserSid);   
 	if (NULL != pCreatorOwnerSid)   
 		::FreeSid(pCreatorOwnerSid);   
 	if (NULL != pEveryOneSid)   
 		::FreeSid(pEveryOneSid);   
 	if (NULL != pLocalSystemSid)   
 		::FreeSid(pLocalSystemSid);   
 	if (NULL != pRestrictedSid)   
 		::FreeSid(pRestrictedSid);   
 	if (NULL != pUserSid)   
 		::FreeSid(pUserSid);  
	if (NULL != pApplicationPackage)
		::FreeSid(pApplicationPackage);

	return fHandled;
}

bool SecurityAPI::SetLowIntegrityFileLevel(LPCWSTR lpPath)
{
	bool fHandled = true;

	OperatingSystem *pOs = Environment::GetOSVersion();
	if (pOs->GetVersion() < Version(6, 0))
		return false;

	PSECURITY_DESCRIPTOR pSD = SecurityAPI::GetLowIntegritySecurityDescriptor();

	try 
	{
		if (!::SetFileSecurity(
			lpPath,   
			(SECURITY_INFORMATION)LABEL_SECURITY_INFORMATION | UNPROTECTED_SACL_SECURITY_INFORMATION,   
			pSD ))
		{
			DWORD dw = ::GetLastError();
			if (ERROR_NO_TOKEN != dw)
				throw new Win32Exception(::GetLastError());
		}
	}
	catch (Exception* px)
	{
//		_TRACE_EX(px);
		delete px;
		fHandled = false;
	}
	catch (...)
	{
//		_TRACE_EX_UNK();
		fHandled = false;
	}
		 
	if (NULL != pSD)
		::LocalFree(pSD);

	return fHandled;
}

PSECURITY_DESCRIPTOR SecurityAPI::GetLowIntegritySecurityDescriptor()
{
	OperatingSystem *pOs = Environment::GetOSVersion();
 	if (pOs->GetVersion() < Version(6, 0))
 		return 0;
 
 	PSECURITY_DESCRIPTOR pSD = NULL;
 	PACL pSacl = NULL;
 	BOOL fSaclPresent = FALSE;
 	BOOL fSaclDefaulted = FALSE;
 
 	try 
 	{
 		if (!::ConvertStringSecurityDescriptorToSecurityDescriptorW(
 			L"S:(ML;CI;NW;;;LW)",
 			SDDL_REVISION_1,
 			&pSD,
 			NULL))
 			throw new Win32Exception(::GetLastError());
 
 
 		if (!::GetSecurityDescriptorSacl(pSD, &fSaclPresent, &pSacl, &fSaclDefaulted))
 			throw new Win32Exception(::GetLastError());
 	}
 	catch (Exception* px)
 	{
// 		_TRACE_EX(px);
 		delete px;
 	}
	catch (...)
	{
//		_TRACE_EX_UNK();
	}
	
	return pSD;
}

void SecurityAPI::SetLowRegSecurity(HKEY hRootKey, LPCWSTR lpRootName)
{
	SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;
	SID_IDENTIFIER_AUTHORITY ntauth = SECURITY_WORLD_SID_AUTHORITY;
	SID_IDENTIFIER_AUTHORITY ntcreator = { 0, 0, 0, 0, 0, 3 }; //SECURITY_CREATOR_SID_AUTHORITY
	PSID pInteractSid = NULL;
	PSID pLocalSystemSid = NULL;
	PSID pAdministratorSid = NULL;
	PSID pAuthedUserSid = NULL;
	PSID pRestrictedSid = NULL;
	PSID pEveryOneSid = NULL;
	PSID pCreatorOwnerSid = NULL;
	PSID pUserSid = NULL;
	PSID pApplicationPackage = NULL;
	PSID pRegistryReadCapabilitySid = NULL;
	SECURITY_DESCRIPTOR sd;
	PACL pDacl = NULL;
	HKEY hKey = NULL;

	try
	{
		pInteractSid = _GetSpecificUserSID(SECURITY_INTERACTIVE_RID);
		pAdministratorSid = _GetAliasAdministratorsSID();
		pLocalSystemSid = _GetLocalSystemSID();
		pAuthedUserSid = _GetAuthenticatedUsersSID();
		pEveryOneSid = _GetEveryoneSID();
		pCreatorOwnerSid = _GetCreatorOwnerSID();
		pRestrictedSid = _GetSpecificUserSID(SECURITY_RESTRICTED_CODE_RID);
		pUserSid = _GetAliasUsersSID();
		if (EOSName::Win8 <= Environment::GetOSName())
		{
			pApplicationPackage = _GetApplicationPackageSID();
			pRegistryReadCapabilitySid = _GetRegistryReadCapabilitySID();
		}

		// compute size of new acl   
		DWORD dwAclSize = sizeof(ACL) + 8 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)) + GetLengthSid(pInteractSid) + GetLengthSid(pAdministratorSid) + GetLengthSid(pEveryOneSid) + GetLengthSid(pRestrictedSid) + GetLengthSid(pLocalSystemSid) + GetLengthSid(pCreatorOwnerSid) + GetLengthSid(pAuthedUserSid) + GetLengthSid(pUserSid);
		if (NULL != pApplicationPackage)
			dwAclSize += 1 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)) + GetLengthSid(pApplicationPackage);
		if (NULL != pRegistryReadCapabilitySid)
			dwAclSize += 1 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)) + GetLengthSid(pRegistryReadCapabilitySid);

		// allocate storage for Acl   
		pDacl = (PACL)::HeapAlloc(GetProcessHeap(), 0, dwAclSize);
		if (NULL == pDacl)
			throw new Win32Exception(::GetLastError());

		if (!::InitializeAcl(pDacl, dwAclSize, ACL_REVISION))
			throw new Win32Exception(::GetLastError());

		_AddAceToDacl(pDacl, pInteractSid, TRUE);
		_AddAceToDacl(pDacl, pAdministratorSid, TRUE);
		_AddAceToDacl(pDacl, pAuthedUserSid, TRUE);
		_AddAceToDacl(pDacl, pCreatorOwnerSid, TRUE);
		_AddAceToDacl(pDacl, pEveryOneSid, TRUE);
		_AddAceToDacl(pDacl, pLocalSystemSid, TRUE);
		_AddAceToDacl(pDacl, pRestrictedSid, TRUE);
		_AddAceToDacl(pDacl, pUserSid, TRUE);
		if (NULL != pApplicationPackage)
			_AddAceToDacl(pDacl, pApplicationPackage, TRUE);
		if (NULL != pRegistryReadCapabilitySid)
			_AddAceToDacl(pDacl, pRegistryReadCapabilitySid, TRUE);

		if (!::InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION))
			throw new Win32Exception(::GetLastError());

		if (!::SetSecurityDescriptorDacl(&sd, TRUE, pDacl, FALSE))
			throw new Win32Exception(::GetLastError());

		LONG lResult = RegOpenKeyEx(
			hRootKey,
			lpRootName,
			0,
			WRITE_DAC,
			&hKey
			);
		if (ERROR_SUCCESS != lResult)
			throw new Win32Exception(lResult);

		//    
		// apply the security descriptor to the registry key   
		//    
		lResult = ::RegSetKeySecurity(
			hKey,
			(SECURITY_INFORMATION)DACL_SECURITY_INFORMATION,
			&sd
			);
		if (ERROR_SUCCESS != lResult)
			throw new Win32Exception(lResult);
	}
	catch (Exception* px)
	{
		_TRACE_EX(px);
		delete px;
	}
	catch (...)
	{
		_TRACE_EX_UNK();
	}

	//
	// free allocated resources   
	//    
	if (NULL != hKey)
		::CloseHandle(hKey);
	if (pDacl != NULL)
		::HeapFree(::GetProcessHeap(), 0, pDacl);
	if (pInteractSid != NULL)
		::FreeSid(pInteractSid);
	if (pLocalSystemSid != NULL)
		::FreeSid(pLocalSystemSid);
	if (pAdministratorSid != NULL)
		::FreeSid(pAdministratorSid);
	if (NULL != pAuthedUserSid)
		::FreeSid(pAuthedUserSid);
	if (NULL != pCreatorOwnerSid)
		::FreeSid(pCreatorOwnerSid);
	if (NULL != pEveryOneSid)
		::FreeSid(pEveryOneSid);
	if (NULL != pRestrictedSid)
		::FreeSid(pRestrictedSid);
	if (NULL != pUserSid)
		::FreeSid(pUserSid);
	if (NULL != pApplicationPackage)
		::FreeSid(pApplicationPackage);
	if (NULL != pRegistryReadCapabilitySid)
		::FreeSid(pRegistryReadCapabilitySid);
}

void SecurityAPI::SetIntegrityRegLevel(HKEY hRootKey, LPCWSTR lpRootName, LPCWSTR lpStringSecurityDescriptor)
{
	if (Environment::GetOSName() <= EOSName::Vista)
		return;

	PSECURITY_DESCRIPTOR pSD = NULL;
	PACL pSacl = NULL;
	BOOL fSaclPresent = FALSE;
	BOOL fSaclDefaulted = FALSE;
	HKEY hKey = NULL;

	try
	{
		if (!::ConvertStringSecurityDescriptorToSecurityDescriptorW(
			lpStringSecurityDescriptor,
			SDDL_REVISION_1,
			&pSD,
			NULL))
			throw new Win32Exception(::GetLastError());

		if (!::GetSecurityDescriptorSacl(pSD, &fSaclPresent, &pSacl, &fSaclDefaulted))
			throw new Win32Exception(::GetLastError());

		LONG lResult = ::RegCreateKeyExW(hRootKey, lpRootName, 0, NULL, REG_OPTION_NON_VOLATILE, WRITE_DAC, NULL, &hKey, NULL);
		if (ERROR_SUCCESS != lResult)
			throw new Win32Exception(lResult);

		if (!::SetSecurityInfo(
			hKey,
			SE_REGISTRY_KEY,
			LABEL_SECURITY_INFORMATION | UNPROTECTED_SACL_SECURITY_INFORMATION,
			NULL, NULL, NULL, pSacl
			))
		{
			DWORD dw = ::GetLastError();
			if (ERROR_NO_TOKEN != dw)
				throw new Win32Exception(dw);
		}
	}
	catch (Exception* px)
	{
		_TRACE_EX(px);
		delete px;
	}
	catch (...)
	{
		_TRACE_EX_UNK();
	}

	if (NULL != hKey)
		::CloseHandle(hKey);
	if (NULL != pSD)
		::LocalFree(pSD);
}

/* FUNCTIONS ******************************************************************/
//Caller FreeSID
//SID:		S-1-5-18
//Use:		User SID
//Name:		SYSTEM
//Domain Name:	NT AUTHORITY
PSID SecurityAPI::_GetLocalSystemSID()
{
	PSID pSid = NULL;

	SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;
	if (!::AllocateAndInitializeSid(
		&sia,
		1,
		SECURITY_LOCAL_SYSTEM_RID,
		0, 0, 0, 0, 0, 0, 0,
		&pSid
		))
		throw new Win32Exception(::GetLastError());

	return pSid;
}

//Caller FreeSID
//SID:		S-1-5-32-544
//Use:		Alias SID
//Name:		Administrators
//Domain Name:	BUILTIN
PSID SecurityAPI::_GetAliasAdministratorsSID()
{	
	PSID pSid = NULL;

	SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;
	if (!::AllocateAndInitializeSid(
		&sia,
		2,
		SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS,
		0, 0, 0, 0, 0, 0,
		&pSid
		))
		throw new Win32Exception(::GetLastError());

	return pSid;
}

PSID SecurityAPI::_GetAliasUsersSID()
{	
	PSID pSid = NULL;

	SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;
	if (!::AllocateAndInitializeSid(
		&sia,
		2,
		SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_USERS,
		0, 0, 0, 0, 0, 0,
		&pSid
		))
		throw new Win32Exception(::GetLastError());

	return pSid;
}

//Caller FreeSID
//SID:		S-1-1-0
//Use:		Well-Known Group SID
//Name:		Everyone
//Domain Name:	
PSID SecurityAPI::_GetEveryoneSID()
{
	PSID pSid = NULL;

	SID_IDENTIFIER_AUTHORITY sia = SECURITY_WORLD_SID_AUTHORITY;
	if (!::AllocateAndInitializeSid(
		&sia,
		1,
		0, 0, 0, 0, 0, 0, 0, 0,
		&pSid
		))
		throw new Win32Exception(::GetLastError());

	return pSid;
}

//Caller FreeSID
//SID:		S-1-5-11
//Use:		Well-Known Group SID
//Name:		Authenticated Users
//Domain Name:	NT AUTHORITY
PSID SecurityAPI::_GetAuthenticatedUsersSID()
{
	PSID pSid = NULL;

	SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;
	if (!::AllocateAndInitializeSid(
		&sia,
		1,
		SECURITY_AUTHENTICATED_USER_RID,
		0, 0, 0, 0, 0, 0, 0,
		&pSid
		))
		throw new Win32Exception(::GetLastError());

	return pSid;
}

PSID SecurityAPI::_GetCreatorOwnerSID()
{
	PSID pSid = NULL;

	SID_IDENTIFIER_AUTHORITY sia = {0,0,0,0,0,3};
	if (!::AllocateAndInitializeSid(
		&sia,
		1,
		0, 0, 0, 0, 0, 0, 0, 0,
		&pSid
		))
		throw new Win32Exception(::GetLastError());

	return pSid;
}

PSID SecurityAPI::_GetSpecificUserSID(DWORD dwRid)
{
	PSID pSid = NULL;

	SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;
	if (!::AllocateAndInitializeSid(
		&sia,
		1,
		dwRid,
		0, 0, 0, 0, 0, 0, 0,
		&pSid
		))
		throw new Win32Exception(::GetLastError());

	return pSid;
}

PSID SecurityAPI::_GetApplicationPackageSID()
{
	PSID pSid = NULL;

	SID_IDENTIFIER_AUTHORITY SIDAuthAppPackage = { 0, 0, 0, 0, 0, 15 };//SECURITY_APP_PACKAGE_AUTHORITY;
	if (!::AllocateAndInitializeSid(&SIDAuthAppPackage,
		2 /*SECURITY_BUILTIN_APP_PACKAGE_RID_COUNT*/,
		2/* SECURITY_APP_PACKAGE_BASE_RID*/,
		1 /*SECURITY_BUILTIN_PACKAGE_ANY_PACKAGE*/,
		0, 0, 0, 0, 0, 0,
		&pSid)
		)
		throw new Win32Exception(::GetLastError());

	return pSid;
}

PSID SecurityAPI::_GetRegistryReadCapabilitySID()
{
	PSID pSid = NULL;

	if (!::ConvertStringSidToSidW(L"S-1-15-3-1024-1065365936-1281604716-3511738428-1654721687-432734479-3232135806-4053264122-3456934681", &pSid))
		throw new Win32Exception(::GetLastError());

	return pSid;
}

VOID SecurityAPI::_AddAceToDacl(PACL pDacl, PSID pSid, BOOL fInherit)
{
	if (!::AddAccessAllowedAceEx(   
		pDacl,   
		ACL_REVISION,
		(fInherit ? (CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE) : 0),
		STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL,   
		pSid))   
		throw new Win32Exception(::GetLastError());
}

HANDLE SecurityAPI::GetLogonUserToken(PDWORD pdwPid)
{
	HANDLE hToken = NULL;
	HANDLE hTokenElevated = NULL;
	HANDLE hSnapProcess = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (INVALID_HANDLE_VALUE == hSnapProcess)
		throw new Win32Exception(::GetLastError());

	PROCESSENTRY32 pe32 = {0};
	pe32.dwSize = sizeof(PROCESSENTRY32);

	DWORD dwActiveSession = SecurityAPI::GetActiveSession();

	for (BOOL f = ::Process32First(hSnapProcess, &pe32); f; f = ::Process32Next(hSnapProcess, &pe32))
	{
		string strFileName = string::Format(L"%ws", (const wchar_t*)pe32.szExeFile).ToLower();
		
		if( pe32.th32ProcessID == 4 || pe32.th32ProcessID == 0 || !strFileName.Equals(L"explorer.exe"))
			continue;
		try
		{
			DWORD dwProcessSessionid;
			if(!::ProcessIdToSessionId(pe32.th32ProcessID, &dwProcessSessionid))
				throw new Win32Exception(::GetLastError());
			if(dwProcessSessionid == dwActiveSession)
			{
				hToken = SecurityAPI::GetProcessToken(pe32.th32ProcessID);
				if (pdwPid)
					*pdwPid = pe32.th32ProcessID;

				string strSID = ToUserStringSID(hToken);
				int32 nID = 0;
				if (8 <= strSID.GetLength())
					nID =  Convert::ToInt32(strSID.Substring(6, 2));

				if (nID == 21)
				{
					if (Version(6, 0) <= Environment::GetOSVersion()->GetVersion())
					{
						DWORD dwLength = 0;
						TOKEN_ELEVATION te = {0};
						if (!::GetTokenInformation(hToken, TokenElevation, &te, sizeof(te), &dwLength))
							throw new Win32Exception(::GetLastError());

						if (te.TokenIsElevated)
						{
							hTokenElevated = hToken;
							hToken = NULL;
						}
						else
						{
							break;
						}
					}
					else
					{
						break;
					}
				}

			}
		}
		catch (Exception* px)
		{
			_TRACE_EX(px);
			delete px;
		}

		if (NULL != hToken)
		{
			::CloseHandle(hToken);
			hToken = NULL;
		}
	}

	if (NULL == hToken && NULL != hTokenElevated)	// 모든 유저토큰이 high일 경우
	{
		hToken = hTokenElevated;
		hTokenElevated = NULL;
	}

	if (NULL != hTokenElevated)
		::CloseHandle(hTokenElevated);

	::CloseHandle(hSnapProcess);
	return hToken;
}

HANDLE SecurityAPI::GetLogonUserElevatedToken()
{
	if (Environment::GetOSVersion()->GetVersion() < Version(6, 0))
		return SecurityAPI::GetLogonUserToken();

	HANDLE hToken = NULL;
	HANDLE hSnapProcess = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (INVALID_HANDLE_VALUE == hSnapProcess)
		throw new Win32Exception(::GetLastError());

	PROCESSENTRY32 pe32 = {0};
	pe32.dwSize = sizeof(PROCESSENTRY32);

	for (BOOL f = ::Process32First(hSnapProcess, &pe32); f; f = ::Process32Next(hSnapProcess, &pe32))
	{
		try
		{
			hToken = GetProcessToken(pe32.th32ProcessID);
			string strSID = ToUserStringSID(hToken);
			int32 nID = 0;
			if (8 <= strSID.GetLength())
				nID = Convert::ToInt32(strSID.Substring(6, 2));

			if (nID == 21)	
			{
				if (!IsElevatedToken(hToken))
				{
					DWORD dwLength = 0;
					TOKEN_LINKED_TOKEN tlt = {0};
					if (!::GetTokenInformation(hToken, TokenLinkedToken, &tlt, sizeof(tlt), &dwLength))
						throw new Win32Exception(::GetLastError());

					::CloseHandle(hToken);
					hToken = tlt.LinkedToken;
				}

				break;
			}
		}
		catch (Exception* px)
		{
			_TRACE_EX(px);
			delete px;
		}

		if (NULL != hToken)
		{
			::CloseHandle(hToken);
			hToken = NULL;
		}
	}

	::CloseHandle(hSnapProcess);

	return hToken;
}

BOOL SecurityAPI::IsAdministrator()
{
	HANDLE access_token; 
	DWORD buffer_size = 0; 
	PSID admin_SID; 
	TOKEN_GROUPS *group_token = NULL; 
	SID_IDENTIFIER_AUTHORITY NT_authority = SECURITY_NT_AUTHORITY; 

	if (!::OpenProcessToken(::GetCurrentProcess(), TOKEN_READ, &access_token))
		return FALSE;

	::GetTokenInformation( 
		access_token, 
		TokenGroups, 
		group_token, 
		0, 
		&buffer_size 
		); 

	group_token = (TOKEN_GROUPS*)::GlobalAlloc(GPTR, buffer_size);
	BOOL succeeded = ::GetTokenInformation( 
		access_token, 
		TokenGroups, 
		group_token, 
		buffer_size, 
		&buffer_size 
		);

	::CloseHandle(access_token); 
	if (!succeeded) 
		return FALSE;

	if (!::AllocateAndInitializeSid( 
		&NT_authority, 
		2, 
		SECURITY_BUILTIN_DOMAIN_RID, 
		DOMAIN_ALIAS_RID_ADMINS, 
		0,0,0,0,0,0, 
		&admin_SID 
		)) 
		return FALSE; 

	BOOL found = FALSE; 
	for (int i=0; !found && i < group_token->GroupCount; i++) 
		found = EqualSid(admin_SID,group_token->Groups[i].Sid); 
	
	::FreeSid(admin_SID); 
	::GlobalFree(group_token);

	return found;
}

HANDLE SecurityAPI::GetLocalSystemToken()
{
	HANDLE hToken = NULL;
	HANDLE hSnapProcess = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (INVALID_HANDLE_VALUE == hSnapProcess)
		throw new Win32Exception(::GetLastError());

	PROCESSENTRY32 pe32 = {0};
	pe32.dwSize = sizeof(PROCESSENTRY32);

	for (BOOL f = ::Process32First(hSnapProcess, &pe32); f; f = ::Process32Next(hSnapProcess, &pe32))
	{
		try
		{
			hToken = GetProcessToken(pe32.th32ProcessID);
			string strSID = ToUserStringSID(hToken);
			int32 nID = 0;
			if (8 <= strSID.GetLength())
				nID = Convert::ToInt32(strSID.Substring(6, 2));

			if (nID == 18)
				break;
		}
		catch (Exception* px)
		{
			_TRACE_EX(px);
			delete px;
		}

		if (NULL != hToken)
		{
			::CloseHandle(hToken);
			hToken = NULL;
		}
	}

	::CloseHandle(hSnapProcess);

	return hToken;
}

HANDLE SecurityAPI::GetLocalSystemTokenFromActiveSession()
{
	HANDLE hToken = NULL;
	HANDLE hSnapProcess = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (INVALID_HANDLE_VALUE == hSnapProcess)
		throw new Win32Exception(::GetLastError());

	PROCESSENTRY32 pe32 = { 0 };
	pe32.dwSize = sizeof(PROCESSENTRY32);

	DWORD dwActiveSession = GetActiveSession();

	for (BOOL f = ::Process32First(hSnapProcess, &pe32); f; f = ::Process32Next(hSnapProcess, &pe32))
	{
		if (pe32.th32ProcessID == 4 || pe32.th32ProcessID == 0)
			continue;

		try
		{
			DWORD dwProcessSessionid;
			if (!::ProcessIdToSessionId(pe32.th32ProcessID, &dwProcessSessionid))
				throw new Win32Exception(::GetLastError());

			if (dwProcessSessionid == dwActiveSession)
			{
				hToken = GetProcessToken(pe32.th32ProcessID);

				string strSID = ToUserStringSID(hToken);
				int32 nID = 0;
				if (8 <= strSID.GetLength())
					nID = Convert::ToInt32(strSID.Substring(6, 2));

				if (nID == 18)
					break;
			}
		}
		catch (Exception* px)
		{
			_TRACE_EX(px);
			delete px;
		}

		if (NULL != hToken)
		{
			::CloseHandle(hToken);
			hToken = NULL;
		}
	}

	::CloseHandle(hSnapProcess);

	return hToken;
}

HANDLE SecurityAPI::GetProcessToken(int nPid)
{
	if (0 == nPid || 4 == nPid)
		return NULL;

	HANDLE hToken = NULL;
	HANDLE hProcess = NULL;
	HANDLE hTokenSrc = NULL;

	try
	{
		hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, nPid);
		if (hProcess)
		{
			if (!::OpenProcessToken(hProcess, TOKEN_DUPLICATE, &hTokenSrc))
				throw new Win32Exception(::GetLastError());

			if (!::DuplicateTokenEx(hTokenSrc, MAXIMUM_ALLOWED, NULL, SecurityImpersonation, TokenPrimary, &hToken))
				throw new Win32Exception(::GetLastError());
		}
	}
	catch (Exception* pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}

	if (hProcess)
		::CloseHandle(hProcess);
	if (hTokenSrc)
		::CloseHandle(hTokenSrc);

	return hToken;
}

string SecurityAPI::GetProcessSID(int nPid)
{
	string strSID;
	HANDLE hToken = SecurityAPI::GetProcessToken(nPid);
	if (hToken)
	{
		strSID = SecurityAPI::ToUserStringSID(hToken);
		::CloseHandle(hToken);
	}
	return strSID;
}

string SecurityAPI::ToUserStringSID(HANDLE hToken)
{
	if (NULL == hToken)
		return L"";

	string strSID;
	LPWSTR lpSID = NULL;

	try
	{
		DWORD dwReturnLength = 0;
		::GetTokenInformation(hToken, TokenUser, NULL, 0, &dwReturnLength);
		array<byte> abTokenUser(dwReturnLength);
		if (!::GetTokenInformation(hToken, TokenUser, abTokenUser.GetBuffer(), abTokenUser.GetLength(), &dwReturnLength))
			throw new Win32Exception(::GetLastError());

		PTOKEN_USER pTokenUSer = (PTOKEN_USER)abTokenUser.GetBuffer();
		if (!::ConvertSidToStringSid(pTokenUSer->User.Sid, &lpSID))
			throw new Win32Exception(::GetLastError());

		strSID = string(lpSID);
	}
	catch (Exception* pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}

	if (lpSID)
		::LocalFree(lpSID);

	return strSID;
}

bool SecurityAPI::IsElevatedToken(HANDLE hToken)
{
	DWORD dwLength = 0;
	TOKEN_ELEVATION te = {0};
	if (!::GetTokenInformation(hToken, TokenElevation, &te, sizeof(te), &dwLength))
		throw new Win32Exception(::GetLastError());

	return (0 != te.TokenIsElevated);
}

DWORD SecurityAPI::GetCsrssProcessID()
{
	DWORD dwPidCsrss = 0;
	try
	{
		if (Environment::GetOSVersion()->GetVersion() < Version(5, 1))
		{
			::GetWindowThreadProcessId(::GetDesktopWindow(), &dwPidCsrss);
		}
		else 
		{
			typedef DWORD (WINAPI* PFCsrGetProcessId)();
			PFCsrGetProcessId pfCsrGetProcessId = (PFCsrGetProcessId)::GetProcAddress(::GetModuleHandleW(L"ntdll.dll"), "CsrGetProcessId");
			if (NULL == pfCsrGetProcessId)
				throw new Win32Exception(::GetLastError());
			dwPidCsrss = pfCsrGetProcessId();
		}
	}
	catch (Exception *pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}
	catch (...)
	{
		_TRACE_EX_UNK();
	}
	return dwPidCsrss;
}

BOOL SecurityAPI::VerifyEmbeddedSignature(LPCWSTR pwszSourceFile)
{
	LONG lStatus;
	DWORD dwLastError;
	BOOL f = FALSE;

	// Initialize the WINTRUST_FILE_INFO structure.

	WINTRUST_FILE_INFO FileData;
	memset(&FileData, 0, sizeof(FileData));
	FileData.cbStruct = sizeof(WINTRUST_FILE_INFO);
	FileData.pcwszFilePath = pwszSourceFile;
	FileData.hFile = NULL;
	FileData.pgKnownSubject = NULL;

	/*
	WVTPolicyGUID specifies the policy to apply on the file
	WINTRUST_ACTION_GENERIC_VERIFY_V2 policy checks:

	1) The certificate used to sign the file chains up to a root 
	certificate located in the trusted root certificate store. This 
	implies that the identity of the publisher has been verified by 
	a certification authority.

	2) In cases where user interface is displayed (which this example
	does not do), WinVerifyTrust will check for whether the  
	end entity certificate is stored in the trusted publisher store,  
	implying that the user trusts content from this publisher.

	3) The end entity certificate has sufficient permission to sign 
	code, as indicated by the presence of a code signing EKU or no 
	EKU.
	*/

	GUID WVTPolicyGUID = WINTRUST_ACTION_GENERIC_VERIFY_V2;
	WINTRUST_DATA WinTrustData;

	// Initialize the WinVerifyTrust input data structure.

	// Default all fields to 0.
	memset(&WinTrustData, 0, sizeof(WinTrustData));

	WinTrustData.cbStruct = sizeof(WinTrustData);

	// Use default code signing EKU.
	WinTrustData.pPolicyCallbackData = NULL;

	// No data to pass to SIP.
	WinTrustData.pSIPClientData = NULL;

	// Disable WVT UI.
	WinTrustData.dwUIChoice = WTD_UI_NONE;

	// No revocation checking.
	WinTrustData.fdwRevocationChecks = WTD_REVOKE_NONE; 

	// Verify an embedded signature on a file.
	WinTrustData.dwUnionChoice = WTD_CHOICE_FILE;

	// Default verification.
	WinTrustData.dwStateAction = 0;

	// Not applicable for default verification of embedded signature.
	WinTrustData.hWVTStateData = NULL;

	// Not used.
	WinTrustData.pwszURLReference = NULL;

	// This is not applicable if there is no UI because it changes 
	// the UI to accommodate running applications instead of 
	// installing applications.
	WinTrustData.dwUIContext = 0;

	// Set pFile.
	WinTrustData.pFile = &FileData;

	// WinVerifyTrust verifies signatures as specified by the GUID 
	// and Wintrust_Data.
	lStatus = WinVerifyTrust(
		NULL,
		&WVTPolicyGUID,
		&WinTrustData);

	switch (lStatus) 
	{
	case ERROR_SUCCESS:
		/*
		Signed file:
		- Hash that represents the subject is trusted.

		- Trusted publisher without any verification errors.

		- UI was disabled in dwUIChoice. No publisher or 
		time stamp chain errors.

		- UI was enabled in dwUIChoice and the user clicked 
		"Yes" when asked to install and run the signed 
		subject.
		*/
		_TRACE_I(Application, L"The file \"%ws\" is signed and the signature "
			L"was verified.\n",
			pwszSourceFile);
		f = TRUE;
		break;

	case TRUST_E_NOSIGNATURE:
		// The file was not signed or had a signature 
		// that was not valid.

		// Get the reason for no signature.
		dwLastError = GetLastError();
		if (TRUST_E_NOSIGNATURE == dwLastError ||
			TRUST_E_SUBJECT_FORM_UNKNOWN == dwLastError ||
			TRUST_E_PROVIDER_UNKNOWN == dwLastError) 
		{
			// The file was not signed.
			_TRACE_I(Application, L"The file \"%ws\" is not signed. %d\n",
				pwszSourceFile, dwLastError);
		} 
		else 
		{
			// The signature was not valid or there was an error 
			// opening the file.
			_TRACE_I(Application, L"An unknown error occurred trying to "
				L"verify the signature of the \"%ws\" file. %d\n",
				pwszSourceFile, dwLastError);
		}

		break;

	case TRUST_E_EXPLICIT_DISTRUST:
		// The hash that represents the subject or the publisher 
		// is not allowed by the admin or user.
		_TRACE_I(Application, L"The signature is present, but specifically "
			L"disallowed.\n");
		f = TRUE;
		break;

	case TRUST_E_SUBJECT_NOT_TRUSTED:
		// The user clicked "No" when asked to install and run.
		_TRACE_I(Application, L"The signature is present, but not "
			L"trusted.\n");
		break;

	case CRYPT_E_SECURITY_SETTINGS:
		/*
		The hash that represents the subject or the publisher 
		was not explicitly trusted by the admin and the 
		admin policy has disabled user trust. No signature, 
		publisher or time stamp errors.
		*/
		_TRACE_I(Application, L"CRYPT_E_SECURITY_SETTINGS - The hash "
			L"representing the subject or the publisher wasn't "
			L"explicitly trusted by the admin and admin policy "
			L"has disabled user trust. No signature, publisher "
			L"or timestamp errors.\n");
		break;

	default:
		// The UI was disabled in dwUIChoice or the admin policy 
		// has disabled user trust. lStatus contains the 
		// publisher or time stamp chain error.
		_TRACE_W( Application, L"Error is: 0x%x.", lStatus );
		f = TRUE;
		break;
	}

	return f;
}

BOOL SecurityAPI::ValidateCheckSum(PTSTR FileName)
{
	typedef DWORD (WINAPI *pMapFileAndCheckSumW)(PWSTR, PDWORD, PDWORD);

	DWORD HdrSum = 0, ChkSum = 0;
	HMODULE hImageHelp = ::LoadLibrary(L"Imagehlp.dll");
	if (hImageHelp)
	{
		pMapFileAndCheckSumW MapFileCheckSum = (pMapFileAndCheckSumW)::GetProcAddress(hImageHelp, "MapFileAndCheckSumW");
		if (MapFileCheckSum)
		{
			MapFileCheckSum(FileName, &HdrSum, &ChkSum);
			if(HdrSum != ChkSum)
				return FALSE;
			else if(0 == HdrSum && 0 == ChkSum)
				return FALSE;
		}
		::FreeLibrary(hImageHelp);
	}
	return TRUE;
}

void SecurityAPI::EnablePrivilege(LPCWSTR lpPrivilege, bool fEnable)
{
	HANDLE hToken = NULL;

	try
	{
		if (!::OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
			throw new Win32Exception(::GetLastError());

		LUID luid;
		if (!::LookupPrivilegeValue(NULL, lpPrivilege, &luid))
			throw new Win32Exception(::GetLastError());

		TOKEN_PRIVILEGES NewState;
		NewState.PrivilegeCount = 1;
		NewState.Privileges[0].Luid = luid;
		NewState.Privileges[0].Attributes = (fEnable ? SE_PRIVILEGE_ENABLED : 0);

		if (!::AdjustTokenPrivileges(hToken, FALSE, &NewState, 0, (PTOKEN_PRIVILEGES) NULL, 0))
			throw new Win32Exception(::GetLastError());

		::CloseHandle(hToken);
	}
	catch (...)
	{
		::CloseHandle(hToken);

		throw;
	}
}

bool SecurityAPI::_IsLowIntegrityLevel(HANDLE hToken)
{
	bool f = true;
	DWORD dwLength;
	DWORD dwIntegrityLevel;

	PTOKEN_MANDATORY_LABEL pTIL = NULL;

	if (!::GetTokenInformation(hToken, TokenIntegrityLevel, NULL, 0, &dwLength))
	{
		if (ERROR_INSUFFICIENT_BUFFER == ::GetLastError())
		{
			pTIL = (PTOKEN_MANDATORY_LABEL)LocalAlloc(0, dwLength);
			if (pTIL != NULL)
			{
				if (GetTokenInformation(hToken, TokenIntegrityLevel, pTIL, dwLength, &dwLength))
				{
					dwIntegrityLevel = *GetSidSubAuthority(pTIL->Label.Sid, (DWORD)(UCHAR)(*GetSidSubAuthorityCount(pTIL->Label.Sid)-1));
					if (dwIntegrityLevel != SECURITY_MANDATORY_LOW_RID)
						f = false;
				}
				LocalFree(pTIL);
			}
		}
	}
	return f;
}

bool SecurityAPI::_IsUntrustedIntegrityLevel(HANDLE hToken)
{
	bool f = true;
	DWORD dwLength;
	DWORD dwIntegrityLevel;

	PTOKEN_MANDATORY_LABEL pTIL = NULL;

	if (!::GetTokenInformation(hToken, TokenIntegrityLevel, NULL, 0, &dwLength))
	{
		if (ERROR_INSUFFICIENT_BUFFER == ::GetLastError())
		{
			pTIL = (PTOKEN_MANDATORY_LABEL)LocalAlloc(0, dwLength);
			if (pTIL != NULL)
			{
				if (::GetTokenInformation(hToken, TokenIntegrityLevel, pTIL, dwLength, &dwLength))
				{
					dwIntegrityLevel = *GetSidSubAuthority(pTIL->Label.Sid, (DWORD)(UCHAR)(*GetSidSubAuthorityCount(pTIL->Label.Sid)-1));
					if (dwIntegrityLevel != SECURITY_MANDATORY_UNTRUSTED_RID)
						f = false;
				}
				LocalFree(pTIL);
			}
		}
	}
	return f;
}

int SecurityAPI::GetActiveSession()
{
	HMODULE hWtsapi32 = NULL;

	typedef BOOL (WINAPI* PFWTSEnumerateSessionsW)(IN HANDLE hServer, IN DWORD Reserved, IN DWORD Version, __deref_out_ecount(*pCount) PWTS_SESSION_INFOW * ppSessionInfo, __out DWORD * pCount);
	typedef VOID (WINAPI* PFWTSFreeMemory)(IN PVOID pMemory);

	PFWTSEnumerateSessionsW pfWTSEnumerateSessionsW = NULL;
	PFWTSFreeMemory pfWTSFreeMemory = NULL;

	PWTS_SESSION_INFOW pWSI = NULL;
	int dwSessionId = 0;

	try
	{
		hWtsapi32 = ::LoadLibraryW(L"wtsapi32.dll");
		if (NULL == hWtsapi32)
			throw new Win32Exception(::GetLastError());

		pfWTSEnumerateSessionsW = (PFWTSEnumerateSessionsW)::GetProcAddress(hWtsapi32, "WTSEnumerateSessionsW");
		if (NULL == pfWTSEnumerateSessionsW)
			throw new Win32Exception(::GetLastError());
		pfWTSFreeMemory = (PFWTSFreeMemory)::GetProcAddress(hWtsapi32, "WTSFreeMemory");
		if (NULL == pfWTSFreeMemory)
			throw new Win32Exception(::GetLastError());

		DWORD dwCount = 0;
		if (!pfWTSEnumerateSessionsW(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pWSI, &dwCount))
			throw new Win32Exception(::GetLastError());

		for (DWORD dw = 0; dw < dwCount; dw++)
		{
			if (pWSI[dw].State == WTSActive)
			{
				dwSessionId = pWSI[dw].SessionId;
				break;
			}
		}
	}
	catch (Exception* px)
	{
		delete px;
	}

	if (pfWTSFreeMemory && pWSI)
		pfWTSFreeMemory(pWSI);
	if (hWtsapi32)
		::FreeLibrary(hWtsapi32);

	return dwSessionId;
}

string SecurityAPI::ExpandEnvironmentString(HANDLE hToken, const string& strPath)
{
	if (NULL == hToken)
		throw new ArgumentNullException;

	array<wchar_t> awc(1024);
	if (!::ExpandEnvironmentStringsForUserW(hToken, strPath, awc.GetBuffer(), awc.GetLength()))
		throw new Win32Exception(::GetLastError());

	return string(awc.GetBuffer());
}

HANDLE SecurityAPI::WaitForLogonUserToken(PDWORD pdwPid)
{
	// !!TO-DO!! logon notification?

	HANDLE hLogonToken = NULL;

	while (true)
	{
		hLogonToken = SecurityAPI::GetLogonUserToken(pdwPid);

		if (NULL == hLogonToken)
			::Sleep(1000);
		else
			break;
	}
	
	return hLogonToken;
}

void SecurityAPI::AddServiceDacl_AdminDeniedToModify(const string& strServiceName)
{
	SC_HANDLE hScm = NULL;
	SC_HANDLE hSvc = NULL;
	PSECURITY_DESCRIPTOR pSd = NULL;
	PACL pNewDacl = NULL;

	try
	{
		hScm = ::OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if (NULL == hScm)
			throw new Win32Exception(::GetLastError());

		hSvc = ::OpenServiceW(hScm, strServiceName, READ_CONTROL | WRITE_DAC);
		if (NULL == hSvc)
			throw new Win32Exception(::GetLastError());

		DWORD dwSdSize = 0;
		::QueryServiceObjectSecurity(hSvc, DACL_SECURITY_INFORMATION, &pSd, 0, &dwSdSize);
		if (0 == dwSdSize)
			throw new Win32Exception(::GetLastError());

		pSd = (PSECURITY_DESCRIPTOR)::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, dwSdSize);
		if (NULL == pSd)
			throw new OutOfMemoryException;

		if (!::QueryServiceObjectSecurity(hSvc, DACL_SECURITY_INFORMATION, pSd, dwSdSize, &dwSdSize))
			throw new Win32Exception(::GetLastError());

		PACL pDacl = NULL;
		BOOL fDaclPresent = FALSE;
		BOOL fDaclDefaulted = FALSE;
		if (!::GetSecurityDescriptorDacl(pSd, &fDaclPresent, &pDacl, &fDaclDefaulted))
			throw new Win32Exception(::GetLastError());

		EXPLICIT_ACCESS_W ea = { 0 };
		::BuildExplicitAccessWithNameW(
			&ea,
			L"Administrators",
			SERVICE_STOP | SERVICE_PAUSE_CONTINUE | SERVICE_CHANGE_CONFIG | DELETE,
			DENY_ACCESS,
			NO_INHERITANCE);

		DWORD dwError = ::SetEntriesInAclW(1, &ea, pDacl, &pNewDacl);
		if (ERROR_SUCCESS != dwError)
			throw new Win32Exception(dwError);

		SECURITY_DESCRIPTOR sdNew = { 0 };
		if (!::InitializeSecurityDescriptor(&sdNew, SECURITY_DESCRIPTOR_REVISION))
			throw new Win32Exception(::GetLastError());

		if (!::SetSecurityDescriptorDacl(&sdNew, TRUE, pNewDacl, FALSE))
			throw new Win32Exception(::GetLastError());

		if (!::SetServiceObjectSecurity(hSvc, DACL_SECURITY_INFORMATION, &sdNew))
			throw new Win32Exception(::GetLastError());

	}
	catch (Exception* px)
	{
		_TRACE_EX(px);
		delete px;
	}

	if (pNewDacl)
		::LocalFree(pNewDacl);
	if (pSd)
		::HeapFree(::GetProcessHeap(), 0, (LPVOID)pSd);
	if (hSvc)
		::CloseServiceHandle(hSvc);
	if (hScm)
		::CloseServiceHandle(hScm);
}

void SecurityAPI::DeleteServiceDacl_Denied(const string& strServiceName)
{
	SC_HANDLE hScm = NULL;
	SC_HANDLE hSvc = NULL;
	PSECURITY_DESCRIPTOR pSd = NULL;
	PEXPLICIT_ACCESS_W pEntries = NULL;
	PACL pNewDacl = NULL;

	try
	{
		hScm = ::OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if (NULL == hScm)
			throw new Win32Exception(::GetLastError());

		hSvc = ::OpenServiceW(hScm, strServiceName, READ_CONTROL | WRITE_DAC);
		if (NULL == hSvc)
			throw new Win32Exception(::GetLastError());

		DWORD dwSdSize = 0;
		::QueryServiceObjectSecurity(hSvc, DACL_SECURITY_INFORMATION, &pSd, 0, &dwSdSize);
		if (0 == dwSdSize)
			throw new Win32Exception(::GetLastError());

		pSd = (PSECURITY_DESCRIPTOR)::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, dwSdSize);
		if (NULL == pSd)
			throw new OutOfMemoryException;

		if (!::QueryServiceObjectSecurity(hSvc, DACL_SECURITY_INFORMATION, pSd, dwSdSize, &dwSdSize))
			throw new Win32Exception(::GetLastError());

		PACL pDacl = NULL;
		BOOL fDaclPresent = FALSE;
		BOOL fDaclDefaulted = FALSE;
		if (!::GetSecurityDescriptorDacl(pSd, &fDaclPresent, &pDacl, &fDaclDefaulted))
			throw new Win32Exception(::GetLastError());

		ULONG ulEntryCount = 0;
		DWORD dwError = ::GetExplicitEntriesFromAclW(pDacl, &ulEntryCount, &pEntries);
		if (ERROR_SUCCESS != dwError)
			throw new Win32Exception(dwError);

		for (ULONG ulIndex = 0; ulIndex < ulEntryCount; ulIndex++)
		{
			if (GRANT_ACCESS == pEntries[ulIndex].grfAccessMode)
			{
				dwError = ::SetEntriesInAclW(ulEntryCount - ulIndex, &pEntries[ulIndex], NULL, &pNewDacl);
				break;
			}
		}

		SECURITY_DESCRIPTOR sdNew = { 0 };
		if (!::InitializeSecurityDescriptor(&sdNew, SECURITY_DESCRIPTOR_REVISION))
			throw new Win32Exception(::GetLastError());

		if (!::SetSecurityDescriptorDacl(&sdNew, TRUE, pNewDacl, FALSE))
			throw new Win32Exception(::GetLastError());

		if (!::SetServiceObjectSecurity(hSvc, DACL_SECURITY_INFORMATION, &sdNew))
			throw new Win32Exception(::GetLastError());

	}
	catch (Exception* px)
	{
		_TRACE_EX(px);
		delete px;
	}

	if (pNewDacl)
		::LocalFree(pNewDacl);
	if (pEntries)
		::LocalFree(pEntries);
	if (pSd)
		::HeapFree(::GetProcessHeap(), 0, (LPVOID)pSd);
	if (hSvc)
		::CloseServiceHandle(hSvc);
	if (hScm)
		::CloseServiceHandle(hScm);
}

}

}