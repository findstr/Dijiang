#pragma once
#include <string>
#include <vector>
#include "engine.h"
#include "render/draw_object.h"

namespace engine {
namespace editor {

class gameview {
public:
	gameview();
	void tick(engine *e, float delta);
public:
	const std::string title;
	bool is_open = true;
private:
	std::vector<draw_object> drawlist;
};

}}

