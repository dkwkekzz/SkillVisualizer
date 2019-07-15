
#include "pch.h"
#include "Alarm.h"

namespace Suite { namespace Module { namespace Infra {
namespace NTime { namespace NAlarm {

struct Alarm::Private
{
	const char* name = "private::alarm";
};

const Handle
Alarm::Regist( IObserver * pObserver, IValue * pValue, CUINT64 ullTick, CBOOL skipLog )
{
	std::cout << "@@Regist" << std::endl;
	return Handle();
}

const Handle
Alarm::Regist( IObserver * pObserver, IValue * pValue, CUINT64 ullTick, Task * task, CBOOL skipLog )
{
	std::cout << "@@Regist2" << std::endl;
	return Handle();
}

const Handle
Alarm::ForcePost( IObserver * pObserver, IValue * pValue, CUINT64 ullTick, CBOOL skipLog )
{
	std::cout << "ForcePost" << std::endl;
	return Handle();
}

const Handle
Alarm::Post( IObserver * pObserver, IValue * pValue, CUINT64 ullTick, CBOOL skipLog )
{
	std::cout << "Post" << std::endl;
	return Handle();
}

const Handle
Alarm::Post( IObserver * pObserver, IValue * pValue, CUINT64 ullTick, Task * task, CBOOL skipLog )
{
	std::cout << "Post2" << std::endl;
	return Handle();
}

void
Alarm::Cancel( const Handle & stHandle, CCHAR * szFileLocation )
{
	std::cout << "Cancel" << std::endl;
}

void
Alarm::OnTick()
{
	std::cout << "OnTick" << std::endl;
}

void
Alarm::OnPost()
{
	std::cout << "OnPost" << std::endl;
}

CINT64
Alarm::GetTreeHeadNodeTick()
{
	CINT64 a = 9999;
	CINT64 b = 9999;
	return a + b;
}

CULONG
Alarm::GetTreeCount()
{
	return 1;
}

CULONG
Alarm::GetPostListCount()
{
	return 1;
}

} /* NAlarm */ } /* NTime */
} /* Infra */ } /* Module */ } /* Suite */
