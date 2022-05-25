#include <iostream>
#include "luavm.h"
#include "luacomponent.h"

namespace engine {

luacomponent::luacomponent(gameobject *go, const std::string &t) :
	component(go), type_(t)
{
	object_handle = luavm::new_component(go, type_);
	update_handle = luavm::get_func(object_handle, "update");
	has_tick_ = update_handle > 0;
	std::cout << "luacomponent:" << object_handle << ":" << update_handle << ":" << has_tick_ << std::endl;
}

luacomponent::~luacomponent()
{
	luavm::collect(object_handle);
	luavm::collect(update_handle);
}

void
luacomponent::start()
{
	luavm::call_with_self(object_handle, "start");
}

void
luacomponent::tick(float delta)
{
	luavm::call_with_self(object_handle, update_handle, delta);
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


