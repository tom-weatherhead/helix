// RSAKey.h - Part of Helix
// Copyright (c) 2002 by Tom Weatherhead.  All rights reserved.
// Started December 17, 2002


#ifndef _RSAKEY_H_
#define _RSAKEY_H_


class CHelixRSAKey
{
private:
	CVersion m_version;
	bool m_bIsPrivate;
	BigNum m_exponent;
	BigNum m_n;

public:
	CHelixRSAKey( bool bIsPrivate, const CVersion & v, const BigNum & e, const BigNum & n );
	CHelixRSAKey( FILE * pSrcFile );
	void WriteToFile( FILE * pDstFile ) const;
	static void HashNumbers( BigNum & d, BigNum & n );
	
	inline const BigNum & GetExponent( void ) const
	{
		return( m_exponent );
	}

	inline const BigNum & GetN( void ) const
	{
		return( m_n );
	}
}; // class CHelixRSAKey


#endif // _RSAKEY_H_


// **** End of File ****
