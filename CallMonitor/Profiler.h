#pragma once
#include "ISync.h"
#include "IListener.h"

class Profiler : public IListener
{
public:
	virtual void SetState(MonitorOption op) override;
	virtual void Push(const MonitorContext& ctx) override;
	virtual void Pop(const MonitorContext& ctx) override;
	virtual void Action(const ActType at) override;

private:
	struct ProfileInfo
	{
		//Start time
		LARGE_INTEGER	m_liStartTime;

		//elapsed time
		LARGE_INTEGER	m_liElapsedTime;

		int overlap;
	};

	void DisplayProfileData();

private:
	static std::map<int, map<std::string, ProfileInfo> > g_mapProfileInfo;

	bool m_run { true };

};