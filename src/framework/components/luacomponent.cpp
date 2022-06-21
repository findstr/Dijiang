#include <string>
#include <iostream>
#include "luavm.h"
#include "luacomponent.h"

namespace engine {

luacomponent::luacomponent(gameobject *go, const std::string &t) :
	component(go), type_(t)
{
	object_handle = luavm::new_component(go, type_);
	tick_handle = luavm::get_func(object_handle, "tick");
	pre_handle = luavm::get_func(object_handle, "pre_tick");
	post_handle = luavm::get_func(object_handle, "post_tick");
	has_tick_ = tick_handle > 0;
	has_pre_tick_ = pre_handle > 0;
	has_post_tick_ = post_handle > 0;
	std::cout << "luacomponent:" << object_handle << ":" << tick_handle << ":" << has_tick_ << std::endl;
}

luacomponent::~luacomponent()
{
	luavm::collect(object_handle);
	luavm::collect(tick_handle);
	luavm::collect(pre_handle);
	luavm::collect(post_handle);
}

void
luacomponent::start()
{
	luavm::call_with_self(object_handle, "start");
}

void
luacomponent::tick(float delta)
{
	luavm::call_with_self(object_handle, tick_handle, delta);
}

void
luacomponent::pre_tick(float delta)
{
	luavm::call_with_self(object_handle, pre_handle, delta);
}

void
luacomponent::post_tick(float delta)
{
	luavm::call_with_self(object_handle, post_handle, delta);
}

void
luacomponent::set_attr(const std::string &key, float value)
{
	luavm::set_table(object_handle, key, value);
}

void
luacomponent::set_attr(const std::string &key, const std::string &value)
{
	luavm::set_table(object_handle, key, value);
}


}


