#pragma once
#include <ResourcePack/ResourcePack.h>
#include <ResourcePack/Module_Common.h>
#include <ResourcePack/Module_Console.h>

class RESOURCEPACK_EXPORT Resource_Image
{
public:
	static QString GetImagePath(Module_Common::ImageType type)			{ return GetImagePath(Module_Common::GetImagePath(type)); }
	static QString GetImagePath(Module_Console::ImageType type)			{ return GetImagePath(Module_Console::GetImagePath(type)); }
private:
	static QString GetImagePath(QString qstrImage);
};

#define GET_IMAGE_PATH(x) Resource_Image::GetImagePath(x)