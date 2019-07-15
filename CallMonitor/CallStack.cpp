#include "stdafx.h"
#include "CallStack.h"
#include "PipeServer.h"

#define BUFSIZE 512 


CallStack::CallStack()
{
}

void 
CallStack::SetState(MonitorOption op)
{
}

void
CallStack::Push(const MonitorContext& ctx)
{
	DWORD tid = GetCurrentThreadId();

	CHAR chBuf[BUFSIZE];
	for (int i = 0; i != ctx.level; i++)
		chBuf[i] = '\t';
	sprintf(&chBuf[ctx.level], "[Enter] %d: %s\n", tid, ctx.callee);

	ctx.pipe->Write(chBuf);
}

void
CallStack::Pop(const MonitorContext& ctx)
{
	DWORD tid = GetCurrentThreadId();

	CHAR chBuf[BUFSIZE];
	for (int i = 0; i != ctx.level; i++)
		chBuf[i] = '\t';
	sprintf(&chBuf[ctx.level], "[Leave] %d: %s -> %llu\n", tid, ctx.callee, ctx.ret);

	ctx.pipe->Write(chBuf);
}