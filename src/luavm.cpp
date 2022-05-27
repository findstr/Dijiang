#include <stdlib.h>
#include "lua.hpp"
#include "luavm.h"

#define max(a, b)    ((a) > (b) ? (a) : (b))

#define STK_TRACE	(1)
#define STK_REF		(2)
#define STK_REQUIRE	(3)
#define STK_LAST	STK_REQUIRE


namespace engine {
namespace luavm {

static struct {
	lua_State *main = nullptr;
	lua_State *ctx = nullptr;
	int ctx_handle = LUA_REFNIL;
} M;

static void *
lua_alloc(void *ud, void *ptr, size_t osize, size_t nsize)
{
	(void) ud;
	(void) osize;
	if (nsize == 0) {
		::free(ptr);
		return NULL;
	} else {
		return realloc(ptr, nsize);
	}
}

static int
ltraceback(lua_State *L)
{
	const char *str = luaL_checkstring(L, 1);
	luaL_traceback(L, L, str, 1);
	return 1;
}

static int
setlibpath(lua_State *L, const char *libpath, const char *clibpath)
{
	const char *path;
	const char *cpath;
	size_t sz1 = strlen(libpath);
	size_t sz2 = strlen(clibpath);
	size_t sz3;
	size_t sz4;
	size_t need_sz;

	lua_getglobal(L, "package");
	lua_getfield(L, -1, "path");
	path = luaL_checklstring(L, -1, &sz3);

	lua_getfield(L, -2, "cpath");
	cpath = luaL_checklstring(L, -1, &sz4);

	need_sz = max(sz1, sz2) + max(sz3, sz4) + 1;
	char new_path[need_sz];

	snprintf(new_path, need_sz, "%s;%s", libpath, path);
	lua_pushstring(L, new_path);
	lua_setfield(L, -4, "path");

	snprintf(new_path, need_sz, "%s;%s", clibpath, cpath);
	lua_pushstring(L, new_path);
	lua_setfield(L, -4, "cpath");

	//clear the stack
	lua_settop(L, 0);
	return 0;
}

void
init()
{
	M.main = lua_newstate(lua_alloc, nullptr);
	luaL_openlibs(M.main);
	setlibpath(M.main, "asset/lua/?.lua", "./?.so");
	M.ctx = lua_newthread(M.main);
	M.ctx_handle = luaL_ref(M.main, LUA_REGISTRYINDEX);
	lua_pushcfunction(M.ctx, ltraceback);
	lua_newtable(M.ctx);
	lua_getglobal(M.ctx, "require");
	return ;
}

void
collect(int handle)
{
	if (handle > 0)
		luaL_unref(M.ctx, STK_REF, handle);
}

int
new_component(gameobject *go, const std::string &name)
{
	auto *L = M.ctx;
	lua_newtable(L);
	lua_pushlightuserdata(L, go);
	lua_setfield(L, -2, "__go");
	if (luaL_newmetatable(L, name.c_str())) {
		lua_pushvalue(L, STK_REQUIRE);
		lua_pushlstring(L, name.c_str(), name.size());
		if (lua_pcall(L, 1, 1, STK_TRACE) != LUA_OK) {
			fprintf(stderr, "[luavm] new_component:%s err:%s\n",
				name.c_str(), lua_tostring(L, -1));
			lua_pop(L, 2);
			return -1;
		}
		lua_setfield(L, -2, "__index");
	}
	lua_setmetatable(L, -2);
	return luaL_ref(L, STK_REF);
}

int
get_func(int handle, const std::string &name)
{
	int func_handle;
	auto *L = M.ctx;
	lua_rawgeti(L, STK_REF, handle);
	lua_getfield(L, -1, name.c_str());
	func_handle = luaL_ref(L, STK_REF);
	lua_pop(L, 1);
	return func_handle;
}

void
call_with_self(int self, int func, float value)
{
	auto *L = M.ctx;
	lua_rawgeti(L, STK_REF, func);
	lua_rawgeti(L, STK_REF, self);
	lua_pushnumber(L, value);
	if (lua_pcall(L, 2, 0, STK_TRACE) != LUA_OK) {
		fprintf(stderr, "[luavm] call_func err:%s\n",
			lua_tostring(L, -1));
		lua_pop(L, 1);
	}
	return ;
}

void
call_with_self(int self, const std::string &func)
{
	auto *L = M.ctx;
	lua_rawgeti(L, STK_REF, self);
	lua_getfield(L, -1, func.c_str());
	lua_pushvalue(L, -2);
	if (lua_pcall(L, 1, 0, STK_TRACE) != LUA_OK) {
		fprintf(stderr, "[luavm] call_func err:%s\n",
			lua_tostring(L, -1));
		lua_pop(L, 1);
	}
	lua_pop(L, 1);
	return ;
}

void
set_table(int table, const std::string &key, float value)
{
	auto *L = M.ctx;
	lua_rawgeti(L, STK_REF, table);
	lua_pushlstring(L, key.c_str(), key.size());
	lua_pushnumber(L, value);
	lua_settable(L, -3);
	lua_pop(L, 1);
	return ;
}

void
set_table(int table, const std::string &key, const std::string &val)
{
	auto *L = M.ctx;
	lua_rawgeti(L, STK_REF, table);
	lua_pushlstring(L, key.c_str(), key.size());
	lua_pushlstring(L, val.c_str(), val.size());
	lua_settable(L, -3);
	lua_pop(L, 1);
	return ;
}


}}

