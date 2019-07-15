#pragma once
#include "MonitorOption.h"

class PipeServer;

struct MonitorContext
{
	//bool isEnter;
	char callee[256];
	int level;
	unsigned long long ret;
	MonitorOption mop;
	PipeServer* pipe;
};