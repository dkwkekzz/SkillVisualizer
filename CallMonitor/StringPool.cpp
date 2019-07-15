// StringPool.cpp
#include "stdafx.h"
#include "StringPool.h"
#define MAX_LENGTH 256

StringPool::StringPool()
{
}

StringPool::StringPool(int capacity)
{
	for (int i = 0; i != capacity; i++)
	{
		auto * p = new char[MAX_LENGTH];
		_queue.push(p);
	}
}

char * 
StringPool::Create()
{
	if (_queue.empty())
	{
		auto * p = new char[MAX_LENGTH];
		_queue.push(p);
	}

	auto * ret = _queue.front();
	_queue.pop();
	return ret;
}

void 
StringPool::Destroy(const char * p)
{
	auto * mp = const_cast<char*>(p);
	mp[0] = '\0';
	_queue.push(mp);
}