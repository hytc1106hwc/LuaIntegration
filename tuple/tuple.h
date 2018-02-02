#pragma once

#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#if !defined(TUPLE_USE_C89) && defined(_WIN32) &&	!defined(_WIN32_WCE)
#define TUPLE_USE_WINDOWS		/* enable goodies for regular Windows */
#endif


#if defined(TUPLE_USE_WINDOWS)
#define	TUPLE_USE_C89
#define TUPLE_BUILD_AS_DLL
#endif

/*
@@ DIRSEP is the directory deperator
*/
#if defined(_WIN32)
#define DIRSEP "\\"
#else
#define DIRSEP "/"
#endif

#if defined (TUPLE_BUILD_AS_DLL)
#ifdef TUPLE_EXPORTS
	#define TUPLE_API __declspec(dllexport)
#else
	#define TUPLE_API __declspec(dllimport)
#endif // TUPLE_EXPORTS
#else
	#define TUPLE_API extern
#endif // (TUPLE_BUILD_AS_DLL)


#ifdef TUPLE_EXPORTS
	#include "lua.hpp"
#else
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
#endif // TUPLE_EXPORTS

struct TUPLE_INFO {
	int MAX_VERSION;
	int MIN_VERSION;
	int BUG_NUM;

} tupleinfo = {
	1,
	0,
	0
};

#define BITS_PER_WORD (CHAR_BIT * sizeof(unsigned int))
#define I_WORD(i) ((unsigned int)(i) / BITS_PER_WORD)
#define I_BIT(i) (1 << ((unsigned int)(i) % BITS_PER_WORD))