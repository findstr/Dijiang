#include "animator.h"
#include "debugger.h"

namespace engine {

void
animator::set_skeleton(const std::shared_ptr<animation::skeleton> &ske) 
{
	skeleton = ske;
}

const animation::skeleton *
animator::get_skeleton() 
{
	return skeleton.get();
}

	
void
animator::tick(float delta)
{
	progress += delta / total;
	progress -= std::floor(progress);
	progress = 0.0f;
	auto *skel = get_skeleton();
	debug_pose.clear();
	skel->apply_animation(progress, debug_pose);
	current_pose.clear();
	current_pose.resize(debug_pose.size());
	std::transform(debug_pose.begin(), debug_pose.end(), current_pose.begin(), [](const animation::skeleton::pose &p) {
		return p.matrix;
	});
#if 1
	auto &dbg = render::debugger::inst();
	for (size_t i = 0; i < debug_pose.size(); i++) {
		auto &joint = debug_pose[i];
		auto parent = skel->parent(joint.id);
		if (parent >= 0) {
			dbg.draw_line(
				transform->rotation * debug_pose[parent].position * transform->scale + transform->position, 
				color(0.0f, 0.0f, 1.f), 
				transform->rotation * joint.position * transform->scale + transform->position, 
				color(0.0, 1.0f, 0.0f)
			);
		}
	}
#endif
}

}
