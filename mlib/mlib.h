#pragma once

#ifdef MLIB_EXPORTS
	#define MLIB_API __declspec(dllexport)
#else
	#define MLIB_API __declspec(dllimport)
#endif

namespace mlib
{
	class Functions
	{
	public:
		static MLIB_API lua_State* getLuaState();
		static MLIB_API void setlimit(lua_State *L, int count);
	};
}