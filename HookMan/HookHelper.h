#pragma once
#include <vector>
#include <string>

// 
// extern
//

extern "C" void* GetOriginal(void * p);
extern "C" void ProcEnter(void* pth, void* paddr);
extern "C" void ProcExit(unsigned long long ret);

extern "C" bool Collect(std::wostream& ofile, const char* fileName);
extern "C" void Release();

// 
// static
//

void HookOpen(void* hModule);
void HookClose();

// 
// specialize
//

//extern "C" void CollectSpecialized(std::wostream& ofile);
extern "C" void* GetAlarmRegist1(void * pth, void * pa, void * pb);
extern "C" void* GetAlarmRegist2(void * pth, void * pa, void * pb);