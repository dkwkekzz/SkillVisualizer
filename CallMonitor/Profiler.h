#pragma once
#include "ISync.h"
#include "IListener.h"

class Profiler : public IListener
{
public:
	virtual void SetState(MonitorOption op) override;
	virtual void Push(const MonitorContext& ctx) override;
	virtual void Pop(const MonitorContext& ctx) override;

private:
	struct ProfileInfo
	{
		ProfileInfo()
		{
			bFilled = false;
		}

		//Name of the function
		char	m_sFunName[256];

		//Thread executig the function
		DWORD   m_dwThreadID;

		//Start time
		LARGE_INTEGER	m_liStartTime;

		//End time
		LARGE_INTEGER	m_liEndTime;

		bool	bFilled;

		void Display(std::ostream& os);
	};

	void DisplayProfileData();

private:
	std::map<int, vector<ProfileInfo> > g_mapProfileInfo;

};