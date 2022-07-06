#pragma once
#include "render/painter.h"
#include "render/draw_object.h"

namespace engine {

class engine {
public:
	void init();
	bool pre_tick(float delta);
	void tick(float delta);
	void post_tick(float delta);
	void get_resolution(int *width, int *height);
	void set_viewport(int x, int y, int width, int height);
	void cleanup();
	render::painter render;
};

}

