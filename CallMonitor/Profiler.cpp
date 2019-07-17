#include "stdafx.h"
#include "Profiler.h"
#include "PipeServer.h"
#include "SpinLock.h"

//
// static
//

std::map<int, map<std::string, Profiler::ProfileInfo> > Profiler::g_mapProfileInfo;
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

	sync.Enter();

	DWORD tid = ::GetCurrentThreadId();
	if (g_mapProfileInfo.count(tid) == 0)
	{
		map<std::string, Profiler::ProfileInfo> mapInst;
		g_mapProfileInfo.emplace(tid, mapInst);
	}

	const auto& itMap = g_mapProfileInfo.find(tid);
	auto& mapProfInfo = (*itMap).second;
	const auto& itProfInfo = mapProfInfo.find(ctx.callee);
	if (itProfInfo != mapProfInfo.end())
	{
		auto& infoPair = *itProfInfo;
		ProfileInfo& info = infoPair.second;

		QueryPerformanceCounter(&info.m_liStartTime);

		info.overlap++;
	}
	else
	{
		ProfileInfo profInfo;

		QueryPerformanceCounter(&profInfo.m_liStartTime);

		profInfo.m_liElapsedTime.QuadPart = 0;
		profInfo.overlap = 1;

		mapProfInfo.emplace(ctx.callee, profInfo);
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
	if (g_mapProfileInfo.count(tid) == 0)
	{
		map<std::string, Profiler::ProfileInfo> mapInst;
		g_mapProfileInfo.emplace(tid, mapInst);
	}

	const auto& itMap = g_mapProfileInfo.find(tid);
	auto& mapProfInfo = (*itMap).second;
	const auto& itProfInfo = mapProfInfo.find(ctx.callee);
	if (itProfInfo != mapProfInfo.end())
	{
		auto& infoPair = *itProfInfo;
		ProfileInfo& info = infoPair.second;

		LARGE_INTEGER endTime;
		QueryPerformanceCounter(&endTime);

		info.m_liElapsedTime.QuadPart += endTime.QuadPart - info.m_liStartTime.QuadPart;
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
	os << "Thread" << ',' << "Function" << ',' << "Elapsed" << ',' << "Overlap" << endl;
	map<int, map<std::string, Profiler::ProfileInfo> >::iterator itThread = g_mapProfileInfo.begin();
	for (; itThread != g_mapProfileInfo.end(); ++itThread)
	{
		DWORD tid = (*itThread).first;

		map<std::string, Profiler::ProfileInfo> &vecProfileInfo = itThread->second;
		map<std::string, Profiler::ProfileInfo>::iterator it = vecProfileInfo.begin();
		for (; it != vecProfileInfo.end(); ++it)
		{
			const std::string& funcName = (*it).first;

			const ProfileInfo& info = (*it).second;

			//Number of ticks-per-sec
			LARGE_INTEGER freq;
			QueryPerformanceFrequency(&freq);

			const auto& elapsed = (info.m_liElapsedTime.QuadPart) * 1000 * 1000 / freq.QuadPart;

			os << tid << ',' << funcName << ',' << elapsed << ',' << info.overlap << endl;
		}
	}

	os.close();

	std::cout << "print profile data: " << g_mapProfileInfo.size() << std::endl;
	g_mapProfileInfo.clear();

	sync.Leave();
}