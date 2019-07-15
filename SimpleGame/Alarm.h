#pragma once
using CUINT64 = const unsigned long long;
using CINT64 = const long long;
using CULONG = const unsigned long;
using CBOOL = const bool;

class Task
{
};

class IObserver
{
};

class IValue
{
};

struct Handle
{
	int value;
};

namespace Suite { namespace Module { namespace Infra {
namespace NTime { namespace NAlarm {

class Alarm
{
public:
	const Handle			Regist( IObserver * pObserver, IValue * pValue, CUINT64 ullTick, CBOOL skipLog = FALSE );
	const Handle			Regist( IObserver * pObserver, IValue * pValue, CUINT64 ullTick, Task * task, CBOOL skipLog = FALSE );
	void					Cancel( const Handle & stHandle, CCHAR * szFileLocation );

	const Handle			Post( IObserver * pObserver, IValue * pValue, CUINT64 ullTick, CBOOL skipLog = FALSE );
	const Handle			Post( IObserver * pObserver, IValue * pValue, CUINT64 ullTick, Task * task, CBOOL skipLog = FALSE );
	const Handle			ForcePost( IObserver * pObserver, IValue * pValue, CUINT64 ullTick, CBOOL skipLog = FALSE );

	void					OnTick();
	void					OnPost();

	CINT64					GetTreeHeadNodeTick();
	CULONG					GetTreeCount();
	CULONG					GetPostListCount();

private:
	struct					Private;
	Private *				This;

};

} /* NAlarm */ } /* NTime */
} /* Infra */ } /* Module */ } /* Suite */
