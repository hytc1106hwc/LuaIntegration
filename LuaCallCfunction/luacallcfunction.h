#pragma once

#define __CPP
#ifdef __CPP
	#include "lua.hpp"
	#include "stdlib.h"
	#include "myclibs.h"
#else
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#endif // __CPP

