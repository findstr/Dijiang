#pragma once
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include "engine.h"
#include "level.h"
#include "math/math.h"
#include "inspector.h"
#include "gameobject.h"
#include "scene_camera.h"

namespace engine {
namespace editor {

class hierarchy {
public:
	class go_inspector : public ::engine::editor::inspector {
	public:
		go_inspector();
		void draw() override;
		void attach(gameobject *obj);
	private:
		gameobject *go = nullptr;
		std::string newname;
	};
public:
	hierarchy();
	void pre_tick(engine *e, float delta);
	inspector *tick(engine *e, float delta);
public:
	const std::string title;
	bool is_open = true;
private:
	void draw_recursive(gameobject *go);
	void draw_level(level *l);
private:
	bool is_selected = false;
	go_inspector inspector;
	gameobject::id_t selected = 0;
	std::vector<gameobject *> root;
	std::unordered_map<gameobject *, std::vector<gameobject *>> tree;
};

}}

