// array.cpp: 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "array.h"

/* 打印栈信息 */
ARRAY_API void stackDump(lua_State *L) {
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

ARRAY_API int newarray(lua_State *L)
{
	unsigned int i;
	size_t nbytes;
	BitArray *a;

	int n = (int)luaL_checkinteger(L, 1);
	luaL_argcheck(L, n >= 1, n, "invalid size");
	//printf("struct size = %d\n", sizeof(BitArray));
	
	/*printf("word size = %d\n", I_WORD(n - 1));
	printf("other size = %d\n", I_WORD(n - 1) * sizeof(unsigned int));*/
	nbytes = sizeof(BitArray) + I_WORD(n - 1) * sizeof(unsigned int);
	a = (BitArray *)lua_newuserdata(L, nbytes);

	/* initialize the array */
	a->size = n;
	for (i = 0; i < I_WORD(n - 1); i++)
	{
		//printf("time = %d\n", i);
		a->values[i] = 0;
	}
	luaL_getmetatable(L, ARRAYLIB_KEY);
	lua_setmetatable(L, -2);
	return 1;
}

ARRAY_API unsigned int *getparams(lua_State *L, unsigned int *mask)
{
	BitArray *a = checkarray(L);
	int index = (int)luaL_checkinteger(L, 2) - 1;

	luaL_argcheck(L, 0 <= index && index < a->size, 2, "index out of range");

	*mask = I_BIT(index);
	return &(a->values[I_WORD(index)]);
}


/* set value with specific index and value */
ARRAY_API int setarray(lua_State *L)
{
	//BitArray *a = (BitArray *)lua_touserdata(L, 1);
	//int index = (int)luaL_checkinteger(L, 2) - 1;
	//// check whether or not parameter is valid
	//luaL_argcheck(L, a != NULL, 1, "'array' expected");
	//luaL_argcheck(L, 0 <= index && index < a->size, 2, "index out of range");
	//luaL_checkany(L, 3);

	//if (lua_toboolean(L, 3))
	//{
	//	a->values[I_WORD(index)] |= I_BIT(index); // set bit
	//}
	//else {
	//	a->values[I_WORD(index)] &= I_BIT(index); // reset bit
	//}

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
ARRAY_API int getarray(lua_State *L)
{
	//BitArray *a = (BitArray *)lua_touserdata(L, 1);
	//int index = (int)luaL_checkinteger(L, 2) - 1;

	//// check whether or not parameter is valid
	//luaL_argcheck(L, a != NULL, 1, "'array' expected");
	//luaL_argcheck(L, 0 <= index && index < a->size, 2, "index out of range");

	//lua_pushboolean(L, a->values[I_WORD(index)] & I_BIT(index));

	unsigned int mask;
	unsigned int *entry = getparams(L, &mask);
	lua_pushboolean(L, *entry & mask);

	return 1;
}

/* get size of array */
ARRAY_API int getsize(lua_State *L)
{
	//BitArray *a = (BitArray *)lua_touserdata(L, 1);
	///* check whether or not parameter is valid */
	//luaL_argcheck(L, a != NULL, 1, "'array' expected");
	BitArray *a = checkarray(L);
	lua_pushinteger(L, a->size);
	return 1;
}

static const luaL_Reg arraylib[] = {
	{ "new", newarray },
	{ "set", setarray },
	{ "get", getarray },
	{ "size", getsize },
	{ NULL, NULL },
};

ARRAY_API int luaopen_array(lua_State *L)
{
	luaL_newmetatable(L, ARRAYLIB_KEY);
	luaL_newlib(L, arraylib);
	lua_setglobal(L, "array");
	return 1;
}
