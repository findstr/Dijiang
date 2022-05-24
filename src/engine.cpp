#include "engine.h"
#include "resource/resource.h"
#include "render/painter.h"
#include "framework/level.h"
#include "framework/components/camera.h"

namespace engine {

struct {
	render::painter render;
	std::vector<draw_object> drawlist;
} E;

void
init()
{
	E.render.init();
	resource::init();
}

void
run()
{
	level::load("asset/main.level");
	/*
	gameobject *go = new gameobject();
	camera *cam = new camera(go);
	go->add_component(cam);
	cam->transform->position.x() = 0;
	cam->transform->position.y() = 0;
	cam->transform->position.z() = -10.0f;
	cam->transform->rotation.identity();
*/
	auto cameras = camera::all_cameras();
	bool running = true;
	while (running) {
		level::tick_all(0.1f);
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

