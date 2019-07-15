#pragma once
#include <cassert>

template< class Type >
class Single
{
public:
	typedef Type	TYPE;

public:
	static Type *	Instance();
	static Type *	INSTANCE();

protected:
	Single();
	virtual			~Single();

protected:
	static Type *	s_pInstance;

};

template< class Type >
Type *
Single<Type>::s_pInstance	= NULL;

template< class Type >
inline
Single<Type>::Single()
{
	assert( s_pInstance == NULL );

	s_pInstance = static_cast<Type *>( this );
}

template< class Type >
inline
Single<Type>::~Single()
{
	assert( s_pInstance != NULL );

	s_pInstance = NULL;
}

template< class Type >
inline Type *
Single<Type>::Instance()
{
	return s_pInstance;
}

template< class Type >
inline Type *
Single<Type>::INSTANCE()
{
	Type * pInstance = Instance();
	assert( pInstance != NULL );

	return pInstance;
}
