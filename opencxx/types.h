/*
  Copyright (C) 1997-2001 Shigeru Chiba, Tokyo Institute of Technology.

  Permission to use, copy, distribute and modify this software and   
  its documentation for any purpose is hereby granted without fee,        
  provided that the above copyright notice appear in all copies and that 
  both that copyright notice and this permission notice appear in 
  supporting documentation.

  Shigeru Chiba makes no representations about the suitability of this 
  software for any purpose.  It is provided "as is" without express or
  implied warranty.
*/

/*
  Copyright (c) 1995, 1996 Xerox Corporation.
  All Rights Reserved.

  Use and copying of this software and preparation of derivative works
  based upon this software are permitted. Any copy of this software or
  of any derivative work must include the above copyright notice of
  Xerox Corporation, this paragraph and the one after it.  Any
  distribution of this software or derivative works must comply with all
  applicable United States export control laws.

  This software is made available AS IS, and XEROX CORPORATION DISCLAIMS
  ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
  PURPOSE, AND NOTWITHSTANDING ANY OTHER PROVISION CONTAINED HEREIN, ANY
  LIABILITY FOR DAMAGES RESULTING FROM THE SOFTWARE OR ITS USE IS
  EXPRESSLY DISCLAIMED, WHETHER ARISING IN CONTRACT, TORT (INCLUDING
  NEGLIGENCE) OR STRICT LIABILITY, EVEN IF XEROX CORPORATION IS ADVISED
  OF THE POSSIBILITY OF SUCH DAMAGES.
*/

#ifndef _types_h
#define _types_h

#include <iostream>

#define	OPENCXX_NAMESPACE	Opencxx
#define USING_OPENCXX_NAMESPACE using namespace OPENCXX_NAMESPACE ;
#define	BEGIN_OPENCXX_NAMESPACE	namespace OPENCXX_NAMESPACE {
#define END_OPENCXX_NAMESPACE }

#ifndef nil
#define nil		0
#endif

#ifndef TRUE
#define TRUE		1
#define FALSE		0
#endif

//#define DONT_GC
//#define IRIX_CC	// If compiled under SGI Irix with CC (not g++)

BEGIN_OPENCXX_NAMESPACE

#if defined(_MSC_VER) && (_MSC_VER <= 1020)	// If MSVC version <= 4.2
#pragma warning(disable:4237)		// to avoid warning related to bool
typedef int bool;
#endif

typedef bool		BOOL;
typedef int		sint;
typedef unsigned int	uint;

#if defined(sun) && (defined(sparc) || defined(__sparc))
#  include <errno.h>
#  ifdef ECHRNG
#    define SUNOS5
#  else
#    define SUNOS4
#  endif
#endif

#define USE_DLOADER		1

#if !defined (_MSC_VER)
#define USE_SO			1
#else
#define USE_SO			0	// Use DLL
#endif

#if USE_DLOADER && defined(_MSC_VER)
#define OCXXMOP __declspec(dllexport)
#pragma warning(disable:4275)
#else
#define OCXXMOP
#endif

#if (defined(__FreeBSD__) && __FreeBSD__ <= 2) || defined (__OpenBSD__) || defined (__NetBSD__)
#define DLSYM_NEED_UNDERSCORE	1
#define SHARED_OPTION		0
#else	// Solaris, SunOS, Linux, Irix, FreeBSD 3.x, MacOS X
#define DLSYM_NEED_UNDERSCORE	0
#define SHARED_OPTION		1
#endif

#if USE_DLOADER && USE_SO

// #include <dlfcn.h> <-- no longer necessary, as we use libltdl

// Some operating systems like FreeBSD and SunOS do not define RTLD_GLOBAL
// or RTLD_LAZY

#ifndef RTLD_LAZY
#define RTLD_LAZY		1
#endif

#ifndef RTLD_GLOBAL
#define RTLD_GLOBAL		0
#endif

#endif /* USE_DLOADER */

/*
  class Object is the root of all classes. If a garbage collector
  is used, Object is an alias of gc_cleanup.
*/
#ifdef DONT_GC

END_OPENCXX_NAMESPACE
#include <string.h>		// For size_t
BEGIN_OPENCXX_NAMESPACE

class LightObject {};
class Object {};
enum GCPlacement {GC, NoGC};

using ::operator new;
using ::operator new[];

inline void* operator new (size_t size, GCPlacement gcp)
{
    return ::operator new (size);
}

inline void* operator new[] (size_t size, GCPlacement gcp)
{
    return ::operator new[] (size);
}

#else

END_OPENCXX_NAMESPACE
#include <gc/gc_cpp.h>
BEGIN_OPENCXX_NAMESPACE

typedef gc LightObject;
typedef gc_cleanup Object;

#endif

END_OPENCXX_NAMESPACE

#endif /* _types_h */
