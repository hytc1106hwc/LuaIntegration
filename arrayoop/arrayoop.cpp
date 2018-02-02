// arrayoop.cpp: 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "arrayoop.h"

// array.cpp: 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "arrayoop.h"

/* 打印栈信息 */
ARRAYOOP_API void stackDump(lua_State *L) {
	int i;
	int top = lua_gettop(L);
	for (i = 1; i <= top; i++) {
		int t = lua_type(L, i);
		switch (t)
		{
		case LUA_TSTRING:
			printf("####'%s'", lua_tostring(L, i));
			break;
		case LUA_TBOOLEAN:
			printf(lua_toboolean(L, i) ? "####true" : "####false");
			break;
		case LUA_TNUMBER:
			printf("####%g", lua_tonumber(L, i));
			break;
		default:
			printf("####%s", lua_typename(L, t));
			break;
		}

		printf("  ");
	}
	printf("\n");
}

ARRAYOOP_API int newarray(lua_State *L)
{
	unsigned int i;
	size_t nbytes;
	BitArray *a;

	int n = (int)luaL_checkinteger(L, 1);
	luaL_argcheck(L, n >= 1, n, "invalid size");
	nbytes = sizeof(BitArray) + I_WORD(n - 1) * sizeof(unsigned int);
	a = (BitArray *)lua_newuserdata(L, nbytes);

	/* initialize the array */
	a->size = n;
	for (i = 0; i < I_WORD(n - 1); i++)
	{
		a->values[i] = 0;
	}
	luaL_getmetatable(L, ARRAYOOPLIB_KEY);
	lua_setmetatable(L, -2);
	return 1;
}

ARRAYOOP_API unsigned int *getparams(lua_State *L, unsigned int *mask)
{
	BitArray *a = checkarray(L);
	int index = (int)luaL_checkinteger(L, 2) - 1;

	luaL_argcheck(L, 0 <= index && index < a->size, 2, "index out of range");

	*mask = I_BIT(index);
	return &(a->values[I_WORD(index)]);
}


/* set value with specific index and value */
ARRAYOOP_API int setarray(lua_State *L)
{
	unsigned int mask;
	unsigned int *entry = getparams(L, &mask);
	luaL_checkany(L, 3);
	if (lua_toboolean(L, 3))
		*entry |= mask;
	else
		*entry &= ~mask;
	return 0;
}

/* get value with specific index */
ARRAYOOP_API int getarray(lua_State *L)
{
	unsigned int mask;
	unsigned int *entry = getparams(L, &mask);
	lua_pushboolean(L, *entry & mask);

	return 1;
}

/* get size of array */
ARRAYOOP_API int getsize(lua_State *L)
{
	BitArray *a = checkarray(L);
	lua_pushinteger(L, a->size);
	return 1;
}

/* output the array in string format */
ARRAYOOP_API int array2string(lua_State *L)
{
	BitArray *a = checkarray(L);
	lua_pushfstring(L, "array(%d)", a->size);
	return 1;
}

/* regular methods */
static const luaL_Reg arraylib_f[] = {
	{ "new", newarray },
	{ NULL, NULL }
};

/* methods array */
static const luaL_Reg arraylib_m[] = {
	{ "set", setarray },
	{ "get", getarray },
	{ "size", getsize },
	{ "__tostring", array2string},
	{ NULL, NULL }
};

ARRAYOOP_API int luaopen_arrayoop(lua_State *L)
{
	luaL_newmetatable(L, ARRAYOOPLIB_KEY);// create metatable
	lua_pushvalue(L, -1);// duplicate the metatable
	lua_setfield(L, -2, "__index"); // mt.__index = mt
	luaL_setfuncs(L, arraylib_m, 0); // register metamethods
	luaL_newlib(L, arraylib_f); // create lib table
	lua_setglobal(L, "arrayoop");
	return 1;
}

