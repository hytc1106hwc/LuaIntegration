// standaloneluainterpreter.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"

using namespace std;

void print_error(lua_State *L, const char *fmt, ...) {
	va_list	argp;
	va_start(argp, fmt);
	vfprintf(stderr, fmt, argp);
	va_end(argp);
	lua_close(L);
	exit(EXIT_FAILURE);
}

/* 打印栈信息 */
void stackDump(lua_State *L) {
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

int main(void)
{
	/*char buff[256];
	int error;*/
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);

	lua_pushstring(L, "xiaohu");
	lua_pushnumber(L, 12);
	lua_pushboolean(L, false);

	stackDump(L);

	lua_createtable(L, 0, 3);

	stackDump(L);

	lua_rotate(L, 1, 1);
	
	stackDump(L);

	int top = lua_gettop(L);
	for (int i = 2; i <= top; i++)
	{
		lua_pushvalue(L, i);
		lua_setfield(L, 1, "" + (i - 1));
	}

	lua_settop(L, 1);

	stackDump(L);
	//while (fgets(buff, sizeof(buff), stdin)) {
	//	error = luaL_loadstring(L, buff) || lua_pcall(L, 0, 0, 0);
	//	if (error) {
	//		//fprintf(stderr, "%s\n", lua_tostring(L, -1));
	//		const char *fmt = "%s\n";
	//		print_error(L, fmt, lua_tostring(L, -1));
	//		lua_pop(L, 1);
	//	}
	//}

	system("pause");

	lua_close(L);
    return 0;
}

