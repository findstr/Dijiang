#pragma once
#include <vector>
#include "math/math.h"
#include "math/rect.h"
#include "gameobject.h"
#include "component.h"

namespace engine {

class camera: public component {
public:
	camera(gameobject *go);
	~camera();
	const std::string type() override { return "camera"; }
	vector3f up() { return transform->rotation * vector3f(0, 1, 0); }
	vector3f left() { return transform->rotation * vector3f(-1, 0, 0); }
	vector3f right() { return transform->rotation * vector3f(1, 0, 0); }
	vector3f forward() { return transform->rotation * vector3f(0, 0, 1); }
	vector3f back() { return transform->rotation * vector3f(0, 0, -1); }
public:
	float fov = 30;
	float aspect = 16.0f/9.9f;
	float clip_near_plane = 1.f;
	float clip_far_plane = 100.f;
	rect viewport;
/////////static
public:
	static const std::vector<camera *> &all_cameras();
private:
	static std::vector<camera *> camera_list;
};



}


