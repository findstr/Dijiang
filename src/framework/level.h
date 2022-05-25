#pragma once
#include <vector>
#include <memory>
#include <string>
#include "gameobject.h"
#include "components/camera.h"
#include "render/draw_object.h"

namespace engine {

class level {
private:
	std::string name;
	std::vector<gameobject *> gobjects;
	std::vector<gameobject *> adding;
public:
	level(const std::string &path);
	~level();
	void tick(float delta);
	void add_gameobject(gameobject *go);
public:
	static void load(const std::string &path);
	static void tick_all(float delta);
	static void cull(camera *cam, std::vector<draw_object> &list);
private:
	static std::vector<std::unique_ptr<level>> levels;
};


}

