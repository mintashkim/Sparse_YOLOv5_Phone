#include "stdafx.h"
#include "ver_maker.h"
#include "file_deploy.h"

int main(int argc, char *argv[])
{
	int nRet = 1;
	if (argc >= 2)
	{
		lstring strArg1(argv[1]);
		if (strArg1.Equals(L"/version", true))
		{
			LConsole_WriteLine(L"Version Maker");

			ver_maker vm;
			nRet = vm.make();
		}
		else if (strArg1.Equals(L"/deploy", true))
		{
			LConsole_WriteLine(L"File deploy");

			file_deploy fd;
			nRet = fd.deploy();
		}
	}
	return nRet;
}
