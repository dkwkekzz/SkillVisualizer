#include "stdafx.h"
#include "SymServer.h"
#include "CallMonitor.h"

//MonitorOption
static MonitorOption serverState;

//
// export
//
//*********************************************************************************************

void EnterSymbol(const char* szCallee)
{
	CallMonitor::threadObj().Push(serverState, szCallee);
}

void LeaveSymbol(unsigned long long ret)
{
	CallMonitor::threadObj().Pop(serverState, ret);
}

const char* CurrentSymbol()
{
	return CallMonitor::threadObj().Top();
}

void SetMonitorState(MonitorOption op)
{
	serverState = op;
	CallMonitor::threadObj().SetState(serverState);
}

void DisplayProfileData()
{
	CallMonitor::threadObj().Action(ActType::PrintProfile);
}

void DisplayLog()
{
	CallMonitor::threadObj().Action(ActType::PrintLog);
}

void Flush()
{
	CallMonitor::threadObj().Flush();
}

void Close()
{
	CallMonitor::threadObj().Close();
}
