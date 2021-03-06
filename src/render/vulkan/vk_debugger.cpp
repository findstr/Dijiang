#include "resource/resource.h"
#include "vk_pipeline.h"
#include "vk_mesh.h"
#include "vk_material.h"
#include "vk_debugger.h"

namespace engine {
namespace vulkan {

void
vk_debugger::init() 
{
	trans.local_position = trans.position = vector3f(0, 0, 0);
	trans.local_scale = trans.scale = vector3f(1, 1, 1);
	trans.local_rotation = trans.rotation = quaternion::identity();
	mat = resource::load_material("asset/material/line.mat");
	((vk_material *)mat.get())->primitive_topolgy = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
	for (int i = 0; i < conf::MAX_FRAMES_IN_FLIGHT; i++)
		meshes[i].reset(new vk_mesh());
}

}}
