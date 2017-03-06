// Helix - An RSA encryption/decryption application
// Copyright (c) 2002 by Tom Weatherhead.  All rights reserved.
// Started November 28, 2002

#include "Common.h"


void CommandGenerateKeys( void )
{
	int nBitLength = 0;
	BigNum d;
	BigNum e;
	BigNum n;
	char acFileName[64];
	char acFileName2[64];
	FILE * dstFile = 0;

	do
	{
		printf( "Bit length (128-4096) : " );
		scanf( "%d", &nBitLength );
	}
	while( nBitLength < 128  ||  nBitLength > 1000000 );

	printf( "Generating keys...\n" );
	GenerateRSAKeys( nBitLength, d, e, n );
	printf( "Keys generated.\n" );

#if 1	// Test code.

	for( ; ; )
	{
		printf( "\nTest keys for errors? (y,n,a) : " );
		char yn;
		
		do
		{
			// Eat any illegal input.
			yn = getchar();
		}
		while( yn != 'y'  &&  yn != 'n'  &&  yn != 'a' );

		if( yn == 'n' )
		{
			break;
		}
		else if( yn == 'a' )
		{
			printf( "Aborting without saving keys.\n" );
			return;
		}

		printf( "Testing keys...\n" );

		BigNum test;

		test.SetToRandom( n.NumSignificantBits() - 1 );

		const BigNum testEncrypted = ExponentMod( test, e, n );
		const BigNum testDecrypted = ExponentMod( testEncrypted, d, n );

		printf( "Original data:\n" );
		test.PrintHex();
		printf( "Encrypted:\n" );
		testEncrypted.PrintHex();
		printf( "Decrypted:\n" );
		testDecrypted.PrintHex();

		if( testDecrypted == test )
		{
			printf( "Test succeeded!\n" );
		}
		else
		{
			printf( "Test failed:\n" );
		}
	}
#endif

	printf( "The key bit length is %d.\n", nBitLength );
	printf( "Filename for new keys (without extension) : " );
	scanf( "%s", acFileName );

	// Write the public key file.
	strcpy( acFileName2, acFileName );
	strcat( acFileName2, ".pub" );
	dstFile = fopen( acFileName2, "wb" );

	if( dstFile == 0 )
	{
		printf( "Failed to open file '%s' for write.\n", acFileName2 );
		return;
	}

	e.WriteToFile( dstFile );
	n.WriteToFile( dstFile );
	fclose( dstFile );

	// Write the private key file.
	strcpy( acFileName2, acFileName );
	strcat( acFileName2, ".prv" );
	dstFile = fopen( acFileName2, "wb" );

	if( dstFile == 0 )
	{
		printf( "Failed to open file '%s' for write.\n", acFileName2 );
		return;
	}

#if 1
	// Before writing, encrypt d and n using an XOR with a password.
	char acPassword[40];

	for( ; ; )
	{
		printf( "Private key password: " );
		acPassword[0] = '\0';
		scanf( "%s", acPassword );

		if( strlen( acPassword ) >= 8 )
		{
			break;
		}

		printf( "The password must be at least 8 characters long.\n" );
	}

	d.HashWithString( acPassword );
	n.HashWithString( acPassword );
#endif

	d.WriteToFile( dstFile );
	n.WriteToFile( dstFile );
	fclose( dstFile );
}


void CommandEncryptFile( void )
{
	char acFileName[64];
	char acFileName2[64];
	FILE * keyFile = 0;
	FILE * srcFile = 0;
	FILE * dstFile = 0;
	BigNum exponent;
	BigNum n;

	printf( "Key filename (without extension) : " );
	scanf( "%s", acFileName );
	strcat( acFileName, ".pub" );
	keyFile = fopen( acFileName, "rb" );

	if( keyFile == 0 )
	{
		printf( "Failed to open file '%s' for read.\n", acFileName );
		return;
	}

	exponent.ReadFromFile( keyFile );
	n.ReadFromFile( keyFile );
	fclose( keyFile );
	printf( "The public key file has been read.\n" );

	printf( "File to encrypt: " );
	scanf( "%s", acFileName );
	srcFile = fopen( acFileName, "rb" );

	if( srcFile == 0 )
	{
		printf( "Failed to open file '%s' for read.\n", acFileName );
		return;
	}

	printf( "File to create: " );
	scanf( "%s", acFileName2 );
	dstFile = fopen( acFileName2, "wb" );

	if( dstFile == 0 )
	{
		printf( "Failed to open file '%s' for write.\n", acFileName2 );
		fclose( srcFile );
		return;
	}

	EncryptFile( srcFile, dstFile, exponent, n );
	fclose( dstFile );
	fclose( srcFile );
	printf( "Finished creating encrypted file '%s'\n", acFileName2 );
}


void CommandDecryptFile( void )
{
	char acFileName[64];
	char acFileName2[64];
	FILE * keyFile = 0;
	FILE * srcFile = 0;
	FILE * dstFile = 0;
	BigNum exponent;
	BigNum n;

	printf( "Key filename (without extension) : " );
	scanf( "%s", acFileName );
	strcat( acFileName, ".prv" );
	keyFile = fopen( acFileName, "rb" );

	if( keyFile == 0 )
	{
		printf( "Failed to open file '%s' for read.\n", acFileName );
		return;
	}

	exponent.ReadFromFile( keyFile );
	n.ReadFromFile( keyFile );
	fclose( keyFile );
	printf( "The private key file has been read.\n" );

#if 1
	// Decrypt the private key.
	char acPassword[40];

	for( ; ; )
	{
		printf( "Private key password: " );
		acPassword[0] = '\0';
		scanf( "%s", acPassword );

		if( strlen( acPassword ) >= 8 )
		{
			break;
		}

		printf( "The password must be at least 8 characters long.\n" );
	}

	exponent.HashWithString( acPassword );
	n.HashWithString( acPassword );
#endif

	printf( "File to decrypt: " );
	scanf( "%s", acFileName );
	srcFile = fopen( acFileName, "rb" );

	if( srcFile == 0 )
	{
		printf( "Failed to open file '%s' for read.\n", acFileName );
		return;
	}

	printf( "File to create: " );
	scanf( "%s", acFileName2 );
	dstFile = fopen( acFileName2, "wb" );

	if( dstFile == 0 )
	{
		printf( "Failed to open file '%s' for write.\n", acFileName2 );
		fclose( srcFile );
		return;
	}

	DecryptFile( srcFile, dstFile, exponent, n );
	fclose( dstFile );
	fclose( srcFile );
	printf( "Finished creating decrypted file '%s'\n", acFileName2 );
}


int main( void )
{
	bool bQuit = false;

	printf( "\nHelix version 0.0.0 ; December 11, 2002.\n" );
	printf( "An implementation of the RSA cryptosystem.\n" );
	printf( "Copyright (c) 2002 by Tom Weatherhead.  All rights reserved.\n" );

#if 0	// Test code.
	const unsigned short x[3] = { 0x0001, 0x0080, 0xC000 };
	const BigNum a( 5 );
	const BigNum b( 3 );
	const BigNum c = ExponentMod( b, b, a );
	
	printf( "\na; b; c = ( b ^ b ) % a:\n" );
	a.PrintDecimal();
	b.PrintDecimal();
	c.PrintDecimal();
#endif

	do
	{
		int nMenuSelection = 0;

		do
		{
			printf( "\nMain menu:\n\n" );
			printf( "1: Generate a pair of keys\n" );
			printf( "2: Encrypt a file\n" );
			printf( "3: Decrypt a file\n" );
			printf( "4: Quit\n" );
			printf( "\nEnter selection: " );
			scanf( "%d", &nMenuSelection );
		}
		while( nMenuSelection < 1  ||  nMenuSelection > 4 );

		try
		{

			switch( nMenuSelection )
			{
				case 1:
					CommandGenerateKeys();
					break;

				case 2:
					CommandEncryptFile();
					break;

				case 3:
					CommandDecryptFile();
					break;

				default:
					bQuit = true;
			}
		}
		catch( BigNumException & e )
		{
			printf( "Exception at file %s, line %d\n", e.m_pFile, e.m_nLine );
		}
		catch( ... )
		{
			printf( "Caught unknown exception in menu.\n" );
		}
	}
	while( !bQuit );

	printf( "\nHelix: Finished.\n" );
	return( 0 );
}


// **** End of File ****
