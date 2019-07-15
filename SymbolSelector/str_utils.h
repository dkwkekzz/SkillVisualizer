// str_utils.h
#include "pch.h"

template<typename T1, typename T2, typename T3>
const std::string 
replace_all(T1&& str, T2&& from, T3&&  to)
{
	size_t start_pos = 0; //string처음부터 검사
	while ((start_pos = str.find(from, start_pos)) != std::string::npos)  //from을 찾을 수 없을 때까지
	{
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // 중복검사를 피하고 from.length() > to.length()인 경우를 위해서
	}
	return str;
}

template<typename T>
const std::string 
remove_ext(T&& str)
{
	int pos = str.rfind(".");
	return str.substr(0, pos);
}

template<typename T>
const std::string
exchange_ext(T&& str, T&& right)
{
	int pos = str.rfind(".");
	return str.replace(pos, str.length() - pos, right);
}

template<typename T1>
const std::string
remove_space(T1&& str)
{
	char temp[_MAX_PATH << 2];
	int j = 0;
	for (int i = 0; i != str.size(); i++)
	{
		if (str[i] == ' ')
			continue;
		if (str[i] == '\t')
			continue;
		temp[j++] = str[i];
	}
	temp[j] = '\0';

	return std::string(temp);
}

template<typename T>
const std::string
backward_namespace(T&& src)
{
	int pos = 0;
	if ((pos = src.find("::")) == std::string::npos)
		return "";
	return src.substr(pos + 2);
}

template<typename T>
const std::vector<std::string>
split(T&& src, const char token)
{
	std::vector<std::string> res;
	int pos = 0;
	for (int i = 0; i != src.size(); i++)
	{
		if (src[i] == token)
		{
			res.emplace_back(std::move(src.substr(pos, i - pos)));
			pos = i + 1;
		}
	}

	res.emplace_back(std::move(src.substr(pos)));
	return res;
}

template<typename T>
const std::vector<std::string>
split(T&& src, const char * tokens)
{
	std::vector<std::string> res;
	int pos = 0;
	int tlen = strlen(tokens);
	for (int i = 0; i != src.size(); i++)
	{
		for (int ti = 0; ti != tlen; ti++)
		{
			if (src[i] == tokens[ti])
			{
				res.emplace_back(std::move(src.substr(pos, i - pos)));
				pos = i + 1;
				break;
			}
		}
	}

	res.emplace_back(std::move(src.substr(pos)));
	return res;
}
