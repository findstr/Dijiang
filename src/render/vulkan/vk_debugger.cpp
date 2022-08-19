#include "resource/resource.h"
#include "vk_pipeline.h"
#include "vk_mesh.h"
#include "vk_debugger.h"

namespace engine {
namespace vulkan {

void
vk_debugger::init() 
{
	trans.set_position(vector3f(0, 0, 0));
	trans.set_scale(vector3f(1, 1, 1));
	trans.set_rotation(quaternion::identity());
	for (int i = 0; i < conf::MAX_FRAMES_IN_FLIGHT; i++)
		meshes[i].reset(new vk_mesh());
}

}}
