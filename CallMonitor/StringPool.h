#pragma once
#include <queue>

class StringPool
{
public:
	StringPool();
	StringPool(int capacity);
	char * Create();
	void Destroy(const char * p);

private:
	std::queue< char* > _queue;

};