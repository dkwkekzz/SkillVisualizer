#pragma once
#include "ISync.h"
#include "IListener.h"

class CallStack : public IListener
{
public:
	CallStack();

	virtual void SetState(MonitorOption op) override;
	virtual void Push(const MonitorContext& ctx) override;
	virtual void Pop(const MonitorContext& ctx) override;

};