#pragma once
#include <string>
#include <memory>
#include <vector>
#include "component.h"
#include "math/math.h"
#include "math/transform.h"
#include "animation/skeleton.h"

namespace engine {
	
class animator : public component {
private:
	std::shared_ptr<animation::skeleton> skeleton;
	std::vector<animation::skeleton::pose> debug_pose;
	std::vector<matrix4f> current_pose;
public:
	animator(gameobject *go) : component(go) {}
	void set_skeleton(const std::shared_ptr<animation::skeleton> &ske);
	const animation::skeleton *get_skeleton();
	std::vector<matrix4f> &get_current_pose() { return current_pose; }

public:
	const std::string type() override { return "animator"; }
	void tick(float delta) override;
	double progress = 0.0f;
	double total = 0.76666998863220215f;
};

}
