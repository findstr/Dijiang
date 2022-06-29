#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <set>
#include <vector>
#include <array>
#include <chrono>

#include "vulkan/vk_object.h"
#include "utils/file.h"
#include "conf.h"
#include "vk_ctx.h"
#include "vk_surface.h"
#include "vk_texture.h"
#include "forward.h"
#include "painter.h"

namespace engine {
namespace render {

using namespace engine::vulkan;

void
painter::init()
{
	surface = surface_new("帝江", 1024, 768);
	vk_ctx_init("帝江", surface, 1024, 768);
	forward = forward_new();
}

void
painter::cleanup()
{
	forward_del(forward);
}

bool
painter::draw(camera *cam, const std::vector<draw_object> &drawlist)
{
	if (surface_poll(surface) != 0)
		return false;
	forward_begin(forward, drawlist.size());
	for (auto &d:drawlist) {
		forward_tick(cam, forward, d);
	}
	forward_end(forward);
	return true;
}

}}

