#include <assert.h>
#include <filesystem>
#include "level.h"
#include "components/meshfilter.h"
#include "components/meshrender.h"
#include "components/skinrender.h"
#include "components/animator.h"
#include "resource/resource.h"

namespace engine {

level::level(const std::string &name)
{
	name_ = name;
}
level::~level()
{
	for (auto &iter :  gobjects)
		delete iter.second;
}

void
level::tick(float delta)
{
	gobjects.reserve(gobjects.size() + adding.size());
	for (auto &iter:adding) {
		int parent = std::get<int>(iter);
		auto *go = std::get<gameobject *>(iter);
		if (parent != 0) {
			auto p = gobjects[parent];
			go->set_parent(p);
		}
		go->start();
		auto ret = gobjects.insert({go->id(), go});
		assert(ret.second == true);
	}
	adding.clear();
	for (auto &iter:gobjects)
		iter.second->pre_tick(delta);
	for (auto &iter:gobjects) {
		auto *go = iter.second;
		auto *p = go->get_parent();
		if (p == nullptr)
			continue;
		auto &trans = go->transform;
		auto &ptrans = p->transform;
		trans.position = ptrans.position + trans.local_position;
		trans.rotation = ptrans.rotation * trans.local_rotation;
		trans.scale = ptrans.scale * trans.local_scale;
	}
	for (auto &iter:gobjects)
		iter.second->tick(delta);
	for (auto &iter:gobjects)
		iter.second->post_tick(delta);
}

void
level::add_gameobject(gameobject *go, int parent)
{
	adding.emplace_back(go, parent);
}

//////---- static level

std::vector<std::unique_ptr<level>> level::levels;

void
level::load(const std::string &path)
{
	std::string name = std::filesystem::path(path).filename().string();
	level *lv = new level(name);
	levels.emplace_back(lv);
	resource::load_level(path, [&lv](gameobject *go, int parent) {
		lv->add_gameobject(go, parent);
	});
}

void
level::cull_shadowcaster(camera *cam, std::vector<draw_object> &list)
{
	for (auto &lv:levels) {
		for (auto &iter:lv->gobjects) {
			auto *go = iter.second;
			auto mf = (meshfilter *)go->get_component("meshfilter");
			auto mr = (meshrender *)go->get_component("meshrender");
			auto sr = (skinrender *)go->get_component("skillrender");
			if ((mf == nullptr || mr == nullptr) && sr == nullptr)
				continue;
			render::mesh *mesh;
			render::material *material;
			if (sr != nullptr) {
				mesh = sr->get_mesh();
				material = sr->get_shadowcaster();
				if (material == nullptr)
					continue;
				auto *ani = (animator *)go->get_component("animator");
				if (ani) {
					auto &pose = ani->get_current_pose();
					list.emplace_back(go->transform, mesh, material, pose);
				} else {
					list.emplace_back(go->transform, mesh, material);
				}
			} else {
				mesh = mf->get_mesh();
				material = mr->get_shadowcaster();
				if (material != nullptr)
				list.emplace_back(go->transform, mesh, material);
			}
		#if IS_EDITOR
			list.back().go = go;
		#endif
		}
	}

}

void
level::cull(camera *cam, std::vector<draw_object> &list)
{
	for (auto &lv:levels) {
		for (auto &iter:lv->gobjects) {
			auto *go = iter.second;
			auto mf = (meshfilter *)go->get_component("meshfilter");
			auto mr = (meshrender *)go->get_component("meshrender");
			auto sr = (skinrender *)go->get_component("skillrender");
			if ((mf == nullptr || mr == nullptr) && sr == nullptr)
				continue;
			render::mesh *mesh;
			render::material *material;
			if (sr != nullptr) {
				mesh = sr->get_mesh();
				material = sr->get_material();
				auto *ani = (animator *)go->get_component("animator");
				if (ani) {
					auto &pose = ani->get_current_pose();
					list.emplace_back(go->transform, mesh, material, pose);
				} else {
					list.emplace_back(go->transform, mesh, material);
				}
			} else {
				mesh = mf->get_mesh();
				material = mr->get_material();
				list.emplace_back(go->transform, mesh, material);
			}
		#if IS_EDITOR
			list.back().go = go;
		#endif
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

