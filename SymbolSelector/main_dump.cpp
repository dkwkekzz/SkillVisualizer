// test.cpp
#include "pch.h"
#include "DIADump.h"
#include "CollectSymbol.h"
#include "Option.h"


int wmain(int argc, wchar_t* argv[])
{
	uint64_t baseAddress = wcstoull(argv[1], NULL, 0);
	std::wstring wszFilename = argv[2];
#if _DEBUG
	std::wcout << "-----argument check-----" << std::endl;
	std::wcout << "baseAddr: " << std::hex << baseAddress << std::endl;
	std::wcout << "filename: " << wszFilename << std::endl;
	std::wcout << "------------------------" << std::endl;
#endif

	// CoCreate() and initialize COM objects
	if (!LoadDataFromPdb(wszFilename.c_str(), &g_pDiaDataSource, &g_pDiaSession, &g_pGlobalSymbol)) {
		return -1;
	}

	//DumpAllPublics(g_pGlobalSymbol);
	//DumpAllSymbols(g_pGlobalSymbol);

	// my code
	g_pDiaSession->put_loadAddress(baseAddress);
	
	std::vector<SymbolFunction> outVec;
	outVec.reserve(0x100000);

	CollectAllFunctions2(g_pGlobalSymbol, outVec);
	CollectAllFunctions(g_pGlobalSymbol, outVec);

	Option opt;
	std::wofstream ofile("symbols");
	for (int i = 0; i != outVec.size(); i++)
	{
		if (opt.Vaild(outVec[i].name.c_str()))
		//if (outVec[i].name.find(L"std::") == std::wstring::npos
		//	&& (outVec[i].name.find(L"public: ") != std::wstring::npos
		//	|| outVec[i].name.find(L"protected: ") != std::wstring::npos
		//	|| outVec[i].name.find(L"private: ") != std::wstring::npos)
		//	//&& outVec[i].name.find(L"Suite::Game::") != std::wstring::npos
		//	)
		{
			ofile << outVec[i].va;
			ofile << '\t' << outVec[i].name;
			ofile << std::endl;
			//ofile << L"" << outVec[i].name << "\t" << outVec[i].va << std::endl;
		}
	}

	ofile.close();

	// release COM objects and CoUninitialize()
	Cleanup();

	return 0;
}