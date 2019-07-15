// Option.cpp
#include "pch.h"
#include "Option.h"


bool StartWith(const wchar_t* keyword, const wchar_t* src)
{
	for (int i = 0; i != wcslen(keyword); i++)
	{
		if (src[i] != keyword[i])
			return false;
	}

	return true;
}

bool Contains(const wchar_t* keyword, const wchar_t* src)
{
	bool found = std::wstring(src).find(keyword) != std::wstring::npos;
	return found;
}

bool IsConstructor(const wchar_t* keyword, const wchar_t* src)
{	
	//auto str = std::wstring(src);
	//return (str.find(L"public: __cdecl") != std::wstring::npos);
	int len = wcslen(src);
	int begin = -1;
	for (int i = len - 1; i >= 1; i--)
	{
		if (src[i] == ':' && src[i - 1] == ':')
		{
			begin = i - 2;
			break;
		}
	}
	
	if (begin < 0)
		return false;
	
	int cmplen = len - 1 - begin;
	for (int i = 0; i != cmplen; i++)
	{
		if (src[len - 1 - i] != src[begin - i])
			return false;
	}
	
	return true;
}

Option::Option()
{
	const char * ctf = "option.txt";
	std::wifstream infile(ctf);
	if (!infile.good())
	{
		std::cerr << "[Option] not exist file: " << ctf << std::endl;
		return;
	}

	while (!infile.eof())
	{
		std::wstring line;
		if (!std::getline(infile, line))
			continue;

		std::transform(line.begin(), line.end(), line.begin(), ::tolower);
		std::vector<std::wstring> tokens;
		int begin = 0;
		int pos = 0;
		while ((pos = line.find(L",", begin)) != std::wstring::npos || (pos = line.find(L"\t", begin)) != std::wstring::npos)
		{
			tokens.emplace_back(line.substr(begin, pos - begin));
			begin = pos + 1;
		}
		tokens.emplace_back(line.substr(begin, line.length() - begin));

		if (tokens.size() < 4)
			continue;

		Line myline;
		myline.target = tokens[1];
		myline.res = tokens[2] == L"true" ? true : false;
		myline.priority = _wtoi(tokens[3].c_str());
		if (tokens[0] == L"startwith")
		{
			myline.checker = StartWith;
		}
		else if (tokens[0] == L"contains")
		{
			myline.checker = Contains;
		}
		else if (tokens[0] == L"isconstructor")
		{
			myline.checker = IsConstructor;
		}

		lines.emplace_back(myline);
	}

	std::sort(lines.begin(), lines.end(), [](const Line& lhs, const Line& rhs) { return lhs.priority < rhs.priority; });

	infile.close();
}

bool
Option::Vaild(const wchar_t* name)
{
	std::wstring strName(name);
	std::transform(strName.begin(), strName.end(), strName.begin(), ::tolower);
	int i = 0;
	while (i < lines.size())
	{
		int p = lines[i].priority;

		bool ret = false;
		do
		{
			ret |= (lines[i].res == lines[i].checker(lines[i].target.c_str(), strName.c_str()));
			i++;
		} while (i < lines.size() && p == lines[i].priority);

		if (!ret)
			return false;
	}

	return true;
}