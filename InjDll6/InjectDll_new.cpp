#include "windows.h"
#include "stdio.h"
#include "tchar.h"
#include <sal.h>
#include <stdlib.h>
#include <tlhelp32.h>
#include <VersionHelpers.h>

typedef struct _CLIENT_ID
{
    PVOID UniqueProcess;
    PVOID UniqueThread;
} CLIENT_ID, *PCLIENT_ID;

typedef long (WINAPI *LPFUN_RtlCreateUserThread)(
		HANDLE,					// ProcessHandle
	    PSECURITY_DESCRIPTOR,	// SecurityDescriptor (OPTIONAL)
	    BOOLEAN,				// CreateSuspended
		ULONG,					// StackZeroBits
	    PULONG,					// StackReserved
		PULONG,					// StackCommit
	    PVOID,					// StartAddress
		PVOID,					// StartParameter (OPTIONAL)
	    PHANDLE,				// ThreadHandle
		PCLIENT_ID				// ClientID
);

typedef DWORD (WINAPI *PFNTCREATETHREADEX)
( 
    PHANDLE                 ThreadHandle,	
    ACCESS_MASK             DesiredAccess,	
    LPVOID                  ObjectAttributes,	
    HANDLE                  ProcessHandle,	
    LPTHREAD_START_ROUTINE  lpStartAddress,	
    LPVOID                  lpParameter,	
    BOOL	                CreateSuspended,	
    DWORD                   dwStackSize,	
    DWORD                   dw1, 
    DWORD                   dw2, 
    LPVOID                  Unknown 
); 

HANDLE bCreateUserThread(HANDLE hHandle, LPVOID loadLibAddr, LPVOID dllPathAddr) {
	/*
		Provided help
			http://syprog.blogspot.com/2012/05/createremotethread-bypass-windows.html?showComment=1338375764336#c4138436235159645886
			http://undocumented.ntinternals.net/UserMode/Undocumented%20Functions/Executable%20Images/RtlCreateUserThread.html
			http://www.rohitab.com/discuss/topic/39493-using-rtlcreateuserthread/
	*/


	HANDLE hRemoteThread = NULL;
	LPVOID rtlCreateUserAddr = NULL;

	CLIENT_ID cid;

	rtlCreateUserAddr = GetProcAddress(GetModuleHandle(TEXT("ntdll.dll")), "RtlCreateUserThread");

	if (rtlCreateUserAddr) {

		LPFUN_RtlCreateUserThread funRtlCreateUserThread = (LPFUN_RtlCreateUserThread)rtlCreateUserAddr;
		funRtlCreateUserThread(
			hHandle,			// ProcessHandle
			NULL,				// SecurityDescriptor (OPTIONAL)
			FALSE,				// CreateSuspended
			0,					// StackZeroBits
			0,					// StackReserved
			0,					// StackCommit
			(PVOID)loadLibAddr,// StartAddress
			(PVOID)dllPathAddr,// StartParameter (OPTIONAL)
			&hRemoteThread,		// ThreadHandle
			&cid				// ClientID
		);

		if (hRemoteThread == NULL) {
			printf("\t[!] RtlCreateUserThread Failed! [%d]\n", GetLastError());
			return NULL;
		}
		else {
			return hRemoteThread;
		}
	}
	else {
		printf("\n[!] Could not find RtlCreateUserThread!\n");
	}
	return NULL;
}

BOOL SetPrivilege(LPCTSTR lpszPrivilege, BOOL bEnablePrivilege) 
{
    TOKEN_PRIVILEGES tp;
    HANDLE hToken;
    LUID luid;

    if( !OpenProcessToken(GetCurrentProcess(),
                          TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, 
			              &hToken) )
    {
        _tprintf(L"OpenProcessToken error: %u\n", GetLastError());
        return FALSE;
    }

    if( !LookupPrivilegeValue(NULL,             // lookup privilege on local system
                              lpszPrivilege,    // privilege to lookup 
                              &luid) )          // receives LUID of privilege
    {
        _tprintf(L"LookupPrivilegeValue error: %u\n", GetLastError() ); 
        return FALSE; 
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    if( bEnablePrivilege )
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    else
        tp.Privileges[0].Attributes = 0;

    // Enable the privilege or disable all privileges.
    if( !AdjustTokenPrivileges(hToken, 
                               FALSE, 
                               &tp, 
                               sizeof(TOKEN_PRIVILEGES), 
                               (PTOKEN_PRIVILEGES) NULL, 
                               (PDWORD) NULL) )
    { 
        _tprintf(L"AdjustTokenPrivileges error: %u\n", GetLastError() ); 
        return FALSE; 
    } 

    if( GetLastError() == ERROR_NOT_ALL_ASSIGNED )
    {
        _tprintf(L"The token does not have the specified privilege. \n");
        return FALSE;
    } 

    return TRUE;
}

// GetVersionEx가 deprecated되었다...
//BOOL IsVistaOrLater()
//{
//    OSVERSIONINFO osvi;
//	
//    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
//    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
//	
//    GetVersionEx(&osvi);
//	
//    if( osvi.dwMajorVersion >= 6 )
//        return TRUE;
//
//    return FALSE;
//}

BOOL MyCreateRemoteThread(HANDLE hProcess, LPTHREAD_START_ROUTINE pThreadProc, LPVOID pRemoteBuf)
{
    HANDLE      hThread = NULL;
    FARPROC     pFunc = NULL;

    if( IsWindows8OrGreater() )
    {
		hThread = bCreateUserThread(hProcess, pThreadProc, pRemoteBuf);
        if( hThread == NULL )
        {
            printf("MyCreateRemoteThread() : NtCreateThreadEx() failed!!! [%d]\n", GetLastError());
            return FALSE;
        }
    }
	else if ( IsWindowsVistaOrGreater() )   // Vista, 7, Server2008
	{
		pFunc = GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtCreateThreadEx");
        if( pFunc == NULL )
        {
            printf("MyCreateRemoteThread() : GetProcAddress(\"NtCreateThreadEx\") failed!!! [%d]\n",
                   GetLastError());
            return FALSE;
        }

        ((PFNTCREATETHREADEX)pFunc)(&hThread,
                                    0x1FFFFF,
                                    NULL,
                                    hProcess,
                                    pThreadProc,
                                    pRemoteBuf,
                                    FALSE,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL);
        if( hThread == NULL )
        {
            printf("MyCreateRemoteThread() : NtCreateThreadEx() failed!!! [%d]\n", GetLastError());
            return FALSE;
        }
	}
    else                    // 2000, XP, Server2003
    {
        hThread = CreateRemoteThread(hProcess, 
                                     NULL, 
                                     0, 
                                     pThreadProc, 
                                     pRemoteBuf, 
                                     0, 
                                     NULL);
        if( hThread == NULL )
        {
            printf("MyCreateRemoteThread() : CreateRemoteThread() failed!!! [%d]\n", GetLastError());
            return FALSE;
        }
    }

	if( WAIT_FAILED == WaitForSingleObject(hThread, INFINITE) )
    {
        printf("MyCreateRemoteThread() : WaitForSingleObject() failed!!! [%d]\n", GetLastError());
        return FALSE;
    }

    return TRUE;
}

BOOL InjectDll(DWORD dwPID, LPCTSTR szDllName)
{
    HANDLE hProcess = NULL;
    LPVOID pRemoteBuf = NULL;
    FARPROC pThreadProc = NULL;
    DWORD dwBufSize = (DWORD)(_tcslen(szDllName)+1) * sizeof(TCHAR);

    if ( !(hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID)) )
    {
        printf("[ERROR] OpenProcess(%d) failed!!! [%d]\n", 
        dwPID, GetLastError());
        return FALSE;
    }

    pRemoteBuf = VirtualAllocEx(hProcess, NULL, dwBufSize, 
                                MEM_COMMIT, PAGE_READWRITE);

    WriteProcessMemory(hProcess, pRemoteBuf, (LPVOID)szDllName, 
                       dwBufSize, NULL);

    pThreadProc = GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryW");

    if( !MyCreateRemoteThread(hProcess, (LPTHREAD_START_ROUTINE)pThreadProc, pRemoteBuf) )
    {
        printf("[ERROR] MyCreateRemoteThread() failed!!!\n");
        return FALSE;
    }

    VirtualFreeEx(hProcess, pRemoteBuf, 0, MEM_RELEASE);

    CloseHandle(hProcess);

    return TRUE;
}

BOOL EjectDll(DWORD dwPID, LPCTSTR szDllName)
{
	MODULEENTRY32 entry;
	entry.dwSize = sizeof(entry);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);

	BOOL exist = Module32First(snapshot, &entry);
	for (; exist; exist = Module32Next(snapshot, &entry))
	{
		if (!_tcsicmp((LPCTSTR)entry.szModule, szDllName) ||
			!_tcsicmp((LPCTSTR)entry.szExePath, szDllName))
		{
			exist = TRUE;
			break;
		}
	}

	if (!exist)
	{
		printf("[ERROR] CreateToolhelp32Snapshot failed!!!\n");
		return FALSE;
	}

	HANDLE hProcess = NULL;
	FARPROC pThreadProc = NULL;

	if (!(hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID)))
	{
		printf("[ERROR] OpenProcess(%d) failed!!! [%d]\n", dwPID, GetLastError());
		return FALSE;
	}

	pThreadProc = GetProcAddress(GetModuleHandle(L"kernel32.dll"), "FreeLibrary");

	if (!MyCreateRemoteThread(hProcess, (LPTHREAD_START_ROUTINE)pThreadProc, entry.modBaseAddr))
	{
		printf("[ERROR] MyCreateRemoteThread() failed!!!\n");
		return FALSE;
	}

	CloseHandle(snapshot);
	CloseHandle(hProcess);

	return TRUE;
}

DWORD getPID(const TCHAR* target)
{
	DWORD pid = _wtoi(target);
	if (pid != 0)
		return pid;

	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(snapshot, &entry) == TRUE)
	{
		while (Process32Next(snapshot, &entry) == TRUE)
		{
			if (_tcsicmp(entry.szExeFile, target) == 0)
			{
				pid = entry.th32ProcessID;
				break;
			}
		}
	}

	CloseHandle(snapshot);
	return pid;
}

int _tmain(int argc, TCHAR *argv[])
{
	// adjust privilege
    SetPrivilege(SE_DEBUG_NAME, TRUE);

    // InjectDll.exe <PID> <dll_path>
    if( argc < 3 )
    {
        printf("usage : %s <PID> <dll_path> <option>\n", argv[0]);
        return 1;
    }

	DWORD pid = getPID(argv[1]);
	if (pid == 0)
	{
		printf("no exist process: %s\n", argv[1]);
		return 1;
	}

	const TCHAR* szProc = (argc >= 4) ? argv[3] : L"-i";
	if (wcscmp(szProc, L"-i") == 0)
	{	// injection
		printf("InjectDll() ");
		if (!InjectDll(pid, argv[2]))
		{
			printf("failed!!!\n");
			return 1;
		}
	}
	else if (wcscmp(szProc, L"-e") == 0)
	{	// ejection
		printf("EjectDll() ");
		if (!EjectDll(pid, argv[2]))
		{
			printf("failed!!!\n");
			return 1;
		}
	}
	else
	{
		printf("unknown option: %s\n", argv[3]);
		return 1;
	}

    printf("succeeded!!!\n");

    return 0;
}
