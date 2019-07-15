// dllmain.cpp : DLL 응용 프로그램의 진입점을 정의합니다.
#include "stdafx.h"
#include "SymServer.h"
#include "CallMonitor.h"


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
#if _DEBUG
		::SetMonitorState(MonitorOption::RealTime | MonitorOption::Display);
#else
		::SetMonitorState(MonitorOption::Log);
#endif
		//CallMonitor::threadAttach(new CallMonitor);

		cout << "[callmonitor] open...!" << endl;
		break;
	}
	case DLL_THREAD_ATTACH:
	{
		//CallMonitor::threadAttach(new CallMonitor);
		break;
	}
	case DLL_PROCESS_DETACH:
	{
		//CallMonitor::threadDetach();

		Close();

		cout << "[callmonitor] close...!" << endl;
		break;
	}
	case DLL_THREAD_DETACH:
	{
		//CallMonitor::threadDetach();
		break;
	}
	break;
	}
	return TRUE;
}

