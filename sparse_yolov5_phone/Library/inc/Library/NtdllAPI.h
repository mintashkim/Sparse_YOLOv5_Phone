#pragma once

#include <Library/ntdll.h>
#include "ntstatus.h"

typedef NTSTATUS(NTAPI* PFNtQuerySystemInformation)(
	__in       SYSTEM_INFORMATION_CLASS SystemInformationClass,
	__inout    PVOID SystemInformation,
	__in       ULONG SystemInformationLength,
	__out_opt  PULONG ReturnLength
	);

class NtdllAPI
{
public:
	PFNtQuerySystemInformation NtQuerySystemInformation;

	static NtdllAPI* GetInstance();

protected:
	NtdllAPI(void);
	virtual ~NtdllAPI(void);
};

