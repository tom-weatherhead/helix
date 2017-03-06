// BigNum.h - Part of Helix
// Copyright (c) 2002 by Tom Weatherhead.  All rights reserved.
// Started November 28, 2002


#ifndef _BIGNUM_H_
#define _BIGNUM_H_


class BigNumException
{
public:
	const char * m_pFile;
	int m_nLine;

	BigNumException( const char * f, int l )
		: m_pFile( f ),
			m_nLine( l )
	{
	}
};


#define ThrowException() throw BigNumException( __FILE__, __LINE__ )


static inline void IntelDebugInterrupt( void )
{
	__asm int 3
}


#define Signal() printf( "Signal at file %s, line %d.\n", __FILE__, __LINE__ ); IntelDebugInterrupt()
#define Assert( b ) if( !(b) ) { Signal(); }


class BigNum
{
private:

#ifdef _BIGNUM_USE_DEQUE_
	typedef deque<unsigned short> DigitContainerType;
#else
	typedef vector<unsigned short> DigitContainerType;
#endif

	DigitContainerType m_v;

	void DiscardLeadingZeros( void );
	void SetToZero( void );
	void AddShifted( const BigNum & Src, int nLeftShift );
	void SetBit( int nBit );

public:
	BigNum( void );
	BigNum( int nNumUShorts, const unsigned short * pusSrc );
	BigNum( unsigned long ulSrc );

	static void DivideAndModulo(
		const BigNum & dividendParam, const BigNum & divisorParam,
		BigNum * pQuotient, BigNum * pRemainder );

	BigNum & operator +=( const BigNum & Src );
	const BigNum operator +( const BigNum & Src ) const;
	BigNum & operator -=( const BigNum & Src );
	const BigNum operator -( const BigNum & Src ) const;
	BigNum & operator *=( const BigNum & Src );
	const BigNum operator *( const BigNum & Src ) const;
	BigNum & operator /=( const BigNum & Src );
	const BigNum operator /( const BigNum & Src ) const;
	BigNum & operator %=( const BigNum & Src );
	const BigNum operator %( const BigNum & Src ) const;
#if 0
	const BigNum operator >>( int nShift ) const;
	const BigNum operator <<( int nShift ) const;
#endif
	BigNum & operator >>=( int nShift );
	BigNum & operator <<=( int nShift );
	bool operator ==( const BigNum & Src ) const;
	bool operator !=( const BigNum & Src ) const;
	bool operator <( const BigNum & Src ) const;
	bool operator >( const BigNum & Src ) const;
	bool operator <=( const BigNum & Src ) const;
	bool operator >=( const BigNum & Src ) const;

	bool IsZero( void ) const;
	int NumSegments( void ) const;
	int NumSignificantBits( void ) const;
	bool TestBit( int nBit ) const;
	void SetToRandom( int nBitLength );
	void SetToRandomPrime( int nPrimeBitLength );
	void PrintDecimal( void ) const;
	void PrintHex( void ) const;
	void HashWithString( const char * pcString );

	void ReadFromFile( FILE * srcFile );
	void WriteToFile( FILE * dstFile ) const;

	unsigned short ReadFromFile( FILE * srcFile, unsigned char * pucSrcBuf, int nReadSize );
	void WriteToFile(
		FILE * dstFile, unsigned char * pucBuf,
		unsigned short usBytesEncrypted, size_t unBytesToWrite ) const;
}; // class BigNum


// Returns ( a ^ b ) mod c.

const BigNum ExponentMod( const BigNum & a, const BigNum & b, const BigNum & c );


// From "Introduction to Algorithms", p. 812.
// Given a and b, finds d, x, and y such that:
// 1) d = gcd( a, b ), and
// 2) d = a * x + b * y.

void ExtendedEuclid(
	const BigNum & a, const BigNum & b,
	BigNum & d, BigNum & x, BigNum & y );


const BigNum GCD( const BigNum & a, const BigNum & b );


// If possible, find x such that a * x == 1 (mod n).

bool MultiplicativeInverse( const BigNum & a, const BigNum & n, BigNum & x );


void GenerateRSAKeys( int nBitLength, BigNum & d, BigNum & e, BigNum & n );


void EncryptFile(
	FILE * srcFile, FILE * dstFile,
	const BigNum & exponent, const BigNum & n );


void DecryptFile(
	FILE * srcFile, FILE * dstFile,
	const BigNum & exponent, const BigNum & n );


#endif // _BIGNUM_H_


// **** End of File ****
