#pragma once
#include <QtCore>
#include <QtGui>
#include <QtWidgets>

#include <Detect/all.h>

#include <System/all.h>
#include <System/Text/all.h>
#include <System/Threading/all.h>
#include <System/Diagnostics/all.h>
#include <System/ServiceProcess/all.h>
#include <System/Collections/Generic/all.h>
#include <System/Security/Cryptography/all.h>
#include <Microsoft/Win32/all.h>
#include <NoAD/IPC/all.h>
#include <NoAD/Win32/all.h>

#include <Library/Library.h>
#include <ResourcePack/ResourcePack.h>

#include "..\Base\Defines.h"

#define ConvertToQString(str)		(QString::fromUtf16((const ushort*)(const wchar_t*)(string&)str))
#define ConvertFromQString(qstr)	((const wchar_t*)qstr.utf16())
