// LuaFuncCalling.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"

/* call a function 'f' defined in Lua */
double f(lua_State *L, double x, double y)
{
	int isnum;
	double z;

	/* push functions and arguments */
	lua_getglobal(L, "f");
	lua_pushnumber(L, x);
	lua_pushnumber(L, y);

	/* do the call (2 arguments, 1 result) */
	if (lua_pcall(L, 2, 1, 0) != LUA_OK) {
		luaL_error(L, "error running function 'f': %s", lua_tostring(L, -1));
	}

	/* retrieve result */
	z = lua_tonumberx(L, -1, &isnum);
	if (!isnum) {
		luaL_error(L, "function 'f' should return a number");
	}
	lua_pop(L, 1); /* pop returned value */
	return z;
}

void call_va(lua_State *L, const char *func, const char *sig, ...)
{
	va_list v1;
	int narg, nres; /* number of arguments and results */

	va_start(v1, sig);

	lua_getglobal(L, func); /* push function */

	/* push and count arguments */
	for (narg = 0; *sig; narg++) {	/* repeat for each argument */
		
		/* check stack space */
		luaL_checkstack(L, 1, "too many arguments");

		switch (*sig++)
		{
			case 'd':
			{
				lua_pushnumber(L, va_arg(v1, double));
				break;
			}
			case 'i':
			{
				lua_pushinteger(L, va_arg(v1, int));
				break;
			}
			case 's':
			{
				lua_pushstring(L, va_arg(v1, char *));
				break;
			}
			case '>':
			{
				goto endargs;
			}
			default:
				luaL_error(L, "invalid option(%c)", *(sig - 1));
		}
	}
	
endargs:
	nres = strlen(sig);

	if (lua_pcall(L, narg, nres, 0) != 0) {
		luaL_error(L, "error calling '%s': '%s'", func, lua_tostring(L, -1));
	}

	nres = -nres;	/* get the first result's stack index */
	while (*sig) {
		switch (*sig++) {
			case 'd':
			{
				int isnum;
				double n = lua_tonumberx(L, nres, &isnum);
				if (!isnum) {
					luaL_error(L, "wrong result type");
				}
				*va_arg(v1, double *) = n;
				break;
			}
			case 'i':
			{
				int isnum;
				int n = (int)lua_tointegerx(L, nres, &isnum);
				if (!isnum) {
					luaL_error(L, "wrong result type");
				}
				*va_arg(v1, int *) = n;
				break;
			}
			case 's':
			{
				const char *s = lua_tostring(L, nres);
				if (s == NULL) {
					luaL_error(L, "wrong result type");
				}
				*va_arg(v1, const char **) = s;
				break;
			}
			default:
				luaL_error(L, "invalid option (%c)", *(sig - 1));
		}
		nres++;
	}

	va_end(v1);
}

using namespace std;
int main()
{
	static const char *fname = "luafunc.lua";
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);

	if (luaL_loadfile(L, fname) || lua_pcall(L, 0, 0, 0)) {
		luaL_error(L, "cannot run luafunc.file: %s", lua_tostring(L, -1));
	}

	fprintf(stdout, "x^2 * +math.sin(y) / (1 - x) = %f\n", f(L, 0.1, 1));

	/*double z = 0;
	call_va(L, "f", "dd>d", (double)0.1, (double)1, &z);

	printf("x^2 * math.sin(y) / (1 - x) = %f\n", z);*/

	system("pause");
    return 0;
}

