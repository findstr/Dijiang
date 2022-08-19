#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "math/math.h"
#include "gameobject.h"
#define LUA_LIB
#include "lua.hpp"

#define UPVAL_GO	(1)

using namespace engine;

gameobject *
to_go(lua_State *L, int idx)
{
	gameobject *go;
	lua_pushvalue(L, lua_upvalueindex(UPVAL_GO));
	lua_gettable(L, 1);
	go = (gameobject *)lua_touserdata(L, -1);
	lua_pop(L, 1);
	return go;
}

static int
lgetsibling(lua_State *L)
{
	size_t sz;
	const char *str = luaL_checklstring(L, 1, &sz);
	printf("get_sibling:%s\n", str);
	return 0;
};

static int
lsetlocalpos(lua_State *L)
{
	float x = luaL_checknumber(L, 2);
	float y = luaL_checknumber(L, 3);
	float z = luaL_checknumber(L, 4);
	to_go(L, 1)->transform.set_local_position(vector3f(x, y, z));
	return 0;
}

static int
lgetlocalpos(lua_State *L)
{
	vector3f pos = to_go(L, 1)->transform.local_position();
	lua_pushnumber(L, pos.x());
	lua_pushnumber(L, pos.y());
	lua_pushnumber(L, pos.z());
	return 3;
}

static int
lsetlocalrot(lua_State *L)
{
	float x = luaL_checknumber(L, 2);
	float y = luaL_checknumber(L, 3);
	float z = luaL_checknumber(L, 4);
	quaternion rot;
	rot.from_euler(x, y, z);
	to_go(L, 1)->transform.set_local_rotation(rot);
	return 0;
}

static int
lgetlocalrot(lua_State *L)
{
	vector3f angles = to_go(L, 1)->transform.local_rotation().to_euler();
	lua_pushnumber(L, angles.x());
	lua_pushnumber(L, angles.y());
	lua_pushnumber(L, angles.z());
	return 3;
}

static int
lsetlocalquaternion(lua_State *L) 
{
	float x = luaL_checknumber(L, 2);
	float y = luaL_checknumber(L, 3);
	float z = luaL_checknumber(L, 4);
	float w = luaL_checknumber(L, 5);
	quaternion rot(x,y,z,w);
	to_go(L, 1)->transform.set_local_rotation(rot);
	return 0;
}

static int
lgetlocalquaternion(lua_State *L) 
{
	auto &quat = to_go(L, 1)->transform.local_rotation();
	lua_pushnumber(L, quat.x());
	lua_pushnumber(L, quat.y());
	lua_pushnumber(L, quat.z());
	lua_pushnumber(L, quat.w());
	return 4;
}

static int
lsetquaternion(lua_State *L) 
{
	auto &trans = to_go(L, 1)->transform;
	quaternion q;
	q.x() = luaL_checknumber(L, 2);
	q.y() = luaL_checknumber(L, 3);
	q.z() = luaL_checknumber(L, 4);
	q.w() = luaL_checknumber(L, 5);
	trans.set_rotation(q);
	return 0;
}

static int
lgetquaternion(lua_State *L) 
{
	auto &quat = to_go(L, 1)->transform.rotation();
	lua_pushnumber(L, quat.x());
	lua_pushnumber(L, quat.y());
	lua_pushnumber(L, quat.z());
	lua_pushnumber(L, quat.w());
	return 4;
}


static int
lrotate(lua_State *L)
{
	float x = luaL_checknumber(L, 2);
	float y = luaL_checknumber(L, 3);
	float z = luaL_checknumber(L, 4);
	quaternion q;
	q.from_euler(x, y, z);
	auto &trans = to_go(L, 1)->transform;
	trans.set_rotation(q * trans.rotation());
	return 0;
}

static int
lmove(lua_State *L)
{
	float x = luaL_checknumber(L, 2);
	float y = luaL_checknumber(L, 3);
	float z = luaL_checknumber(L, 4);
	auto &trans = to_go(L, 1)->transform;
	trans.set_position(trans.position() + vector3f(x, y, z));
	return 0;
}

static int
lsetlocalscale(lua_State *L)
{
	float x = luaL_checknumber(L, 2);
	float y = luaL_checknumber(L, 3);
	float z = luaL_checknumber(L, 4);
	to_go(L, 1)->transform.set_scale(vector3f(x, y, z));
	return 0;
}

static int
lgetlocalscale(lua_State *L)
{
	vector3f scale = to_go(L, 1)->transform.scale();
	lua_pushnumber(L, scale.x());
	lua_pushnumber(L, scale.y());
	lua_pushnumber(L, scale.z());
	return 3;
}

extern "C" LUAMOD_API int
luaopen_engine_component(lua_State *L)
{
	luaL_Reg tbl[] = {
		{"get_sibling", lgetsibling},
		{"set_local_position", lsetlocalpos},
		{"get_local_position", lgetlocalpos},
		{"set_local_rotation", lsetlocalrot},
		{"get_local_rotation", lgetlocalrot},
		{"set_local_quaternion", lsetlocalquaternion},
		{"get_local_quaternion", lgetlocalquaternion},
		{"set_local_scale", lsetlocalscale},
		{"get_local_scale", lgetlocalscale},
		{"set_quaternion", lsetquaternion},
		{"get_quaternion", lgetquaternion},

		{"rotate", lrotate},
		{"move", lmove},
		{NULL, NULL},
	};
	luaL_checkversion(L);
	luaL_newlibtable(L, tbl);
	lua_pushliteral(L, "__go");
	luaL_setfuncs(L, tbl, 1);

	return 1;
}

