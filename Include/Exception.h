// Exception.h - Part of Helix
// Copyright (c) 2002 by Tom Weatherhead.  All rights reserved.
// Started December 17, 2002


#ifndef _HEXCEPTION_H_
#define _HEXCEPTION_H_


class CHelixException
{
public:
	const char * m_pMsg;
	const char * m_pFile;
	int m_nLine;

	CHelixException( const char * m, const char * f, int l )
		: m_pMsg( m ),
			m_pFile( f ),
			m_nLine( l )
	{
	}
}; // class CHelixException


#define ThrowHelixException( m ) throw CHelixException( ( m ), __FILE__, __LINE__ )


#endif // _HEXCEPTION_H_


// **** End of File ****
