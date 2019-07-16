#include "stdafx.h"
#include "Profiler.h"
#include "PipeServer.h"
#include "SpinLock.h"

//
// static
//

std::map<int, vector<Profiler::ProfileInfo> > Profiler::g_mapProfileInfo;
static SpinLock sync;


void
Profiler::SetState(MonitorOption op)
{
	if (op | MonitorOption::Profiling)
	{
		m_run = true;
	}
	else
	{
		m_run = false;
	}
}

void
Profiler::Push(const MonitorContext& ctx)
{
	if (!m_run)
		return;

	ProfileInfo profInfo;
	strcpy(profInfo.m_sFunName, ctx.callee);

	QueryPerformanceCounter(&profInfo.m_liStartTime);

	DWORD tid = ::GetCurrentThreadId();
	profInfo.m_dwThreadID = tid;

	sync.Enter();

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

	sync.Leave();
}

void
Profiler::Pop(const MonitorContext& ctx)
{
	if (!m_run)
		return;

	sync.Enter();

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

	sync.Leave();
}


void 
Profiler::Action(const ActType at)
{
	if (!m_run)
		return;

	if (at == ActType::PrintProfile)
	{
		DisplayProfileData();
		return;
	}

	if (at == ActType::Clear)
	{
		g_mapProfileInfo.clear();
		return;
	}
}

//Iterate through the map and for each thread id print the profile
//data strored in the vector
void
Profiler::DisplayProfileData()
{
	sync.Enter();

	std::ofstream os("ProfileData.csv");

	os << "##########################Profile Information############################" << endl;
	os << "Thread" << ',' << "Function" << ',' << "Elapsed" << endl;
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

	std::cout << "print profile data: " << g_mapProfileInfo.size() << std::endl;
	g_mapProfileInfo.clear();

	sync.Leave();
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

	os << m_dwThreadID << ',' << m_sFunName << ',' << elapsedTime.QuadPart << endl;
}
//*******************************************************************************