// LuaConfig.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"

void error(lua_State *L, const char *fmt, ...)
{
	va_list argp;
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

/*---------------------------------------------------------------------
 * 基本用法
 *--------------------------------------------------------------------*/
int getglobint(lua_State *L, const char *var)
{
	int isnum, result;
	lua_getglobal(L, var);
	result = (int)lua_tointegerx(L, -1, &isnum);
	if (!isnum) {
		error(L, "'%s' should be a number\n", var);
	}
	lua_pop(L, 1);
	return result;
}

void load(lua_State *L, const char *fname, int *w, int *h)
{
	if (luaL_loadfile(L, fname) || lua_pcall(L, 0, 0, 0)) {
		error(L, "cannot run config.file: %s", lua_tostring(L, -1));
	}
	*w = getglobint(L, "width");
	*h = getglobint(L, "height");
	fprintf(stdout, "width=%d\nheight=%d\n", *w, *h);
}


/*---------------------------------------------------------------------
 * 使用table保存颜色值
 *--------------------------------------------------------------------*/

struct ColorTable {
	const char* name;
	unsigned char red, green, blue;
} colortable[] = {
	{ "WHITE",	MAX_COLOR,	MAX_COLOR,	MAX_COLOR },
	{ "RED",	MAX_COLOR,			0,			0 },
	{ "GREEN",			0,	MAX_COLOR,			0 },
	{ "BLUE",			0,			0,	MAX_COLOR },
	{ "NULL",			0,			0,			0 }
};

/* get color from the table */
int getcolorfield(lua_State *L, const char* key)
{
	int result;
	//int isnum;
	/*lua_pushstring(L, key);
	lua_gettable(L, -2);
	result = (int)lua_tonumberx(L, -1, &isnum);
	if (!isnum) {
		luaL_error(L, "Invalid component '%s' in color", key);
	}*/

	if (lua_getfield(L, -1, key) != LUA_TNUMBER) {
		luaL_error(L, "Invalid component '%s' in color", key);
	}
	result = (int)lua_tonumber(L, -1);
	lua_pop(L, 1);
	return result;
}

/* put color into the table */
void setcolorfield(lua_State* L, const char* index, int value)
{
	/*lua_pushstring(L, index);
	lua_pushnumber(L, (value / MAX_COLOR));
	lua_settable(L, -3);*/

	lua_pushnumber(L, (value / MAX_COLOR));
	lua_setfield(L, -2, index);
}

void setcolor(lua_State* L, struct ColorTable* ct)
{
	/*lua_newtable(L);*/
	lua_createtable(L, 0, 3);
	setcolorfield(L, "red", ct->red);
	setcolorfield(L, "green", ct->green);
	setcolorfield(L, "blue", ct->blue);
	lua_setglobal(L, ct->name);
}

using namespace std;
int main()
{
	static const char* cfg = "config.lua";
	int w = 0, h = 0;
	int red, green, blue;

	lua_State *L = luaL_newstate();
	luaL_openlibs(L);

	// 基本用法
	load(L, cfg, &w, &h);
	stackDump(L);

	/*

	lua_getglobal(L, "background");
	if (!lua_istable(L, -1)) {
		luaL_error(L, "'background' is not a table.");
	}

	red		= getcolorfield(L, "red");
	green	= getcolorfield(L, "green");
	blue	= getcolorfield(L, "blue");

	fprintf(stdout, "red=%d\ngreen=%d\nblue=%d\n", red, green, blue);*/

	
	/*lua_getglobal(L, "background");
	if (lua_isstring(L, -1)) {
		const char *name = lua_tostring(L, -1);
		int i;
		for (i = 0; colortable[i].name != NULL; i++) {
			if (strcmp(name, colortable[i].name) == 0)
				break;
		}

		if (colortable[i].name == NULL) {
			luaL_error(L, "Invalid color name (%s)", name);
		}
		else {
			red = colortable[i].red;
			green = colortable[i].green;
			blue = colortable[i].blue;
		}
	}
	else if (lua_istable(L, -1)) {
		red = getcolorfield(L, "red");
		green = getcolorfield(L, "green");
		blue = getcolorfield(L, "blue");
	}
	else {
		luaL_error(L, "Invalid value for 'background'");
	}

	fprintf(stdout, "red=%d\ngreen=%d\nblue=%d\n", red, green, blue);*/

	int i = 0;
	while (colortable[i].name != "NULL")
	{
		setcolor(L, &colortable[i++]);
	}
	
	lua_getglobal(L, "BLUE");
	red = getcolorfield(L, "red");
	green = getcolorfield(L, "green");
	blue = getcolorfield(L, "blue");

	fprintf(stdout, "red=%d\ngreen=%d\nblue=%d\n", red, green, blue);

	system("pause");
    return 0;
}

