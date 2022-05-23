#pragma once
#include <ShlObj.h>
#include <sddl.h>

// S:(ML;CI;NW;;;ME)
#define SDDL_SACL_MEDIUM_INTEGRITY_CI	SDDL_SACL SDDL_DELIMINATOR SDDL_ACE_BEGIN SDDL_MANDATORY_LABEL SDDL_SEPERATOR SDDL_CONTAINER_INHERIT SDDL_SEPERATOR SDDL_NO_WRITE_UP SDDL_SEPERATOR SDDL_SEPERATOR SDDL_SEPERATOR SDDL_ML_MEDIUM SDDL_ACE_END
// S:(ML;CI;NW;;;LW)
#define SDDL_SACL_LOW_INTEGRITY_CI		SDDL_SACL SDDL_DELIMINATOR SDDL_ACE_BEGIN SDDL_MANDATORY_LABEL SDDL_SEPERATOR SDDL_CONTAINER_INHERIT SDDL_SEPERATOR SDDL_NO_WRITE_UP SDDL_SEPERATOR SDDL_SEPERATOR SDDL_SEPERATOR SDDL_ML_LOW SDDL_ACE_END

namespace Library
{
namespace API
{

class SecurityAPI
{
public:
	static bool SetLowFileSecurity(LPCWSTR lpPath);
	static bool SetLowIntegrityFileLevel(LPCWSTR lpPath);
	static PSECURITY_DESCRIPTOR GetLowIntegritySecurityDescriptor();

	static void SetLowRegSecurity(HKEY hRootKey, LPCWSTR lpRootName);
	static void SetIntegrityRegLevel(HKEY hRootKey, LPCWSTR lpRootName, LPCWSTR lpStringSecurityDescriptor);

	static HANDLE GetLogonUserToken(PDWORD pdwPid = NULL);
	static HANDLE GetLogonUserElevatedToken();
	static HANDLE GetLocalSystemToken();
	static HANDLE GetLocalSystemTokenFromActiveSession();
	
	static HANDLE GetProcessToken(int nPid);
	static string GetProcessSID(int nPid);
	static string ToUserStringSID(HANDLE hToken);
	static bool IsElevatedToken(HANDLE hToken);

	static DWORD GetCsrssProcessID();

	static BOOL VerifyEmbeddedSignature(LPCWSTR pwszSourceFile);
	static BOOL ValidateCheckSum(PTSTR FileName);

	static void EnablePrivilege(LPCWSTR lpPrivilege, bool fEnable);
	static int GetActiveSession();

	static BOOL IsAdministrator();

	static string ExpandEnvironmentString(HANDLE hToken, const string& strPath);

	static HANDLE WaitForLogonUserToken(PDWORD pdwPid = NULL);

	// #1869
	static void AddServiceDacl_AdminDeniedToModify(const string& strServiceName);
	static void DeleteServiceDacl_Denied(const string& strServiceName);

	static PSID _GetLocalSystemSID();
	static PSID _GetAliasAdministratorsSID();
	static PSID _GetAliasUsersSID();
	static PSID _GetEveryoneSID();
	static PSID _GetAuthenticatedUsersSID();
	static PSID _GetCreatorOwnerSID();
	static PSID _GetSpecificUserSID(DWORD dwRid);
	static PSID _GetApplicationPackageSID();
	static PSID _GetRegistryReadCapabilitySID();

private:
	static VOID _AddAceToDacl(PACL pDacl, PSID pSid, BOOL fInherit);

	static bool _IsLowIntegrityLevel(HANDLE hToken);
	static bool _IsUntrustedIntegrityLevel(HANDLE hToken);

};
}
}