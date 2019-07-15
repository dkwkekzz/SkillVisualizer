#pragma once
#include "ISync.h"
#include "IListener.h"

class SimpleDisplay : public IListener
{
public:
	virtual void SetState(MonitorOption op) override;
	virtual void Push(const Context& ctx) override;
	virtual void Pop(const Context& ctx) override;
	virtual void Close() override;

private:
	void CreateRedirect();

};