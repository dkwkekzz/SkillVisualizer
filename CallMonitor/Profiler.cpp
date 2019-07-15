#include "stdafx.h"
#include "Profiler.h"
#include "PipeServer.h"

#define BUFSIZE 512 


void
Profiler::SetState(MonitorOption op)
{
}

void
Profiler::Push(const MonitorContext& ctx)
{
	ProfileInfo profInfo;
	strcpy(profInfo.m_sFunName, ctx.callee);

	QueryPerformanceCounter(&profInfo.m_liStartTime);

	DWORD tid = ::GetCurrentThreadId();
	profInfo.m_dwThreadID = tid;

	map<int, vector<ProfileInfo> >::iterator itTID = g_mapProfileInfo.find(tid);
	if (itTID != g_mapProfileInfo.end())
	{
		itTID->second.push_back(profInfo);
	}
	else
	{
		vector<ProfileInfo> vecProfInfo;
		vecProfInfo.push_back(profInfo);
		g_mapProfileInfo.insert(make_pair(tid, vecProfInfo));
	}
}

void
Profiler::Pop(const MonitorContext& ctx)
{
	DWORD tid = ::GetCurrentThreadId();
	map<int, vector<ProfileInfo> >::iterator itTID = g_mapProfileInfo.find(tid);
	if (itTID != g_mapProfileInfo.end())
	{
		vector<ProfileInfo>	&vecProfileInfo = itTID->second;
		vector<ProfileInfo>::iterator it = vecProfileInfo.end();
		--it;
		while (1)
		{
			if (strcmp(it->m_sFunName, ctx.callee) == 0 && (!it->bFilled))
			{
				QueryPerformanceCounter(&it->m_liEndTime);
				it->bFilled = true;
				break;
			}
			if (it == vecProfileInfo.begin())
				break;
			--it;
		}
	}
}

//Iterate through the map and for each thread id print the profile
//data strored in the vector
void
Profiler::DisplayProfileData()
{
	std::ofstream os("ProfileData");

	os << "##########################Profile Information############################" << endl;
	os << "Function" << ',' << "Thread" << ',' << "Elapsed" << endl;
	map<int, vector<ProfileInfo> >::iterator itThread = g_mapProfileInfo.begin();
	for (; itThread != g_mapProfileInfo.end(); ++itThread)
	{
		vector<ProfileInfo> &vecProfileInfo = itThread->second;
		vector<ProfileInfo>::iterator it = vecProfileInfo.begin();
		for (; it != vecProfileInfo.end(); ++it)
		{
			it->Display(os);
		}
	}

	os.close();
}

//*******************************************************************************
void
Profiler::ProfileInfo::Display(std::ostream& os)
{
	//Number of ticks
	LARGE_INTEGER elapsedTime;
	elapsedTime.QuadPart = m_liEndTime.QuadPart - m_liStartTime.QuadPart;

	//Number of ticks-per-sec
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);

	elapsedTime.QuadPart = (elapsedTime.QuadPart) * 1000 * 1000 / freq.QuadPart;

	os << m_sFunName << ',' << m_dwThreadID << ',' << elapsedTime.QuadPart << endl;
}
//*******************************************************************************