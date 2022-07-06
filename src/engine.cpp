#include "luavm.h"
#include "resource/resource.h"
#include "render/painter.h"
#include "render/input.h"
#include "render/debugger.h"
#include "level.h"
#include "components/camera.h"
#include "engine.h"

namespace engine {

void
engine::init()
{
	luavm::init();
	render.init();
	resource::init();
	render::debugger::inst().init();
	level::load("asset/main.level");
}

bool
engine::pre_tick(float delta)
{
	(void)delta;
	bool ret = render.framebegin();
	render::debugger::inst().begin();
	return ret;
}

void
engine::tick(float delta)
{
	input::update(delta);
	level::tick_all(delta);
}

void
engine::post_tick(float delta)
{
	(void)delta;	
	render.frameend();
}


void
engine::cleanup()
{
	render.cleanup();
}

	
void
engine::get_resolution(int *width, int *height)
{
	render.get_resolution(width, height);
}

void
engine::set_viewport(int x, int y, int width, int height)
{
	render.set_viewport(x, y, width, height);
}


}

