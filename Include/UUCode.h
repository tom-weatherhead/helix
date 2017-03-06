// UUCode.h - Part of Helix
// Copyright (c) 2002 by Tom Weatherhead.  All rights reserved.
// Started December 16, 2002


#ifndef __UUCODE_H_
#define __UUCODE_H_

void UUEncode( char * pSrcFilename, FILE * pSrcFile, FILE * pDstFile );

bool UUDecode( FILE * pSrcFile );

#endif


// **** End of File ****
