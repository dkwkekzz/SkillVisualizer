// main.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"
#include "utils.h"
#include "ctags.h"
#include "ProcessManager.h"
#include "SymbolTable.h"


void exe_ctags(const char* option, const char* rootPath, const char* outputPath)
{
	change_dir(rootPath);

	if (strchr(option, 'c') != NULL)
	{
		remove("tags");
	}

	ctags ct(rootPath, outputPath, 4);
	ct.install();
	ct.select([](const symbol& sym) { return sym.type == 'f'; });
	//ct.select([](const symbol& sym) { return sym.tag.find( "Suite::Game" ) != std::string::npos; });
	ct.flush();
	ct.uninstall();
}

int wmain(int argc, wchar_t* argv[])
{
	uint64_t baseAddr = wcstoull(argv[1], NULL, 0);
	std::wstring wszFilename = argv[2];
#if _DEBUG
	std::wcout << "-----argument check-----" << std::endl;
	std::wcout << "baseAddr: " << std::hex << baseAddr << std::endl;
	std::wcout << "filename: " << wszFilename << std::endl;
	std::wcout << "------------------------" << std::endl;
#endif

	CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	SymbolTable st(wszFilename);
	st.SetBase(baseAddr);

	st.EnumerateAll();

	std::vector<std::wstring> namelist;
	std::vector<ULONGLONG> ptrlist;
	st.EnumerateAllByAddr(namelist, ptrlist);

	std::wofstream os("symbols");
	for (int i = 0; i != namelist.size(); i++)
	{
		std::wcout << namelist[i] << '\t' << std::hex << ptrlist[i] << std::endl;
		os << namelist[i] << '\t' << ptrlist[i] << std::endl;
	}
	os.close();

	return 0;
}