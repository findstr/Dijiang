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
#include <Eigen/Core>

#include "vulkan/vk_object.h"
#include "utils/file.h"
#include "conf.h"
#include "vk_surface.h"
#include "vk_texture.h"
#include "renderctx.h"
#include "forward.h"
#include "render.h"

using namespace engine::vulkan;

namespace engine {
namespace render {

class HelloVulkan {
private:
	std::unique_ptr<renderctx> ctx;
	surface *surface = nullptr;
	forward *forward = nullptr;
public:
	void initVulkan() {
		surface = surface_new("帝江", 800, 600);
		ctx = std::make_unique<renderctx>();
		auto ctx_ptr = ctx.get();
		renderctx_init(ctx_ptr, "帝江", surface, 800, 600);
		forward = forward_new(ctx_ptr);
	}
private:
	void cleanup() {
		forward_del(ctx.get(), forward);
	}
	void mainLoop() {
		while (!surface_poll(surface)) {
			forward_tick(ctx.get(), forward);
		}
		printf("mainLoop\n");
		printf("mainLoop2\n");
	}

public:
	void run() {
		mainLoop();
		cleanup();
	}
};


HelloVulkan app;

int
init(const char *name, int width, int height)
{
	/*
	device.name = name;
	device.width = width;
	device.height = height;
	*/
	app.initVulkan();
	return 0;
}

int
drawframe()
{
	app.run();
	return -1;
}

void
cleanup()
{

}

}}

