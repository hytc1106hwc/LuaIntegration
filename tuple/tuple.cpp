// tuple.cpp: 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "tuple.h"

/* 打印栈信息 */
TUPLE_API void stackDump(lua_State *L) {
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

/* tuple constructor's closure */
TUPLE_API int t_tuple(lua_State *L)
{
	lua_Integer op = luaL_optinteger(L, 1, 0);
	if (op == 0) { /* no arguments */
		int i;
		/* push all upvalue onto the stack */
		for (i = 0; !lua_isnone(L, lua_upvalueindex(i)); i++) {
			lua_pushvalue(L, lua_upvalueindex(i));
		}
		return (i - 1);
	}
	else { /* get field 'op' */
		/* index should between 0 and 256 */
		luaL_argcheck(L, 0 < op && op <= 256, 1, "index out of range");
		/* no such field in upvalue */
		if (lua_isnone(L, lua_upvalueindex((int)op)))
			return 0;

		/* find it, push it onto the stack */
		lua_pushvalue(L, lua_upvalueindex((int)op));
		return 1;
	}
}

/* get tuple library's version */
TUPLE_API int t_version(lua_State *L)
{
	int top = lua_gettop(L);
	luaL_argcheck(L, top == 0, top, "此函数没有参数");
	
	// get shared data
	lua_getfield(L, lua_upvalueindex(1), "VERSION");
	lua_getfield(L, lua_upvalueindex(1), "MAX_VERSION");
	lua_getfield(L, lua_upvalueindex(1), "MIN_VERSION");
	lua_getfield(L, lua_upvalueindex(1), "BUG_COUNT");

	const char *version = lua_tostring(L, -4);
	const char *maxver	= lua_tostring(L, -3);
	const char *minver	= lua_tostring(L, -2);
	const char *bugcount = lua_tostring(L, -1);
	lua_pop(L, 4);
	
	/*	
	printf("int = %d\n", sizeof(int));
	printf("unsigned int = %d\n", sizeof(unsigned int));*/
	printf("%d word\n", I_WORD(1));
	printf("%d bit\n", I_BIT(1));

	lua_pushfstring(L, "version=%s max=%s min=%s bug=%s\n", version, maxver, minver, bugcount);

	return 1;
}

/* tuple constructor */
TUPLE_API int t_new(lua_State *L)
{
	/* top is the total number of the arguments */
	int top = lua_gettop(L);
	luaL_argcheck(L, top < 256, top, "too many fields");
	lua_pushcclosure(L, t_tuple, top);
	return 1; // return the function's closure
}

static int create_sharedata(lua_State *L)
{
	// push MAX_VERSION
	lua_pushinteger(L, tupleinfo.MAX_VERSION);
	lua_setfield(L, -2, "MAX_VERSION");

	// push MIN_VERSION
	lua_pushinteger(L, tupleinfo.MIN_VERSION);
	lua_setfield(L, -2, "MIN_VERSION");

	// push BUG_NUM
	lua_pushinteger(L, tupleinfo.BUG_NUM);
	lua_setfield(L, -2, "BUG_COUNT");

	// push VERSION
	const int buf_size = sizeof(char) * 2;
	const char sep[2] = ".";
	const int size = 5 * buf_size;
	char maxver[buf_size], minver[buf_size], bugnum[buf_size], version[size];
	_itoa_s(tupleinfo.MAX_VERSION,	maxver, buf_size, 10);
	_itoa_s(tupleinfo.MIN_VERSION,	minver, buf_size, 10);
	_itoa_s(tupleinfo.BUG_NUM,		bugnum, buf_size, 10);

	/*printf("maxver=%s, size=%d\n", maxver, sizeof(maxver));
	printf("minver=%s, size=%d\n", minver, sizeof(minver));
	printf("bugnum=%s, size=%d\n", bugnum, sizeof(bugnum));*/

	// note: first should use strcpy_s
	strcpy_s(version, sizeof(version), maxver);
	strcat_s(version, sizeof(version), sep);
	strcat_s(version, sizeof(version), minver);
	strcat_s(version, sizeof(version), sep);
	strcat_s(version, sizeof(version), bugnum);
	//printf("version=%s\n", version);

	lua_pushstring(L, version);
	lua_setfield(L, -2, "VERSION");

	return 1;
}

static const luaL_Reg tuplelib[] = {
	{"new", t_new},
	{"version", t_version},
	{NULL, NULL}
};

TUPLE_API int luaopen_tuple(lua_State *L)
{
	//luaL_newlib(L, tuplelib);
	// create library table
	luaL_newlibtable(L, tuplelib);

	// create shared upvalue
	//create_sharedata(L);

	lua_newtable(L);
	create_sharedata(L);
	//lua_pushstring(L, "1.0.0");
	//lua_setfield(L, -2, "VERSION");

	/* add functions in list 'lib' to the new library, sharing
	previous table as upvalue */
	luaL_setfuncs(L, tuplelib, 1);

	lua_setglobal(L, "tuple");
	return 1;
}

