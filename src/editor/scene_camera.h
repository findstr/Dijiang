#pragma once
#include <vector>
#include "components/camera.h"
#include "components/light.h"
#include "gameobject.h"

namespace engine {

class scene_camera: public camera {
public:
	scene_camera(gameobject *go);
public:
	const std::string name() override { return "scene_camera"; }
	void render();
public:
	bool show_skeleton = true;
	bool show_camera = true;
private:
	void draw_camera(camera *cam);
	void draw_light(camera *cam, light *li);
};



}


