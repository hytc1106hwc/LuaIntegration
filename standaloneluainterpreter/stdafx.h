// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>



// TODO: 在此处引用程序需要的其他头文件
#define __cpp
#ifdef __cpp
	#include "lua.hpp"
#else
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
#endif