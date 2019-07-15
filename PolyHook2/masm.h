#pragma once
#include <windows.h>
#include <stack>

extern "C" void _dummy();
extern "C" void _basecamp();
extern "C" void _henter();
extern "C" void _hexit();
extern "C" void PushVal(UINT64 val);
extern "C" UINT64 PopVal();

//
// impl
//

static std::stack< UINT64 > asmSt;

void PushVal(UINT64 val)
{
	asmSt.push(val);
}

UINT64 PopVal()
{
	UINT64 ret = asmSt.top();
	asmSt.pop();
	return ret;
}
