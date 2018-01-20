#pragma once

#include <limits.h>
#include <stddef.h>

#if !defined(MYCLIBS_USE_C89) && defined(_WIN32) &&	!defined(_WIN32_WCE)
#define MYCLIBS_USE_WINDOWS		/* enable goodies for regular Windows */
#endif


#if defined(MYCLIBS_USE_WINDOWS)
#define	MYCLIBS_USE_C89
#define MYCLIBS_BUILD_AS_DLL
#endif

/*
@@ MYCLIBS_DIRSEP is the directory deperator
*/
#if defined(_WIN32)
#define MYCLIBS_DIRSEP "\\"
#else
#define MYCLIBS_DIRSEP "/"
#endif

#if defined (MYCLIBS_BUILD_AS_DLL)
#ifdef MYCLIBS_EXPORTS
#define MYCLIBS_API __declspec(dllexport)
#else
#define MYCLIBS_API __declspec(dllimport)
#endif // MYCLIBS_EXPORTS
#else
#define MYCLIBS_API extern
#endif // (MYCLIBS_BUILD_AS_DLL)
