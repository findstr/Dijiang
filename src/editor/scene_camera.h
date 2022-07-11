#pragma once
#include <vector>
#include "components/camera.h"
#include "gameobject.h"

namespace engine {

class scene_camera: public camera {
public:
	scene_camera(gameobject *go);
public:
	const std::string type() override { return "scene_camera"; }
	void render();
	void show_skeleton(bool enable) { show_skeleton_ = enable; }
private:
	bool show_skeleton_ = false;
};



}


