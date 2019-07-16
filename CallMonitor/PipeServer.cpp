// PipeServer.cpp
#include "stdafx.h"
#include "PipeServer.h"
#include "SpinLock.h"
#include "SymServer.h"

#define PIPE_BUF_SIZE 1024
#define PIPE_TIMEOUT 5000
#define BUFSIZE 512 
#define CONNECTING_STATE 0 
#define READING_STATE 1 
#define WRITING_STATE 2 

typedef struct
{
	OVERLAPPED oOverlap;
	HANDLE hPipeInst;
	DWORD dwState;
	BOOL fPendingIO;
} PIPEINST, *LPPIPEINST;


static VOID DisconnectAndReconnect(DWORD i);
static BOOL ConnectToNewClient(HANDLE hPipe, LPOVERLAPPED lpo);


const char* lpszPipename1 = "\\\\.\\pipe\\myNamedPipe1";
const char* lpszPipename2 = "\\\\.\\pipe\\myNamedPipe2";

static unsigned long __stdcall NET_RvThr(void * pParam);
static HANDLE hInPipe, hOutPipe;
//static HANDLE hThread = NULL;
static BOOL bFinished = TRUE;
static SpinLock sync;

static const char* lpszPipename = "\\\\.\\pipe\\callmonitor";
static PIPEINST stOutPipe;
static HANDLE hEvent;
static HANDLE hListener;

// DisconnectAndReconnect(DWORD) 
// This function is called when an error occurs or when the client 
// closes its handle to the pipe. Disconnect from this client, then 
// call ConnectNamedPipe to wait for another client to connect. 

VOID DisconnectAndReconnect(DWORD i)
{
	// Disconnect the pipe instance. 

	if (!DisconnectNamedPipe(stOutPipe.hPipeInst))
	{
		printf("DisconnectNamedPipe failed with %d.\n", GetLastError());
	}

	// Call a subroutine to connect to the new client. 

	stOutPipe.fPendingIO = ConnectToNewClient(
		stOutPipe.hPipeInst,
		&stOutPipe.oOverlap);

	stOutPipe.dwState = stOutPipe.fPendingIO ?
		CONNECTING_STATE : // still connecting 
		READING_STATE;     // ready to write 
}

// ConnectToNewClient(HANDLE, LPOVERLAPPED) 
// This function is called to start an overlapped connect operation. 
// It returns TRUE if an operation is pending or FALSE if the 
// connection has been completed. 

BOOL ConnectToNewClient(HANDLE hPipe, LPOVERLAPPED lpo)
{
	BOOL fConnected, fPendingIO = FALSE;

	// Start an overlapped connection for this pipe instance. 
	fConnected = ConnectNamedPipe(hPipe, lpo);

	// Overlapped ConnectNamedPipe should return zero. 
	if (fConnected)
	{
		printf("ConnectNamedPipe failed with %d.\n", GetLastError());
		return 0;
	}

	switch (GetLastError())
	{
		// The overlapped connection in progress. 
	case ERROR_IO_PENDING:
		fPendingIO = TRUE;
		break;

		// Client is already connected, so signal an event. 

	case ERROR_PIPE_CONNECTED:
		if (SetEvent(lpo->hEvent))
			break;

		// If an error occurs during the connect operation... 
	default:
	{
		printf("ConnectNamedPipe failed with %d.\n", GetLastError());
		return 0;
	}
	}

	return fPendingIO;
}

unsigned long __stdcall NET_RvThr(void * pParam)
{
	if ((stOutPipe.hPipeInst == NULL || stOutPipe.hPipeInst == INVALID_HANDLE_VALUE))
	{
		printf("[Pipe::NET_RvThr] invalid handle pipe on read - (error %d)\n", GetLastError());
		return -1;
	}

	BOOL fSuccess;
	char chBuf[BUFSIZE];
	DWORD dwBytesToWrite = BUFSIZE - 1;
	DWORD cbRead;
	int i;

	while (1)
	{
		fSuccess = ReadFile(stOutPipe.hPipeInst, chBuf, dwBytesToWrite, &cbRead, NULL);
		if (fSuccess && cbRead > 0)
		{
			printf("C++ App: Received %d Bytes : ", cbRead);
			for (i = 0; i < cbRead; i++)
				printf("%c", chBuf[i]);
			printf("\n");

			Execute(chBuf);
		}

		if (bFinished)
			break;
	}

	return 0;
}

DWORD
PipeServer::Initialize()
{
	if (!bFinished)
		return 0;

	// Create an event object for this instance. 

	hEvent = CreateEvent(
		NULL,    // default security attribute 
		TRUE,    // manual-reset event 
		TRUE,    // initial state = signaled 
		NULL);   // unnamed event object 

	if (hEvent == NULL)
	{
		printf("CreateEvent failed with %d.\n", GetLastError());
		return 0;
	}

	stOutPipe.oOverlap.hEvent = hEvent;

	stOutPipe.hPipeInst = CreateNamedPipe(
		lpszPipename,            // pipe name 
		PIPE_ACCESS_DUPLEX |     // read/write access 
		FILE_FLAG_OVERLAPPED,    // overlapped mode 
		PIPE_TYPE_MESSAGE |      // message-type pipe 
		PIPE_READMODE_MESSAGE |  // message-read mode 
		PIPE_WAIT,               // blocking mode 
		1,						 // number of instances 
		BUFSIZE * sizeof(TCHAR),   // output buffer size 
		BUFSIZE * sizeof(TCHAR),   // input buffer size 
		PIPE_TIMEOUT,            // client time-out 
		NULL);                   // default security attributes 

	if (stOutPipe.hPipeInst == INVALID_HANDLE_VALUE)
	{
		printf("CreateNamedPipe failed with %d.\n", GetLastError());
		return 0;
	}

	// Call the subroutine to connect to the new client

	stOutPipe.fPendingIO = ConnectToNewClient(
		stOutPipe.hPipeInst,
		&stOutPipe.oOverlap);

	stOutPipe.dwState = stOutPipe.fPendingIO ?
		CONNECTING_STATE : // still connecting 
		READING_STATE;     // ready to write

	DWORD tid;
	hListener = CreateThread(
		NULL,
		0,
		&NET_RvThr,
		NULL,
		0,
		&tid);

	bFinished = FALSE;
}

void
PipeServer::Write(const char* chBuf)
{
	if ((stOutPipe.hPipeInst == NULL || stOutPipe.hPipeInst == INVALID_HANDLE_VALUE))
	{
		printf("[Pipe::Write] invalid handle pipe on write - (error %d)\n", GetLastError());
		return;
	}

	DWORD nBytesRead, nBytesWrote;
	nBytesRead = strlen(chBuf);

	sync.Enter();

	if (!WriteFile(stOutPipe.hPipeInst, chBuf, nBytesRead, &nBytesWrote, NULL))
	{
		if (stOutPipe.dwState == WRITING_STATE)
		{
			printf("[Pipe::Write] Push failed (%d)\n", GetLastError());
			DisconnectAndReconnect(0);
		}
		else
		{
			FlushFileBuffers(stOutPipe.hPipeInst);
		}
	}
	else
	{
		stOutPipe.dwState = WRITING_STATE;
	}

	sync.Leave();
}

//void 
//PipeServer::Write(const MonitorContext& ctx)
//{
//	if ((stOutPipe.hPipeInst == NULL || stOutPipe.hPipeInst == INVALID_HANDLE_VALUE))
//	{
//		printf("[Pipe::Write] invalid handle pipe on write - (error %d)\n", GetLastError());
//		return;
//	}
//
//	DWORD tid = GetCurrentThreadId();
//
//	CHAR chBuf[BUFSIZE];
//	for (int i = 0; i != ctx.level; i++)
//		chBuf[i] = '\t';
//	if (ctx.isEnter)
//		sprintf(&chBuf[ctx.level], "[Enter] %d: %s\n", tid, ctx.callee);
//	else
//		sprintf(&chBuf[ctx.level], "[Leave] %d: %s\n", tid, ctx.callee);
//
//	DWORD nBytesRead, nBytesWrote;
//	nBytesRead = strlen(chBuf);
//
//	sync.Enter();
//
//	if (!WriteFile(stOutPipe.hPipeInst, chBuf, nBytesRead, &nBytesWrote, NULL))
//	{
//		if (stOutPipe.dwState == WRITING_STATE)
//		{
//			printf("[Pipe::Write] Push failed (%d)\n", GetLastError());
//			DisconnectAndReconnect(0);
//		}
//	}
//	else
//	{
//		stOutPipe.dwState = WRITING_STATE;
//	}
//
//	sync.Leave();
//}

void
PipeServer::Close()
{
	bFinished = TRUE;

	if (hOutPipe != NULL && hOutPipe != INVALID_HANDLE_VALUE)
		CloseHandle(hOutPipe);
	if (hInPipe != NULL && hInPipe != INVALID_HANDLE_VALUE)
		CloseHandle(hInPipe);

	DWORD dwTemp;
	if (GetExitCodeThread(hListener, &dwTemp))
	{
		if (dwTemp == STILL_ACTIVE)
			TerminateThread(hListener, 3);
	}
	if (hListener != NULL && hListener != INVALID_HANDLE_VALUE)
		CloseHandle(hListener);
}
