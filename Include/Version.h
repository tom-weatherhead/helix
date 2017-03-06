// Version.h - Part of Helix
// Copyright (c) 2002 by Tom Weatherhead.  All rights reserved.
// Started December 17, 2002


#ifndef _HVERSION_H_
#define _HVERSION_H_


class CVersion
{
private:
	unsigned int m_unPrimaryVersion;
	unsigned int m_unSecondaryVersion;
	unsigned int m_unTertiaryVersion;

public:
	CVersion( unsigned int v1 = 0, unsigned int v2 = 0, unsigned int v3 = 0 );
	CVersion( FILE * pSrcFile );
	void ReadFromFile( FILE * pSrcFile );
	void WriteToFile( FILE * pDstFile ) const;
	void Print( void ) const;
	bool operator==( const CVersion & Src ) const;
	bool operator<( const CVersion & Src ) const;

	inline bool operator!=( const CVersion & Src ) const
	{
		return( !( *this == Src ) );
	}

	inline bool operator>( const CVersion & Src ) const
	{
		return( Src < *this );
	}

	inline bool operator<=( const CVersion & Src ) const
	{
		return( !( Src < *this ) );
	}

	inline bool operator>=( const CVersion & Src ) const
	{
		return( !( *this < Src ) );
	}

	inline unsigned int GetPrimaryVersion( void ) const
	{
		return( m_unPrimaryVersion );
	}

	inline unsigned int GetSecondaryVersion( void ) const
	{
		return( m_unSecondaryVersion );
	}

	inline unsigned int GetTertiaryVersion( void ) const
	{
		return( m_unTertiaryVersion );
	}
}; // class CVersion


#endif // _HVERSION_H_


// **** End of File ****
