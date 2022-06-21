#pragma once
#include <vector>
#include "math/math.h"
#include "math/rect.h"
#include "component.h"

namespace engine {

class luacomponent: public component {
private:
	std::string type_;
	int object_handle = -1;
	int tick_handle = -1;
	int pre_handle = -1;
	int post_handle = -1;
public:
	luacomponent (gameobject *go, const std::string &type_);
	~luacomponent();
	void set_attr(const std::string &key, float value);
	void set_attr(const std::string &key, const std::string &value);
	const std::string type() override { return type_; }
	void start() override;
	void tick(float delta) override;
	void pre_tick(float delta) override;
	void post_tick(float delta) override;
};



}


