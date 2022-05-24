#include "level.h"
#include "framework/components/meshfilter.h"
#include "framework/components/meshrender.h"
#include "resource/resource.h"

namespace engine {

level::level(const std::string &path)
{
	name = path;
}
level::~level()
{
	for (auto *go :  gobjects)
		delete go;
}

void
level::tick(float delta)
{
	for (auto *go : gobjects)
		go->tick(delta);
}

void
level::add_gameobject(gameobject *go)
{
	gobjects.emplace_back(go);
}

//////---- static level

std::vector<std::unique_ptr<level>> level::levels;

void
level::load(const std::string &path)
{
	level *lv = new level(path);
	levels.emplace_back(lv);
	resource::load_level(path, [&lv](gameobject *go) {
		lv->add_gameobject(go);
	});
}

void
level::cull(camera *cam, std::vector<draw_object> &list)
{
	for (auto &lv:levels) {
		for (auto go:lv->gobjects) {
			auto mf = (meshfilter *)go->get_component("meshfilter");
			auto mr = (meshrender *)go->get_component("meshrender");
			if (mf == nullptr || mr == nullptr)
				continue;
			auto *mesh = mf->get_mesh();
			auto *material = mr->get_material();
			list.emplace_back(go->transform, mesh, material);
		}
	}
}

void
level::tick_all(float delta)
{
	for (auto &lv:levels)
		lv->tick(delta);
}

}

