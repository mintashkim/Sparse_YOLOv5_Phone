#pragma once

#ifdef RESOURCEPACK_LIB

#ifndef RESOURCEPACK_EXPORT
#define RESOURCEPACK_EXPORT _declspec(dllexport)
#endif

#else

#ifndef RESOURCEPACK_EXPORT
#define RESOURCEPACK_EXPORT _declspec(dllimport)
#endif

#ifdef _DEBUG
#pragma comment(lib, "abgresd.lib")
#else
#pragma comment(lib, "abgres.lib")
#endif

#endif

#include <ResourcePack/Resource_Lang.h>
#include <ResourcePack/Resource_Image.h>
#include <ResourcePack/Resource_Style.h>

namespace ResourcePack
{
	RESOURCEPACK_EXPORT bool Initialize();
	RESOURCEPACK_EXPORT bool Finalize();
}