// dllmain.cpp : DLL 응용 프로그램의 진입점을 정의합니다.
#define PLH_HIDE_DEBUG_MESSAGES 1
#include "stdafx.h"
#include "HookHelper.h"
#include <iostream>
#include <fstream>


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
	{
		::HookOpen(hModule);

		std::wofstream ofile("hooklog.txt");

		::CollectSpecialized(ofile);
		::Collect(ofile, "symbols");

		ofile.close();

		std::cout << "[hookman] open...!" << std::endl;
		break;
	}
    case DLL_THREAD_ATTACH:
		break;
    case DLL_THREAD_DETACH:
		break;
    case DLL_PROCESS_DETACH:
	{
		::HookClose();

		std::cout << "[hookman] close..." << std::endl;
		break;
	}
    }
    return TRUE;
}

