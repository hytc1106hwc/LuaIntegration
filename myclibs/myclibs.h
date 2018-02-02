#pragma once

#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include "myclibsconf.h"

#define MYCLIBS_VERSION_MAJOR "5"
#define MYCLIBS_VERSION_MINOR "3"
#define MYCLIBS_VERSION_RELEASE "4"

#define MYCLIBS_VERSION "Myclibs" MYCLIBS_VERSION_MAJOR "." MYCLIBS_VERSION_MINOR
#define MYCLIBS_RELEASE MYCLIBS_VERSION_MAJOR "." MYCLIBS_VERSION_MINOR
#define MYCLIBS_COPYRIGHT MYCLIBS_RELEASE "Copyright(C) 2018-2020 aris.org.cn"
#define MYCLIBS_AUTHORS "Aris Hu"

#ifdef MYCLIBS_EXPORTS
	#include "lua.hpp"
#else
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
#endif // MYCLIBS_EXPORTS


MYCLIBS_API int luaopen_myclibs(lua_State *L);

#define myclibs_lock(L)		((void) 0)
#define myclibs_unlock(L)   ((void) 0)

#define TRANS_KEY ((char)'t')

static int myclibs_settrans_upvalues(lua_State *L);
static int ref = 0;

