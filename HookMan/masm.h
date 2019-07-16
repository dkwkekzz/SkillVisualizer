#pragma once
#include <windows.h>
#include <stack>
#include <map>
#include "SpinLock.h"

extern "C" void _basecamp();
extern "C" void _dummy();
//extern "C" void _henter();
extern "C" void _hexit();
extern "C" void PushVal(UINT64 val);
extern "C" UINT64 PopVal();

extern "C" void _dummy_alarmRegist();

static SpinLock spin;
static std::map<DWORD, std::stack<UINT64> > stMap;
//static DWORD tlsSlot = 0xFFFFFFFF;

void PushVal(UINT64 val)
{
	spin.lock();

	DWORD tid = GetCurrentThreadId();
	const auto& iter = stMap.find(tid);
	if (iter == stMap.end())
	{
		std::stack<UINT64> st;
		st.emplace(val);
		stMap.emplace(tid, st);
	}
	else
	{
		(*iter).second.emplace(val);
	}
	//if (tlsSlot == 0xFFFFFFFF)
	//	tlsSlot = TlsAlloc();
	//
	//auto * asmSt = (std::stack< UINT64 >*)TlsGetValue(tlsSlot);
	//if (nullptr == asmSt)
	//{
	//	asmSt = new std::stack< UINT64 >();
	//	TlsSetValue(tlsSlot, asmSt);
	//}
	//
	//asmSt->push(val);

	spin.unlock();
}

UINT64 PopVal()
{
	spin.lock();

	DWORD tid = GetCurrentThreadId();
	std::stack<UINT64>& st = stMap[tid];
	UINT64 ret = st.top();
	st.pop();
	//auto * asmSt = (std::stack< UINT64 >*)TlsGetValue(tlsSlot);
	//UINT64 ret = asmSt->top();
	//asmSt->pop();

	spin.unlock();
	return ret;
}
