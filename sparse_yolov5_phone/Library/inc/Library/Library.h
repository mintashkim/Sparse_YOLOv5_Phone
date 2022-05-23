#pragma once

#include <Library/ErrorCode.h>
#include <Library/ProductException.h>

#include <Library/WindowProfile.h>

#include <Library/XmlObject2.h>
#include "../Base/Defines.h"

using namespace Library;

#include <Library/API/all.h>

#ifndef MW_LIBRARY_IMPL
#ifdef _DEBUG
#pragma comment (lib, "abgLibraryd.lib")
#else
#pragma comment (lib, "abgLibrary.lib")
#endif
#endif