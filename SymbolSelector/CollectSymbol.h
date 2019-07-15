#pragma once
#include <dia2.h>
#include "SymbolFunction.h"


bool CollectAllFunctions(IDiaSymbol *, std::vector<SymbolFunction>&);
bool CollectAllFunctions2(IDiaSymbol *, std::vector<SymbolFunction>&);
void EnumerateAllByAddr(IDiaSession* session_, std::vector<std::wstring>& namelist, std::vector<ULONGLONG>& ptrlist);