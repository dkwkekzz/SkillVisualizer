#pragma once
#include "stdafx.h"
#include "MonitorOption.h"
#include "MonitorContext.h"

class IListener
{
public:
	virtual ~IListener() {}
	virtual void SetState(MonitorOption op) = 0;
	virtual void Push(const MonitorContext& ctx) {}
	virtual void Pop(const MonitorContext& ctx) {}
};