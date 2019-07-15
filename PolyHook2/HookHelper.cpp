#include "HookHelper.h"
#include "SymServer.h"
#include "masm.h"
#include "PolyHook.hpp"
#include "archer2.hpp"
#include "monster.hpp"
#include "blademaster.hpp"
#include <fstream>
#include <queue>

static std::map<void*, std::shared_ptr<PLH::Detour>> hookedmap;

// 
// extern
//

void* __stdcall GetOriginal(void * p)
{
	return hookedmap[p]->GetOriginal<void*>();
}

void* temp;
void* GetOriginal2()
{
	return temp;
}

// 
// static
//

bool attachHook(const string& funcName)
{
	TCHAR  buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
	memset(&buffer, 0, sizeof(buffer));
	PSYMBOL_INFO    pSymbolInfo = (PSYMBOL_INFO)buffer;
	pSymbolInfo->SizeOfStruct = sizeof(SYMBOL_INFO);
	pSymbolInfo->MaxNameLen = MAX_SYM_NAME;
	BOOL bResult = SymFromName(GetCurrentProcess(), funcName.c_str(), pSymbolInfo);
	if (FALSE == bResult)
	{
		cerr << "[attachHook] fail to found func symbol: " << funcName << std::endl;
		return false;
	}

	std::shared_ptr<PLH::Detour> Detour_Ex(new PLH::Detour);
	assert(Detour_Ex->GetType() == PLH::HookType::Detour);

	Detour_Ex->SetupHook((BYTE*)pSymbolInfo->Address, (BYTE*)&_dummy, (BYTE*)&_basecamp); //can cast to byte* to
	if (!Detour_Ex->Hook())
	{
		return false;
	}

	hookedmap.emplace((void*)pSymbolInfo->Address, Detour_Ex);

	cout << "[attachHook] success hook: " << funcName << std::endl;
	return true;
}

void release()
{
	for (const auto & hk : hookedmap)
		hk.second->UnHook();
	hookedmap.clear();
}

void testcase1()
{
	::SetMonitorState(MonitorOption::RealTime | MonitorOption::Log);

	attachHook("powerworld::archer2::forceattack");
	attachHook("powerworld::archer2::attack");

	powerworld::archer2::forceattack(0x22);
	powerworld::archer2 a2;
	int ret = a2.attack(0x444);

	::DisplayLog();
	::DisplayProfileData();
	::Flush();
	::release();
}

void testcase2()
{
	::SetMonitorState(MonitorOption::RealTime | MonitorOption::Log);

	const char * ctf = "symbols.txt";
	std::ifstream infile(ctf);
	if (!infile.good())
		return;

	while (!infile.eof())
	{
		std::string line;
		if (!std::getline(infile, line))
			continue;

		attachHook(line);
	}

	cout << "[testcase2] wait for exit..." << std::endl;
	while (cin.get() != 0);

	::release();
}

void testcase3()
{
	::SetMonitorState(MonitorOption::RealTime | MonitorOption::Log);

	attachHook("powerworld::blademaster::blademaster");
	attachHook("powerworld::blademaster::attack");
	attachHook("powerworld::blademaster::defence");
	//attachHook("powerworld::blademaster::skillmoonblade");
	//attachHook("powerworld::monster::attack");
	//attachHook("powerworld::monster::monster");
	//attachHook("powerworld::monster::defence");
	//attachHook("powerworld::monster::getCombat");
	//attachHook("powerworld::monster::getHealth");
	//attachHook("powerworld::archer2::archer2");
	//attachHook("powerworld::archer2::forceattack");
	//attachHook("powerworld::archer2::attack");
	//attachHook("powerworld::archer2::defence");

	powerworld::blademaster b1;
	b1.attack(0x999);
	//
	powerworld::archer2 a3;
	int ret3 = a3.attack(0x444);
	a3.defence();
	
	powerworld::monster mon(1);
	mon.defence(0x999);
	
	powerworld::archer2::forceattack(0x22);
	powerworld::archer2 a2;
	int ret = a2.attack(0x444);
	
	queue<powerworld::monster> monque;
	monque.push(powerworld::monster(1));
	
	auto * pMon = &monque.front();
	pMon->defence(a2.attack(0x99));

	::DisplayLog();
	::DisplayProfileData();
	::Flush();
	::release();
}
