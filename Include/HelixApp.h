// HelixApp.h - Part of Helix
// Copyright (c) 2002 by Tom Weatherhead.  All rights reserved.
// Started December 17, 2002


#ifndef _HELIXAPP_H_
#define _HELIXAPP_H_


class CHelixApp
{
private:
	const CVersion m_version;

	bool FileExists( char * pFilename ) const;
	bool TestKeys( const BigNum & d, const BigNum & e, const BigNum & n ) const;
	void HashPrivateKey( BigNum & d, BigNum & n ) const;
	void EncryptFile(
		FILE * srcFile, FILE * dstFile,
		const BigNum & exponent, const BigNum & n ) const;
	void DecryptFile(
		FILE * srcFile, FILE * dstFile,
		const BigNum & exponent, const BigNum & n ) const;

	// Command functions.
	void CommandGenerateKeys( void ) const;
	void CommandEncryptFile( void ) const;
	void CommandDecryptFile( void ) const;
	void CommandImportVersion000Keys( void ) const;
	void CommandChangeKeyPassword( void ) const;
	void CommandUUEncodeFile( void ) const;
	void CommandUUDecodeFile( void ) const;

public:
	CHelixApp( void );

	void Run( void ) const;
}; // class CHelixApp


#endif


// **** End of File ****
