
#include "stdafx.h"
#include "luacallcfunction.h"

void print_error(lua_State *L, const char *fmt, ...) {
	va_list	argp;
	va_start(argp, fmt);
	vfprintf(stderr, fmt, argp);
	va_end(argp);
	lua_close(L);
	exit(EXIT_FAILURE);
}

using namespace std;
int main()
{
	static const char *fname = "luafunc.lua";
	char buff[256];
	int error;
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);

	while (fgets(buff, sizeof(buff), stdin)) {
		error = luaL_loadstring(L, buff) || lua_pcall(L, 0, 0, 0);
		if (error) {
			//fprintf(stderr, "%s\n", lua_tostring(L, -1));
			const char *fmt = "%s\n";
			print_error(L, fmt, lua_tostring(L, -1));
			lua_pop(L, 1);
		}
	}
	lua_close(L);
	system("pause");
	return 0;
}