#pragma once
//#include "MonitorContext.h"

class PipeServer
{
public:
	DWORD Initialize();
	void Write(const char* chBuf);
	//void Write(const MonitorContext& ctx);
	void Close();
};