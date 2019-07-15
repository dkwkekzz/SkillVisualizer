// SymbolTable.cpp
#include "stdafx.h"
#include "SymbolTable.h"
#include <iostream>
#include <imagehlp.h>
#include <tlhelp32.h>
#include <intrin.h>
#include <strsafe.h>
#include <map>


//Flag to indicate the result of symbol initialization
static BOOL		bInitResult = FALSE;

//Base address of the loaded module
static DWORD64  dwBaseAddr = 0;

//Symbol Cache
static CRITICAL_SECTION csProfileInfo;
static std::map<void*, FunctionData> cached;

//*********************************************************************************************
void SplitPath(char* dest, const char* src)
{
	int len = strlen(src);
	int begin = 0;
	for (int i = len - 1; i >= 0; i--)
	{
		if (src[i] == '\\')
		{
			begin = i + 1;
			break;
		}
	}

	memcpy(dest, &src[begin], (len - begin) * sizeof(wchar_t));
	dest[len - begin] = '\0';
}

MODULEENTRY32 GetModuleInfo(const char* ModuleName)
{
	void* hSnap = nullptr;
	MODULEENTRY32 Mod32 = { 0 };

	if ((hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, NULL)) == INVALID_HANDLE_VALUE)
		return Mod32;

	Mod32.dwSize = sizeof(MODULEENTRY32);
	while (Module32Next(hSnap, &Mod32))
	{
		if (!strcmp(ModuleName, Mod32.szModule))
		{
			CloseHandle(hSnap);
			return Mod32;
		}
	}

	CloseHandle(hSnap);
	return { 0 };
}

PSYMBOL_INFO AllocSymbol(int nameLen)
{
	void* space = malloc(sizeof(SYMBOL_INFO) + nameLen);
	memset(space, 0, sizeof(SYMBOL_INFO) + nameLen);
	PSYMBOL_INFO sym = reinterpret_cast<PSYMBOL_INFO>(space);
	sym->NameLen = nameLen;
	// SizeOfStruct is ment to point at the actual size of the struct and not
	// of the whole memory allocated
	sym->SizeOfStruct = sizeof(SYMBOL_INFO);
	return sym;
}

void FreeSymbol(PSYMBOL_INFO symbol)
{
	free(symbol);
}

bool GetSymbol(const char* name, PSYMBOL_INFO& symbol)
{
	PSYMBOL_INFO sym = AllocSymbol(MAX_SYM_NAME);
	if (SymFromName(GetCurrentProcess(), name, sym) == FALSE)
	{
		FreeSymbol(sym);
		return false;
	}

	symbol = sym;
	return true;
}

//*********************************************************************************************
//Function to load the symbols of the module
void InitSymbols(void* pAddress )
{
	if (bInitResult)
		return;

	::InitializeCriticalSection(&csProfileInfo);

	//Query the memory
	char moduleName[MAX_PATH];
	char targetPath[MAX_PATH];
	char targetName[MAX_PATH];
	MEMORY_BASIC_INFORMATION mbi;

	//Get the module name where the address is available
	VirtualQuery((void*)pAddress,&mbi,sizeof(mbi));
	GetModuleFileNameA((HMODULE)mbi.AllocationBase, moduleName, MAX_PATH );

	//Initialize the symbols
	bInitResult = SymInitialize(GetCurrentProcess(),moduleName, TRUE );

	//Load the module
	dwBaseAddr = SymLoadModule64(GetCurrentProcess(),
									NULL,
									(PCSTR)moduleName,
									NULL,
									(DWORD)mbi.AllocationBase,
									0);

	//Get exe name
	GetModuleFileName(NULL, targetPath, MAX_PATH);
	SplitPath(targetName, targetPath);
	auto exeMod = GetModuleInfo(targetName);
	UINT64 exeBaseAddr = (UINT64)exeMod.modBaseAddr;

	//Set the options
	SymSetOptions( SymGetOptions()   &~SYMOPT_UNDNAME );	

	//open process for read virtualaddress...
	PROCESS_INFORMATION pi;
	STARTUPINFO si;

	// Set up the start up info struct.
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	si.dwFlags = STARTF_USESTDHANDLES;
	// Use this if you want to hide the child:
	//     si.wShowWindow = SW_HIDE;
	// Note that dwFlags must include STARTF_USESHOWWINDOW if you want to
	// use the wShowWindow flags.


	// Launch the process that you want to redirect (in this case,
	// Child.exe). Make sure Child.exe is in the same directory as
	// redirect.c launch redirect from a command line to prevent location
	// confusion.
	char cmd[256];
	sprintf(cmd, "SymbolSelector.exe %llu %s", exeBaseAddr, targetName);
	if (!CreateProcess(NULL, (LPSTR)cmd, NULL, NULL, TRUE,
		NULL, NULL, NULL, &si, &pi))
	{
		std::cerr << "[InitSymbols] fail to open process..." << std::endl;
		return;
	}

	WaitForSingleObject(pi.hProcess, INFINITE);

	// Close any unnecessary handles.
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
}

//*********************************************************************************************
void FindFunction(void* pa, FunctionData & stFuncData )
{
	// CriticalSection
	::EnterCriticalSection(&csProfileInfo);

	std::map<void*, FunctionData>::iterator iter = cached.find(pa);
	if (iter != cached.end())
	{
		stFuncData = (*iter).second;
		return;
	}

	DWORD64 symDisplacement = 0;

	char undName[1024];
	if( dwBaseAddr )
	{			
		//Allocate the memory for PSYMBOL_INFO
		TCHAR  buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)] ;
		memset(&buffer,0,sizeof(buffer));
		PSYMBOL_INFO    pSymbolInfo	= ( PSYMBOL_INFO)buffer;
		pSymbolInfo->SizeOfStruct = sizeof(SYMBOL_INFO) ;
		pSymbolInfo->MaxNameLen	= MAX_SYM_NAME;

		//Get the name of the symbol using the address
		BOOL bResult = SymFromAddr( GetCurrentProcess(), (DWORD64)pa,&symDisplacement, pSymbolInfo );

		LPVOID lpMsgBuf;
		LPVOID lpDisplayBuf;

		//If symbol is found, then get its undecorated name.The name could be a decorated one
		//as the name mangling schemes are deployed
		if ( bResult )
		{
			if (  UnDecorateSymbolName( pSymbolInfo->Name, undName,
											sizeof(undName),
											UNDNAME_NO_MS_KEYWORDS |
											UNDNAME_NO_ACCESS_SPECIFIERS |
											UNDNAME_NO_FUNCTION_RETURNS |
											UNDNAME_NO_ALLOCATION_MODEL |
											UNDNAME_NO_ALLOCATION_LANGUAGE |
											UNDNAME_NO_ARGUMENTS  |
											UNDNAME_NO_SPECIAL_SYMS |
											UNDNAME_NO_MEMBER_TYPE))

			{
				//Ignore the unnecessary calls emulated from std library
				if( strstr(undName,"std::")) 
				{
					//Skip the symbol
				}
				else
				{
					strcpy_s(undName,pSymbolInfo->Name);

					FunctionData fdata;
					fdata.name = new char [ strlen(undName) + 2 ];
					strcpy_s(fdata.name, strlen(undName) + 1, undName);

					if (strstr(fdata.name, "::OnAlarm"))
					{
						fdata.sort = FunctionSort::Alarm;
					}
					else
					{
						fdata.sort = FunctionSort::Default;
					}

					cached.emplace(pa, fdata);
					stFuncData = cached[pa];
				}
			}

		}
		else
		{
			DWORD lastError = GetLastError();
			FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			lastError,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &lpMsgBuf,
			0, NULL );

			lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
						(lstrlen((LPCTSTR)lpMsgBuf) + 40) * sizeof(TCHAR)); 
		

			StringCchPrintf((LPTSTR)lpDisplayBuf, 
							LocalSize(lpDisplayBuf) / sizeof(TCHAR),
							TEXT("failed with error %d: %s"), 
							lastError, lpMsgBuf); 

		}
	}

	::LeaveCriticalSection(&csProfileInfo);
}