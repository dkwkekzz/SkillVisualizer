//SpinLock.cpp
#include "stdafx.h"
#include "SpinLock.h"

void
SpinLock::Enter()
{
	lock();
}

void
SpinLock::Leave()
{
	unlock();
}