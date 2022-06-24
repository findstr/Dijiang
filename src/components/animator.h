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
public:
	animator(gameobject *go) : component(go) {}
	void set_skeleton(const std::shared_ptr<animation::skeleton> &ske);
	const animation::skeleton *get_skeleton();

public:
	const std::string type() override { return "animator"; }
	void tick(float delta) override;
};

}
