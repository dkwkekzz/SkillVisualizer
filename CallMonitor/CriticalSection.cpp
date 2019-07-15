//SpinLock.cpp
#include "stdafx.h"
#include "CriticalSection.h"


void
CriticalSection::Init()
{
	//Intitalize the critical section
	::InitializeCriticalSection(&g_csProfileInfo);
}

void
CriticalSection::Enter()
{
	::EnterCriticalSection(&g_csProfileInfo);
}

void
CriticalSection::Leave()
{
	::LeaveCriticalSection(&g_csProfileInfo);
}