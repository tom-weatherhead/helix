// Common.h - Part of Helix
// Copyright (c) 2002 by Tom Weatherhead.  All rights reserved.
// Started November 28, 2002


#ifdef _MSC_VER			// TAW 2002/03/11 : ie. if MSVC
#ifndef MACINTOSH		// TAW 2002/02/23 : ie. if MSVC

// Suppress warning C4786: Identifier truncated to 255 chars in the debug info.
#pragma warning(disable : 4786)

#endif
#endif


#define _HELIX_USE_STDIO_		1


// Standard includes.

#ifdef _HELIX_USE_STDIO_
#include <cstdio>
#else
#include <iostream>
#endif

//#include <string>
#include <vector>
#include <ctime>

// Using directives.

#ifndef _HELIX_USE_STDIO_
using std::cin;
using std::cout;
using std::endl;
#endif

//using std::string;
using std::vector;

// Custom includes.
#include "Exception.h"
#include "Version.h"
#include "BigNum.h"
#include "RSAKey.h"
#include "UUCode.h"
#include "HelixApp.h"


// **** End of File ****
