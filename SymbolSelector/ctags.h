#pragma once
#include "pch.h"
#include "spinlock.h"

struct symbol
{
	int uid;
	std::string tag;
	char type;
	bool is_public;
};

class ctags
{
public:
	using symbollist = std::vector<symbol>;
	using Predictator = bool(*)(const symbol& sym);

	ctags(const char * root, const char * output, int tc = 1);
	void install();
	void uninstall();
	void flush();

	inline void select(Predictator pf)
	{
		predictators.emplace_back(pf);
	}

private:
	struct token
	{
		int begin;
		int len;
		token(int b, int l) : begin(b), len(l) {}
		inline std::string to_string(const std::string& src) const { return src.substr(begin, len); }
	};

	const char * rootPath;
	const char * outputPath;
	const int threadCount;
	symbollist symbols;
	std::vector<Predictator> predictators;
	spinlock lock;

private:
	void _insert(const std::string & line, const std::vector<token> & tvec);

};