// CollectSymbol
#include "pch.h"
#include "CollectSymbol.h"


//
// private
//

inline static void Break()
{
	static_cast<void>(0);
}

bool MakeFunctionSymbol(IDiaSymbol *pSymbol, SymbolFunction& sf)
{
	BSTR bstrName;

	if (pSymbol->get_virtualAddress(&sf.va) != S_OK)
	{
		sf.va = 0xFFFFFFFF;
	}

	// must be a function or a data symbol

	BSTR bstrUndname;
	if (pSymbol->get_name(&bstrName) == S_OK) {
		sf.name = bstrName;
		SysFreeString(bstrName);
	}

	return true;
}

bool MakeFunctionSymbolUndecorated(IDiaSymbol *pSymbol, SymbolFunction& sf)
{
	BSTR bstrName;

	if (pSymbol->get_virtualAddress(&sf.va) != S_OK)
	{
		sf.va = 0xFFFFFFFF;
	}

	// must be a function or a data symbol

	BSTR bstrUndname;
	if (pSymbol->get_name(&bstrName) == S_OK) {
		if (pSymbol->get_undecoratedName(&bstrUndname) == S_OK) {
			sf.name = bstrUndname;

			SysFreeString(bstrUndname);
		}
		else {
			sf.name = bstrName;
		}

		SysFreeString(bstrName);
	}

	return true;
}

//
// public
//

bool CollectAllFunctions2(IDiaSymbol *pGlobal, std::vector<SymbolFunction>& outVec)
{
	IDiaEnumSymbols *pEnumSymbols;

	if (FAILED(pGlobal->findChildren(SymTagCompiland, NULL, nsNone, &pEnumSymbols))) {
		return false;
	}

	IDiaSymbol *pCompiland;
	ULONG celt = 0;

	wprintf(L"\n");
	while (SUCCEEDED(pEnumSymbols->Next(1, &pCompiland, &celt)) && (celt == 1)) {
		wprintf(L"** Module: ");

		// Retrieve the name of the module

		BSTR bstrName;

		if (pCompiland->get_name(&bstrName) != S_OK) {
			wprintf(L"(???)\n");
		}
		
		else {
			//if (!StrStr(bstrName, L"x64"))
			//{
			//	wprintf(L"no x64... so skip!\n");
			//	continue;
			//}

			wprintf(L"%s\n", bstrName);

			SysFreeString(bstrName);
		}

		// Find all the symbols defined in this compiland and print their info

		IDiaEnumSymbols *pEnumChildren;

		if (SUCCEEDED(pCompiland->findChildren(SymTagFunction, NULL, nsNone, &pEnumChildren))) {
			IDiaSymbol *pSymbol;
			ULONG celtChildren = 0;

			while (SUCCEEDED(pEnumChildren->Next(1, &pSymbol, &celtChildren)) && (celtChildren == 1)) {

				DWORD dwSymTag;

				if (pSymbol->get_symTag(&dwSymTag) != S_OK)
					continue;

				if (dwSymTag != SymTagFunction)
					continue;

				SymbolFunction sf;
				if (MakeFunctionSymbol(pSymbol, sf))
					outVec.emplace_back(sf);

				pSymbol->Release();
			}

			pEnumChildren->Release();
		}

		pCompiland->Release();
	}

	pEnumSymbols->Release();
}

int testcount2 = 0;
bool CollectAllFunctions(IDiaSymbol *pGlobal, std::vector<SymbolFunction>& outVec)
{
	IDiaEnumSymbols *pEnumSymbols;

	if (FAILED(pGlobal->findChildren(SymTagPublicSymbol, NULL, nsNone, &pEnumSymbols))) {
		return false;
	}

	IDiaSymbol *pSymbol;
	ULONG celt = 0;

	while (SUCCEEDED(pEnumSymbols->Next(1, &pSymbol, &celt)) && (celt == 1)) {

		DWORD dwSymTag;
		BOOL ret = FALSE;

		if (pSymbol->get_symTag(&dwSymTag) != S_OK)
			continue;

		if (dwSymTag == SymTagThunk)
			continue;

		pSymbol->get_function(&ret);

		if (ret)
		{
			if (++testcount2 % 10000 == 0)
				std::wcout << L"[CollectAllFunctions] count: " << testcount2 << std::endl;

			SymbolFunction sf;
			if (MakeFunctionSymbolUndecorated(pSymbol, sf))
				outVec.emplace_back(sf);
		}

		pSymbol->Release();
	}

	pEnumSymbols->Release();

	return true;
}

void EnumerateAllByAddr(IDiaSession* session_, std::vector<std::wstring>& namelist, std::vector<ULONGLONG>& ptrlist)
{
	HRESULT hr;

	CComPtr<IDiaEnumSymbolsByAddr> pEnumByAddr;
	if (FAILED(session_->getSymbolsByAddr(&pEnumByAddr)))
	{
		std::cout << "[failed] getSymbolsByAddr" << std::endl;
		return;
	}
	CComPtr<IDiaSymbol> pSym;
	if (FAILED(pEnumByAddr->symbolByAddr(1, 0, &pSym)))
	{
		std::cout << "[failed] symbolByAddr" << std::endl;
		return;
	}

	DWORD rvaLast = 0;
	DWORD celt = 0;
	if (pSym->get_relativeVirtualAddress(&rvaLast) == S_OK)
	{
		pSym = 0;
		if (FAILED(pEnumByAddr->symbolByRVA(rvaLast, &pSym)))
		{
			std::cout << "[failed] symbolByAddr" << std::endl;
			return;
		}

		printf("Symbols in order\n");
		do
		{
			DWORD tag;
			if (pSym->get_symTag(&tag) == S_OK)
			{
				//std::wcout << "\t[tag] " << getName<enum SymTagEnum>((enum SymTagEnum)tag) << std::endl;
				if (tag != SymTagFunction)
				{
					pSym = 0;
					celt = 0;
					if (FAILED(hr = pEnumByAddr->Next(1, &pSym, &celt)))
						break;
					continue;
				}
			}

			BSTR name;
			if (pSym->get_name(&name) != S_OK)
			{
				//printf("\t0x%08X (%ws) <no name>\n", rvaLast);
			}
			else
			{
				//printf("\t0x%08X %ws\n", rvaLast, name);
			}

			ULONGLONG va;
			if (pSym->get_virtualAddress(&va) == S_OK)
			{
				//std::wcout << L"  virtualAddress\t" << std::hex << va << std::dec << std::endl;
			}

			namelist.emplace_back(name);
			ptrlist.push_back(va);

			pSym = 0;
			celt = 0;
			if (FAILED(hr = pEnumByAddr->Next(1, &pSym, &celt)))
			{
				break;
			}
		} while (celt == 1);
	}
}