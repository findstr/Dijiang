#include "level.h"
#include "camera.h"

namespace engine {

camera::camera(gameobject *go) : component(go)
{
	camera_list.emplace_back(this);
}

camera::~camera()
{
	if (camera_list.size() > 0)
		std::erase(camera_list, this);
}

void
camera::render()
{
	level::cull(this, draw_list);


}

std::vector<camera *> camera::camera_list;
const std::vector<camera *> &
camera::all_cameras()
{
	return camera_list;
}


}


