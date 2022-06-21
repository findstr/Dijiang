#include <stdlib.h>
#include <string.h>
#define LUA_LIB
#include "lua.hpp"

static int
ldestroy(lua_State *L)
{
	size_t sz;
	const char *str = luaL_checklstring(L, 1, &sz);
	printf("get_sibling:%s\n", str);
	return 0;
};

extern "C" 
LUAMOD_API int
luaopen_engine_gameobject(lua_State *L)
{
	luaL_Reg tbl[] = {
		{"destroy", ldestroy},
		{NULL, NULL},
	};
	luaL_checkversion(L);
	luaL_newlibtable(L, tbl);
	luaL_setfuncs(L, tbl, 0);
	return 1;
}

