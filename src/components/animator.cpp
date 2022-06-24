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
#if 1
	std::vector<animation::skeleton::pose> pose;
	auto *skel = get_skeleton();
	skel->apply_animation(go->transform, pose);
	auto &dbg = render::debugger::inst();
	for (size_t i = 0; i < pose.size(); i++) {
		auto &joint = pose[i];
		auto parent = skel->parent(joint.id);
		if (parent >= 0) {
			dbg.draw_line(
				pose[parent].position, color(0.0f, 0.0f, 1.f), 
				joint.position, color(0.0, 1.0f, 0.0f)
			);
		}
	}
#endif
}

}
