#pragma once


namespace Suite { namespace Module { namespace Toolkit {
namespace NFetch { namespace NFlyweight {

class IValue
{
public:
	virtual					~IValue() {}
	virtual const char *			GetName() = 0;
};

class Index
:	public					IValue
{
public:
	Index(const char* name) : m_szName(name) { }

	int						Get() const
	{
		return m_wValue;
	}

	const int				Get()
	{
		return m_wValue;
	}
	
private:
	virtual const char *	GetName()	{ return m_szName; }

	int						m_wValue;
	const char *			m_szName;
};

class Integer64
	: public					IValue
{
public:
	Integer64(const char* name) : m_szName(name) { }

	long long				Get() const
	{
		return m_wValue;
	}

	const long long				Get()
	{
		return m_wValue;
	}

private:
	virtual const char *	GetName() { return m_szName; }

	long long				m_wValue;
	const char *			m_szName;
};

} /* NFlyweight */ } /* NFetch */
} /* Toolkit */ } /* Module */ } /* Suite */
