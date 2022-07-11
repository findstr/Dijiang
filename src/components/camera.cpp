#include "level.h"
#include "camera.h"
#include "system/render_system.h"

namespace engine {

camera::camera(gameobject *go) : component(go)
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
	level::cull(this, draw_list);
	RENDER_SYSTEM.set_camera(this);
	for (auto &d:draw_list) { 
		RENDER_SYSTEM.draw(d);
	}
	draw_list.clear();
}

std::vector<camera *> camera::camera_list;
const std::vector<camera *> &
camera::all_cameras()
{
	return camera_list;
}


}


