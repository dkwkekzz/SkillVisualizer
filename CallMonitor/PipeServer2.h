#pragma once
#include "MonitorContext.h"

#define CONNECTING_STATE 0 
#define READING_STATE 1 
#define WRITING_STATE 2 
#define INSTANCES 1
#define PIPE_TIMEOUT 5000
#define BUFSIZE 512


typedef struct
{
	OVERLAPPED oOverlap;
	HANDLE hPipeInst;
	TCHAR chRequest[BUFSIZE];
	DWORD cbRead;
	TCHAR chReply[BUFSIZE];
	DWORD cbToWrite;
	DWORD dwState;
	BOOL fPendingIO;
} PIPEINST, *LPPIPEINST;

static struct CallMsg
{
	bool isEnter;
	int level;
	char callee[BUFSIZE];
};


DWORD InitializePipeServer();
void WriteCall(const MonitorContext& ctx);
