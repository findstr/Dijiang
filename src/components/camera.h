#pragma once
#include <vector>
#include "math/math.h"
#include "math/rect.h"
#include "draw_object.h"
#include "gameobject.h"
#include "component.h"
#include "render/render_texture.h"

namespace engine {

class camera: public component {
public:
	camera(gameobject *go);
	~camera();
public:
	const std::string name() override { return "camera"; }
	vector3f up() { return transform->rotation() * vector3f(0, 1, 0); }
	vector3f left() { return transform->rotation() * vector3f(-1, 0, 0); }
	vector3f right() { return transform->rotation() * vector3f(1, 0, 0); }
	vector3f forward() { return transform->rotation() * vector3f(0, 0, 1); }
	vector3f back() { return transform->rotation() * vector3f(0, 0, -1); }
public:
	void reg();
	void unreg();
	void render();
public:
	float fov = 30;
	float aspect = 16.0f/9.9f;
	float clip_near_plane = 1.f;
	float clip_far_plane = 100.f;
	bool perspective = true;
	float orthographic_size = 5.0f;
	rect viewport = rect(0.f, 0.f, 1.f, 1.f);
	render_texture *render_target = nullptr;
protected:
	bool regged = false;
	std::vector<draw_object> draw_list;
/////////static
public:
	static const std::vector<camera *> &all_cameras();
private:
	static std::vector<camera *> camera_list;
};



}


