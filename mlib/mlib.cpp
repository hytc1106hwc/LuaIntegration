// mlib.cpp: 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "mlib.h"

using namespace std;
namespace mlib
{
	void * alloc(void *ud, void *ptr, size_t osize, size_t nsize)
	{
		lua_Alloc oloc = lua_getallocf(, *ud);
		if (checkTotalMemory(nsize, osize)) {
			lua_Alloc(ud, ptr, osize, nsize);
		}
		else {
			return NULL;
		}
	}

	bool checkTotalMemory(size_t newsize, size_t total)
	{
		return newsize <= total;
	}

	lua_State* Functions::getLuaState()
	{
		void *ud;
		lua_State *L = lua_newstate(alloc, ud);
		return NULL;
	}

	void Functions::setlimit(lua_State *L, int count)
	{
		
	}
}