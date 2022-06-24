#include <stdlib.h>
#include <string.h>
#include "math/math.h"
#define LUA_LIB
#include "lua.hpp"

enum class value_type {
	VECTOR2F,
	VECTOR3F,
	QUATERNION,
};

struct math_value {
	value_type type;
	union {
		engine::vector2f vector2f;
		engine::vector3f vector3f;
		engine::quaternion quaternion;
	}u;
};

constexpr auto UP_X = 1;
constexpr auto UP_Y = 2;
constexpr auto UP_Z = 3;
constexpr auto UP_W = 4;
constexpr auto UP_COUNT = 4;

struct math_stack {
private:
	int cap = 0;
	int count =  0;
	math_value buf[64];
 public:
	bool freed = false;
	math_value *data = nullptr;
	bool reset() {
		if (freed)
			return false;
		count = 0;
		cap = sizeof(buf) / sizeof(buf[0]);
		if (data != buf) {
			free(data);
			data = buf;
		}
		return true;
	}
	engine::vector2f &newvec2f(int *id) {
		auto *val = newval(value_type::VECTOR2F);
		*id = val - data;
		return val->u.vector2f;
	}
	engine::vector3f &newvec3f(int *id) {
		auto *val = newval(value_type::VECTOR3F);
		*id = val - data;
		return val->u.vector3f;
	}
	engine::quaternion &newquaternion(int *id) {
		auto *val = newval(value_type::QUATERNION);
		*id = val - data;
		return val->u.quaternion;
	}
	inline engine::vector2f &getvec2f(lua_State *L, int arg) {
		auto &val = getval(L, arg, value_type::VECTOR2F);
		return val.u.vector2f;
	}
	inline engine::vector3f &getvec3f(lua_State *L, int arg) {
		auto &val = getval(L, arg, value_type::VECTOR3F);
		return val.u.vector3f;
	}
	inline engine::quaternion &getquaternion(lua_State *L, int arg) {
		auto &val = getval(L, arg, value_type::QUATERNION);
		return val.u.quaternion;
	}
	inline math_value &getval(lua_State *L, int arg) {
		int id = luaL_checkinteger(L, arg);
		luaL_argcheck(L, id < count, arg, "invalid value");
		return data[id];
	}
	inline math_value &getval(lua_State *L, int arg, value_type t) {
		auto &v = getval(L, arg);
		luaL_argcheck(L, v.type == t, arg, "invalid value");
		return v;
	}
	void reserve(int n = 1) {
		int need = count + n;
		if (need > cap) {
			while (cap < need)
				cap *= 2;
			if (data == buf) {
				data = (math_value *)malloc(sizeof(data[0]) * cap);
				memcpy(data, buf, sizeof(buf));
			} else {
				void *ptr = data;
				data = (math_value *)realloc(ptr, cap * sizeof(data[0]));
			}
		}
	}
private:
	math_value *newval(value_type t) {
		reserve(1);
		auto *v = &data[count++];
		v->type = t;
		return v;
	}
};

static int
lgc(lua_State *L) 
{
	math_stack *stk = (math_stack *)luaL_checkudata(L, 1, "engine.math");
	stk->reset();
	return 0;
}

static int
lclose(lua_State *L) 
{
	math_stack *stk = (math_stack *)luaL_checkudata(L, 1, "engine.math");
	if (stk->reset()) {
		int n = lua_rawget(L, lua_upvalueindex(1));
		lua_pushvalue(L, 1);
		lua_seti(L, lua_upvalueindex(1), n+1);
	}
	return 0;
}

static int
lbegin(lua_State *L)
{
	math_stack *ctx;
	int n = lua_rawlen(L, lua_upvalueindex(1));
	if (n > 0) {
		lua_geti(L, lua_upvalueindex(1), n);
		lua_pushnil(L);
		lua_seti(L, lua_upvalueindex(1), n);
		ctx = (math_stack *)lua_touserdata(L, 1);
		ctx->freed = false;
	} else {
		ctx = (math_stack *)lua_newuserdatauv(L, sizeof(math_stack), 0);
		ctx->data = nullptr;
		ctx->freed = false;
		ctx->reset();
		luaL_getmetatable(L, "engine.math");
		lua_setmetatable(L, -2);
	}
	return 1;
}


template<typename T> void
set_value_field(lua_State *L, int table, const T &v, int n) 
{
	for (int i = 0; i < n; i++) {
		lua_pushvalue(L, lua_upvalueindex(i+1));
		lua_pushnumber(L, v(i));
		lua_settable(L, table);
	}
}

static int
lsave(lua_State *L) 
{
	int n = 0;
	math_stack *stk = (math_stack *)luaL_checkudata(L, 1, "engine.math");
	math_value &v = stk->getval(L, 2);
	if (lua_gettop(L) == 3) {
		switch (v.type) {
		case value_type::VECTOR2F:
			for (int i = 0; i < 2; i++) {
				lua_pushvalue(L, lua_upvalueindex(i+1));
				lua_pushnumber(L, v.u.vector2f(i));
				lua_settable(L, 3);
			}
			break;
		case value_type::VECTOR3F:
			for (int i = 0; i < 3; i++) {
				lua_pushvalue(L, lua_upvalueindex(i+1));
				lua_pushnumber(L, v.u.vector3f(i));
				lua_settable(L, 3);
			}
			break;
		case value_type::QUATERNION: {
			auto q = v.u.quaternion.coeffs();
			for (int i = 0; i < 4; i++) {
				lua_pushvalue(L, lua_upvalueindex(i+1));
				lua_pushnumber(L, q(i));
				lua_settable(L, 3);
			}
			break;}
		}
		return 0;
	} else {
		switch (v.type) {
		case value_type::VECTOR2F:
			for (int i = 0; i < 2; i++)
				lua_pushnumber(L, v.u.vector2f(i));
			return 2;
		case value_type::VECTOR3F:
			for (int i = 0; i < 3; i++)
				lua_pushnumber(L, v.u.vector3f(i));
			return 3;
		case value_type::QUATERNION: {
			auto q = v.u.quaternion.coeffs();
			for (int i = 0; i < 4; i++)
				lua_pushnumber(L, q(i));
			return 4;}
		}
		return 0;
	}
}

static int
ldot(lua_State *L) 
{
	math_stack *stk = (math_stack *)luaL_checkudata(L, 1, "engine.math");
	auto &v1 = stk->getval(L, 2);
	auto &v2 = stk->getval(L, 3);
	float result = 0.0f;
	luaL_argcheck(L, v1.type == v2.type, 3, "different type");
	switch (v1.type) { 
	case value_type::VECTOR2F:
		result = v1.u.vector2f.dot(v2.u.vector2f);
		break;
	case value_type::VECTOR3F:
		result = v1.u.vector3f.dot(v2.u.vector3f);
		break;
	case value_type::QUATERNION:
		return luaL_error(L, "quaternion has no dot function");
		break;
	}
	lua_pushnumber(L, result);
	return 1;
}

static int
lmul(lua_State *L) 
{
	int v3_id = INT_MAX;
	math_stack *stk = (math_stack *)luaL_checkudata(L, 1, "engine.math");
	stk->reserve();
	auto &v1 = stk->getval(L, 2);
	auto &v2 = stk->getval(L, 3);
	if (v1.type == value_type::QUATERNION) {
		auto &v3 = stk->newvec3f(&v3_id);
		luaL_argcheck(L, v2.type == value_type::VECTOR3F, 2, "need vector3f");
		v3 = v1.u.quaternion * v2.u.vector3f;
	} else {
		luaL_argcheck(L, v2.type == v1.type, 2, "vector multiple need same type");
		if (v1.type == value_type::VECTOR2F) {
			auto &v3 = stk->newvec2f(&v3_id);
			v3 = v1.u.vector2f * v2.u.vector2f;
		} else {
			auto &v3 = stk->newvec3f(&v3_id);
			v3 = v1.u.vector3f * v2.u.vector3f;
		}
	}
	lua_pushinteger(L, v3_id);
	return 1;
}

static float
get_value_field(lua_State *L, int table,  int field)
{
	float v;
	lua_pushvalue(L, lua_upvalueindex(field));
	lua_gettable(L, table);
	v = luaL_optnumber(L, -1, 0.0f);
	lua_pop(L, 1);
	return v;
}

static int
lvector2f(lua_State *L) 
{
	int id;
	math_stack *stk = (math_stack *)luaL_checkudata(L, 1, "engine.math");
	auto &val = stk->newvec2f(&id);
	if (lua_type(L, 2) == LUA_TTABLE) {
		val.x() = get_value_field(L, 2, UP_X);
		val.y() = get_value_field(L, 2, UP_Y);
	} else {
		val.x() = luaL_checknumber(L, 2);
		val.y() = luaL_checknumber(L, 3);
	}
	lua_pushinteger(L, id);
	return 1;
}

static int
lvector3f(lua_State *L) 
{
	int id;
	math_stack *stk = (math_stack *)luaL_checkudata(L, 1, "engine.math");
	auto &val = stk->newvec3f(&id);
	if (lua_type(L, 2) == LUA_TTABLE) {
		val.x() = get_value_field(L, 2, UP_X);
		val.y() = get_value_field(L, 2, UP_Y);
		val.z() = get_value_field(L, 2, UP_Z);
	} else {
		val.x() = luaL_checknumber(L, 2);
		val.y() = luaL_checknumber(L, 3);
		val.z() = luaL_checknumber(L, 4);
	}
	lua_pushinteger(L, id);
	return 1;
}


static inline int
lvector3f_push(lua_State *L, const engine::vector3f &v) 
{
	int id;
	math_stack *stk = (math_stack *)luaL_checkudata(L, 1, "engine.math");
	stk->newvec3f(&id) = v;
	lua_pushinteger(L, id);
	return 1;
}

static int
lvector3f_up(lua_State *L) 
{
	return lvector3f_push(L, engine::vector3f::up());
}

static int
lvector3f_down(lua_State *L) 
{
	return lvector3f_push(L, engine::vector3f::down());
}

static int
lvector3f_left(lua_State *L)
{
	return lvector3f_push(L, engine::vector3f::left());
}

static int
lvector3f_right(lua_State *L) 
{
	return lvector3f_push(L, engine::vector3f::right());
}

static int
lvector3f_forward(lua_State *L) 
{
	return lvector3f_push(L, engine::vector3f::forward());
}

static int
lvector3f_back(lua_State *L) 
{
	return lvector3f_push(L, engine::vector3f::back());
}

static int
lvector3f_cross(lua_State *L) 
{
	int id;
	math_stack *stk = (math_stack *)luaL_checkudata(L, 1, "engine.math");
	auto &val = stk->newvec3f(&id);
	auto &v1 = stk->getvec3f(L, 2);
	auto &v2 = stk->getvec3f(L, 3);
	val = v1.cross(v2);
	lua_pushinteger(L, id);
	return 1;
}

static int
lquaternion(lua_State *L) 
{
	int id;
	math_stack *stk = (math_stack *)luaL_checkudata(L, 1, "engine.math");
	auto &val = stk->newquaternion(&id);
	if (lua_type(L, 2) == LUA_TTABLE) {
		val.x() = get_value_field(L, 2, UP_X);
		val.y() = get_value_field(L, 2, UP_Y);
		val.z() = get_value_field(L, 2, UP_Z);
		val.w() = get_value_field(L, 2, UP_Z);
	} else {
		val.x() = luaL_checknumber(L, 2);
		val.y() = luaL_checknumber(L, 3);
		val.z() = luaL_checknumber(L, 4);
		val.w() = luaL_checknumber(L, 5);
	}
	lua_pushinteger(L, id);
	return 1;
}

static int
lquaternion_inverse(lua_State *L) 
{
	int id;
	math_stack *stk = (math_stack *)luaL_checkudata(L, 1, "engine.math");
	auto &v = stk->newquaternion(&id);
	auto &q = stk->getquaternion(L, 2);
	v = q.inverse();
	lua_pushinteger(L, id);
	return 1;
}

static int
lquaternion_to_euler(lua_State *L) 
{
	math_stack *stk = (math_stack *)luaL_checkudata(L, 1, "engine.math");
	auto &q = stk->getquaternion(L, 2);
	auto euler = q.to_euler();
	lua_pushnumber(L, euler.x());
	lua_pushnumber(L, euler.y());
	lua_pushnumber(L, euler.z());
	return 3;
}

static int
lquaternion_look_at(lua_State *L) 
{
	int id;
	math_stack *stk = (math_stack *)luaL_checkudata(L, 1, "engine.math");
	auto &q = stk->newquaternion(&id);
	auto &forward = stk->getvec3f(L, 2);
	auto &up = stk->getvec3f(L, 3);
	q = engine::quaternion::look_at(forward, up);
	lua_pushinteger(L, id);
	return 1;
}
	
static int
lquaternion_from_axis_angle(lua_State *L) 
{
	int id;
	math_stack *stk = (math_stack *)luaL_checkudata(L, 1, "engine.math");
	auto &q = stk->newquaternion(&id);
	auto &axis = stk->getvec3f(L, 2);
	float angle = luaL_checknumber(L, 3);
	q.from_axis_angle(axis, angle);
	lua_pushinteger(L, id);
	return 1;
}


extern "C" LUAMOD_API int 
luaopen_engine_math(lua_State *L) {
	luaL_Reg funcs[] = {
		{"begin", nullptr}, 
		{"save", lsave},
		{"dot", ldot},
		{"mul", lmul},
		////////////vector2f
		{"vector2f", lvector2f},
		////////////vector3f
		{"vector3f", lvector3f},
		{"vector3f_up", lvector3f_up},
		{"vector3f_down", lvector3f_down},
		{"vector3f_left", lvector3f_left},
		{"vector3f_right", lvector3f_right},
		{"vector3f_forward", lvector3f_forward},
		{"vectro3f_back", lvector3f_back},
		{"vector3f_cross", lvector3f_cross},
		////////////quaternion
		{"quaternion", lquaternion},
		{"quaternion_inverse", lquaternion_inverse},
		{"quaternion_to_euler", lquaternion_to_euler},
		{"quaternion_look_at", lquaternion_look_at},
		{"quaternion_from_axis_angle", lquaternion_from_axis_angle},
		{NULL, NULL},
	};
	luaL_Reg func_upval[] = {
		{"begin", lbegin}, 
		{NULL, NULL},
	};
	luaL_checkversion(L);
	lua_newtable(L);
	if (luaL_newmetatable(L, "engine.weaktable")) {
		lua_pushliteral(L, "kv");
		lua_setfield(L, -2, "__index");
	}
	lua_setmetatable(L, -2);
	luaL_newlibtable(L, funcs);
	lua_pushliteral(L, "x");
	lua_pushliteral(L, "y");
	lua_pushliteral(L, "z");
	lua_pushliteral(L, "w");
	luaL_setfuncs(L, funcs, UP_COUNT);
	lua_pushvalue(L, -2);
	luaL_setfuncs(L, func_upval, 1);
	if (luaL_newmetatable(L, "engine.math")) {
		lua_pushvalue(L, -2);
		lua_setfield(L, -2, "__index");
		lua_pushvalue(L, -3);
		lua_pushcclosure(L, lclose, 1);
		lua_setfield(L, -2, "__close");
		lua_pushvalue(L, -3);
		lua_pushcclosure(L, lgc, 1);
		lua_setfield(L, -2, "__gc");

	}
	lua_pop(L, 1);
	lua_replace(L, -2);
	return 1;
}

