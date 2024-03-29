#pragma once

enum class FunctionSort : int
{
	None = 0,
	Default,
	Alarm,
};

struct FunctionData
{
	char* name;
	FunctionSort sort;
};

//
// static
//

void InitSymbols(void* pAddress);
void FindFunction(void* pa, FunctionData& stFuncData);