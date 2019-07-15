#pragma once
#include "stdafx.h"
#include "ISync.h"

class CriticalSection : public ISync
{
	CRITICAL_SECTION g_csProfileInfo;

public:
	virtual void Init() override;
	virtual void Enter() override;
	virtual void Leave() override;

};