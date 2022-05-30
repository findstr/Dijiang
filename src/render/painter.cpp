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
#include "vk_surface.h"
#include "vk_texture.h"
#include "renderctx.h"
#include "forward.h"
#include "painter.h"


namespace engine {
namespace render {

using namespace engine::vulkan;

void
painter::init()
{
	surface = surface_new("帝江", 800, 600);
	ctx = std::make_unique<renderctx>();
	auto ctx_ptr = ctx.get();
	renderctx_init(ctx_ptr, "帝江", surface, 800, 600);
	forward = forward_new(ctx_ptr);
}

void
painter::cleanup()
{
	forward_del(ctx.get(), forward);
}

bool
painter::draw(camera *cam, const std::vector<draw_object> &drawlist)
{
	if (surface_poll(surface) != 0)
		return false;
	forward_begin(forward);
	printf("draw list=====\n");
	for (auto &d:drawlist) {
		forward_tick(cam, ctx.get(), forward, d);
		printf("draw+\n");
	}
	forward_end(forward);
	return true;
}

}}

