#pragma once

class ISync
{
public:
	virtual ~ISync() {}
	virtual void Init() {}
	virtual void Enter() = 0;
	virtual void Leave() = 0;
};