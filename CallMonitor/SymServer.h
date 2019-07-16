#ifndef __SYMSERVERDLL__
#define __SYMSERVERDLL__
#include "MonitorOption.h"


#ifdef CALLMONITOR_EXPORTS
	#define MYEXPORT __declspec( dllexport )
#else
	#define MYEXPORT __declspec( dllimport )
#endif

//
// export
//

extern "C" MYEXPORT void EnterSymbol(const char* szCallee, const char* szCaller = nullptr);
extern "C" MYEXPORT void LeaveSymbol(unsigned long long ret);
extern "C" MYEXPORT const char* CurrentSymbol();
	   
extern "C" MYEXPORT void SetMonitorState(MonitorOption op);
//extern "C" MYEXPORT void DisplayProfileData(void);
//extern "C" MYEXPORT void DisplayLog(void);
extern "C" MYEXPORT void Execute(const char* cmd);
extern "C" MYEXPORT void Flush(void);
extern "C" MYEXPORT void Close(void);

//
// static
//

void InitSymbols(void* pAddress);

#endif
