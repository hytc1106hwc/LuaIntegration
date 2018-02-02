#pragma once

#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#if !defined(ARRAYOOP_USE_C89) && defined(_WIN32) &&	!defined(_WIN32_WCE)
#define ARRAYOOP_USE_WINDOWS		/* enable goodies for regular Windows */
#endif


#if defined(ARRAYOOP_USE_WINDOWS)
#define	ARRAYOOP_USE_C89
#define ARRAYOOP_BUILD_AS_DLL
#endif

/*
@@ DIRSEP is the directory deperator
*/
#if defined(_WIN32)
#define DIRSEP "\\"
#else
#define DIRSEP "/"
#endif

#if defined (ARRAYOOP_BUILD_AS_DLL)
#ifdef ARRAYOOP_EXPORTS
#define ARRAYOOP_API __declspec(dllexport)
#else
#define ARRAYOOP_API __declspec(dllimport)
#endif // ARRAYOOP_EXPORTS
#else
#define ARRAYOOP_API extern
#endif // (ARRAYOOP_BUILD_AS_DLL)


#ifdef ARRAYOOP_EXPORTS
#include "lua.hpp"
#else
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#endif // ARRAYOOP_EXPORTS


#define BITS_PER_WORD (CHAR_BIT * sizeof(unsigned int))
#define I_WORD(i) ((unsigned int)(i) / BITS_PER_WORD)
#define I_BIT(i) (1 << ((unsigned int)(i) % BITS_PER_WORD))

#define ARRAYOOPLIB_KEY ("LuaBook.array")
#define checkarray(L) ((BitArray *)luaL_checkudata(L, 1, ARRAYOOPLIB_KEY))

typedef struct BitArray {
	int size;
	unsigned int values[1]; /* variable part */
} BitArray ;

