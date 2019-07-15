#pragma once
#include <windows.h>
#include <stack>

extern "C" void _basecamp();
extern "C" void _dummy();
//extern "C" void _henter();
extern "C" void _hexit();
extern "C" void PushVal(UINT64 val);
extern "C" UINT64 PopVal();

extern "C" void _dummy_alarmRegist();


static DWORD tlsSlot = 0xFFFFFFFF;

void PushVal(UINT64 val)
{
	if (tlsSlot == 0xFFFFFFFF)
		tlsSlot = TlsAlloc();

	auto * asmSt = (std::stack< UINT64 >*)TlsGetValue(tlsSlot);
	if (nullptr == asmSt)
	{
		asmSt = new std::stack< UINT64 >();
		TlsSetValue(tlsSlot, asmSt);
	}

	asmSt->push(val);
}

UINT64 PopVal()
{
	auto * asmSt = (std::stack< UINT64 >*)TlsGetValue(tlsSlot);
	UINT64 ret = asmSt->top();
	asmSt->pop();
	return ret;
}
