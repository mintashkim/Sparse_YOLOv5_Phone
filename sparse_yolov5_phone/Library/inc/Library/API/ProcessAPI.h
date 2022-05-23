#pragma once
namespace Library
{
namespace API
{

class ProcessAPI
{
public:
	static int GetSystemPid();
	static int GetLsmPid();
	static int GetSmssPid();
	static int GetWininitPid();
	static int GetWinlogonPid();
	static int GetServicesPid();
	static uint32 GetServicePid(const string& strServiceName);
	static void GetProcessIDListByName(string strExeName, List<DWORD>* lstPid);

	static void ShellExecuteExtension(string strOp, string strFile, string strParam, string strDir);
	static void ShellExecuteAsAdministrator(string strOp, string strFile, string strParam, string strDir);

	static void TerminateProcessByName(string strExeName);
	static void TerminateProcessRecursive(DWORD dwPid);
	static void TerminateServiceProcess(string strServiceName);
	static HANDLE GetProcessHandleByName(string strExeName, DWORD dwDesiredAccess);
	static HANDLE GetProcessHandleOfService(const string& strServiceName);
	static DWORD GetProcessIDByPath(string strPath);
	static DWORD GetProcessIDByName(string strExeName);
	static DWORD GetParentProcessID(DWORD dwPID);
	static array<DWORD> GetChildProcessID(DWORD dwPid);
	static string GetProcessPathByPid(DWORD dwPid);
	static string GetCommandLineByPid(DWORD dwPid);
	static string ToProcessName(const string& strProcessPath);
	static bool IsStronglyNamedProcess(DWORD dwPid);
	static bool IsActiveProcess(HANDLE hProcess);

	static bool LaunchProcessAsUser(string strFilename, string strParameter, string strCurrentDirectory, string strDesktop, PROCESS_INFORMATION *pPI);
	static bool LaunchProcessAsElevatedUser(string strFilename, string strParameter, string strCurrentDirectory, string strDesktop, PROCESS_INFORMATION *pPI);
	static bool LaunchProcessAsLocalSystem(string strFilename, string strParameter, string strCurrentDirectory, string strDesktop, PROCESS_INFORMATION *pPI);
	static bool LaunchProcessWithToken(string strApplicationName, string strCommandLine, string strCurrentDirectory, HANDLE hToken, int nNewParentPid, string strDesktop, DWORD dwCreationFlags, PROCESS_INFORMATION* pPI);

	//static bool CheckMirageWorksVirtualZoneProcess(DWORD dwPid);
	static bool CheckNeedElevation(string strPath);

	static bool CheckDoosanDlpRunning();

	// Services & Process
	static void EnableServices(string strServiceName);
	static void DisableServices(string strServiceName);

	static bool StartServiceByName(string strServiceName);
	static bool StopServiceByName(string strServiceName);
	static bool IsServiceRunning(string strServiceName);
	static bool IsProcessRunning(string strProcessName);

	static void SetServiceRecovery(const string& strServiceName, bool fEnable);

	static void ExecuteCmdLine(string strCmdLine, int nWaitMilliSecond = 0);
	
	static string GetCurControlSetString();
	static bool ExecutePowerShell(string strArg);
	static bool ExecuteGroupUpdate();

	static bool IsExecutableFile(const string& strPath);

	static uint64 GetProcessCreatedTime(uint32 nPid);
};

}
}