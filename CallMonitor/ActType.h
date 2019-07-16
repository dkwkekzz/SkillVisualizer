#pragma once

enum ActType
{
	PrintLog,
	PrintProfile,
	Clear,
};

inline ActType operator|(ActType a, ActType b)
{
	return static_cast<ActType>(static_cast<int>(a) | static_cast<int>(b));
}

inline ActType operator&(ActType a, ActType b)
{
	return static_cast<ActType>(static_cast<int>(a) & static_cast<int>(b));
}

inline void operator|=(ActType& a, ActType b)
{
	int ret = static_cast<int>(a) | static_cast<int>(b);
	a = static_cast<ActType>(ret);
}

inline void operator&=(ActType& a, ActType b)
{
	int ret = static_cast<int>(a) & static_cast<int>(b);
	a = static_cast<ActType>(ret);
}