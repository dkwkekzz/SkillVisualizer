#pragma once
namespace Suite
{
	using CUINT64 = const unsigned long long;
	using CINT64 = const long long;
	using CULONG = const unsigned long;
	using CBOOL = const bool;
	using CCHAR = const char;

	class IObserver
	{
	public:
		virtual ~IObserver() {}
		virtual void			OnAlarm(CUINT64 ullTick) = 0;
		virtual void			OnCancel(CUINT64 ullTick) = 0;
	};
}

class Task
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
	Alarm();

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

namespace Suite { namespace Game { namespace Function {
namespace NTimer {

class BuffPackage : public IObserver
{
public:
	virtual void			OnAlarm( CUINT64 ullTick );
	virtual void			OnCancel( CUINT64 ullTick );

private:
	struct					Private;
	Private *				This;

};

} /* NTimer */
} /* Function */ } /* Game */ } /* Suite */