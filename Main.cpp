// Helix - An RSA encryption/decryption application
// Copyright (c) 2002 by Tom Weatherhead.  All rights reserved.
// Started November 28, 2002

// **** BEGIN Release History ****

// 0.0.0	December 12, 2002
// - Initial release.  Key generation, encryption, decryption.

// 0.1.0	December 18, 2002
// - Digital Signatures: Allow encryption of a message with a private key.
// - Migrate keys from previous file formats (specifically, 0.0.0) to the current file format.
// - Embed the Helix version number in keys and messages.
// - Byte swap multi-byte data on big-endian machines after read and before write.
// - Allow password changes on private keys.
// - UUEncoding and UUDecoding added.
// - In MultiplicativeInverse, set x %= n before returning; may speed up decryption.
// - Remove Asserts and Signals from the release build.
// - Extended filename buffers from 64 bytes to 128 bytes.
// - An improved multiplication algorithm increases processing speed by a factor of 3 or 4.
// - When searching for primes, indicates which prime (#1 or #2) is being looked for.

// 0.1.1	?
// - Inlined some functions.
// - Wrote a specialized function to shift a BigNum right by 1.
// - Wrote an iterator-based subtraction function.

// **** END Release History ****

// Future Features:
// - Extract, decode, and display a digital signature embedded in a decrypted message.
// - Improved password protection (MD5?) of private keys.
// - Prompt before overwriting existing files.


#include "Common.h"


CHelixApp::CHelixApp( void )
	: m_version( 0, 1, 1 )	// This is version 0.1.1.
{
}


bool CHelixApp::FileExists( char * pFilename ) const
{
	FILE * f = fopen( pFilename, "rb" );

	if( f == 0 )
	{
		return( false );
	}

	fclose( f );
	return( true );
}


bool CHelixApp::TestKeys( const BigNum & d, const BigNum & e, const BigNum & n ) const
{

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
			return( false );
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
			printf( "Test failed.\n" );
			return( false );
		}
	}

	return( true );
}


void CHelixApp::HashPrivateKey( BigNum & d, BigNum & n ) const
{
	CHelixRSAKey::HashNumbers( d, n );
}


void CHelixApp::CommandGenerateKeys( void ) const
{
	int nBitLength = 0;
	BigNum d;
	BigNum e;
	BigNum n;
	char acFileName[128];
	char acFileName2[128];
	FILE * dstFile = 0;
	const int knMinBitLength = 128;
	const int knAdvertisedMaxBitLength = 4096;
	const int knActualMaxBitLength = 65536;

	// The number of segments in a BigNum must fit into an unsigned short,
	// so the maximum size of a BigNum is 1048576 bits;
	// but generating a key bigger than 65536 bits is probably impractical.

	do
	{
		printf( "Bit length (%d-%d) : ", knMinBitLength, knAdvertisedMaxBitLength );
		scanf( "%d", &nBitLength );
	}
	while( nBitLength < knMinBitLength  ||  nBitLength > knActualMaxBitLength );

	printf( "Generating keys...\n" );
	GenerateRSAKeys( nBitLength, d, e, n );
	printf( "Keys generated.\n" );

	if( !TestKeys( d, e, n ) )
	{
		return;
	}

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

	const CHelixRSAKey kPubKey( false, m_version, e, n );

	kPubKey.WriteToFile( dstFile );
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

	const CHelixRSAKey kPrvKey( true, m_version, d, n );

	kPrvKey.WriteToFile( dstFile );
	fclose( dstFile );
}


void CHelixApp::CommandEncryptFile( void ) const
{
	char acFileName[128];
	char acFileName2[128];
	FILE * keyFile = 0;
	FILE * srcFile = 0;
	FILE * dstFile = 0;

	printf( "Key filename (without extension) : " );
	scanf( "%s", acFileName );

	int nKeyType = 0;

	do
	{
		printf( "\nUse which kind of key?\n" );
		printf( "\n1) Public (for encrypting a message)\n" );
		printf( "2) Private (for encrypting a signature)\n" );
		printf( "\nEnter 1 or 2: " );
		scanf( "%d", &nKeyType );
	}
	while( nKeyType < 1  ||  nKeyType > 2 );

	strcat( acFileName, ( nKeyType == 1 ) ? ".pub" : ".prv" );
	keyFile = fopen( acFileName, "rb" );

	if( keyFile == 0 )
	{
		printf( "Failed to open file '%s' for read.\n", acFileName );
		return;
	}

	const CHelixRSAKey kKey( keyFile );

	fclose( keyFile );
	printf( "The key file '%s' has been read.\n", acFileName );

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

	EncryptFile( srcFile, dstFile, kKey.GetExponent(), kKey.GetN() );
	fclose( dstFile );
	fclose( srcFile );
	printf( "Finished creating encrypted file '%s'\n", acFileName2 );
}


void CHelixApp::CommandDecryptFile( void ) const
{
	char acFileName[128];
	char acFileName2[128];
	FILE * keyFile = 0;
	FILE * srcFile = 0;
	FILE * dstFile = 0;

	printf( "Key filename (without extension) : " );
	scanf( "%s", acFileName );

	int nKeyType = 0;

	do
	{
		printf( "\nUse which kind of key?\n" );
		printf( "\n1) Public (for decrypting a signature)\n" );
		printf( "2) Private (for decrypting a message)\n" );
		printf( "\nEnter 1 or 2: " );
		scanf( "%d", &nKeyType );
	}
	while( nKeyType < 1  ||  nKeyType > 2 );

	strcat( acFileName, ( nKeyType == 1 ) ? ".pub" : ".prv" );

	keyFile = fopen( acFileName, "rb" );

	if( keyFile == 0 )
	{
		printf( "Failed to open file '%s' for read.\n", acFileName );
		return;
	}

	const CHelixRSAKey kKey( keyFile );

	fclose( keyFile );
	printf( "The key file '%s' has been read.\n", acFileName );

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

	DecryptFile( srcFile, dstFile, kKey.GetExponent(), kKey.GetN() );
	fclose( dstFile );
	fclose( srcFile );
	printf( "Finished creating decrypted file '%s'\n", acFileName2 );
}


void CHelixApp::CommandImportVersion000Keys( void ) const
{
	char acFilename[128];
	char acPubFilename[128];
	char acPrvFilename[128];
	BigNum d;
	BigNum e;
	BigNum n;
	BigNum n2;
	FILE * pSrcPubKeyFile = 0;
	FILE * pSrcPrvKeyFile = 0;

	do
	{

		if( pSrcPubKeyFile != 0 )
		{
			fclose( pSrcPubKeyFile );
			pSrcPubKeyFile = 0;
		}

		if( pSrcPrvKeyFile != 0 )
		{
			fclose( pSrcPrvKeyFile );
			pSrcPrvKeyFile = 0;
		}

		printf( "\nVersion 0.0.0 key filename (without extension) : " );
		scanf( "%s", acFilename );
		strcpy( acPubFilename, acFilename );
		strcat( acPubFilename, ".pub" );
		strcpy( acPrvFilename, acFilename );
		strcat( acPrvFilename, ".prv" );

		// Ensure that both key files exist.
		pSrcPubKeyFile = fopen( acPubFilename, "rb" );
		pSrcPrvKeyFile = fopen( acPrvFilename, "rb" );
		
		if( pSrcPubKeyFile == 0 )
		{
			printf( "File '%s' does not exist.\n", acPubFilename );
		}

		if( pSrcPrvKeyFile == 0 )
		{
			printf( "File '%s' does not exist.\n", acPrvFilename );
		}
	}
	while( pSrcPubKeyFile == 0  ||  pSrcPrvKeyFile == 0 );

	// Read both key files.
	e.ReadFromFile( pSrcPubKeyFile );
	n.ReadFromFile( pSrcPubKeyFile );
	d.ReadFromFile( pSrcPrvKeyFile );
	n2.ReadFromFile( pSrcPrvKeyFile );
	fclose( pSrcPubKeyFile );
	fclose( pSrcPrvKeyFile );

	// Un-hash the private key.
	HashPrivateKey( d, n2 );

	// Ensure that n == n2;

	if( n != n2 )
	{
		printf( "Either the keys don't match, or the password is incorrect.\n" );
		return;
	}

	// Create the key objects.
	const CHelixRSAKey PubKey( false, m_version, e, n );
	const CHelixRSAKey PrvKey( true, m_version, d, n );

	// Write the key objects to the new files.
	FILE * pDstPubKeyFile = 0;
	FILE * pDstPrvKeyFile = 0;

	do
	{

		if( pDstPubKeyFile != 0 )
		{
			fclose( pDstPubKeyFile );
			pDstPubKeyFile = 0;
		}

		if( pDstPrvKeyFile != 0 )
		{
			fclose( pDstPrvKeyFile );
			pDstPrvKeyFile = 0;
		}

		printf( "\nNew key filename (without extension) : " );
		scanf( "%s", acFilename );
		strcpy( acPubFilename, acFilename );
		strcat( acPubFilename, ".pub" );
		strcpy( acPrvFilename, acFilename );
		strcat( acPrvFilename, ".prv" );

		// Ensure that both key files exist.
		pDstPubKeyFile = fopen( acPubFilename, "wb" );
		pDstPrvKeyFile = fopen( acPrvFilename, "wb" );
		
		if( pDstPubKeyFile == 0 )
		{
			printf( "Cannot write file '%s'.\n", acPubFilename );
		}

		if( pDstPrvKeyFile == 0 )
		{
			printf( "Cannot write file '%s'.\n", acPrvFilename );
		}
	}
	while( pDstPubKeyFile == 0  ||  pDstPrvKeyFile == 0 );

	PubKey.WriteToFile( pDstPubKeyFile );
	PrvKey.WriteToFile( pDstPrvKeyFile );
	fclose( pDstPubKeyFile );
	fclose( pDstPrvKeyFile );
	printf( "New key files written.\n" );
}


void CHelixApp::CommandChangeKeyPassword( void ) const
{
	char acFilename[128];
	FILE * pSrcPrvKeyFile = 0;
	FILE * pDstPrvKeyFile = 0;

	printf( "\nSource private key filename (without extension) : " );
	scanf( "%s", acFilename );
	strcat( acFilename, ".prv" );

	// Ensure that both key files exist.
	pSrcPrvKeyFile = fopen( acFilename, "rb" );
		
	if( pSrcPrvKeyFile == 0 )
	{
		printf( "File '%s' does not exist.\n", acFilename );
		return;
	}

	const CHelixRSAKey kPrivateKey( pSrcPrvKeyFile );

	fclose( pSrcPrvKeyFile );

	printf( "Destination private key filename (without extension) : " );
	scanf( "%s", acFilename );
	strcat( acFilename, ".prv" );

	// Ensure that both key files exist.
	pDstPrvKeyFile = fopen( acFilename, "wb" );
		
	if( pDstPrvKeyFile == 0 )
	{
		printf( "File '%s' cannot be written.\n", acFilename );
		return;
	}

	kPrivateKey.WriteToFile( pDstPrvKeyFile );
	fclose( pDstPrvKeyFile );
	printf( "File '%s' successfully written.\n", acFilename );
}


void CHelixApp::CommandUUEncodeFile( void ) const
{
	FILE * pSrcFile = 0;
	FILE * pDstFile = 0;
	char acSrcFileName[128];
	char acDstFileName[128];

	printf( "File to UUEncode: " );
	scanf( "%s", acSrcFileName );
	pSrcFile = fopen( acSrcFileName, "rb" );

	if( pSrcFile == 0 )
	{
		printf( "Failed to open file '%s' for read.\n", acSrcFileName );
		return;
	}

	printf( "Name of file to create: " );
	scanf( "%s", acDstFileName );
	pDstFile = fopen( acDstFileName, "w" );

	if( pDstFile == 0 )
	{
		printf( "Failed to open file '%s' for write.\n", acDstFileName );
		fclose( pSrcFile );
		return;
	}

	UUEncode( acSrcFileName, pSrcFile, pDstFile );
	fclose( pSrcFile );
	fclose( pDstFile );
}


void CHelixApp::CommandUUDecodeFile( void ) const
{
	FILE * pSrcFile = 0;
	char acFileName[128];

	printf( "File to UUDecode: " );
	scanf( "%s", acFileName );
	pSrcFile = fopen( acFileName, "r" );

	if( pSrcFile == 0 )
	{
		printf( "Failed to open file '%s' for read.\n", acFileName );
		return;
	}

	UUDecode( pSrcFile );
	fclose( pSrcFile );
}


void CHelixApp::Run( void ) const
{
	bool bQuit = false;

	printf( "\nHelix version %d.%d.%d ; December 17, 2002.\n",
		m_version.GetPrimaryVersion(),
		m_version.GetSecondaryVersion(),
		m_version.GetTertiaryVersion() );
	printf( "An implementation of the RSA cryptosystem.\n" );
	printf( "Copyright (c) 2002 by Tom Weatherhead.  All rights reserved.\n" );

	do
	{
		int nMenuSelection = 0;
		const int knLastMenuItem = 8;

		do
		{
			printf( "\nMain menu:\n\n" );
			printf( "1: Generate a pair of keys\n" );
			printf( "2: Encrypt a file\n" );
			printf( "3: Decrypt a file\n" );
			printf( "4: Import a version 0.0.0 key pair\n" );
			printf( "5: Change a private key's password\n" );
			printf( "6: UUEncode a file\n" );
			printf( "7: UUDecode a file\n" );
			printf( "%d: Quit\n", knLastMenuItem );
			printf( "\nEnter selection: " );
			scanf( "%d", &nMenuSelection );
		}
		while( nMenuSelection < 1  ||  nMenuSelection > knLastMenuItem );

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

				case 4:
					CommandImportVersion000Keys();
					break;

				case 5:
					CommandChangeKeyPassword();
					break;

				case 6:
					CommandUUEncodeFile();
					break;

				case 7:
					CommandUUDecodeFile();
					break;

				default:
					bQuit = true;
					break;
			}
		}
		catch( BigNumException & e )
		{
			printf( "Exception at file %s, line %d.\n", e.m_pFile, e.m_nLine );
		}
		catch( CHelixException & e )
		{
			printf( "Exception at file %s, line %d.\n", e.m_pFile, e.m_nLine );

			if( e.m_pMsg != 0 )
			{
				printf( "Message: %s\n", e.m_pMsg );
			}
		}
#if 0	// Turn this off to allow crashes.
		catch( ... )
		{
			printf( "Caught unknown exception in menu.\n" );
		}
#endif
	}
	while( !bQuit );

	printf( "\nHelix: Finished.\n" );
}


int main( void )	// int argc, char * argv
{
	const CHelixApp app;

	app.Run();
	return( 0 );
}


// **** End of File ****
