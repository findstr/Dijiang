#include "level.h"
#include "system/render_system.h"
#include "animation/skeleton.h"
#include "components/animator.h"
#include "render/debugger.h"
#include "scene_camera.h"

namespace engine {

scene_camera::scene_camera(gameobject *go) : camera(go)
{

}

void
scene_camera::render()
{
	level::cull(this, draw_list, render_pass::FORWARD);
	RENDER_SYSTEM.set_camera(this);
	if (show_skeleton_) {
		for (auto &d:draw_list) {
			RENDER_SYSTEM.draw(d);
			if (d.skeleton_pose == nullptr)
				continue;
			auto &dbg = render::debugger::inst();
			auto &pose = *d.skeleton_pose;
			auto *skel = ((animator *)d.go->get_component("animator"))->get_skeleton();
			for (size_t i = 0; i < pose.size(); i++) {
				auto &joint = pose[i];
				auto parent = skel->parent(joint.id);
				if (parent >= 0) {
					auto &trans = d.go->transform;
					dbg.draw_line(
						trans.rotation * pose[parent].position * trans.scale + trans.position, 
						color(0.0f, 0.0f, 1.f), 
						trans.rotation * joint.position * trans.scale + trans.position, 
						color(0.0, 1.0f, 0.0f)
					);
				}
			}
		}
	}
	render::debugger::inst().cull(this, draw_list);
	RENDER_SYSTEM.renderpass_begin();
	for (auto &d:draw_list)
		RENDER_SYSTEM.draw(d);
	draw_list.clear();
	RENDER_SYSTEM.renderpass_end();
}

}


