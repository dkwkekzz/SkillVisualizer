#include "stdafx.h"
#include "CallMonitor.h"
#include "SymServer.h"
#include "SpinLock.h"
#include "utils.h"
#include "CallStack.h"
#include "Profiler.h"

//
// static
//

DWORD CallMonitor::tlsSlot = 0xFFFFFFFF;
PipeServer CallMonitor::pipe;

//
// member
//

CallMonitor::CallMonitor()
	: pStack(new CallStack), pProfiler(new Profiler)
{
	pipe.Initialize();
}

CallMonitor::~CallMonitor()
{
}

const char*
CallMonitor::Top()
{
	if (m_stSym.empty())
		return nullptr;
	return m_stSym.top();
}

void 
CallMonitor::SetState(MonitorOption op)
{
	if (pProfiler) pProfiler->SetState(op);
	if (pStack) pStack->SetState(op);
}

CallMonitor 
&CallMonitor::threadObj()
{
	CallMonitor *self = (CallMonitor *)
		TlsGetValue(tlsSlot);

	if (self == nullptr)
	{
		if (tlsSlot == 0xFFFFFFFF) tlsSlot = TlsAlloc();
		self = new CallMonitor;
		TlsSetValue(tlsSlot, self);
	}

	return *self;
}

//void CallMonitor::threadAttach(CallMonitor *newObj)
//{
//	if (tlsSlot == 0xFFFFFFFF) tlsSlot = TlsAlloc();
//	TlsSetValue(tlsSlot, newObj);
//}
//
//void CallMonitor::threadDetach()
//{
//	delete &threadObj();
//}

void
CallMonitor::Push( MonitorOption op, const char* szCallee, const char* szCaller )
{
	if (NULL == szCallee)
	{
		m_stSym.push(NULL);
		return;
	}

	MonitorContext ctx;
	strcpy(ctx.callee, szCallee);
	ctx.level = m_stSym.size();
	ctx.mop = op;
	ctx.pipe = &pipe;
	ctx.caller = szCaller;

	if (pProfiler) pProfiler->Push(ctx);
	if (pStack) pStack->Push(ctx);
	//pipe.Write(ctx);
	//for (auto * listener : m_listeners)
	//	listener->Push(ctx);

	auto * pDupCallee = m_sq.Create();
	strcpy(pDupCallee, szCallee);
	m_stSym.emplace(pDupCallee);
}

void
CallMonitor::Pop(MonitorOption op, UINT64 ret)
{
	if (m_stSym.empty())
		return;

	auto * pCallee = m_stSym.top();
	m_stSym.pop();

	if (NULL == pCallee)
		return;

	MonitorContext ctx;
	strcpy(ctx.callee, pCallee);
	ctx.level = m_stSym.size();
	ctx.ret = ret;
	ctx.mop = op;
	ctx.pipe = &pipe;

	if (pProfiler) pProfiler->Pop(ctx);
	if (pStack) pStack->Pop(ctx);
	//pipe.Write(ctx);

	//for (auto * listener : m_listeners)
	//	listener->Pop(ctx);

	m_sq.Destroy( pCallee );
}

void
CallMonitor::Action(ActType at)
{
	if (pProfiler) pProfiler->Action(at);
}

void
CallMonitor::Flush()
{
	while (!m_stSym.empty())
		m_stSym.pop();
	//for (auto * listener : m_listeners)
	//	listener->Flush();
}

void
CallMonitor::Close()
{
	//for (auto * listener : m_listeners)
	//	listener->Close();
	pipe.Close();
	delete &threadObj();
}