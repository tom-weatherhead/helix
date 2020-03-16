// RSAKey.cpp - Part of Helix
// Copyright (c) 2002 by Tom Weatherhead.  All rights reserved.
// Started December 17, 2002

#include "Common.h"


CHelixRSAKey::CHelixRSAKey( bool bIsPrivate, const CVersion & v, const BigNum & e, const BigNum & n )
	: m_version( v ),
		m_bIsPrivate( bIsPrivate ),
		m_exponent( e ),
		m_n( n )
{
}


CHelixRSAKey::CHelixRSAKey( FILE * pSrcFile )
{
	m_version.ReadFromFile( pSrcFile );

	unsigned int unBitField = 0;

	if( fread( &unBitField, sizeof( unBitField ), 1, pSrcFile ) != 1 )
	{
		Signal();
		ThrowHelixException( "Failure to read RSA key bit field." );
	}

#ifdef BIG_ENDIAN
	ByteSwapUnsignedInt( unBitField );
#endif

	m_bIsPrivate = ( unBitField & 1 ) != 0;

	printf( "Reading a version " );
	m_version.Print();
	printf( " %s key.\n", m_bIsPrivate ? "private" : "public" );

	m_exponent.ReadFromFile( pSrcFile );
	m_n.ReadFromFile( pSrcFile );

	if( m_bIsPrivate )
	{
		HashNumbers( m_exponent, m_n );
	}
}


void CHelixRSAKey::WriteToFile( FILE * pDstFile ) const
{
	// Version 0.1.0 key file header:
	// 12 bytes for the version of Helix that created it;
	// 4 bytes for a bitfield:
	// - bit 0 : 0 for public key, 1 for private.
	m_version.WriteToFile( pDstFile );

	unsigned int unBitField = 0;

	unBitField |= m_bIsPrivate ? 1 : 0;

#ifdef BIG_ENDIAN
	ByteSwapUnsignedInt( unBitField );
#endif

	if( fwrite( &unBitField, sizeof( unBitField ), 1, pDstFile ) != 1 )
	{
		Signal();
		ThrowHelixException( "Failure to write RSA key bit field." );
	}

	if( m_bIsPrivate )
	{
		// WriteToFile() is const, so hash copies of the numbers.
		BigNum e = m_exponent;
		BigNum n = m_n;

		HashNumbers( e, n );
		e.WriteToFile( pDstFile );
		n.WriteToFile( pDstFile );
	}
	else
	{
		m_exponent.WriteToFile( pDstFile );
		m_n.WriteToFile( pDstFile );
	}
}


void CHelixRSAKey::HashNumbers( BigNum & d, BigNum & n )
{
	// After reading or before writing the private key,
	// encrypt d and n using an XOR with a password.
	char acPassword[41];

	do
	{
		printf( "Private key password (8 to %d chars): ", (int)(sizeof( acPassword ) - 1) );
		acPassword[0] = '\0';
		scanf( "%s", acPassword );
	}
	while( strlen( acPassword ) < 8 );

	d.HashWithString( acPassword );
	n.HashWithString( acPassword );
}


// **** End of File ****
