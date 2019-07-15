#pragma once
#include <vector>

class Option
{
public:
	Option();
	bool Vaild(const wchar_t* name);

private:
	struct Line
	{
		int priority;
		bool(*checker)(const wchar_t* keyword, const wchar_t* src);
		std::wstring target;
		bool res;
	};

	std::vector< Line > lines;

};