// Symbols.cpp : DLL 응용 프로그램을 위해 내보낸 함수를 정의합니다.
//

#include "stdafx.h"
#include "Symbols.h"
#include "SymbolTable.h"


void ReadTable(const std::wstring& wszFilename, const void* base, std::vector<std::pair<std::wstring, void*> >& symbols)
{
	SymbolTable st(wszFilename);
	st.SetBase((uint64_t)base);

	std::vector<std::pair<std::wstring, void*> > temp(symbols.size());
	st.EnumerateAllByAddr(&temp);

	std::vector<std::wstring> includes;
	std::vector<std::wstring> excludes;
	ReadLines(L"includes.txt", includes);
	ReadLines(L"excludes.txt", excludes);

	for (const std::pair<std::wstring, void*>& sympair : temp)
	{
		if (Vaild(sympair.first, includes, excludes))
		{
			std::wcout << "[ReadTable] emplace: " << sympair.first << " / " << sympair.second << std::endl;
			symbols.emplace_back(sympair);
		}
	}
}

bool ReadLines(const wchar_t* fileName, std::vector<std::wstring>& outlist)
{
	std::wifstream infile(fileName);
	if (!infile.good())
	{
		std::cerr << "[ReadInclude] not exist file: " << fileName << std::endl;
		return false;
	}

	while (!infile.eof())
	{
		std::wstring line;
		if (!std::getline(infile, line))
			continue;

		outlist.emplace_back(line);
	}

	infile.close();
	return true;
}

bool Vaild(const std::wstring& src, const std::vector<std::wstring>& includes, const std::vector<std::wstring>& excludes)
{
	for (const auto & inc : includes)
	{
		if (!StrStr(src.c_str(), inc.c_str()))
			return false;
	}

	for (const auto & exc : excludes)
	{
		if (StrStr(src.c_str(), exc.c_str()))
			return false;
	}

	return true;
}
