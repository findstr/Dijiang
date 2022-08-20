#include <chrono>
#include "level.h"
#include "camera.h"
#include "glm/glm.hpp"
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
	/*
	RENDER_SYSTEM.shadowpass_begin();
	RENDER_SYSTEM.set_light(light::all_lights()[0], this);
	RENDER_SYSTEM.set_light_camera(light::all_lights()[0], this);
	level::cull(this, draw_list, render::shader::light_mode::SHADOWCASTER);
	RENDER_SYSTEM.init_for_object(draw_list);
	RENDER_SYSTEM.draw(draw_list);
	RENDER_SYSTEM.shadowpass_end();
	*/
	RENDER_SYSTEM.renderpass_begin(render_target);
	draw_list.clear();
	RENDER_SYSTEM.set_camera(this);
	level::cull(this, draw_list, render::shader::light_mode::FORWARD);
	RENDER_SYSTEM.init_for_object(draw_list);
	RENDER_SYSTEM.draw(draw_list);
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


