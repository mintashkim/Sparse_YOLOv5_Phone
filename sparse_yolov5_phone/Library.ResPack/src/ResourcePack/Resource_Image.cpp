#include "StdAfx.h"
#include <ResourcePack/Resource_Image.h>

QString Resource_Image::GetImagePath(QString qstrImage)
{
	QString qstrPath_1 = ":/" + qstrImage;
	QString qstrPath_2 = ":/custom/" + qstrImage;

	if (QFile::exists(qstrPath_2))
		return qstrPath_2;
	else
		return qstrPath_1;
}