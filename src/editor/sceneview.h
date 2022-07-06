#pragma once
#include <string>
#include "engine.h"
#include "render/draw_object.h"

namespace engine {
namespace editor {

class sceneview {
public:
	sceneview();
	void pre_tick(engine *e, float delta);
	void tick(engine *e, float delta);
public:
	const std::string title;
	bool is_open = true;
private:
	std::vector<draw_object> drawlist;
};

}}

