#include "level.h"
#include "camera.h"
#include "system/render_system.h"

namespace engine {

camera::camera(gameobject *go) : 
	component(go)
{
}

camera::~camera()
{
	if (regged)
		unreg();
}

void 
camera::reg()
{
	camera_list.emplace_back(this);
	regged = true;
}

void
camera::unreg()
{
	if (camera_list.size() > 0)
		std::erase(camera_list, this);
}

void
camera::render()
{
	RENDER_SYSTEM.shadowpass_begin();
	auto *li = light::all_lights()[0];
	camera light_cam(li->go);
	light_cam.perspective = false;
	light_cam.orthographic_size = 30.f;
	light_cam.viewport.x = 0;
	light_cam.viewport.y = 0;
	light_cam.viewport.width = 1.0;
	light_cam.viewport.height = 1.0;
	RENDER_SYSTEM.set_light(light::all_lights()[0]);
	RENDER_SYSTEM.set_camera(&light_cam);
	level::cull(this, draw_list, render_pass::SHADOW);
	for (auto &d:draw_list) { 
		RENDER_SYSTEM.draw(d);
	}
	RENDER_SYSTEM.shadowpass_end();
	RENDER_SYSTEM.renderpass_begin();
	draw_list.clear();
	RENDER_SYSTEM.set_camera(this);
	level::cull(this, draw_list, render_pass::FORWARD);
	for (auto &d:draw_list) { 
		RENDER_SYSTEM.draw(d);
	}
	RENDER_SYSTEM.renderpass_end();
	draw_list.clear();
}

std::vector<camera *> camera::camera_list;
const std::vector<camera *> &
camera::all_cameras()
{
	return camera_list;
}



}


