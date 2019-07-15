#ifndef __SYMSERVERDLL__
#define __SYMSERVERDLL__
#include "MonitorOption.h"


#ifdef SYMBOLSERVER_EXPORTS
	#define MYEXPORT __declspec( dllexport )
#else
	#define MYEXPORT __declspec( dllimport )
#endif

//
// export
//

extern"C" void MYEXPORT EnterSymbol(const char* szCallee);
extern"C" void MYEXPORT LeaveSymbol(unsigned long long ret);
extern"C" const char* MYEXPORT CurrentSymbol();

extern"C" void MYEXPORT SetMonitorState( MonitorOption op );
extern"C" void MYEXPORT DisplayProfileData(void);
extern"C" void MYEXPORT DisplayLog(void);
extern"C" void MYEXPORT Flush(void);
extern"C" void MYEXPORT Close(void);

//
// static
//

void InitSymbols(void* pAddress);

#endif
