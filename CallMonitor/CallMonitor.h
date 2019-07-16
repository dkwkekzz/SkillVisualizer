#pragma once
#include "stdafx.h"
#include "ISync.h"
#include "IListener.h"
#include "ActType.h"
#include "StringPool.h"
#include "PipeServer.h"


class CallMonitor
{
public:
	CallMonitor();
	~CallMonitor();

	// Thread-local singleton accessor
	static CallMonitor &threadObj();

	//static void threadAttach(CallMonitor *newObj);
	//static void threadDetach();

	const char* Top();
	void SetState( MonitorOption op );
	void Push( MonitorOption op, const char* szCallee, const char* szCaller );
	void Pop( MonitorOption op, UINT64 ret );
	void Action( ActType at );
	void Flush();
	void Close();

private:
	static DWORD tlsSlot;
	static PipeServer pipe;

	std::stack<const char*> m_stSym;
	StringPool m_sq;
	IListener* pStack;
	IListener* pProfiler;
};

