#include "StdAfx.h"
#include <System/ServiceProcess/all.h>
#include <Library/API/ProcessAPI.h>
#include <Library/API/SecurityAPI.h>
#include <Library/ntdll.h>
#include <Library/NtdllException.h>
#include <System/all.h>
#include <System/io/all.h>
#include <Microsoft/Win32/all.h>
#include <NoAD/Win32/all.h>
#include <psapi.h>
#include <TlHelp32.h>
#include <userenv.h>
#include <sddl.h>
#include <ShlObj.h>
#include <ShellAPI.h>

#include "wpp.h"
#include "ProcessAPI.tmh"

#define _WIN32_MSI 110
#include <msi.h> 
#pragma comment(lib, "msi")
#pragma comment(lib, "psapi.lib")
#pragma comment(lib,  "userenv.lib")
typedef BOOL (WINAPI* PFInitializeProcThreadAttributeList)(LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList, DWORD dwAttributeCount, DWORD dwFlags, PSIZE_T lpSize);
typedef VOID (WINAPI* PFDeleteProcThreadAttributeList)(LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList);
typedef BOOL (WINAPI* PFUpdateProcThreadAttribute)(LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList, DWORD dwFlags, DWORD_PTR Attribute, PVOID lpValue, SIZE_T cbSize, PVOID lpPreviousValue, PSIZE_T lpReturnSize);
typedef NTSTATUS (NTAPI* PFNtQueryInformationProcess)(
	IN HANDLE ProcessHandle,
	IN PROCESSINFOCLASS ProcessInformationClass,
	OUT PVOID ProcessInformation,
	IN ULONG ProcessInformationLength,
	OUT PULONG ReturnLength OPTIONAL
	);


typedef ULONG (WINAPI* PCheckElevation)(
	__in PWSTR FileName,
	__inout PULONG Flags, // Have a ULONG set to 0, and pass a pointer to it
	__in_opt HANDLE TokenHandle, // Use NULL
	__out_opt PULONG Output1, // On output, is 2 or less.
	__out_opt PULONG Output2
	);

namespace Library
{
namespace API
{
	
int ProcessAPI::GetSystemPid()
{
	return (Environment::GetOSVersion()->GetVersion() < Version(5, 1) ? 8 : 4);
}

int ProcessAPI::GetLsmPid()
{
	string strPath = Environment::GetFolderPath(ESpecialFolder::System) + L"\\lsm.exe";
	return GetProcessIDByPath(strPath);
}

int ProcessAPI::GetSmssPid()
{
	return ProcessAPI::GetProcessIDByName(L"smss.exe");
}

int ProcessAPI::GetWininitPid()
{
	return ProcessAPI::GetProcessIDByName(L"wininit.exe");
}

int ProcessAPI::GetWinlogonPid()
{
	return ProcessAPI::GetProcessIDByName(L"winlogon.exe");
}

int ProcessAPI::GetServicesPid()
{
	string strPath = Environment::GetFolderPath(ESpecialFolder::System) + L"\\services.exe";
	return GetProcessIDByPath(strPath);
}

uint32 ProcessAPI::GetServicePid(const string& strServiceName)
{
	uint32 uPid = 0;
	SC_HANDLE hSCM = NULL;
	SC_HANDLE hService = NULL;

	try
	{
		hSCM = ::OpenSCManagerW(NULL, NULL, GENERIC_READ);
		if (NULL == hSCM)
			throw new Win32Exception(::GetLastError());

		hService = ::OpenServiceW(hSCM, strServiceName, SERVICE_QUERY_STATUS);
		if (NULL == hService)
			throw new Win32Exception(::GetLastError());

		SERVICE_STATUS_PROCESS ssp = { 0 };
		DWORD cbBytesNeeded = 0;
		if (!::QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(SERVICE_STATUS_PROCESS), &cbBytesNeeded))
			throw new Win32Exception(::GetLastError());

		uPid = ssp.dwProcessId;
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

	::CloseServiceHandle(hService);
	::CloseServiceHandle(hSCM);

	return uPid;
}

void ProcessAPI::GetProcessIDListByName(string strExeName, List<DWORD>* lstPid)
{
	try
	{
		HANDLE hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (INVALID_HANDLE_VALUE == hSnapshot)
			throw new Win32Exception(::GetLastError());

		PROCESSENTRY32 entry;
		memset(&entry, 0, sizeof(PROCESSENTRY32));
		entry.dwSize = sizeof(PROCESSENTRY32);
		BOOL b = ::Process32First(hSnapshot, &entry);
		while (b)
		{
			string strExe = string(entry.szExeFile);
			if (strExe.ToLower() == strExeName.ToLower())
			{
				if (entry.th32ProcessID > 0)
					lstPid->Add(entry.th32ProcessID);
			}
			entry.dwSize = sizeof(PROCESSENTRY32);
			b = ::Process32Next(hSnapshot, &entry);
		}
		::CloseHandle(hSnapshot);
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
}

void ProcessAPI::TerminateProcessByName(string strExeName)
{
	try
	{
		HANDLE hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0) ;
		if (INVALID_HANDLE_VALUE == hSnapshot)
			throw new Win32Exception(::GetLastError());

		PROCESSENTRY32 entry;
		memset(&entry, 0, sizeof(PROCESSENTRY32));
		entry.dwSize = sizeof(PROCESSENTRY32) ;
		BOOL b = ::Process32First(hSnapshot, &entry);
		while(b)
		{
			string strExe = string(entry.szExeFile);
			if (strExe.ToLower() == strExeName.ToLower())
			{
				HANDLE hProcess = ::OpenProcess(PROCESS_TERMINATE, FALSE, entry.th32ProcessID);
				if (INVALID_HANDLE_VALUE != hProcess && NULL != hProcess)
				{
					::TerminateProcess(hProcess, 0);
					::CloseHandle(hProcess);
				}
			}
			entry.dwSize = sizeof(PROCESSENTRY32) ;
			b = ::Process32Next(hSnapshot, &entry);
		}
		::CloseHandle(hSnapshot);
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
}

HANDLE ProcessAPI::GetProcessHandleByName(string strExeName, DWORD dwDesiredAccess)
{
	HANDLE hResult = NULL;
	try
	{
		HANDLE hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0) ;
		if (INVALID_HANDLE_VALUE == hSnapshot)
			throw new Win32Exception(::GetLastError());

		PROCESSENTRY32 entry;
		memset(&entry, 0, sizeof(PROCESSENTRY32));
		entry.dwSize = sizeof(PROCESSENTRY32) ;
		BOOL b = ::Process32First(hSnapshot, &entry);
		while(b)
		{
			string strExe = string(entry.szExeFile);
			if (strExe.ToLower() == strExeName.ToLower())
			{
				hResult = ::OpenProcess(dwDesiredAccess, FALSE, entry.th32ProcessID);
				if (NULL != hResult)
					break;				
			}
			entry.dwSize = sizeof(PROCESSENTRY32) ;
			b = ::Process32Next(hSnapshot, &entry);
		}
		::CloseHandle(hSnapshot);
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
	return hResult;
}

HANDLE ProcessAPI::GetProcessHandleOfService(const string& strServiceName)
{
	HANDLE handle = NULL;
	SC_HANDLE hSCM = NULL;
	SC_HANDLE hService = NULL;

	try
	{
		hSCM = ::OpenSCManagerW(NULL, NULL, GENERIC_READ);
		if (NULL == hSCM)
			throw new Win32Exception(::GetLastError());

		hService = ::OpenServiceW(hSCM, strServiceName, SERVICE_QUERY_STATUS);
		if (NULL == hService)
			throw new Win32Exception(::GetLastError());

		SERVICE_STATUS_PROCESS ssp = {0};
		DWORD cbBytesNeeded = 0;
		if (!::QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(SERVICE_STATUS_PROCESS), &cbBytesNeeded))
			throw new Win32Exception(::GetLastError());

		handle = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, ssp.dwProcessId);
		if (handle == NULL)
			throw new Win32Exception(::GetLastError());
	}
	catch (Exception* pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}

	if (NULL != hService)
		::CloseServiceHandle(hService);
	if (NULL != hSCM)
		::CloseServiceHandle(hSCM);

	if (NULL == handle)
		_TRACE_E(Application, "GetServiceHandle(%ws) FAILED", (const wchar_t*)strServiceName);

	return handle;
}

DWORD ProcessAPI::GetProcessIDByPath(string strPath)
{
	DWORD dwResult = 0;
	try
	{
		HANDLE hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (INVALID_HANDLE_VALUE == hSnapshot)
			throw new Win32Exception(::GetLastError());

		PROCESSENTRY32 entry;
		memset(&entry, 0, sizeof(PROCESSENTRY32));
		entry.dwSize = sizeof(PROCESSENTRY32);
		BOOL b = ::Process32First(hSnapshot, &entry);
		while (b)
		{
			if (4 < entry.th32ProcessID)
			{
				string strCurrentPath = ProcessAPI::GetProcessPathByPid(entry.th32ProcessID);
				if (strPath.Equals(strCurrentPath, true))
				{
					dwResult = entry.th32ProcessID;
					break;
				}
			}

			entry.dwSize = sizeof(PROCESSENTRY32);
			b = ::Process32Next(hSnapshot, &entry);
		}
		::CloseHandle(hSnapshot);
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
	return dwResult;
}

DWORD ProcessAPI::GetProcessIDByName(string strExeName)
{
	DWORD dwResult = 0;
	try
	{
		HANDLE hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0) ;
		if (INVALID_HANDLE_VALUE == hSnapshot)
			throw new Win32Exception(::GetLastError());

		PROCESSENTRY32 entry;
		memset(&entry, 0, sizeof(PROCESSENTRY32));
		entry.dwSize = sizeof(PROCESSENTRY32) ;
		BOOL b = ::Process32First(hSnapshot, &entry);
		while(b)
		{
			string strExe = string(entry.szExeFile);
			if (strExe.ToLower() == strExeName.ToLower())
			{
				if (entry.th32ProcessID > 0)
				{
					dwResult = entry.th32ProcessID;
					break;
				}
			}
			entry.dwSize = sizeof(PROCESSENTRY32) ;
			b = ::Process32Next(hSnapshot, &entry);
		}
		::CloseHandle(hSnapshot);
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
	return dwResult;
}

DWORD ProcessAPI::GetParentProcessID(DWORD dwPID)
{
	NTSTATUS ntStatus;
	DWORD dwParentPID = 0xffffffff;
	HANDLE hProcess;
	ULONG ulRetLen;

	HMODULE hModule = ::LoadLibraryW(L"ntdll.dll");
	if (NULL == hModule)
		return dwParentPID;

	PFNtQueryInformationProcess NtQueryInformationProcess = 
		(PFNtQueryInformationProcess)::GetProcAddress(hModule, "NtQueryInformationProcess");

	PROCESS_BASIC_INFORMATION pbi = {0};

	//  get process handle
	hProcess = OpenProcess(
		PROCESS_QUERY_INFORMATION,
		FALSE,
		dwPID
		);

	//  could fail due to invalid PID or insufficiant privileges
	if (!hProcess)
	{
		FreeLibrary(hModule);
		return (0xffffffff);
	}

	//  gather information
	ntStatus = NtQueryInformationProcess(
		hProcess,
		ProcessBasicInformation,
		(void*)&pbi,
		sizeof(PROCESS_BASIC_INFORMATION),
		&ulRetLen
		);

	//  copy PID on success
	if (!ntStatus)
		dwParentPID = pbi.InheritedFromUniqueProcessId;

	CloseHandle(hProcess);
	FreeLibrary(hModule);

	return (dwParentPID);
}

void ProcessAPI::TerminateProcessRecursive(DWORD dwPid)
{
	if (dwPid <= 0)
		return;
	try
	{
		array<DWORD> adwChildProcesses = ProcessAPI::GetChildProcessID(dwPid);
		for (int i = 0 ; i < adwChildProcesses.GetLength() ; i++)
			ProcessAPI::TerminateProcessRecursive(adwChildProcesses[i]);

		HANDLE hProcess = ::OpenProcess(PROCESS_TERMINATE, FALSE, dwPid);
		if (INVALID_HANDLE_VALUE != hProcess && NULL != hProcess)
		{
			::TerminateProcess(hProcess, 0);
			::CloseHandle(hProcess);
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
}

array<DWORD> ProcessAPI::GetChildProcessID(DWORD dwPid)
{
	array<DWORD> adwResult;
	try
	{
		List<DWORD> dwProcess;

		HANDLE hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0) ;
		if (INVALID_HANDLE_VALUE == hSnapshot)
			throw new Win32Exception(::GetLastError());

		PROCESSENTRY32 entry;
		memset(&entry, 0, sizeof(PROCESSENTRY32));
		entry.dwSize = sizeof(PROCESSENTRY32) ;
		BOOL b = ::Process32First(hSnapshot, &entry);
		while(b)
		{
			if (entry.th32ParentProcessID == dwPid)
				dwProcess.Add(entry.th32ProcessID);

			entry.dwSize = sizeof(PROCESSENTRY32) ;
			b = ::Process32Next(hSnapshot, &entry);
		}
		::CloseHandle(hSnapshot);

		if (dwProcess.GetCount() > 0)
		{
			adwResult = array<DWORD>(dwProcess.GetCount());
			dwProcess.CopyTo(adwResult, 0);
		}
	}
	catch (Exception* pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}
	catch (...)
	{
		_TRACE_EX_UNK();
	}
	return adwResult;
}

string ProcessAPI::GetProcessPathByPid(DWORD dwPid)
{
	HANDLE hProcess = NULL;
	string strPath = L"";

	try
	{
		hProcess = ::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, dwPid);
		if (NULL == hProcess)
		{
			DWORD dwLastError = ::GetLastError();
			if (ERROR_INVALID_PARAMETER != dwLastError)
				throw new Win32Exception(::GetLastError());
		}

		if (hProcess)
		{
			PFNtQueryInformationProcess pfNtQueryInformationProcess = (PFNtQueryInformationProcess)::GetProcAddress(::GetModuleHandleW(L"ntdll.dll"), "NtQueryInformationProcess");
			if (NULL == pfNtQueryInformationProcess)
				throw new Win32Exception(::GetLastError());

			ULONG ulReturnLength = 0;
			NTSTATUS status = pfNtQueryInformationProcess(
				hProcess,
				ProcessImageFileNameWin32,
				NULL,
				0,
				&ulReturnLength
			);

			if (0 < ulReturnLength)
			{
				array<byte> abBuffer(ulReturnLength);
				status = pfNtQueryInformationProcess(
					hProcess,
					ProcessImageFileNameWin32,
					abBuffer.GetBuffer(),
					abBuffer.GetLength(),
					&ulReturnLength
				);

				PUNICODE_STRING pusz = (PUNICODE_STRING)abBuffer.GetBuffer();
				strPath = string(pusz->Buffer, 0, pusz->Length / sizeof(wchar_t));
			}
		}
	}
	catch (Exception* pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}
	catch (...)
	{
		_TRACE_EX_UNK();
	}

	if (hProcess)
		::CloseHandle(hProcess);

	return strPath;
}

string ProcessAPI::GetCommandLineByPid(DWORD dwPid)
{
	string strCmdLine;
	HANDLE hProcess = NULL;

	do
	{
		hProcess = ::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, dwPid);
		if (NULL == hProcess)
			break;

		PFNtQueryInformationProcess pfNtQueryInformationProcess = (PFNtQueryInformationProcess)::GetProcAddress(::GetModuleHandleW(L"ntdll.dll"), "NtQueryInformationProcess");
		if (NULL == pfNtQueryInformationProcess)
			break;

		PROCESS_BASIC_INFORMATION pbi = { 0 };
		NTSTATUS status = pfNtQueryInformationProcess(
			hProcess,
			ProcessBasicInformation,
			&pbi,
			sizeof(pbi),
			NULL
			);
		if (!NT_SUCCESS(status))
			break;

		if (NULL == pbi.PebBaseAddress)
			break;

		PEB_ peb = { 0 };
		if (!::ReadProcessMemory(
			hProcess,
			pbi.PebBaseAddress,
			&peb,
			sizeof(peb),
			NULL
			))
		{
			break;
		}

		RTL_USER_PROCESS_PARAMETERS params = { 0 };
		if (!::ReadProcessMemory(
			hProcess,
			peb.ProcessParameters,
			&params,
			sizeof(params),
			NULL
			))
		{
			break;
		}

		if (params.CommandLine.Length <= 0)
			break;

		array<wchar_t> awcCmdLine(params.CommandLine.Length);
		if (!::ReadProcessMemory(
			hProcess,
			params.CommandLine.Buffer,
			awcCmdLine.GetBuffer(),
			awcCmdLine.GetLength(),
			NULL
			))
		{
			break;
		}

		strCmdLine = string(awcCmdLine.GetBuffer(), 0, awcCmdLine.GetLength() / sizeof(wchar_t));

	} while (false);

	if (hProcess)
		::CloseHandle(hProcess);

	return strCmdLine;
}

string ProcessAPI::ToProcessName(const string& strProcessPath)
{
	string strProcessName;

	int nIndex = strProcessPath.LastIndexOf(L'\\');
	if (0 < nIndex && nIndex + 1 < strProcessPath.GetLength())
		strProcessName = strProcessPath.Substring(nIndex + 1);
	else
		strProcessName = strProcessPath;

	return strProcessName;
}

bool ProcessAPI::IsStronglyNamedProcess(DWORD dwPid)
{
	bool f = false;
	HANDLE hProcess = NULL;

	do
	{
		hProcess = ::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, dwPid);
		if (NULL == hProcess)
			break;

		PFNtQueryInformationProcess pfNtQueryInformationProcess = (PFNtQueryInformationProcess)::GetProcAddress(::GetModuleHandleW(L"ntdll.dll"), "NtQueryInformationProcess");
		if (NULL == pfNtQueryInformationProcess)
			break;

		PROCESS_EXTENDED_BASIC_INFORMATION pebi = { 0 };
		pebi.Size = sizeof(pebi);

		NTSTATUS status = pfNtQueryInformationProcess(hProcess, ProcessBasicInformation, &pebi, sizeof(pebi), NULL);
		if (!NT_SUCCESS(status))
			break;

		f = (pebi.IsStronglyNamed);

	} while (false);

	if (hProcess)
		::CloseHandle(hProcess);

	return f;
}

bool ProcessAPI::IsActiveProcess(HANDLE hProcess)
{
	bool f = false;

	DWORD dwExitCode;
	if (::GetExitCodeProcess(hProcess, &dwExitCode))
		f = (STILL_ACTIVE == dwExitCode);
	else
		throw new Win32Exception(::GetLastError());

	return f;
}

bool ProcessAPI::LaunchProcessAsUser(string strFilename, string strParameter, string strCurrentDirectory, string strDesktop, PROCESS_INFORMATION *pPI)
{
	bool f = false;

	string strCommandLine;
	if (strParameter.IsEmpty())
		strCommandLine = String::Format(L"\"%ws\"", (const wchar_t*)strFilename);
	else
		strCommandLine = String::Format(L"\"%ws\" %ws", (const wchar_t*)strFilename, (const wchar_t*)strParameter);

	_TRACE_I(Application, L"launch = %ws", (const wchar_t*)strCommandLine);

	HANDLE hToken = Library::API::SecurityAPI::GetLogonUserToken();

	DWORD dwCreationFlags = 0;
	f = ProcessAPI::LaunchProcessWithToken(strFilename, strCommandLine, strCurrentDirectory, hToken, 0, strDesktop, dwCreationFlags, pPI);

	if (hToken)
		::CloseHandle(hToken);

	return f;
}

bool ProcessAPI::LaunchProcessAsElevatedUser(string strFilename, string strParameter, string strCurrentDirectory, string strDesktop, PROCESS_INFORMATION *pPI)
{
	bool f = false;

	string strCommandLine;
	if (strParameter.IsEmpty())
		strCommandLine = String::Format(L"\"%ws\"", (const wchar_t*)strFilename);
	else
		strCommandLine = String::Format(L"\"%ws\" %ws", (const wchar_t*)strFilename, (const wchar_t*)strParameter);
	
	_TRACE_I(Application, L"launch = %ws", (const wchar_t*)strCommandLine);

	HANDLE hToken = Library::API::SecurityAPI::GetLogonUserElevatedToken();

	DWORD dwCreationFlags = 0;
	f = ProcessAPI::LaunchProcessWithToken(strFilename, strCommandLine, strCurrentDirectory, hToken, 0, strDesktop, dwCreationFlags, pPI);

	if (hToken)
		::CloseHandle(hToken);

	return f;
}

bool ProcessAPI::LaunchProcessAsLocalSystem(string strFilename, string strParameter, string strCurrentDirectory, string strDesktop, PROCESS_INFORMATION *pPI)
{
	bool f = false;

	string strCommandLine;
	if (strParameter.IsEmpty())
		strCommandLine = String::Format(L"\"%ws\"", (const wchar_t*)strFilename);
	else
		strCommandLine = String::Format(L"\"%ws\" %ws", (const wchar_t*)strFilename, (const wchar_t*)strParameter);
	
	_TRACE_I(Application, L"launch as system = %ws", (const wchar_t*)strCommandLine);

	HANDLE hToken = Library::API::SecurityAPI::GetLocalSystemTokenFromActiveSession();
	if (NULL == hToken)
		hToken = Library::API::SecurityAPI::GetLocalSystemToken();

	DWORD dwCreationFlags = 0;
	f = LaunchProcessWithToken(strFilename, strCommandLine, strCurrentDirectory, hToken, 0, strDesktop, dwCreationFlags, pPI);

	if (hToken)
		::CloseHandle(hToken);

	return f;
}

bool ProcessAPI::LaunchProcessWithToken(string strApplicationName, string strCommandLine, string strCurrentDirectory, HANDLE hToken, int nNewParentPid, string strDesktop, DWORD dwCreationFlags, PROCESS_INFORMATION* pPI)
{
	bool f = true;
	if (NULL == pPI)
		throw new ArgumentNullException;

	HMODULE hModule = NULL;
	PFInitializeProcThreadAttributeList pfInitializeProcThreadAttributeList = NULL;
	PFDeleteProcThreadAttributeList pfDeleteProcThreadAttributeList = NULL;
	PFUpdateProcThreadAttribute pfUpdateProcThreadAttribute = NULL;

	if (Version(6, 0) <= Environment::GetOSVersion()->GetVersion())
	{
		hModule = ::LoadLibrary(L"kernel32.dll");
		if (hModule)
		{
			pfInitializeProcThreadAttributeList = (PFInitializeProcThreadAttributeList)::GetProcAddress(hModule, "InitializeProcThreadAttributeList");
			pfDeleteProcThreadAttributeList = (PFDeleteProcThreadAttributeList)::GetProcAddress(hModule, "DeleteProcThreadAttributeList");
			pfUpdateProcThreadAttribute = (PFUpdateProcThreadAttribute)::GetProcAddress(hModule, "UpdateProcThreadAttribute");
		}
	}

	bool fUseExtendedStartupInfo = false;
	STARTUPINFO si = {0};
	si.cb = sizeof(si);
	STARTUPINFOEXW sie = {0};
	sie.StartupInfo.cb = sizeof(STARTUPINFOEXW);
	if (!strDesktop.IsEmpty())
	{
		if (!strDesktop.ToLower().StartsWith(L"winsta0\\"))
			strDesktop = L"WinSta0\\" + strDesktop;

		si.lpDesktop = strDesktop;
		sie.StartupInfo.lpDesktop = strDesktop;
	}

	HANDLE hParentProcess = NULL;
	LPVOID lpEnvironment = NULL;
	LPWSTR lpApplicationName = strApplicationName;
	LPWSTR lpCommandLine = strCommandLine;
	LPWSTR lpCurrentDirectory = NULL;
	if (!strCurrentDirectory.IsEmpty())
		lpCurrentDirectory = strCurrentDirectory;

	if (0 != nNewParentPid && Version(6, 0) <= Environment::GetOSVersion()->GetVersion())
	{
		hParentProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, nNewParentPid);
		if (NULL != hParentProcess)
			fUseExtendedStartupInfo = true;
	}

	try
	{
		if (hToken)
		{
			if (NULL == lpEnvironment)
			{
				if (!::CreateEnvironmentBlock(&lpEnvironment, hToken, TRUE))
					throw new Win32Exception(::GetLastError());
				dwCreationFlags |= CREATE_UNICODE_ENVIRONMENT;
			}

			array<byte> abAL;
			if (fUseExtendedStartupInfo)
			{
				SIZE_T sizeAL = 0;
				pfInitializeProcThreadAttributeList(NULL, 1, 0, &sizeAL);
				abAL = array<byte>((int)sizeAL);

				sie.lpAttributeList = (LPPROC_THREAD_ATTRIBUTE_LIST)abAL.GetBuffer();
				if (pfInitializeProcThreadAttributeList && !pfInitializeProcThreadAttributeList(sie.lpAttributeList, 1, 0, &sizeAL))
					throw new Win32Exception(::GetLastError());
				if (pfUpdateProcThreadAttribute && !pfUpdateProcThreadAttribute(sie.lpAttributeList, 0, PROC_THREAD_ATTRIBUTE_PARENT_PROCESS, &hParentProcess, sizeof(hParentProcess), NULL, NULL))
					throw new Win32Exception(::GetLastError());

				dwCreationFlags |= EXTENDED_STARTUPINFO_PRESENT;
			}

			if (!::CreateProcessAsUserW(
				hToken,
				lpApplicationName,
				lpCommandLine,
				NULL,
				NULL,
				FALSE,
				dwCreationFlags,
				lpEnvironment,
				lpCurrentDirectory,
				(fUseExtendedStartupInfo ? (LPSTARTUPINFOW)&sie : &si),
				pPI
				))
				throw new Win32Exception(::GetLastError());
		}
		else
		{
			if (!::CreateProcessW(
				lpApplicationName,
				lpCommandLine,
				NULL,
				NULL,
				FALSE,
				dwCreationFlags,
				lpEnvironment,
				lpCurrentDirectory,
				&si,
				pPI
				))
				throw new Win32Exception(::GetLastError());
		}
	}
	catch (Exception* pe)
	{
		_TRACE_EX(pe);
		delete pe;

		f = false;
	}

	if (lpEnvironment)
		::DestroyEnvironmentBlock(lpEnvironment);
	if (hParentProcess)
		::CloseHandle(hParentProcess);
	if (pfDeleteProcThreadAttributeList)
		pfDeleteProcThreadAttributeList(sie.lpAttributeList);
	if (hModule)
		::FreeLibrary(hModule);

	return f;
}

void ProcessAPI::ShellExecuteExtension(string strOp, string strFile, string strParam, string strDir)
{
	_TRACE_I(Application, L"operation = %ws", (const wchar_t*)strOp);
	_TRACE_I(Application, L"filename = %ws", (const wchar_t*)strFile);
	_TRACE_I(Application, L"parameter = %ws", (const wchar_t*)strParam);
	_TRACE_I(Application, L"working directory = %ws", (const wchar_t*)strDir);

	try
	{
		if (FileInfo(strFile).Exists() && strFile.ToLower().EndsWith(L".lnk"))
		{
			wchar_t tszCompCode[MAX_FEATURE_CHARS+1];
			wchar_t tszProdCode[MAX_FEATURE_CHARS+1];
			wchar_t szTargetPath[MAX_PATH];

			tszCompCode[0] = L'\0';
			tszProdCode[0] = L'\0';
			szTargetPath[0] = L'\0';

			if (ERROR_SUCCESS == MsiGetShortcutTarget((const wchar_t*)strFile, tszProdCode, NULL, tszCompCode))
			{
				DWORD dw = MAX_PATH;
				if (INSTALLSTATE_LOCAL == MsiGetComponentPath(tszProdCode, tszCompCode, szTargetPath, &dw))
					strFile = szTargetPath;
			}
			else
			{
				Shortcut sc;
				sc.Load(strFile);
				strFile = sc.GetPath();
				strParam = sc.GetArguments();
				strDir = sc.GetWorkingDirectory();
			}
		}

		if (!strFile.IsEmpty())
			strFile = Environment::ExpandEnvironmentVariables(strFile);
		if (!strParam.IsEmpty())
			strParam = Environment::ExpandEnvironmentVariables(strParam);
		if (!strDir.IsEmpty())
			strDir = Environment::ExpandEnvironmentVariables(strDir);
		
		if (FILE_ATTRIBUTE_DIRECTORY & ::GetFileAttributes(strFile))
		{
			strOp = L"open";
			strParam = strFile;
			strDir = Environment::GetFolderPath(ESpecialFolder::Windows);
			strFile = strDir + L"\\explorer.exe";
		}

		if (strDir.IsEmpty() && !strFile.IsEmpty())
		{
			int nLastIndex = strFile.LastIndexOf(L'\\');
			if (0 < nLastIndex)
				strDir = strFile.Substring(0, nLastIndex);
			else
				strDir = Environment::GetCurrentDirectory();
		}

		//if (strFile.ToLower().Contains(L"chrome.exe"))
		//{
		//	if (CheckMirageWorksVirtualZoneProcess(GetCurrentProcessId()))
		//	{
		//		string strPublisher = Application::GetPublisherFromCertification(strFile);
		//		if (strPublisher.ToLower().Equals(L"google inc"))
		//		{
		//			if (ServiceInstaller::Exists(L"SepMasterService") || ServiceInstaller::Exists(L"NS"))
		//			{
		//				if (strParam.Contains(L"--no-sandbox"))
		//				{

		//				}
		//				else if (strParam.Contains(L"-- "))
		//				{
		//					strParam = strParam.Replace(L"-- ", L"--no-sandbox ");
		//				}
		//				else
		//				{
		//					strParam += L"--no-sandbox";
		//				}
		//			}
		//		}
		//	}
		//}

		if (strFile.EndsWith(L"CBUpdater.exe", true))
		{
			strParam = strFile;
			strDir = Environment::GetFolderPath(ESpecialFolder::Windows);
			strFile = strDir + L"\\explorer.exe";

			STARTUPINFOW si = { 0 };
			PROCESS_INFORMATION pi = { 0 };
			si.cb = sizeof(si);

			string strCommandLine = String::Format(L"\"%ws\" \"%ws\"", (const wchar_t*)strFile, (const wchar_t*)strParam);

			if (!::CreateProcessW(
				NULL,
				strCommandLine,
				NULL,
				NULL,
				FALSE,
				0,
				NULL,
				NULL,
				&si,
				&pi))
				throw new Win32Exception(::GetLastError());

			::CloseHandle(pi.hProcess);
			::CloseHandle(pi.hThread);
		}
		else
		{
			::ShellExecuteW(
				0,
				(const wchar_t*)strOp,
				(const wchar_t*)strFile,
				(const wchar_t*)strParam,
				(const wchar_t*)strDir,
				SW_SHOWNORMAL
				);
		}
	}
	catch (Exception *pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}
}

void ProcessAPI::ShellExecuteAsAdministrator(string strOp, string strFile, string strParam, string strDir)
{
	if (Version(6, 0) <= Environment::GetOSVersion()->GetVersion())
	{
		try
		{
			if (FileInfo(strFile).Exists() && strFile.ToLower().EndsWith(L".lnk"))
			{
				wchar_t tszCompCode[MAX_FEATURE_CHARS + 1];
				wchar_t tszProdCode[MAX_FEATURE_CHARS + 1];
				wchar_t szTargetPath[MAX_PATH];

				tszCompCode[0] = L'\0';
				tszProdCode[0] = L'\0';
				szTargetPath[0] = L'\0';

				if (ERROR_SUCCESS == MsiGetShortcutTarget((const wchar_t*)strFile, tszProdCode, NULL, tszCompCode))
				{
					DWORD dw = MAX_PATH;
					if (INSTALLSTATE_LOCAL == MsiGetComponentPath(tszProdCode, tszCompCode, szTargetPath, &dw))
						strFile = szTargetPath;
				}
				else
				{
					Shortcut sc;
					sc.Load(strFile);
					strFile = sc.GetPath();
					strParam = sc.GetArguments();
					strDir = sc.GetWorkingDirectory();
				}
			}

			if (!strFile.IsEmpty())
				strFile = Environment::ExpandEnvironmentVariables(strFile);
			if (!strParam.IsEmpty())
				strParam = Environment::ExpandEnvironmentVariables(strParam);
			if (!strDir.IsEmpty())
				strDir = Environment::ExpandEnvironmentVariables(strDir);


			if (FILE_ATTRIBUTE_DIRECTORY & ::GetFileAttributes(strFile))
			{
				strOp = L"open";
				strParam = strFile;
				strDir = Environment::GetFolderPath(ESpecialFolder::Windows);
				strFile = strDir + L"\\explorer.exe";
			}

			if (strDir.IsEmpty() && !strFile.IsEmpty())
			{
				int nLastIndex = strFile.LastIndexOf(L'\\');
				if (0 < nLastIndex)
					strDir = strFile.Substring(0, nLastIndex);
				else
					strDir = Environment::GetCurrentDirectory();
			}

			SHELLEXECUTEINFOW sei;
			::ZeroMemory(&sei, sizeof(sei));

			sei.cbSize = sizeof(SHELLEXECUTEINFOW);
			sei.hwnd = NULL;
			sei.fMask = SEE_MASK_FLAG_DDEWAIT | SEE_MASK_FLAG_NO_UI;
			sei.lpVerb = strOp.IsEmpty() ? L"runas" : strOp;
			sei.lpFile = strFile;
			sei.lpParameters = strParam;
			sei.nShow = SW_SHOW;
			sei.lpDirectory = (const wchar_t*)strDir;
			::ShellExecuteExW(&sei);

			::WaitForSingleObject(sei.hProcess, INFINITE);
			::CloseHandle(sei.hProcess);

		}
		catch (Exception *pe)
		{
			_TRACE_EX(pe);
			delete pe;
		}
	}
	else
		ProcessAPI::ShellExecuteExtension(strOp, strFile, strParam, strDir);
}

void ProcessAPI::TerminateServiceProcess(string strServiceName)
{
	SERVICE_STATUS_PROCESS ssp;
	SC_HANDLE hSCManager = NULL;
	SC_HANDLE hSCService = NULL;
	HANDLE hProcess = NULL;
	DWORD needed;

	try
	{
		if (NULL == (hSCManager = ::OpenSCManager(
			NULL, 
			NULL, 
			SC_MANAGER_ALL_ACCESS
			)))
			throw new Win32Exception(::GetLastError());

		if (NULL == (hSCService = ::OpenServiceW(
			hSCManager, 
			(const wchar_t*)strServiceName, 
			SERVICE_ALL_ACCESS
			)))
			throw new Win32Exception(::GetLastError());

		if (!QueryServiceStatusEx(
			hSCService, 
			SC_STATUS_PROCESS_INFO,
			(LPBYTE) &ssp, 
			sizeof(ssp), 
			&needed))
			throw new Win32Exception(::GetLastError());

		if (NULL == (hProcess = ::OpenProcess(
			PROCESS_TERMINATE,
			FALSE,
			ssp.dwProcessId
			)))
			throw new Win32Exception(::GetLastError());

		if (!TerminateProcess(hProcess, 0))
			throw new Win32Exception(::GetLastError());
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

	if (hProcess != null)
		CloseHandle(hProcess);
	if (hSCService != null)
		CloseServiceHandle(hSCService); 
	if (hSCManager != null)
		CloseServiceHandle(hSCManager);
}

bool ProcessAPI::CheckNeedElevation(string strPath)
{
	bool fElevation = false;

	HMODULE hModule = ::LoadLibrary(L"kernel32.dll");
	if (hModule)
	{
		PCheckElevation pfCheckElevation = (PCheckElevation)::GetProcAddress(hModule, "CheckElevation");

		if (null != pfCheckElevation)
		{
			ULONG uFlag = 0;
			ULONG uOutput1 = 0;

			if (ERROR_SUCCESS == pfCheckElevation(strPath, &uFlag, NULL, &uOutput1, NULL))
			{
				if (0 < uOutput1)
					fElevation = true;
			}
		}

		::FreeLibrary(hModule);
	}

	return fElevation;
}

bool ProcessAPI::CheckDoosanDlpRunning()
{
	bool fEDPA = false, fWDP = false;

	HANDLE hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 entry;
		memset(&entry, 0, sizeof(PROCESSENTRY32));
		entry.dwSize = sizeof(PROCESSENTRY32) ;
		BOOL b = ::Process32First(hSnapshot, &entry);
		while(b)
		{
			string strExe = string(entry.szExeFile);
			if (strExe.ToLower() == L"edpa.exe")
				fEDPA = true;
			else if (strExe.ToLower() == L"wdp.exe")
				fWDP = true;
			entry.dwSize = sizeof(PROCESSENTRY32) ;
			b = ::Process32Next(hSnapshot, &entry);
		}
		::CloseHandle(hSnapshot);
	}

	return (fEDPA && fWDP);
}

void ProcessAPI::EnableServices(string strServiceName)
{
	RegistryKey *pKey = NULL;
	try
	{
		pKey = Registry::GetLocalMachine()->OpenSubKey(L"SYSTEM\\CurrentControlSet\\Services\\" + strServiceName, true);
		if (null != pKey)
			pKey->SetValue(L"Start", (uint32)SERVICE_DEMAND_START);
	}
	catch (Exception* px)
	{
		_TRACE_EX(px);
		delete px;
	}
	if (pKey != NULL)
		delete pKey;
}

void ProcessAPI::DisableServices(string strServiceName)
{
	RegistryKey *pKey = NULL;
	try
	{
		pKey = Registry::GetLocalMachine()->OpenSubKey(L"SYSTEM\\CurrentControlSet\\Services\\" + strServiceName, true);
		if (null != pKey)
			pKey->SetValue(L"Start", (uint32)SERVICE_DISABLED);
	}
	catch (Exception* px)
	{
		_TRACE_EX(px);
		delete px;
	}
	if (pKey != NULL)
		delete pKey;
}

bool ProcessAPI::StartServiceByName(string strServiceName)
{
	SC_HANDLE hSCManager = NULL;
	SC_HANDLE hSCService = NULL;

	bool fStartService = true;

	try
	{
		if (NULL == (hSCManager = ::OpenSCManager(
			NULL, 
			NULL, 
			SC_MANAGER_ALL_ACCESS
			)))
			throw new Win32Exception(::GetLastError(), Convert::ToString(__LINE__));

		if (NULL == (hSCService = ::OpenServiceW(
			hSCManager, 
			strServiceName, 
			SERVICE_START
			)))
			throw new Win32Exception(::GetLastError(), Convert::ToString(__LINE__));

		::SetLastError(0);
		if (!::StartService(hSCService, 0, NULL) && ::GetLastError() != ERROR_SERVICE_MARKED_FOR_DELETE)
			throw new Win32Exception(::GetLastError(), Convert::ToString(__LINE__));
	}
	catch (Exception *pe)
	{
		fStartService = false;
		_TRACE_EX(pe);
		delete pe;
	}
	catch(...)
	{
		fStartService = false;
		_TRACE_EX_UNK();
	}

	if (hSCService)
		::CloseServiceHandle(hSCService);
	if (hSCManager)
		::CloseServiceHandle(hSCManager);

	return fStartService;
}

bool ProcessAPI::StopServiceByName(string strServiceName)
{
	SC_HANDLE hSCManager = NULL;
	SC_HANDLE hSCService = NULL;

	bool fStopService = true;

	try
	{
		if (NULL == (hSCManager = ::OpenSCManager(
			NULL, 
			NULL, 
			SC_MANAGER_ALL_ACCESS
			)))
			throw new Win32Exception(::GetLastError());

		if (NULL == (hSCService = ::OpenServiceW(
			hSCManager, 
			strServiceName, 
			SERVICE_ALL_ACCESS
			)))
			throw new Win32Exception(::GetLastError());

		SERVICE_STATUS ss;
		::QueryServiceStatus(hSCService, &ss);
		if (ss.dwCurrentState == SERVICE_RUNNING)
		{
			::ControlService(hSCService, SERVICE_CONTROL_STOP, &ss);
			int nCnt = 20;
			while (SERVICE_STOPPED != ss.dwCurrentState && nCnt-- > 0)
			{
				Sleep(100);
				::QueryServiceStatus(hSCService, &ss);
			}
		}
	}
	catch (Exception *pe)
	{
		fStopService = false;
		_TRACE_EX(pe);
		delete pe;
	}
	catch(...)
	{
		fStopService = false;
		_TRACE_EX_UNK();
	}

	if (hSCService)
		::CloseServiceHandle(hSCService);
	if (hSCManager)
		::CloseServiceHandle(hSCManager);

	return fStopService;
}

bool ProcessAPI::IsServiceRunning(string strServiceName)
{
	SC_HANDLE hSCManager = NULL;
	SC_HANDLE hSCService = NULL;

	bool fServiceRunning = false;

	try
	{
		if (NULL == (hSCManager = ::OpenSCManager(
			NULL,
			NULL,
			SC_MANAGER_ALL_ACCESS
			)))
			throw new Win32Exception(::GetLastError());

		if (NULL != (hSCService = ::OpenServiceW(
			hSCManager,
			strServiceName,
			SERVICE_QUERY_STATUS
			)))
		{
			SERVICE_STATUS ss;
			QueryServiceStatus(hSCService, &ss);
			if (ss.dwCurrentState == SERVICE_RUNNING)
				fServiceRunning = true;
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

	if (hSCService)
		::CloseServiceHandle(hSCService);
	if (hSCManager)
		::CloseServiceHandle(hSCManager);

	return fServiceRunning;
}

bool ProcessAPI::IsProcessRunning(string strProcessName)
{
	strProcessName = strProcessName.ToLower();
	bool fRunningProcess = false;
	try
	{
		HANDLE hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0) ;
		if (INVALID_HANDLE_VALUE == hSnapshot)
			throw new Win32Exception(::GetLastError());

		PROCESSENTRY32 entry;
		memset(&entry, 0, sizeof(PROCESSENTRY32));
		entry.dwSize = sizeof(PROCESSENTRY32) ;
		BOOL b = ::Process32First(hSnapshot, &entry);
		while(b)
		{
			string strExe = string(entry.szExeFile);
			if (strExe.ToLower().Contains(strProcessName))
			{
				HANDLE hProcess = ::OpenProcess(PROCESS_TERMINATE, FALSE, entry.th32ProcessID);
				if (INVALID_HANDLE_VALUE != hProcess && NULL != hProcess)
				{
					//Debug_WriteLine(L"%ws Process Is Running", (const wchar_t*)strProcessName);
					::CloseHandle(hProcess);
					fRunningProcess = true;
					b = false;
					break;
				}
			}
			entry.dwSize = sizeof(PROCESSENTRY32) ;
			b = ::Process32Next(hSnapshot, &entry);
		}
		::CloseHandle(hSnapshot);
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

	return fRunningProcess;
}

void ProcessAPI::ExecuteCmdLine(string strCmdLine, int nWaitMilliSecond /* = 0 */)
{
	_TRACE_I(Application, L"cmd = %ws", (const wchar_t*)strCmdLine);

	STARTUPINFOW si = {0};
	PROCESS_INFORMATION pi = {0};
	memset(&si, 0, sizeof(si));
	memset(&pi, 0, sizeof(pi));
	si.cb = sizeof(si);

	do 
	{
		if (!::CreateProcessW(
			NULL,
			strCmdLine,
			NULL,
			NULL,
			FALSE,
			CREATE_NO_WINDOW,
			NULL,
			NULL,
			&si,
			&pi
			))
			break;

		if (nWaitMilliSecond == 0)
			::WaitForSingleObject(pi.hProcess, INFINITE);
		else
		{
			while (WAIT_TIMEOUT == ::WaitForSingleObject(pi.hProcess, nWaitMilliSecond))
			{
				::TerminateProcess(pi.hProcess, 0);
				break;
			}
		}

		::CloseHandle(pi.hProcess);
		::CloseHandle(pi.hThread);

	} while (FALSE);
}

string ProcessAPI::GetCurControlSetString()
{
	RegistryKey* pKey = null;
	string strCurControlSet = L"ControlSet001";
	try
	{
		pKey = Registry::GetLocalMachine()->OpenSubKey(L"SYSTEM\\Select");
		if (pKey)
		{
			uint32 nCurNum = 0;
			pKey->GetValue(L"Current", nCurNum);
			strCurControlSet = String::Format(L"ControlSet00%d", nCurNum);
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
	if (pKey)
		delete pKey;

	return strCurControlSet;
}

bool ProcessAPI::ExecutePowerShell(string strArg)
{
	bool fRet = false;

	string strCommand = L"";
	string strPowerShellPath = L"";
	
	
	RegistryKey *pKey = NULL;

	try
	{
		pKey = Registry::GetLocalMachine()->OpenSubKey(L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment");

		if (NULL != pKey)
		{
			pKey->GetValue(L"PSModulePath", strPowerShellPath);
		}
	}
	catch (Exception *pe)
	{
		_TRACE_EX(pe);
		delete pe;
	}

	if (NULL != pKey)
	{
		delete pKey;
		pKey = NULL;
	}
	
	if (strPowerShellPath.IsEmpty() || INVALID_FILE_ATTRIBUTES == ::GetFileAttributesW(strPowerShellPath))
	{
		try
		{
			pKey = Registry::GetLocalMachine()->OpenSubKey(L"SYSTEM\\%ws\\Control\\Session Manager\\Environment", 
				(const wchar_t*)ProcessAPI::GetCurControlSetString());

			if (NULL != pKey)
			{
				pKey->GetValue(L"PSModulePath", strPowerShellPath);
			}
		}
		catch (Exception *pe)
		{
			_TRACE_EX(pe);
			delete pe;
		}
	}

	do 
	{
		string strRealPath = L"";
		array<string> astrPowerShellPath = strPowerShellPath.Split(String(L";").GetChars(), EStringSplitOptions::RemoveEmptyEntries);
		
		for (int i = 0; i < astrPowerShellPath.GetLength(); i++)
		{
			string strPath = Environment::ExpandEnvironmentVariables(astrPowerShellPath[i]) + L"\\powershell.exe";

			if (!strPath.IsEmpty() && INVALID_FILE_ATTRIBUTES != ::GetFileAttributesW(strPath))
			{
				strRealPath = strPath;
				break;
			}
		}
		
		if (strRealPath.IsEmpty())
			strRealPath = L"powershell.exe";

		strCommand = String::Format(L"\"%ws\" \"%ws\"", (const wchar_t*)strRealPath, (const wchar_t*)strArg);
		ExecuteCmdLine(strCommand);

		fRet = true;

	} while (FALSE);

	return fRet;
}

bool ProcessAPI::ExecuteGroupUpdate()
{
	bool fRet = false;

	string strCommand = L"";
	string strGroupUpdatePath = Environment::GetFolderPath(ESpecialFolder::System) + L"\\gpupdate.exe";

	do 
	{
		if (INVALID_FILE_ATTRIBUTES == ::GetFileAttributesW(strGroupUpdatePath))
			strGroupUpdatePath = L"gpupdate.exe";

		strCommand = String::Format(L"\"%ws\" /force /wait:100", (const wchar_t*)strGroupUpdatePath);
		ExecuteCmdLine(strCommand, 10);

		fRet = true;

	} while (FALSE);

	return fRet;
}

bool ProcessAPI::IsExecutableFile(const string& strPath)
{
	bool f = false;

	HANDLE hFile = INVALID_HANDLE_VALUE;
	HANDLE hFileMap = NULL;
	LPBYTE lpMapView = NULL;

	try
	{
		hFile = ::CreateFileW(strPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 0, NULL);
		if (INVALID_HANDLE_VALUE == hFile)
			throw new Win32Exception(::GetLastError());

		hFileMap = ::CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
		if (INVALID_HANDLE_VALUE == hFileMap)
			throw new Win32Exception(::GetLastError());

		lpMapView = (LPBYTE)::MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 0);
		if (!lpMapView)
			throw new Win32Exception(::GetLastError());

		DWORD dwFileSize = ::GetFileSize(hFile, NULL);
		if (INVALID_FILE_SIZE == dwFileSize)
			throw new Win32Exception(::GetLastError());

		do
		{
			if (0 == dwFileSize)
				break;

			array<byte> abMapView(lpMapView, 0, dwFileSize);

			PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)(&abMapView[0]);
			if (IMAGE_DOS_SIGNATURE != pDosHeader->e_magic)
				break;

			PIMAGE_NT_HEADERS pNtHeader = (PIMAGE_NT_HEADERS)(&abMapView[pDosHeader->e_lfanew]);
			if (IMAGE_NT_SIGNATURE != pNtHeader->Signature)
				break;

			if (IMAGE_FILE_DLL & pNtHeader->FileHeader.Characteristics)
				break;

			f = (IMAGE_SUBSYSTEM_WINDOWS_GUI == pNtHeader->OptionalHeader.Subsystem ||
				IMAGE_SUBSYSTEM_WINDOWS_CUI == pNtHeader->OptionalHeader.Subsystem);

		} while (false);
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

	if (NULL != lpMapView)
		::UnmapViewOfFile(lpMapView);

	if (NULL != hFileMap)
		::CloseHandle(hFileMap);

	if (INVALID_HANDLE_VALUE != hFile)
		::CloseHandle(hFile);

	return f;
}

void ProcessAPI::SetServiceRecovery(const string& strServiceName, bool fEnable)
{
	SC_HANDLE hScm = NULL;
	SC_HANDLE hSvc = NULL;

	try
	{
		hScm = ::OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if (NULL == hScm)
			throw new Win32Exception(::GetLastError());

		hSvc = ::OpenServiceW(hScm, strServiceName, SERVICE_ALL_ACCESS);
		if (NULL == hSvc)
			throw new Win32Exception(::GetLastError());

		#define _NUM_ACTIONS 3
		SC_ACTION sa[_NUM_ACTIONS];
		for (int n = 0; n < _NUM_ACTIONS; n++)
		{
			sa[n].Delay = 1000;
			sa[n].Type = (fEnable ? SC_ACTION_RESTART : SC_ACTION_NONE);
		}

		SERVICE_FAILURE_ACTIONSW sfa = { 0 };
		sfa.dwResetPeriod = 1 * 60 * 60 * 1000;
		sfa.lpRebootMsg = L"";
		sfa.lpCommand = L"";
		sfa.cActions = _NUM_ACTIONS;
		sfa.lpsaActions = (SC_ACTION*)sa;

		if (!::ChangeServiceConfig2W(hSvc, SERVICE_CONFIG_FAILURE_ACTIONS, &sfa))
			throw new Win32Exception(::GetLastError());
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

	if (hSvc)
		::CloseServiceHandle(hSvc);
	if (hScm)
		::CloseServiceHandle(hScm);
}

uint64 ProcessAPI::GetProcessCreatedTime(uint32 nPid)
{
	uint64 nCreatedTime = 0;
	
	HANDLE hProcess = NULL;

	try
	{
		hProcess = ::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, nPid);
		if (NULL == hProcess)
			throw new Win32Exception(::GetLastError());

		FILETIME ftCreation, ftExit, ftKernel, ftUser;
		if (!::GetProcessTimes(hProcess, &ftCreation, &ftExit, &ftKernel, &ftUser))
			throw new Win32Exception(::GetLastError());

		ULARGE_INTEGER li;
		li.LowPart = ftCreation.dwLowDateTime;
		li.HighPart = ftCreation.dwHighDateTime;

		nCreatedTime = li.QuadPart;
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

	if (hProcess)
		::CloseHandle(hProcess);

	return nCreatedTime;
}

}
}