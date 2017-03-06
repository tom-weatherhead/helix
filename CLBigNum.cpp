// CLBigNum.cpp - Part of Helix
// Copyright (c) 2002 by Tom Weatherhead.  All rights reserved.
// Started December 21, 2002

// This file contains an implementation of a limited-size big number class.
// This allows the buffer to be allocated as an array when the object is created,
// so that the buffer is contiguous and doesn't need to grow.

// The CLBigNumFactory class allows CLBigNum objects to be reused,
// instead of unnecessarily created and destroyed.

// The CLBigNum class currently requires a little-endian architecture.

#include "Common.h"


class CLBigNumFactory;

class CLBigNum	// Limited-size big number.
{
private:
	typedef unsigned long SegmentType;

	CLBigNumFactory & m_Factory;
	SegmentType m_a[knNumSegments];
	int m_nSize;	// Number of segments in use.

public:
	CLBigNum( CLBigNumFactory & factory );
	CLBigNumFactory & GetFactory( void );	// Const or not const?
	void Reset( void );

	// Insert other members here.
}; // class CLBigNum


CLBigNum::CLBigNum( CLBigNumFactory & factory )
	: m_Factory( factory )
{
	Reset();
}


CLBigNumFactory & CLBigNum::GetFactory( void )
{
	return( m_Factory );
}


void CLBigNum::Reset( void )
{
	memset( m_a, 0, knNumSegments * sizeof( SegmentType ) );
	m_nSize = 0;
}


class CLBigNumFactory
{
private:
	stack<CLBigNum *> m_stack;

private:
	CLBigNumFactory( void );
	~CLBigNumFactory( void );	// Not virtual, so not part of a class heirarchy.
	CLBigNum * Acquire( void );
	void Release( CLBigNum * p );
}; // class CLBigNumFactory


CLBigNumFactory::CLBigNumFactory( void )
{
}


CLBigNumFactory::~CLBigNumFactory( void )
{

	while( !m_stack.empty() )
	{
		CLBigNum * p = m_stack.top();

		m_stack.pop();
		delete p;
	}
}


CLBigNum * CLBigNumFactory::Acquire( void )
{
	CLBigNum * p = 0;

	if( !m_stack.empty() )
	{
		p = m_stack.top();
		m_stack.pop();
		p->Reset();
	}
	else
	{

		try
		{
			p = new CLBigNum( *this );
		}
		catch( ... )
		{
		}

		if( p == 0 )
		{
			ThrowHelixException( "Failed to create a CLBigNum object." );
		}
	}

	return( p );
}


void CLBigNumFactory::Release( CLBigNum * p )
{
	m_stack.push( p );
}


class CLBigNumPtr
{
private:
	CLBigNum * m_ptr;

public:

	CLBigNumPtr( CLBigNumFactory & factory )
		: m_ptr( factory.Acquire() )
	{
	}

	~CLBigNumPtr( void )	// Not virtual, so not part of a class heirarchy.
	{
		m_ptr->GetFactory().Release( m_ptr );
		m_ptr = 0;
	}

	inline CLBigNum & operator*( void )
	{
		return( *m_ptr );
	}

	inline CLBigNum * operator->( void )
	{
		return( m_ptr );
	}
}; // class CLBigNumPtr


// **** End of File ****
