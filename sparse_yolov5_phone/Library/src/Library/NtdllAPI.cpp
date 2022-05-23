#include "stdafx.h"
#include <Library/NtdllAPI.h>

#include <wpp.h>
#include "NtdllAPI.tmh"

NtdllAPI::NtdllAPI(void)
{
	HMODULE hModule = ::LoadLibraryW(L"ntdll.dll");
	if (NULL == hModule)
		throw new Win32Exception(::GetLastError());

	NtQuerySystemInformation = (PFNtQuerySystemInformation)::GetProcAddress(hModule, "NtQuerySystemInformation");
}

NtdllAPI::~NtdllAPI(void)
{
}

NtdllAPI* NtdllAPI::GetInstance()
{
	static NtdllAPI _ntdllAPI;
	return &_ntdllAPI;
}
