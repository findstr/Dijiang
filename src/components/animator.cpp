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
	auto *skel = get_skeleton();
	skel->apply_animation(progress, current_pose);
}

}
