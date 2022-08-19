#include <stdlib.h>
#include <string.h>
#include "render/input.h"
#define LUA_LIB
#include "lua.hpp"

#define KEY_NONE	1
#define KEY_DOWN	2
#define KEY_UP		3
#define KEY_UP_COUNT	3

#define TOUCH_FINTERID		1
#define TOUCH_PHASE		2
#define TOUCH_POS_X		3
#define TOUCH_POS_Y		4
#define TOUCH_DELTATIME		5
#define TOUCH_DELTA_X		6
#define TOUCH_DELTA_Y		7
#define TOUCH_PHASE_BEGAN	8
#define TOUCH_PHASE_MOVED	9
#define TOUCH_PHASE_STATIONARY	10
#define TOUCH_PHASE_ENDED	11
#define TOUCH_PHASE_CANCELED	12
#define TOUCH_UP_COUNT		12
static int 
lgetkey(lua_State *L) 
{
	int upval = KEY_NONE;
	int key = luaL_checkinteger(L, 1);
	auto action = engine::input::inst().get_key((engine::input::key_code)key);
	switch (action) {
        case engine::input::key_action::DOWN:
		upval = KEY_DOWN;
		break;
        case engine::input::key_action::UP:
		upval = KEY_UP;
		break;
        case engine::input::key_action::NONE:
		upval = KEY_NONE;
		break;
	}
	lua_pushvalue(L, lua_upvalueindex(upval));
	return 1;
}

static int
ltouchcount(lua_State *L) 
{
	int count = engine::input::inst().touch_count();
	lua_pushinteger(L, count);
	return 1;
}

static inline void
set_table(lua_State *L, int t, int key, int val) 
{
	lua_pushvalue(L, lua_upvalueindex(key));
	lua_pushinteger(L, val);
	lua_settable(L, t);
}

static inline void
set_table(lua_State *L, int t, int key, float val) 
{
	lua_pushvalue(L, lua_upvalueindex(key));
	lua_pushnumber(L, val);
	lua_settable(L, t);
}

static inline void
set_kv(lua_State *L, int t, int k, int v)
{
	lua_pushvalue(L, lua_upvalueindex(k));
	lua_pushvalue(L, lua_upvalueindex(v));
	lua_settable(L, t);
}

static inline int
phase_to_id(engine::input::touch_phase phase) 
{
	switch (phase) {
        case engine::input::touch_phase::BEGAN:
		return TOUCH_PHASE_BEGAN;
        case engine::input::touch_phase::MOVED:
		return TOUCH_PHASE_MOVED;
        case engine::input::touch_phase::STATIONARY:
		return TOUCH_PHASE_STATIONARY;
        case engine::input::touch_phase::ENDED:
		return TOUCH_PHASE_ENDED;
        default:
		return TOUCH_PHASE_CANCELED;
	}
}

static int
ltouchinfo(lua_State *L) 
{
	engine::input::touch t;
	int index = luaL_checkinteger(L, 1);
	engine::input::inst().touch_get(index, &t);
	set_table(L, 2, TOUCH_FINTERID, t.finger_id);
	set_kv(L, 2, TOUCH_PHASE, phase_to_id(t.phase));
	set_table(L, 2, TOUCH_POS_X, t.position.x());
	set_table(L, 2, TOUCH_POS_Y, t.position.y());
	set_table(L, 2, TOUCH_DELTATIME, t.delta_time);
	set_table(L, 2, TOUCH_DELTA_X, t.delta_position.x());
	set_table(L, 2, TOUCH_DELTA_Y, t.delta_position.y());
	return 0;
}

extern "C" LUAMOD_API int 
luaopen_engine_input(lua_State *L) {
	luaL_Reg key_funcs [] = {
		{"get_key", lgetkey}, 
		{NULL, NULL},
	};
	luaL_Reg touch_funcs[] = {
		{"touch_count", ltouchcount}, 
		{"touch_info",  ltouchinfo},
		{NULL, NULL},
	};
	struct key_code {
		engine::input::key_code key;
		const char *name;
	} key_codes[] = {
          { engine::input::key_code::A, "A" },
          { engine::input::key_code::S, "S" },
          { engine::input::key_code::D, "D" },
          { engine::input::key_code::W, "W" },
	};
	luaL_checkversion(L);
	int key_func_count = sizeof(key_funcs) / sizeof(key_funcs[0]) - 1;
	int touch_func_count = sizeof(touch_funcs) / sizeof(touch_funcs[0]) - 1;
	int key_code_count = sizeof(key_codes) / sizeof(key_codes[0]);
	lua_createtable(L, 0, key_func_count + touch_func_count + key_code_count);
	lua_pushliteral(L, "none");
	lua_pushliteral(L, "down");
	lua_pushliteral(L, "up");
	luaL_setfuncs(L, key_funcs, KEY_UP_COUNT);
	lua_pushliteral(L, "finger_id");
	lua_pushliteral(L, "phase");
	lua_pushliteral(L, "x");
	lua_pushliteral(L, "y");
	lua_pushliteral(L, "delta_time");
	lua_pushliteral(L, "delta_x");
	lua_pushliteral(L, "delta_y");
	lua_pushliteral(L, "began");
	lua_pushliteral(L, "moved");
	lua_pushliteral(L, "stationary");
	lua_pushliteral(L, "ended");
	lua_pushliteral(L, "canceled");
	luaL_setfuncs(L, touch_funcs, TOUCH_UP_COUNT);
	int n = lua_gettop(L);
	for (int i = 0; i < key_code_count; i++) {
		lua_pushinteger(L, (int)key_codes[i].key);
		lua_setfield(L, -2, key_codes[i].name);
	}
	return 1;
}

