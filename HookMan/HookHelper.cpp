#include "HookHelper.h"
#include "X64NewDetour.hpp"
#include "SymServer.h"
#include "SymbolTable.h"
#include "masm.h"
#include <windows.h>
#include <iostream>
#include <map>
#include <fstream>
#pragma comment (lib, "CallMonitor.lib")

static uint8_t* pRoot;
static std::map<void*, std::shared_ptr<PLH::AbstractDetour>> hookMap;
static std::map<void*, const char*> linkMap;
static std::shared_ptr<PLH::AbstractDetour> hkAlarmRegist;

static void RangeHookByAddr(std::wostream& ofile, const std::vector<uint64_t> & listPtr, const std::vector<std::wstring> & listFunc);
static bool HookSpecialized(std::wostream& ofile, const std::wstring& funcName, BYTE* hkfp, std::shared_ptr<PLH::AbstractDetour>& detour);
static void RangeHook(const std::vector<std::wstring> & listFunc);
static bool attachHook(const std::string& funcName);

// 
// extern
//

void* GetOriginal(void * p)
{
	return hookMap[p]->GetOriginal<void*>();
}

void ProcEnter(void* pa, void* pth)
{
	const auto & iter = linkMap.find(pth);
	if (iter != linkMap.end())
	{
		linkMap.erase(iter);
	}

	const char* szCallee = NULL;
	FindFunction(pa, szCallee);
	EnterSymbol(szCallee);
}

void ProcExit(unsigned long long ret)
{
	LeaveSymbol(ret);
}

bool Collect(std::wostream& ofile, const char* fileName)
{
	const char * ctf = fileName;
	std::wifstream infile(ctf);
	if (!infile.good())
	{
		std::cerr << "[Collect] not exist file: " << ctf << std::endl;
		return false;
	}

	std::cout << "[Collect] start..." << ctf << std::endl;

	std::vector<std::wstring> listName;
	std::vector<uint64_t> listPtr;
	while (!infile.eof())
	{
		std::wstring line;
		if (!std::getline(infile, line))
			continue;

		int pos = 0;
		if ((pos = line.find(L"\t")) != std::wstring::npos)
		{
			auto strptr = line.substr(0, pos);
			unsigned long long ptr = wcstoull(strptr.c_str(), NULL, 0);
			listPtr.emplace_back(ptr);

			auto strname = line.substr(pos + 1, line.length() - pos - 1);
			listName.emplace_back(strname);
		}
		else
		{
			listName.emplace_back(line);
		}
	}

	if (listPtr.size() > 0)
	{
		RangeHookByAddr(ofile, listPtr, listName);
	}
	else
	{
		RangeHook(listName);
	}

	if (listName.size() == 0 && listPtr.size() == 0)
	{
		std::cerr << "[Collect] no function to hook: " << ctf << std::endl;
		return false;
	}

	std::cout << "[Collect] completed!" << ctf << std::endl;
	return true;
}

void Release()
{
	hookMap.clear();
	linkMap.clear();
	VirtualFree(pRoot, 0, MEM_RELEASE);
}

// 
// specialize
//

void* GetAlarmRegist(void * pth, void * pob)
{
	const char* callee = CurrentSymbol();
	std::cout << "GetAlarmRegist from: " << callee << std::endl;

	linkMap.emplace(pob, callee);
	return hkAlarmRegist->GetOriginal<void*>();
}

void CollectSpecialized(std::wostream& ofile)
{
	HookSpecialized(ofile, L"Suite::Module::Infra::NTime::NAlarm::Alarm::Regist", (BYTE*)&_dummy_alarmRegist, hkAlarmRegist);
}

// 
// static
//

void HookOpen(void* hModule)
{
	InitSymbols(hModule);
}

void HookClose()
{
	Release();
	SymCleanup(GetCurrentProcess());
}

void RangeHook(const std::vector<std::wstring> & listFunc)
{
	std::vector< std::shared_ptr<PLH::X64NewDetour> > listHook;

	TCHAR  buffer[sizeof(SYMBOL_INFOW) + MAX_SYM_NAME * sizeof(TCHAR)];
	memset(&buffer, 0, sizeof(buffer));
	for (auto iter = begin(listFunc); iter != end(listFunc); iter++)
	{
		PSYMBOL_INFOW    pSymbolInfo = (PSYMBOL_INFOW)buffer;
		pSymbolInfo->SizeOfStruct = sizeof(SYMBOL_INFOW);
		pSymbolInfo->MaxNameLen = MAX_SYM_NAME;
		BOOL bResult = SymFromNameW(GetCurrentProcess(), (*iter).c_str(), pSymbolInfo);
		if (FALSE == bResult)
		{
			std::wcout << "[RangeHook] fail to found function symbol: " << (*iter) << std::endl;
			continue;
		}

		DWORD64 symDisplacement = 0;
		bResult = SymFromAddrW(GetCurrentProcess(), pSymbolInfo->Address, &symDisplacement, pSymbolInfo);
		if (FALSE == bResult || 0 != wcscmp((*iter).c_str(), pSymbolInfo->Name))
		{
			std::wcout << "[RangeHook] fail to match address: " << (*iter) << std::endl;
			continue;
		}

		std::shared_ptr<PLH::X64NewDetour> Detour_Ex(new PLH::X64NewDetour);
		Detour_Ex->SetupHook((BYTE*)pSymbolInfo->Address, (BYTE*)&_basecamp); //can cast to byte* to
		if (FALSE == Detour_Ex->CheckLength())
		{
			std::wcout << "[RangeHook] Function to small to hook: " << (*iter) << std::endl;
			continue;
		}

		listHook.push_back(Detour_Ex);
		hookMap.emplace((BYTE*)pSymbolInfo->Address, Detour_Ex);
	}

	SIZE_T chunk = 32*8;	// 0x1000;
	pRoot = (uint8_t*)VirtualAlloc(0, chunk * listHook.size(), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	for (const auto & hk : listHook)
	{
		hk->AllocateAt(pRoot);
		pRoot += chunk;
	}

	//TO-DO: Add single step support in case processes RIP is on/in the section we write to
	PLH::Tools::ThreadManager ThreadMngr;
	ThreadMngr.SuspendThreads();

	for (const auto & hk : listHook)
	{
		hk->Hook();
	}

	//Done hooking, resume threads and flush cache (cache flush is usually just a no-op)
	ThreadMngr.ResumeThreads();

	for (const auto & hk : listHook)
	{
		hk->FlushCache();
	}
}

void RangeHookByAddr(std::wostream& ofile, const std::vector<uint64_t> & listPtr, const std::vector<std::wstring> & listFunc)
{
	std::vector< std::shared_ptr<PLH::X64NewDetour> > listHook;

	TCHAR  buffer[sizeof(SYMBOL_INFOW) + MAX_SYM_NAME * sizeof(TCHAR)];
	memset(&buffer, 0, sizeof(buffer));
	PSYMBOL_INFOW    pSymbolInfo = (PSYMBOL_INFOW)buffer;
	pSymbolInfo->SizeOfStruct = sizeof(SYMBOL_INFOW);
	pSymbolInfo->MaxNameLen = MAX_SYM_NAME;
	DWORD64 symDisplacement = 0;

	for (int i = 0; i != listFunc.size(); i++)
	{
		if (hookMap.count((void*)listPtr[i]) > 0)
		{
			ofile << "[RangeHookByAddr][Warning] duplicate address: " << (listFunc[i]) << std::endl;
			continue;
		}

		//BOOL bResult = SymFromNameW(GetCurrentProcess(), listFunc[i].c_str(), pSymbolInfo);
		//if (FALSE == bResult)
		//{
		//	ofile << "[RangeHookByAddr][Critical] fail to found function symbol: " << (listFunc[i]) << std::endl;
		//	continue;
		//}

		// validate
		BOOL bResult = SymFromAddrW(GetCurrentProcess(), listPtr[i], &symDisplacement, pSymbolInfo);
		if (FALSE == bResult)
		{
			ofile << "[RangeHookByAddr][Critical] fail to found function symbol2: " << (listFunc[i]) << std::endl;
			continue;
		}

		//if (0 != wcscmp((listFunc[i]).c_str(), pSymbolInfo->Name))
		if (!wcsstr((listFunc[i]).c_str(), pSymbolInfo->Name))
		{
			ofile << "[RangeHookByAddr][Warning] fail to match address2 --> org: " << (listFunc[i]) << " / found: " << pSymbolInfo->Name << std::endl;
			continue;
		}

		std::shared_ptr<PLH::X64NewDetour> Detour_Ex(new PLH::X64NewDetour);
		Detour_Ex->SetupHook((BYTE*)listPtr[i], (BYTE*)&_basecamp); //can cast to byte* to
		if (FALSE == Detour_Ex->CheckLength())
		{
			ofile << "[RangeHookByAddr][Warning] Function to small to hook: " << pSymbolInfo->Name << std::endl;
			continue;
		}

		listHook.push_back(Detour_Ex);
		hookMap.emplace((BYTE*)listPtr[i], Detour_Ex);
	}

	if (hookMap.size() == 0)
		return;

	uint8_t* firstPtr = (uint8_t*)(*hookMap.begin()).first;
	size_t AllocDelta = 0;
	SIZE_T chunk = 32 * 8;	// 0x1000;
	pRoot = (uint8_t*)PLH::Tools::AllocateWithin2GB(firstPtr, chunk * listHook.size(), AllocDelta);
	//pRoot = (uint8_t*)VirtualAlloc(0, chunk * listHook.size(), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	for (const auto & hk : listHook)
	{
		hk->AllocateAt(pRoot);
		pRoot += chunk;
	}

	//TO-DO: Add single step support in case processes RIP is on/in the section we write to
	PLH::Tools::ThreadManager ThreadMngr;
	ThreadMngr.SuspendThreads();

	for (const auto & hk : listHook)
	{
		hk->Hook();
	}

	//Done hooking, resume threads and flush cache (cache flush is usually just a no-op)
	ThreadMngr.ResumeThreads();

	for (const auto & hk : listHook)
	{
		hk->FlushCache();
	}
}

bool attachHook(const std::string& funcName)
{
	TCHAR  buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
	memset(&buffer, 0, sizeof(buffer));
	PSYMBOL_INFO    pSymbolInfo = (PSYMBOL_INFO)buffer;
	pSymbolInfo->SizeOfStruct = sizeof(SYMBOL_INFO);
	pSymbolInfo->MaxNameLen = MAX_SYM_NAME;
	BOOL bResult = SymFromName(GetCurrentProcess(), funcName.c_str(), pSymbolInfo);
	if (FALSE == bResult)
	{
		std::cout << "[attachHook] fail to found function symbol: " << funcName << std::endl;
		return false;
	}

	DWORD64 symDisplacement = 0;
	bResult = SymFromAddr(GetCurrentProcess(), pSymbolInfo->Address, &symDisplacement, pSymbolInfo);
	if (FALSE == bResult || 0 != strcmp(funcName.c_str(), pSymbolInfo->Name))
	{
		std::cout << "[attachHook] fail to match address: " << funcName << std::endl;
		return false;
	}

	std::shared_ptr<PLH::Detour> Detour_Ex(new PLH::Detour);
	assert(Detour_Ex->GetType() == PLH::HookType::Detour);

	Detour_Ex->SetupHook((BYTE*)pSymbolInfo->Address, (BYTE*)&_dummy); //can cast to byte* to
	if (FALSE == Detour_Ex->Hook())
	{
		std::cout << "[attachHook] fail to hook: " << funcName << std::endl;
		return false;
	}

	hookMap.emplace((BYTE*)pSymbolInfo->Address, Detour_Ex);

	std::cout << "[attachHook] success: " << funcName << std::endl;
	return true;
}

bool HookSpecialized(std::wostream& ofile, const std::wstring& funcName, BYTE* hkfp, std::shared_ptr<PLH::AbstractDetour>& detour)
{
	TCHAR  buffer[sizeof(SYMBOL_INFOW) + MAX_SYM_NAME * sizeof(TCHAR)];
	memset(&buffer, 0, sizeof(buffer));
	PSYMBOL_INFOW    pSymbolInfo = (PSYMBOL_INFOW)buffer;
	pSymbolInfo->SizeOfStruct = sizeof(SYMBOL_INFOW);
	pSymbolInfo->MaxNameLen = MAX_SYM_NAME;
	BOOL bResult = SymFromNameW(GetCurrentProcess(), funcName.c_str(), pSymbolInfo);
	if (FALSE == bResult)
	{
		std::wcout << "[HookSpecialized] fail to found function symbol: " << funcName << std::endl;
		return false;
	}

	detour = std::make_shared<PLH::X64Detour>(); 
	detour->SetupHook((BYTE*)pSymbolInfo->Address, hkfp); //can cast to byte* to
	if (FALSE == detour->Hook())
	{
		std::wcout << "[HookSpecialized][Warning] Function to small to hook: " << funcName << std::endl;
		return false;
	}

	return true;
}