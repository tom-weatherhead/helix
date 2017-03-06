// UUCode.cpp - Part of Helix
// Copyright (c) 2002 by Tom Weatherhead.  All rights reserved.
// Started December 14, 2002

#include "Common.h"


void UUEncode( char * pSrcFilename, FILE * pSrcFile, FILE * pDstFile )
{
	char acTable[64];
	int i;

	acTable[0] = '`';	// We could use a space instead.

	for( i = 1; i < 64; ++i )
	{
		acTable[i] = (char)( i + 32 );
	}

	Assert( pSrcFilename != 0 );

	for( i = strlen( pSrcFilename ); i > 0  &&  pSrcFilename[i - 1] != '\\'; --i )
	{
	}

	Assert( strlen( pSrcFilename + i ) > 0 );
	fprintf( pDstFile, "begin 664 %s\n", pSrcFilename + i );

	while( !feof( pSrcFile ) )
	{
		unsigned char aucSrcData[45];
		const int knBytesRead = fread( aucSrcData, 1, 45, pSrcFile );

		fputc( acTable[knBytesRead], pDstFile );

		for( i = 0; i < knBytesRead; i += 3 )
		{
			const unsigned int k = (unsigned int)aucSrcData[i] + ( (unsigned int)aucSrcData[i + 1] << 8 ) + ( (unsigned int)aucSrcData[i + 2] << 16 );

			fputc( acTable[k & 63], pDstFile );
			fputc( acTable[(k >> 6) & 63], pDstFile );
			fputc( acTable[(k >> 12) & 63], pDstFile );
			fputc( acTable[k >> 18], pDstFile );
		}

		fputc( '\n', pDstFile );
	}

	//fprintf( pDstFile, "`\n" );
	fprintf( pDstFile, "end\n" );
}


bool UUDecode( FILE * pSrcFile )
{
	FILE * pDstFile = 0;

	while( !feof( pSrcFile ) )
	{
		// Get a line of the src text file.
		char acSrcData[128];

		fgets( acSrcData, sizeof( acSrcData ), pSrcFile );

		// Truncate any appended newline:
		acSrcData[sizeof( acSrcData ) - 1] = '\0';

		if( acSrcData[strlen( acSrcData ) - 1] == '\n' )
		{
			acSrcData[strlen( acSrcData ) - 1] = '\0';
		}

		if( strlen( acSrcData ) == 0 )
		{
			// continue;
		}
		else if( !memcmp( acSrcData, "begin", 5 )  &&  strlen( acSrcData ) > 10  &&  pDstFile == 0 )
		{
			pDstFile = fopen( acSrcData + 10, "wb" );
		}
		else if( pDstFile == 0 )
		{
			// continue;
		}
		else if( !memcmp( acSrcData, "end", 3 ) )
		{
			fclose( pDstFile );
			return( true );
		}
		else if( acSrcData[0] > 32  &&  acSrcData[0] <= 77 )
		{
			const int knBytesToWrite = (int)( acSrcData[0] - 32 );
			char acDstData[45];
			int nDstIndex = 0;

			for( int i = 1; i < strlen( acSrcData ); i += 4 )
			{
				int j;
				int anSrc1[4];

				for( j = 0; j < 4; ++j )
				{
					const char c = acSrcData[i + j];

					if( c < 32  ||  c > 96 )
					{
						break;
					}

					anSrc1[j] = (int)( ( c - 32 ) & 63 );
				}

				for( ; j < 4; ++j )
				{
					anSrc1[j] = 0;
				}

				int k = 0;

				for( j = 0; j < 4; ++j )
				{
					k |= ( anSrc1[j] << ( j * 6 ) );
				}

				for( j = 0; j < 3; ++j )
				{
					acDstData[nDstIndex++] = (char)( k >> ( j * 8 ) );
				}
			}

			fwrite( acDstData, 1, knBytesToWrite, pDstFile );
		}
	}

	if( pDstFile != 0 )
	{
		fclose( pDstFile ); // Dst file truncated?
	}

	return( false ); // No "end" found.
}


// **** End of File ****
