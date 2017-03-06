// Version.cpp - Part of Helix
// Copyright (c) 2002 by Tom Weatherhead.  All rights reserved.
// Started December 17, 2002

#include "Common.h"


CVersion::CVersion( unsigned int v1, unsigned int v2, unsigned int v3 )
	: m_unPrimaryVersion( v1 ),
		m_unSecondaryVersion( v2 ),
		m_unTertiaryVersion( v3 )
{
}


CVersion::CVersion( FILE * pSrcFile )
{
	ReadFromFile( pSrcFile );
}


void CVersion::ReadFromFile( FILE * pSrcFile )
{
	unsigned int aunV[3];
	const int knIntsRead = fread( aunV, sizeof( unsigned int ), 3, pSrcFile );

	if( knIntsRead != 3 )
	{
		Signal();
		ThrowHelixException( "Failure to read version from file." );
	}

#ifdef BIG_ENDIAN
	
	for( int i = 0; i < 3; ++i )
	{
		ByteSwapUnsignedInt( aunV[i] );
	}
#endif

	m_unPrimaryVersion = aunV[0];
	m_unSecondaryVersion = aunV[1];
	m_unTertiaryVersion = aunV[2];
}


void CVersion::WriteToFile( FILE * pDstFile ) const
{
	int aunV[3];

	aunV[0] = m_unPrimaryVersion;
	aunV[1] = m_unSecondaryVersion;
	aunV[2] = m_unTertiaryVersion;

#ifdef BIG_ENDIAN
	
	for( int i = 0; i < 3; ++i )
	{
		ByteSwapUnsignedInt( aunV[i] );
	}
#endif

	const int knIntsWritten = fwrite( aunV, sizeof( unsigned int ), 3, pDstFile );

	if( knIntsWritten != 3 )
	{
		Signal();
		ThrowHelixException( "Failure to write version to file." );
	}
}


void CVersion::Print( void ) const
{
	printf( "%d.%d.%d", m_unPrimaryVersion, m_unSecondaryVersion, m_unTertiaryVersion );
}


bool CVersion::operator==( const CVersion & Src ) const
{
	return(
		m_unPrimaryVersion == Src.m_unPrimaryVersion  &&
		m_unSecondaryVersion == Src.m_unSecondaryVersion  &&
		m_unTertiaryVersion == Src.m_unTertiaryVersion );
}


bool CVersion::operator<( const CVersion & Src ) const
{
	return(
		m_unPrimaryVersion < Src.m_unPrimaryVersion  ||
		( m_unPrimaryVersion == Src.m_unPrimaryVersion  &&
			m_unSecondaryVersion < Src.m_unSecondaryVersion )  ||
		( m_unPrimaryVersion == Src.m_unPrimaryVersion  &&
			m_unSecondaryVersion == Src.m_unSecondaryVersion  &&
			m_unTertiaryVersion < Src.m_unTertiaryVersion ) );
}


// **** End of File ****
