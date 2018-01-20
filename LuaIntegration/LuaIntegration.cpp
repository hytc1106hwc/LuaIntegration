
#include "main.h"

extern "C" {
	static int l_cppfunction(lua_State *L) {
		double arg = luaL_checknumber(L, 1);
		lua_pushnumber(L, arg * 0.5);
		return 1;
	}
}

static void stackDump(lua_State *L) {
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

class MyClass
{
	public:
		MyClass();
		~MyClass();

	private:

};

MyClass::MyClass()
{
}

MyClass::~MyClass()
{
}

using namespace std;
static void LuaIntegration() {
	cout << "** Test Lua embedding" << endl;
	cout << "** Init Lua" << endl;

	cout << "" << endl;
	/* open Lua */
	lua_State *L = luaL_newstate();
	cout << "** Load the optional standard libraries, to have the print functioin" << endl;
	/* open the standard libraries */
	luaL_openlibs(L);

	cout << "" << endl;

	cout << "** Load chunk. without executing it" << endl;
	if (luaL_loadfile(L, "luascript.lua")) {
		cerr << "Something went wrong loading the chunk(syntax error?)" << endl;
		cerr << lua_tostring(L, -1) << endl;
		lua_pop(L, 1);
	}
	cout << "" << endl;

	cout << "** Make a insert a global var into Lua from C++" << endl;
	lua_pushnumber(L, 1.1);
	lua_setglobal(L, "cppvar");

	stackDump(L);

	cout << "" << endl;

	cout << "** Execute the lua chunk" << endl;
	if (lua_pcall(L, 0, LUA_MULTRET, 0)) {
		cerr << "Something went wrong during execution" << endl;
		cerr << lua_tostring(L, -1) << endl;
		lua_pop(L, 1);
	}

	stackDump(L);

	cout << "" << endl;

	cout << "***********************************************************" << endl;
	cout << "** Read a global var from Lua into C++" << endl;
	lua_getglobal(L, "luavar");
	double luavar = lua_tonumber(L, -1);
	lua_pop(L, 1);
	cout << "C++ can read the value set from Lua, luavar = " << luavar << endl;

	cout << "** Execute a Lua function from C++" << endl;
	lua_getglobal(L, "myluafunction");
	lua_pushnumber(L, 5);

	stackDump(L);

	lua_pcall(L, 1, 1, 0);
	const char *values = lua_tostring(L, -1);
	cout << "The return value of the function was " << values << endl;
	lua_pop(L, 1);

	stackDump(L);

	cout << "" << endl;

	cout << "************************************************************" << endl;
	cout << "** Execute a C++ function from Lua" << endl;
	cout << "**** First register the function in Lua" << endl;
	lua_pushcfunction(L, l_cppfunction);
	lua_setglobal(L, "cppfunction");

	cout << "**** call a Lua function that uses the C++ function" << endl;
	lua_getglobal(L, "myfunction");
	lua_pushnumber(L, 5);

	stackDump(L);

	lua_pcall(L, 1, 1, 0);
	double num = lua_tonumber(L, -1);
	cout << "The return value of the function was " << num << endl;

	cout << "" << endl;
	cout << "** Release the Lua environment" << endl;
	lua_close(L);
	system("pause");
}