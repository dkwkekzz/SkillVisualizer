#pragma once
#include <windows.h>
#include <map>

// 
// extern
//

extern "C" void* GetOriginal(void * p);
extern "C" void* GetOriginal2();

// 
// static
//

bool attachHook(const std::string& funcName);
void release();
void testcase1();
void testcase2();
void testcase3();