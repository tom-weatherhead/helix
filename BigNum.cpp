// BigNum.cpp - Part of Helix
// Copyright (c) 2002 by Tom Weatherhead.  All rights reserved.
// Started November 29, 2002

#include "Common.h"


#ifdef BIG_ENDIAN
void ByteSwapUnsignedShort( unsigned short & us )
{
	const unsigned short kusByte0 = us & 0x00ff;
	const unsigned short kusByte1 = us & 0xff00;

	us = ( kusByte0 << 8 ) | ( kusByte1 >> 8 );
}


void ByteSwapUnsignedInt( unsigned int & un )
{
	const unsigned int kunByte0 = un & 0x000000ff;
	const unsigned int kunByte1 = un & 0x0000ff00;
	const unsigned int kunByte2 = un & 0x00ff0000;
	const unsigned int kunByte3 = un & 0xff000000;

	un = ( kunByte0 << 24 ) | ( kunByte1 << 8 ) | ( kunByte2 >> 8 ) | ( kunByte3 >> 24 );
}
#endif


// The compiler will generate the following functions automatically:
// 2) Copy constructor
// 3) Destructor
// 4) Assignment operator


BigNum::BigNum( void )
{
}


BigNum::BigNum( int nNumUShorts, const unsigned short * pusSrc )
{
	int i;

	for( i = 0; i < nNumUShorts; ++i )
	{
		m_v.push_back( pusSrc[i] );
	}

	DiscardLeadingZeros();
}


BigNum::BigNum( unsigned long ulSrc )
{
	const unsigned short kusLow = (unsigned short)( ulSrc & 0xFFFF );
	const unsigned short kusHigh = (unsigned short)( ulSrc >> 16 );

	m_v.push_back( kusLow );
	m_v.push_back( kusHigh );
	DiscardLeadingZeros();
}


void BigNum::HashWithString( const char * pcString )
{
	const int knStringLength = strlen( pcString );

	if( knStringLength == 0 )
	{
		return;
	}

	int i = 0;
	DigitContainerType::iterator it = m_v.begin();

	for( ; it != m_v.end(); ++it )
	{
		const unsigned short kusLow = (unsigned short)pcString[i];

		i = ( i + 1 ) % knStringLength;

		const unsigned short kusHigh = (unsigned short)pcString[i];

		i = ( i + 1 ) % knStringLength;

		const unsigned short kusMask = ( kusHigh << 8 ) | kusLow;

		*it ^= kusMask;
	}
}


void BigNum::PrintDecimal( void ) const
{
	DigitContainerType::const_reverse_iterator i = m_v.rbegin();

	for( ; i != m_v.rend(); ++i )
	{
		printf( "%u ", *i );
	}

	printf( "\n" );
}


void BigNum::PrintHex( void ) const
{
	DigitContainerType::const_reverse_iterator i = m_v.rbegin();

	for( ; i != m_v.rend(); ++i )
	{
		printf( "%04X ", *i );
	}

	printf( "\n" );
}


void BigNum::DiscardLeadingZeros( void )
{

	while( m_v.size() > 0  &&  m_v.back() == 0 )
	{
		m_v.pop_back();
	}
}


int BigNum::NumSignificantBits( void ) const
{

	if( m_v.empty() )
	{
		return( 0 );
	}

	int nRtn = 16 * m_v.size();
	const unsigned short kusBack = m_v.back();
	unsigned short usMask = 32768;

	Assert( kusBack != 0 );

	while( usMask > 0  &&  ( kusBack & usMask ) == 0 )
	{
		--nRtn;
		usMask >>= 1;
	}

	return( nRtn );
}


void BigNum::SetBit( int nBit )
{

	if( nBit < 0 )
	{
		Signal();
		return;
	}

	const int knSegment = nBit / 16;

	while( m_v.size() <= knSegment )
	{
		m_v.push_back( 0 );
	}

	m_v[knSegment] |= (unsigned short)( 1 << ( nBit % 16 ) );
}


bool BigNum::TestBit( int nBit ) const
{
	const int knSegment = nBit / 16;

	if( nBit < 0  ||  knSegment >= m_v.size() )
	{
		return( false );
	}

	return( ( m_v[knSegment] & ( 1 << ( nBit % 16 ) ) ) != 0 );
}


// Shift quantity is in UShorts, not bits.

void BigNum::AddShifted( const BigNum & Src, int nLeftShift )
{

	if( nLeftShift < 0 )
	{
		ThrowException();
	}

	if( this == &Src )
	{
		const BigNum tempSrc( Src );
		AddShifted( tempSrc, nLeftShift );
		return;
	}

	while( m_v.size() < nLeftShift )
	{
		m_v.push_back( 0 );
	}

	int i;
	unsigned long ulCarry = 0;
	const int knSrcSize = Src.m_v.size();

	for( i = nLeftShift; i - nLeftShift < knSrcSize  ||  ulCarry > 0; ++i )
	{
		const bool kbPastEndOfThis = i >= m_v.size();

		if( !kbPastEndOfThis )
		{
			ulCarry += (unsigned long)m_v[i];
		}

		if( i - nLeftShift < knSrcSize )
		{
			ulCarry += (unsigned long)Src.m_v[i - nLeftShift];
		}

		const unsigned short kusSum = (unsigned short)( ulCarry & 65535 );

		ulCarry >>= 16;

		if( kbPastEndOfThis )
		{
			m_v.push_back( kusSum );
		}
		else
		{
			m_v[i] = kusSum;
		}
	}
} // BigNum::AddShifted()


BigNum & BigNum::operator +=( const BigNum & Src )
{
	AddShifted( Src, 0 );
	return( *this );
} // BigNum::operator +=()


const BigNum BigNum::operator +( const BigNum & Src ) const
{
	BigNum sum( *this );

	sum += Src;
	return( sum );
}


BigNum & BigNum::operator -=( const BigNum & Src )
{

	if( this == &Src )	// ie. x -= x;
	{
		SetToZero();
		return( *this );
	}

#ifndef NDEBUG
	if( *this < Src )
	{
		Signal();
		ThrowException();
	}
#endif

	// Use iterators.
	unsigned short usBorrow = 0;
	DigitContainerType::iterator iDst;
	DigitContainerType::const_iterator iSrc = Src.m_v.begin();

	for( iDst = m_v.begin(); iSrc != Src.m_v.end()  ||  usBorrow > 0; ++iDst )
	{
		Assert( iDst != m_v.end() );

		unsigned short usOperand2 = usBorrow;

		if( iSrc != Src.m_v.end() )
		{
			usOperand2 += *iSrc++;
		}

		usBorrow = ( *iDst < usOperand2 ) ? 1 : 0;
		*iDst -= usOperand2;
	}

	DiscardLeadingZeros();
	return( *this );
}


const BigNum BigNum::operator -( const BigNum & Src ) const
{
	BigNum diff( *this );

	diff -= Src;
	return( diff );
}


const BigNum BigNum::operator *( const BigNum & Src ) const
{
	BigNum product;
	int i;
	int j;

	for( i = 0; i < m_v.size(); ++i )
	{
		const unsigned long kulThisUShort = (unsigned long)m_v[i];

		for( j = 0; j < Src.m_v.size(); ++j )
		{
			const unsigned long kulPartialProduct = kulThisUShort * (unsigned long)Src.m_v[j];
			const BigNum pp( kulPartialProduct );

			product.AddShifted( pp, i + j );
		}
	}

	return( product );
}


BigNum & BigNum::operator *=( const BigNum & Src )
{
	return( *this = *this * Src );
}


// Return *this * us.

const BigNum BigNum::MultiplyWithUShort( unsigned short us ) const
{
	unsigned long ulCarry = 0;
	const unsigned long kulFactor = (unsigned long)us;
	BigNum prod;
	DigitContainerType::const_iterator i;

	for( i = m_v.begin(); i != m_v.end(); ++i )
	{
		ulCarry += kulFactor * (unsigned long)*i;
		prod.m_v.push_back( (unsigned short)( ulCarry & 0xFFFF ) );
		ulCarry >>= 16;
	}

	prod.m_v.push_back( (unsigned short)( ulCarry & 0xFFFF ) );
	prod.DiscardLeadingZeros();
	return( prod );
}


// Return ( *this * b ) % n.

const BigNum BigNum::MultiplyMod( const BigNum & b, const BigNum & n ) const
{
	BigNum c;
	BigNum d = b;	// A mutable copy of b.
	DigitContainerType::const_iterator i;

	for( i = m_v.begin(); i != m_v.end(); ++i )
	{
		c += d.MultiplyWithUShort( *i );
#if 1
		d <<= 16;
		d %= n;
#else
		d = ( d << 16 ) % n;
#endif
	}

	return( c % n );
}


// Static function.

void BigNum::DivideAndModulo(
	const BigNum & dividendParam, const BigNum & divisorParam,
	BigNum * pQuotient, BigNum * pRemainder )
{
	BigNum dividend( dividendParam );
	BigNum divisor( divisorParam );
	BigNum quotient;
	const int knDivisorLeftShift = dividend.NumSignificantBits() - divisor.NumSignificantBits();
	int i;

	if( knDivisorLeftShift > 0 )
	{
		divisor <<= knDivisorLeftShift;
	}

	for( i = knDivisorLeftShift; i >= 0; --i )
	{
		
		if( dividend >= divisor )
		{
			dividend -= divisor;
			Assert( dividend < divisor );

			if( pQuotient != 0 )
			{
				quotient.SetBit( i );
			}
		}

#if 1
		divisor.ShiftRightBy1();
#else
		divisor >>= 1;
#endif
	}

	if( pQuotient != 0 )
	{
		*pQuotient = quotient;
	}

	if( pRemainder != 0 )
	{
		*pRemainder = dividend;
	}
}


const BigNum BigNum::operator /( const BigNum & Src ) const
{
	BigNum quotient;

	DivideAndModulo( *this, Src, &quotient, 0 );
	return( quotient );
}


BigNum & BigNum::operator /=( const BigNum & Src )
{
	return( *this = *this / Src );
}


const BigNum BigNum::operator %( const BigNum & Src ) const
{
	BigNum remainder;

	DivideAndModulo( *this, Src, 0, &remainder );
	return( remainder );
}


BigNum & BigNum::operator %=( const BigNum & Src )
{
	return( *this = *this % Src );
}


BigNum & BigNum::operator >>=( int nShift )
{

	if( nShift == 0 )
	{
		return( *this );
	}
	else if( nShift < 0 )
	{
		return( *this <<= -nShift );
	}

	const int knMajorShift = nShift / 16;
	const int knMinorShift = nShift % 16;

	if( knMajorShift >= m_v.size() )
	{
		SetToZero();
		return( *this );
	}

	if( knMinorShift == 0 )
	{
		// If this was a deque, we could just pop_front() knMajorShift times.
		int i;

		for( i = 0; i + knMajorShift < m_v.size(); ++i )
		{
			m_v[i] = m_v[i + knMajorShift];
		}

		// These leading zeros will be discarded below.

		for( ; i < m_v.size(); ++i )
		{
			m_v[i] = 0;
		}
	}
	else
	{
		int i;

		for( i = 0; i + knMajorShift + 1 < m_v.size(); ++i )
		{
			m_v[i] = (unsigned short)( ( m_v[i + knMajorShift] >> knMinorShift ) | ( m_v[i + knMajorShift + 1] << ( 16 - knMinorShift ) ) );
		}

		m_v[i] = (unsigned short)( m_v[i + knMajorShift] >> knMinorShift );

		// These leading zeros will be discarded below.

		for( ++i; i < m_v.size(); ++i )
		{
			m_v[i] = 0;
		}
	}

	DiscardLeadingZeros();
	return( *this );
}


#if 1
BigNum & BigNum::ShiftRightBy1( void )
{
	DigitContainerType::iterator iNext = m_v.begin();
	DigitContainerType::iterator iCurrent = iNext;

	if( iNext == m_v.end() )
	{
		// *this == zero.
		return( *this );
	}

	while( ++iNext != m_v.end() )
	{
		*iCurrent = ( *iCurrent >> 1 ) | ( *iNext << 15 );
		iCurrent = iNext;
	}

	*iCurrent >>= 1;
	DiscardLeadingZeros();
	return( *this );
}
#endif


// TODO: Rewrite <<= in terms of <<.

BigNum & BigNum::operator <<=( int nShift )
{

	if( nShift == 0 )
	{
		return( *this );
	}
	else if( nShift < 0 )
	{
		return( *this >>= -nShift );
	}

	const int knMajorShift = nShift / 16;
	const int knMinorShift = nShift % 16;
	int i;
	BigNum result;

	for( i = 0; i < knMajorShift; ++i )
	{
		result.m_v.push_back( 0 );
	}

	if( knMinorShift == 0 )
	{
		// If this was a deque, we could just push_front( 0 ) knMajorShift times.

		for( i = 0; i < m_v.size(); ++i )
		{
			result.m_v.push_back( m_v[i] );
		}
	}
	else
	{
		unsigned short usPrev = 0;

		for( i = 0; i < m_v.size(); ++i )
		{
			const unsigned short kusNext = m_v[i];
			const unsigned short kusNew = (unsigned short)( ( usPrev >> ( 16 - knMinorShift ) ) | ( kusNext << knMinorShift ) );

			result.m_v.push_back( kusNew );
			usPrev = kusNext;
		}

		result.m_v.push_back( (unsigned short)( usPrev >> ( 16 - knMinorShift ) ) );
	}

	*this = result;
	DiscardLeadingZeros();
	return( *this );
}


bool BigNum::operator ==( const BigNum & Src ) const
{

	if( m_v.size() != Src.m_v.size() )
	{
		return( false );
	}

	DigitContainerType::const_reverse_iterator kriThis = m_v.rbegin();
	DigitContainerType::const_reverse_iterator kriSrc = Src.m_v.rbegin();

	while( kriThis != m_v.rend()  &&  kriSrc != Src.m_v.rend() )
	{

		if( *kriThis != *kriSrc )
		{
			return( false );
		}

		++kriThis;
		++kriSrc;
	}

	return( true );
}


bool BigNum::operator <( const BigNum & Src ) const
{
	
	if( m_v.size() < Src.m_v.size() )
	{
		return( true );
	}
	else if( m_v.size() > Src.m_v.size() )
	{
		return( false );
	}

	DigitContainerType::const_reverse_iterator kriThis = m_v.rbegin();
	DigitContainerType::const_reverse_iterator kriSrc = Src.m_v.rbegin();

	while( kriThis != m_v.rend()  &&  kriSrc != Src.m_v.rend() )
	{

		if( *kriThis < *kriSrc )
		{
			return( true );
		}
		else if( *kriThis > *kriSrc )
		{
			return( false );
		}

		++kriThis;
		++kriSrc;
	}

	return( false );
}


void BigNum::ReadFromFile( FILE * srcFile )
{
	unsigned short usReadSize = 0;
	size_t unNumItemsRead = fread( &usReadSize, sizeof( usReadSize ), 1, srcFile );

	if( unNumItemsRead != 1 )
	{
		Signal();
		ThrowException(  );
	}

#ifdef BIG_ENDIAN
	ByteSwapUnsignedShort( usReadSize );
#endif

	m_v.clear();

	while( usReadSize-- > 0 )
	{
		unsigned short usData = 0;

		unNumItemsRead = fread( &usData, sizeof( usData ), 1, srcFile );

		if( unNumItemsRead != 1 )
		{
			Signal();
			ThrowException(  );
		}

#ifdef BIG_ENDIAN
		ByteSwapUnsignedShort( usData );
#endif

		m_v.push_back( usData );
	}

	DiscardLeadingZeros();
}


void BigNum::WriteToFile( FILE * dstFile ) const
{
	unsigned short usWriteSize = (unsigned short)m_v.size();

#ifdef BIG_ENDIAN
	ByteSwapUnsignedShort( usWriteSize );
#endif

	size_t unNumItemsWritten = fwrite( &usWriteSize, sizeof( usWriteSize ), 1, dstFile );

	if( unNumItemsWritten != 1 )
	{
		Signal();
		ThrowException(  );
	}

	int i;

	for( i = 0; i < m_v.size(); ++i )
	{
		unsigned short usData = m_v[i];

#ifdef BIG_ENDIAN
		ByteSwapUnsignedShort( usData );
#endif

		unNumItemsWritten = fwrite( &usData, sizeof( usData ), 1, dstFile );

		if( unNumItemsWritten != 1 )
		{
			Signal();
			ThrowException(  );
		}
	}
}


unsigned short BigNum::ReadFromFile( FILE * srcFile, unsigned char * pucSrcBuf, int nReadSize )
{
	bool bDataIsEncrypted = false;
	unsigned short usBytesEncrypted = 0;

	if( nReadSize <= 0 )
	{
		// We're reading encrypted data.
		// The first UShort is the number of bytes of raw, unencrypted data represented in this BigNum.
		// The second UShort is the number of segments (UShorts) in this BigNum.
		unsigned short usReadSize = 0;
		unsigned char acHeaderBuf[4];
		const size_t unNumItemsRead = fread( acHeaderBuf, sizeof( unsigned char ), 4, srcFile );

		if( unNumItemsRead == 0  &&  feof( srcFile ) )
		{
			// End of source file.
			return( 0 );
		}
		else if( unNumItemsRead != 4 )
		{
			Signal();
			ThrowException(  );
		}

		usBytesEncrypted = *(unsigned short *)acHeaderBuf;
		usReadSize = *(unsigned short *)( acHeaderBuf + 2 );

#ifdef BIG_ENDIAN
		ByteSwapUnsignedShort( usBytesEncrypted );
		ByteSwapUnsignedShort( usReadSize );
#endif

		nReadSize = (int)usReadSize;
		bDataIsEncrypted = true;
	}

	size_t unNumBytesRead = fread( pucSrcBuf, sizeof( unsigned char ), sizeof( unsigned short ) * (size_t)nReadSize, srcFile );

	if( !bDataIsEncrypted )
	{

		if( unNumBytesRead == 0  &&  feof( srcFile ) )
		{
			// End of source file.
			return( 0 );
		}

		usBytesEncrypted = (unsigned short)unNumBytesRead;
	}

	m_v.clear();

	// This loop may write a partial UShort at the end;
	// clear the SrcBuf beforehand.

	for( ; unNumBytesRead > 0; unNumBytesRead -= sizeof( unsigned short ) )
	{
		unsigned short usData = *(unsigned short *)pucSrcBuf;

#ifdef BIG_ENDIAN
		if( bDataIsEncrypted )
		{
			ByteSwapUnsignedShort( usData );
		}
#endif

		m_v.push_back( usData );
		pucSrcBuf += 2;
	}

	DiscardLeadingZeros();
	return( usBytesEncrypted );
}


void BigNum::WriteToFile(
	FILE * dstFile, unsigned char * pucBuf,
	unsigned short usBytesEncrypted, size_t unBytesToWrite ) const
{
	int nBufSizeInUShorts = 0;
	unsigned short * pusBuf = (unsigned short *)pucBuf;

	if( usBytesEncrypted > 0 )
	{
		// We're writing encrypted data.
		// The first UShort is the number of bytes of raw, unencrypted data represented in this BigNum.
		// The second UShort is the number of segments (UShorts) in this BigNum.
		pusBuf[0] = usBytesEncrypted;
		pusBuf[1] = (unsigned short)NumSegments();
		nBufSizeInUShorts = 2;
	}

	int i;

	for( i = 0; i < m_v.size(); ++i )
	{
		pusBuf[nBufSizeInUShorts + i] = m_v[i];

#if 0
		// Do byte-swapping here on the Mac.
		ByteSwapUShort( ... );
#endif
	}

	nBufSizeInUShorts += m_v.size();
	
	if( unBytesToWrite == 0 )
	{
		unBytesToWrite = nBufSizeInUShorts * sizeof( unsigned short );

#ifdef BIG_ENDIAN
		for( i = 0; i < nBufSizeInUShorts; ++i )
		{
			ByteSwapUnsignedShort( pusBuf[i] );
		}
#endif
	}

	const size_t unNumBytesWritten = fwrite( pusBuf, 1, unBytesToWrite, dstFile );

	if( unNumBytesWritten != unBytesToWrite )
	{
		Signal();
		ThrowException(  );
	}
}


void BigNum::SetToRandom( int nBitLength )
{
	int nNumUShorts = ( nBitLength + 15 ) / 16;

	m_v.clear();

	while( nNumUShorts-- > 0 )
	{
		m_v.push_back( (unsigned short)( rand() & 65535 ) );
	}

	const unsigned short kusMSBMask = (unsigned short)( 1 << ( ( nBitLength + 15 ) & 15 ) );

	m_v.back() &= (unsigned short)( kusMSBMask - 1 );
	m_v.back() |= kusMSBMask;
}


// From "Introduction to Algorithms", p. 840.

bool MillerRabinWitness( const BigNum & a, const BigNum & n )
{
	int i;
	BigNum d( 1 );
	const BigNum one( 1 );
	const BigNum nMinus1 = n - one;

	for( i = nMinus1.NumSignificantBits() - 1; i >= 0; --i )
	{
		const BigNum x = d;

#if 1
		d = d.MultiplyMod( d, n );
#else
		d = ( d * d ) % n;
#endif

		if( d == one  &&  x != one  &&  x != nMinus1 )
		{
			// x is a non-trivial square root of 1 (mod n).
			return( true );
		}

		if( nMinus1.TestBit( i ) )
		{
#if 1
			d = d.MultiplyMod( a, n );
#else
			d = ( d * a ) % n;
#endif
		}
	}

	return( d != one );
}


// From "Introduction to Algorithms", p. 841.

bool MillerRabinIsComposite( const BigNum & n, int s )
{
	int i;

	printf( "Witness # " );

	for( i = 1; i <= s; ++i )
	{
		BigNum a;

		do
		{
			a.SetToRandom( n.NumSignificantBits() - 1 );
		}
		while( a.IsZero() );

		printf( "%d ", i );

		if( MillerRabinWitness( a, n ) )
		{
			printf( "Composite.\n" );
			return( true );
		}
	}

	printf( "Probably prime.\n" );
	return( false );	// n is probably prime.
}


void BigNum::SetToRandomPrime( int nPrimeBitLength, int nWhichPrime )
{
	// The probability of returning a composite number is 2^-20.
	static const int knMillerRabinIterations = 20;
	int i = 0;

	do
	{
		SetToRandom( nPrimeBitLength );

		// Make the number odd.
		m_v.front() |= 1;

		if( nWhichPrime > 0 )
		{
			printf( "Prime #%d, ", nWhichPrime );
		}

		printf( "Attempt %d: Testing for primality: ", ++i );
		PrintHex();
	}
	while( MillerRabinIsComposite( *this, knMillerRabinIterations ) );

}


// Compute ( a ^ b ) mod c.

const BigNum ExponentMod( const BigNum & a, const BigNum & b, const BigNum & c )
{
	int i;
	BigNum result( 1 );

	for( i = b.NumSignificantBits() - 1; i >= 0; --i )
	{
#if 1
		result = result.MultiplyMod( result, c );
#else
		result = ( result * result ) % c;
#endif

		if( b.TestBit( i ) )
		{
#if 1
			result = result.MultiplyMod( a, c );
#else
			result = ( result * a ) % c;
#endif
		}
	}

	return( result );
}


// From "Introduction to Algorithms", p. 812.
// Given a and b, finds d, x, and y such that:
// 1) d = gcd( a, b ), and
// 2) d = a * x + b * y.

// The algorithm has been modified so that d == a * x - b * y,

void ExtendedEuclid(
	const BigNum & a, const BigNum & b,
	BigNum & d, BigNum & x, BigNum & y )
{
	
	if( b.IsZero() )
	{
		d = a;
		x = BigNum( 1 );
		y = BigNum( 0 );
	}
	else
	{
		BigNum ADivB;
		BigNum AModB;
		BigNum x2;
		BigNum y2;

		BigNum::DivideAndModulo( a, b, &ADivB, &AModB );
		ExtendedEuclid( b, AModB, d, x2, y2 );
#if 0
		printf( "d, a, b, ADivB, AModB, x2, y2:\n" );
		d.PrintHex();
		a.PrintHex();
		b.PrintHex();
		ADivB.PrintHex();
		AModB.PrintHex();
		x2.PrintHex();
		y2.PrintHex();
#endif
#if 1
		// Modify the algorithm so that d == a * x - b * y,
		// with x >= 0 and y >= 0.
		Assert( d == b * x2 - AModB * y2 );
		const BigNum temp1 = x2 + ADivB * y2;
		BigNum n = y2 / b + 1;
		const BigNum n2 = temp1 / a + 1;

		if( n2 > n )
		{
			n = n2;
		}

		x = n * b - y2;
		y = n * a - temp1;
		Assert( d == a * x - b * y );
#else
		x = y2;
		y = x2 - ADivB * y2;
		Assert( d == a * x + b * y );
#endif
	}
}


const BigNum GCD( const BigNum & a, const BigNum & b )
{
	BigNum d;
	BigNum x;
	BigNum y;

	ExtendedEuclid( a, b, d, x, y );
	return( d );
}


// If possible, find x such that a * x == 1 (mod n).

bool MultiplicativeInverse( const BigNum & a, const BigNum & n, BigNum & x )
{
	BigNum d;
	BigNum k;

	ExtendedEuclid( a, n, d, x, k );
	
	if( d != BigNum( 1 ) )
	{
		return( false );
	}

	x %= n;
	return( true );
}


// From "Introduction to Algorithms", p. 834.
// The public key is ( e, n ).
// The private key is ( d, n ).

void GenerateRSAKeys( int nBitLength, BigNum & d, BigNum & e, BigNum & n )
{
	BigNum p;
	BigNum q;
	const int knPrimeBitLength = nBitLength / 2 + 1;

	// 1) Choose p and q.
	srand( time( 0 ) );
	p.SetToRandomPrime( knPrimeBitLength, 1 );
	q.SetToRandomPrime( knPrimeBitLength, 2 );

	// 2) Compute n.
	n = p * q;
	printf( "Found n.\n" );

	// 3 and 4) Find e and d.
	const BigNum phiN = ( p - BigNum( 1 ) ) * ( q - BigNum( 1 ) );

	// Seed the random number generator with the current time.
	srand( time( 0 ) );

	do
	{
		// Choose a small odd positive integer, greater than 1.
		e = BigNum( (unsigned long)( ( rand() & 65535 ) | 1 ) );

		printf( "Candidate for e: " );
		e.PrintHex();
	}
	while( e == BigNum( 1 )  ||  !MultiplicativeInverse( e, phiN, d ) );

}


void CHelixApp::EncryptFile(
	FILE * srcFile, FILE * dstFile,
	const BigNum & exponent, const BigNum & n ) const
{
	const int knReadSize = n.NumSegments() - 1;
	//unsigned char * pucSrcBuf,	// Big enough to hold n - 1 UShort.
	//unsigned char * pucDstBuf		// Big enough to hold n + 2 extra UShorts.
	unsigned char * pucSrcBuf = new unsigned char[knReadSize * sizeof( unsigned short )];
	const size_t kunDstBufSize = ( knReadSize + 3 ) * sizeof( unsigned short );
	unsigned char * pucDstBuf = new unsigned char[kunDstBufSize];
	int nBytesRead = 0;
	const int nStartTime = time( 0 );

	printf( "\nBytes encrypted: " );
	m_version.WriteToFile( dstFile );

	while( !feof( srcFile ) )
	{
		BigNum x;
		const unsigned short kusBytesEncrypted = x.ReadFromFile( srcFile, pucSrcBuf, knReadSize );

#if 1
		if( kusBytesEncrypted == 0  &&  feof( srcFile ) )
		{
			break;
		}
#endif

		Assert( x < n );

		const BigNum y = ExponentMod( x, exponent, n );

		memset( pucDstBuf, 0, kunDstBufSize );
		y.WriteToFile( dstFile, pucDstBuf, kusBytesEncrypted, 0 );
		nBytesRead += (int)kusBytesEncrypted;
		printf( "%d ", nBytesRead );
	}

	const int nSeconds = time( 0 ) - nStartTime;

	printf( "\nEncryption finished in %d minute(s) %d second(s)\n",
		nSeconds / 60, nSeconds % 60 );
	delete [] pucSrcBuf;
	delete [] pucDstBuf;
}


void CHelixApp::DecryptFile(
	FILE * srcFile, FILE * dstFile,
	const BigNum & exponent, const BigNum & n ) const
{
	const int knNumSegmentsInN = n.NumSegments();
	unsigned char * pucSrcBuf = new unsigned char[( knNumSegmentsInN + 2 ) * sizeof( unsigned short )];
	const size_t kunDstBufSize = ( knNumSegmentsInN - 1 ) * sizeof( unsigned short );
	unsigned char * pucDstBuf = new unsigned char[kunDstBufSize];
	int nBytesWritten = 0;
	const CVersion kVersion( srcFile );
	const int nStartTime = time( 0 );

	printf( "Encrypted file created with Helix version " );
	kVersion.Print();
	printf( "\nBytes decrypted: " );

	while( !feof( srcFile ) )
	{
		BigNum x;
		const unsigned short kusBytesToWrite = x.ReadFromFile( srcFile, pucSrcBuf, 0 );

#if 1
		if( kusBytesToWrite == 0  &&  feof( srcFile ) )
		{
			break;
		}
#endif

		Assert( x < n );

		const BigNum y = ExponentMod( x, exponent, n );

		memset( pucDstBuf, 0, kunDstBufSize );
		y.WriteToFile( dstFile, pucDstBuf, 0, (size_t)kusBytesToWrite );
		nBytesWritten += (int)kusBytesToWrite;
		printf( "%d ", nBytesWritten );
	}

	const int nSeconds = time( 0 ) - nStartTime;

	printf( "\nDecryption finished in %d minute(s) %d second(s)\n",
		nSeconds / 60, nSeconds % 60 );
	delete [] pucSrcBuf;
	delete [] pucDstBuf;
}


// **** End of File ****
