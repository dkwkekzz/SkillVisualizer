#include "stdafx.h"
#include "SymServer.h"
#include "CallMonitor.h"

//MonitorOption
static MonitorOption serverState;

//
// export
//
//*********************************************************************************************

void EnterSymbol(const char* szCallee, const char* szCaller)
{
	CallMonitor::threadObj().Push(serverState, szCallee, szCaller);
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

//void DisplayProfileData()
//{
//	CallMonitor::threadObj().Action(ActType::PrintProfile);
//}
//
//void DisplayLog()
//{
//	CallMonitor::threadObj().Action(ActType::PrintLog);
//}

void Execute(const char* cmd)
{
	if (strlen(cmd) <= 0)
		return;

	std::string strCmd(cmd);
	// extract value
	std::string strValue;
	int pos = -1;
	if ((pos = strCmd.rfind(" ")) != std::string::npos)
	{
		strValue = strCmd.substr(pos + 1);
	}

	if ((pos = strCmd.find("-help")) != std::string::npos)
	{
		std::cout << "not implementation" << std::endl;
		return;
	}

	if ((pos = strCmd.find("-set ")) != std::string::npos)
	{
		int nop = atoi(strValue.c_str());
		SetMonitorState((MonitorOption)nop);
		return;
	}

	if ((pos = strCmd.find("-print profile")) != std::string::npos ||
		(pos = strCmd.find("-pp")) != std::string::npos)
	{
		CallMonitor::threadObj().Action(ActType::PrintProfile);
		return;
	}

	if ((pos = strCmd.find("-clear ")) != std::string::npos ||
		(pos = strCmd.find("-clr ")) != std::string::npos)
	{
		CallMonitor::threadObj().Action(ActType::Clear);
		return;
	}

}

void Flush()
{
	CallMonitor::threadObj().Flush();
}

void Close()
{
	CallMonitor::threadObj().Close();
}
