#pragma once

#include <Light/System/all.h>
#include <Light/System/IO/all.h>
#include <Light/System/Text/all.h>
#include <Light/System/Diagnostics/all.h>
#include <Light/System/Collections/Generic/all.h>
#include <Light/System/Security/Cryptography/all.h>

#define LConsole_WriteLine(x, ...)	LConsole::WriteLine(LString::Format(x, __VA_ARGS__))