#include <chrono>
#include "luavm.h"
#include "resource/resource.h"
#include "render/painter.h"
#include "render/input.h"
#include "framework/level.h"
#include "framework/components/camera.h"
#include "engine.h"

namespace engine {

struct {
	render::painter render;
	std::vector<draw_object> drawlist;
        std::chrono::time_point<std::chrono::high_resolution_clock> last_tick;
} E;

void
init()
{
	luavm::init();
	E.render.init();
	resource::init();
}

void
run()
{
	level::load("asset/main.level");
	auto cameras = camera::all_cameras();
	bool running = true;
	E.last_tick = std::chrono::high_resolution_clock::now();
	while (running) {
		auto now = std::chrono::high_resolution_clock::now();
		float delta = std::chrono::duration<float, std::chrono::seconds::period>(
			now - E.last_tick).count();
		E.last_tick = now;
		input::update(delta);
		level::tick_all(delta);
		for (auto cam:cameras) {
			E.drawlist.clear();
			level::cull(cam, E.drawlist);
			running = E.render.draw(cam, E.drawlist) && running;
		}
	}
}

void
cleanup()
{
	E.render.cleanup();
}

}

