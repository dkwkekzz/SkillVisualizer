
#include "pch.h"
#include "Alarm.h"
#include <mutex>

namespace Suite { namespace Module { namespace Infra {
namespace NTime { namespace NAlarm {

struct Alarm::Private
{
	const char* name = "private::alarm";
	UINT64 tick = 0;
	IObserver * ob{nullptr};
	mutex m;
};

Alarm::Alarm()
{
	This = new Alarm::Private;
}

const Handle
Alarm::Regist( IObserver * pObserver, IValue * pValue, CUINT64 ullTick, CBOOL skipLog )
{
	This->m.lock();
	std::cout << "@@Regist" << std::endl;
	This->ob = pObserver;
	This->m.unlock();
	return Handle();
}

const Handle
Alarm::Regist( IObserver * pObserver, IValue * pValue, CUINT64 ullTick, Task * task, CBOOL skipLog )
{
	This->m.lock();
	std::cout << "@@Regist2" << std::endl;
	This->ob = pObserver;
	This->m.unlock();
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
	This->m.lock();
	if (This->ob)
	{
		This->ob->OnAlarm(This->tick += 10);
		This->ob = nullptr;
	}
	This->m.unlock();
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

namespace Suite { namespace Game { namespace Function {
namespace NTimer {

void
BuffPackage::OnAlarm( CUINT64 ullTick )
{
	std::cout << "@@BuffPackage::OnAlarm1" << std::endl;
	std::cout << "@@BuffPackage::OnAlarm2" << std::endl;
	std::cout << "@@BuffPackage::OnAlarm3" << std::endl;
}

void
BuffPackage::OnCancel( CUINT64 ullTick )
{
	std::cout << "@@OnCancel" << std::endl;
}

} /* NTimer */
} /* Function */ } /* Game */ } /* Suite */
