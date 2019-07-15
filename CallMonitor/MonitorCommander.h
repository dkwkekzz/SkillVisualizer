#pragma once
#include "ISync.h"
#include "IListener.h"

class MonitorCommander : public IListener
{
public:
	virtual void SetState(MonitorOption op) override;
	virtual void Close() override;

private:
	void CreateRedirect();

};