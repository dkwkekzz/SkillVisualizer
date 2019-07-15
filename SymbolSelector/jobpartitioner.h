#pragma once
#include "pch.h"

class jobpartitioner
{
	Concurrency::concurrent_queue<std::pair<int, int>> chkq;

public:
	jobpartitioner(int tc, int len)
	{
		if (tc == 1)
		{
			chkq.push(std::pair<int, int>(0, len));
		}
		else
		{
			int cur = 0;
			int count = tc;
			int chunk = std::max<int>((len >> 2) / tc, 1);
			while (cur < len)
			{
				int f = std::min<int>(cur, len);
				int s = std::min<int>((cur += chunk), len);
				if (f == s)
					break;

				chkq.push(std::pair<int, int>(f, s));

				if (chunk <= 2)
					continue;

				if (--count == 0)
				{
					chunk >>= 1;
					count = tc;
				}
			}
		}
	}

	inline const bool get_job(std::pair<int, int>& dest)
	{
		return chkq.try_pop(dest);
	}
};
