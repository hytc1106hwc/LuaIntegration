// MyClibs.cpp: 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "myclibs.h"

#define MYCLIBS_NAME "myclibs"
using namespace std;

MYCLIBS_API void error(lua_State *L, const char *fmt, ...)
{
	va_list argp;
	va_start(argp, fmt);
	vfprintf(stderr, fmt, argp);
	va_end(argp);
	lua_close(L);
	exit(EXIT_FAILURE);
}

/* 打印栈信息 */
MYCLIBS_API void stackDump(lua_State *L) {
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

MYCLIBS_API bool myclibs_checkrange(lua_State *L, double r)
{
	if (r <= MININT32 || r >= MAXINT32)
	{
		error(L, "result should between %d and %d", MININT32, MAXINT32);
	}
	return true;
}

MYCLIBS_API int myclibs_tablepack(lua_State *L)
{
	int n = lua_gettop(L);		/* number of elements to pack */
	lua_createtable(L, n, 1);	/* create result table */
	lua_pushinteger(L, n);		
	lua_setfield(L, -2, "n");	/* t.n = number of elements */

	if (n > 0) {
		int i;
		lua_pushvalue(L, 1);
		lua_rawseti(L, -2, 1);	/* insert first element */
		//lua_replace(L, 1);
		lua_rotate(L, 1, 1);
		for (i = n; i >= 2; i--)/* assign other elements */
			lua_rawseti(L, 1, i);
		lua_settop(L, 1);
	}
	//stackDump(L);
	return 1;
}

MYCLIBS_API int myclibs_tableunpack(lua_State *L)
{
	//int i;
	int top = lua_gettop(L);
	if (top == 1 && lua_typename(L, lua_type(L, -1)) == "table")
	{
		lua_touserdata(L, -1);
	}
	return 1;
}

MYCLIBS_API int myclibs_reverse(lua_State *L)
{
	int i;
	int n = lua_gettop(L);
	for (i = n; i >= 1; i--)
	{
		lua_pushvalue(L, i);
	}
	return n;
}

MYCLIBS_API int myclibs_foreach(lua_State *L)
{
	lua_pushvalue(L, -1);			/* copy function */
	lua_pushnil(L);					/* add first key */
	//stackDump(L);
	while (lua_next(L, -4) != 0)
	{
		//stackDump(L);
		myclibs_lock(L);
		lua_pushvalue(L, -2);
		lua_insert(L, 1);
		//stackDump(L);
		lua_call(L, 2, 0);
		//stackDump(L);
		lua_pushvalue(L, -1);		/* copy function */
		
		lua_pushvalue(L, 1);		/* move stored key to the top */
		lua_remove(L, 1);
		//stackDump(L);
		myclibs_unlock(L);
	}
	return 0;
}


MYCLIBS_API int myclibs_map(lua_State *L)
{
	int i, n;

	/* first argument must be a table */
	luaL_checktype(L, 1, LUA_TTABLE);

	/* second argument must be a function */
	luaL_checktype(L, 2, LUA_TFUNCTION);

	/* get size of table */
	n = (int)luaL_len(L, 1);

	for (i = 1; i <= n; i++)
	{
		lua_pushvalue(L, 2);	/* push function f */
		lua_geti(L, 1, i);		/* push t[i] */
		lua_call(L, 1, 1);		/* call f(t[i]) */
		lua_seti(L, 1, i);		/* t[i] = result */
	}
	return 0;
}



/* summation of multi values */
MYCLIBS_API int myclibs_summation(lua_State *L)
{
	int i;
	int top = lua_gettop(L);
	double result = 0;
	if (top == 0)
	{
		lua_pushnumber(L, result);
	}
	else 
	{
		for (i = 1; i <= top; i++)
		{
			int t = lua_type(L, i);
			switch (t)
			{
				case LUA_TNUMBER:
				{
					myclibs_checkrange(L, result);	/* check value */
					result += (double)lua_tonumber(L, i);
					lua_pushnumber(L, result);
					break;
				}
				default:
				{
					const char *tyname = lua_typename(L, t);
					error(L, "bad argument #%d to 'summation'(number expected, got %s", i, tyname);
					break;
				}
			}
		}
	}
	return 1;
}

/* subversion of multi values */
MYCLIBS_API int myclibs_subversion(lua_State *L)
{
	int i;
	int top = lua_gettop(L);
	double result = 0;
	if (top == 0)
	{
		lua_pushnumber(L, result);
	}
	else
	{
		for (i = 1; i <= top; i++) {
			int t = lua_type(L, i);
			switch (t)
			{
				case LUA_TNUMBER:
				{
					myclibs_checkrange(L, result); /* check value */
					result -= (double)lua_tonumber(L, i);
					lua_pushnumber(L, result);
					break;
				}
				default:
				{
					const char *tyname = lua_typename(L, t);
					error(L, "bad argument #%d to 'summation'(number expected, got %s", i, tyname);
					break;
				}
			}
		}
	}
	return 1;
}

MYCLIBS_API int myclibs_filter(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TTABLE);		/* first argument should be a table */
	luaL_checktype(L, 2, LUA_TFUNCTION);	/* second argument should be a function */

	lua_newtable(L);
	int tbsize = lua_rawlen(L, 1);
	for (int i = 1, idx = 1; i <= tbsize; i++)
	{
		lua_pushvalue(L, 2);	/* push function */
		lua_rawgeti(L, 1, i);	/* push function arg */
		lua_call(L, 1, 1);		/* call the function */
		int result = lua_toboolean(L, -1);
		lua_pop(L, 1);
		//stackDump(L);
		if (result)
		{
			lua_rawgeti(L, 1, i);
			lua_rawseti(L, 3, idx++);
		}
		//stackDump(L);
	}
	return 1;
}

/*--------------------------------------------------------------------------------------*/
/* Technique for writing CFunctions
/*--------------------------------------------------------------------------------------*/

/**====== Array Manipulation ======**/
MYCLIBS_API int myclibs_getat(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TTABLE);	/* 1st argument should be a table */
	luaL_checknumber(L, 2);				/* 2nd argument should be a number */

	LUA_INTEGER index = lua_tointeger(L, 2);	/* get key */
	lua_geti(L, 1, index);						/* get t[key] and push it onto the top of stack */
	return 1;
}

/**======String Manipulation======**/
/* split function using strchr function */
MYCLIBS_API int myclibs_split(lua_State *L)
{
	const char *s = luaL_checkstring(L, 1);		/* original string */
	const char *sep = luaL_checkstring(L, 2);	/* seperator */
	const char *e;
	int i = 1;

	lua_newtable(L);	/* result table */

	while ((e = strchr(s, *sep)) != NULL)
	{
		lua_pushlstring(L, s, e - s);	/* push substring */
		lua_rawseti(L, 3, i++);			/* insert it into the result table */
		s = e + 1;						/* skip seperator */
	}
	// insert last string
	lua_pushstring(L, s);
	lua_rawseti(L, -2, i);				/* return the result table */

	return 1;
}

/* split function using memchr function */
MYCLIBS_API int myclibs_split2(lua_State *L)
{
	const char *s = luaL_checkstring(L, 1);
	const char *sep = luaL_checkstring(L, 2);

	lua_newtable(L);		/* create a table for stroing results */

	size_t i = 1, len = (size_t)luaL_len(L, 1);
	//printf("**%d\n", sep);
	const char *e;
	while ((e = (const char *)memchr(s, *sep, len)) != NULL)
	{
		lua_pushlstring(L, s, e - s);	/* push substring */
		lua_rawseti(L, 3, i++);			/* insert it into the result table */
		s = e + 1;						/* skip seperator */
	}
	// insert last string
	lua_pushstring(L, s);
	lua_rawseti(L, 3, i);
	return 1;	// retunrn the table
}

/* changer each char of a string to its upper case */
MYCLIBS_API int myclibs_upper(lua_State *L)
{
	size_t len;
	size_t i;

	// first, declare a luaL_Buffer variable
	luaL_Buffer b;
	const char *s = luaL_checklstring(L, 1, &len);

	// second, get a pointer for the buffer with the given size
	char *p = luaL_buffinitsize(L, &b, len);
	
	// then, create our string using the buffer
	for (i = 0; i < len; i++)
	{
		p[i] = toupper(UCHAR(s[i]));
	}

	// last, convert the buffer contents into a Lua string,
	// and push that string onto the stack
	luaL_pushresultsize(&b, len);
	return 1;
}

/**====== Storing State in C Functions ======**/

/** Registry Usage **/
static int myclibs_transliterate_arr(lua_State *L, int count)
{
	size_t i, len;
	const char *str = luaL_checklstring(L, -2, &len);	/* first argument should be a string */
	luaL_checktype(L, -1, LUA_TTABLE);					/* second argument should be a table */			
	
	// string buffer
	luaL_Buffer b;						/* declare a luaL_Buffer variable */
	luaL_buffinitsize(L, &b, len);		/* initialize the Buffer */

	for (i = 0; i < len; i++)
	{
		char ch[] = "x";
		ch[0] = str[i];
		lua_getfield(L, -1, ch);
		//lua_pushfstring(L, "%c", ch);		/* push key */
		//lua_rawget(L, -2);				/* get table[key] */
		int nil = lua_isnil(L, -1);
		if (nil) {
			lua_pop(L, 1);
		}
		else {
			const char *result = lua_tostring(L, -1);
			luaL_addvalue(&b);				/* add result to buffer */
		}
	}

	if (count == NULL) {
		lua_pushfstring(L, "%s\n", "calling with one argument");
	}
	else {
		lua_pushfstring(L, "%s\n", "calling with two argument");
	}
	luaL_pushresult(&b);		/* push the result string onto the stack */
	return 2;
}

/* set the transliteration table using registry */
MYCLIBS_API int myclibs_settrans(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TTABLE);				/* first argument should be a table */
	lua_pushlightuserdata(L, (void *)TRANS_KEY);	/* push address */
	lua_pushvalue(L, 1);							/* push value */
	lua_settable(L, LUA_REGISTRYINDEX);				/* registry[&key] = table */
	return 0;
}

/* get the transliteration table from the registry table */
MYCLIBS_API int myclibs_gettrans(lua_State *L)
{
	lua_pushlightuserdata(L, (void *)TRANS_KEY);	/* push address */
	lua_gettable(L, LUA_REGISTRYINDEX);				/* retrieve the table and push it onto the stack */
	return 1;
}

/* transliterate function */
MYCLIBS_API int myclibs_transliterate(lua_State *L)
{
	// get and push table onto the stack
	int top = lua_gettop(L), nres = 0;

	//printf("%d\n", top);
	if (top == 1)
	{
		myclibs_gettrans(L);
		nres = myclibs_transliterate_arr(L, NULL);
	}
	else if (top == 2) {
		nres = myclibs_transliterate_arr(L, 2);
	}
	else
	{
		luaL_error(L, "bad argument size");
	}
	return nres;
}

MYCLIBS_API int myclibs_setobject(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TTABLE);
	lua_pushvalue(L, 1);
	// get a reference from registry table
	ref = luaL_ref(L, LUA_REGISTRYINDEX);
	lua_rawsetp(L, LUA_REGISTRYINDEX, (void *)ref);
	return 0;
}

MYCLIBS_API int myclibs_getobject(lua_State *L)
{
	int top = lua_gettop(L);
	luaL_argcheck(L, top == 0, top, "This function has no args");
	// get object by the reference from registry table
	lua_rawgetp(L, LUA_REGISTRYINDEX, (void *)ref);
	return 1;
}

//static int myclibs_settrans_upvalues(lua_State *L)
//{
//	if (lua_isnone(L, lua_upvalueindex(1))) {
//		return 0;
//	}
//	else {
//		size_t len = (size_t)luaL_len(L, 1) * 2;
//		while ((lua_next(L, lua_upvalueindex(1))) != 0)
//		{
//			lua_pushvalue(L, -1);
//			lua_pop(L, 1);
//		}
//		return len;
//	}
//}

/* set the transliteration table using upvalues */
MYCLIBS_API int myclibs_settrans_up(lua_State *L)
{
	int top = lua_gettop(L);
	luaL_argcheck(L, top == 1, top, "too many arguments");
	luaL_checktype(L, 1, LUA_TTABLE);	/* first argument should be a table */
	// set transliteration table as a key in upvalue table
	lua_setfield(L, lua_upvalueindex(1), "trans_table");
	return 0;
}

MYCLIBS_API int myclibs_gettrans_up(lua_State *L)
{
	lua_pushvalue(L, lua_upvalueindex(1));
	lua_getfield(L, 1, "trans_table");
	return 1;
}

MYCLIBS_API int myclibs_transliterate_up(lua_State *L)
{
	int top = lua_gettop(L), nres = 0;
	luaL_argcheck(L, top == 1, top, "too many arguments");
	const char *str = luaL_checkstring(L, 1);
	lua_pushvalue(L, lua_upvalueindex(1));	/* push upvalue onto the stack */
	lua_getfield(L, 2, "trans_table");		/* get the table */
	lua_remove(L, 2);	/* remove upvalue table from the stack */
						//stackDump(L);
	nres = myclibs_transliterate_arr(L, 2);
	return nres;
}



/* define functions to be registered */
static const luaL_Reg myclibs[] = {
	{"getat",			myclibs_getat},

	{"split",			myclibs_split },
	{"split2",			myclibs_split2 },
	{"upper",			myclibs_upper },

	{"settrans",		myclibs_settrans },
	{"gettrans",		myclibs_gettrans },
	{"transliterate",	myclibs_transliterate },

	{"setobject",		myclibs_setobject },
	{"getobject",		myclibs_getobject },

	{"sum",				myclibs_summation},
	{"sub",				myclibs_subversion},
	{"tablepack",		myclibs_tablepack},
	{"reverse",			myclibs_reverse },
	{"foreach",			myclibs_foreach },
	
	
	{"filter",			myclibs_filter},
	
	{"settrans_up",		myclibs_settrans_up },
	{"gettrans_up",		myclibs_gettrans_up },
	{"transliterate_up",myclibs_transliterate_up},
	{ NULL, NULL }
};

/* create a new lib and set an alias for the lib */
MYCLIBS_API int luaopen_myclibs(lua_State *L)
{
	//luaL_newlib(L, myclibs);
	luaL_newlibtable(L, myclibs);
	lua_newtable(L);
	luaL_setfuncs(L, myclibs, 1);
	lua_setglobal(L, MYCLIBS_NAME);
	return 1;
}