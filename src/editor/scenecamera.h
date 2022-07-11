#pragma once
#include <vector>
#include "components/camera.h"

namespace engine {

class scene_camera: public camera {
public:
	scene_camera(gameobject *go);
	~scene_camera();
public:
	void render();
};

}


