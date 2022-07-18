#pragma once
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include "gameobject.h"
#include "components/camera.h"
#include "render/draw_object.h"

namespace engine {

class level {
private:
	std::string name_;
	std::unordered_map<gameobject::id_t, gameobject *>gobjects;
	std::vector<std::tuple<gameobject *, int>> adding;
public:
	level(const std::string &path);
	~level();
	void tick(float delta);
	const std::string &name() const { return name_; }
	void add_gameobject(gameobject *go, int parent);
	const std::unordered_map<gameobject::id_t, gameobject *> &all_object() const { return gobjects; };
public:
	static void load(const std::string &path);
	static void tick_all(float delta);
	static void cull(camera *cam, std::vector<draw_object> &list, render_pass::path path);
	static const std::vector<std::unique_ptr<level>> &all_levels() { return levels; }
private:
	static std::vector<std::unique_ptr<level>> levels;
};


}

