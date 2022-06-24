#include "camera.h"

namespace engine {

camera::camera(gameobject *go) : component(go)
{
	camera_list.emplace_back(this);
}

camera::~camera()
{
	std::erase(camera_list, this);
}

std::vector<camera *> camera::camera_list;
const std::vector<camera *> &
camera::all_cameras()
{
	return camera_list;
}


}


