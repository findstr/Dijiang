#include "luavm.h"
#include "resource/resource.h"
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
	resource::init();
	render::debugger::inst().init();
	level::load("asset/main.level");
}

bool
engine::pre_tick(float delta)
{
	(void)delta;
	return true;
}

void
engine::tick(float delta)
{
	level::tick_all(delta);
}

void
engine::post_tick(float delta)
{
	(void)delta;	
}


void
engine::cleanup()
{
}


}

