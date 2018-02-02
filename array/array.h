#pragma once

#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#if !defined(ARRAY_USE_C89) && defined(_WIN32) &&	!defined(_WIN32_WCE)
#define ARRAY_USE_WINDOWS		/* enable goodies for regular Windows */
#endif


#if defined(ARRAY_USE_WINDOWS)
#define	ARRAY_USE_C89
#define ARRAY_BUILD_AS_DLL
#endif

/*
@@ DIRSEP is the directory deperator
*/
#if defined(_WIN32)
#define DIRSEP "\\"
#else
#define DIRSEP "/"
#endif

#if defined (ARRAY_BUILD_AS_DLL)
#ifdef ARRAY_EXPORTS
#define ARRAY_API __declspec(dllexport)
#else
#define ARRAY_API __declspec(dllimport)
#endif // ARRAY_EXPORTS
#else
#define ARRAY_API extern
#endif // (ARRAY_BUILD_AS_DLL)


#ifdef ARRAY_EXPORTS
#include "lua.hpp"
#else
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#endif // ARRAY_EXPORTS


#define BITS_PER_WORD (CHAR_BIT * sizeof(unsigned int))
#define I_WORD(i) ((unsigned int)(i) / BITS_PER_WORD)
#define I_BIT(i) (1 << ((unsigned int)(i) % BITS_PER_WORD))

#define ARRAYLIB_KEY ("LuaBook.array")
#define checkarray(L) ((BitArray *)luaL_checkudata(L, 1, ARRAYLIB_KEY))

typedef struct BitArray {
	int size;
	unsigned int values[1]; /* variable part */
} BitArray;

