#pragma once

enum MonitorOption
{
	None = 0,
	RealTime = 1,
	Log = 1 << 1,
	Display = 1 << 2,
	Profiling = 1 << 3,
	Commander = 1 << 4,
};

inline MonitorOption operator|(MonitorOption a, MonitorOption b)
{
	return static_cast<MonitorOption>(static_cast<int>(a) | static_cast<int>(b));
}

inline MonitorOption operator&(MonitorOption a, MonitorOption b)
{
	return static_cast<MonitorOption>(static_cast<int>(a) & static_cast<int>(b));
}

inline void operator|=(MonitorOption& a, MonitorOption b)
{
	int ret = static_cast<int>(a) | static_cast<int>(b);
	a = static_cast<MonitorOption>(ret);
}

inline void operator&=(MonitorOption& a, MonitorOption b)
{
	int ret = static_cast<int>(a) & static_cast<int>(b);
	a = static_cast<MonitorOption>(ret);
}