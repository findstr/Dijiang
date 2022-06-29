#pragma once
#include <memory>
#include "vulkan/vk_ctx.h"
#include "vulkan/forward.h"
#include "math/math.h"
#include "render/mesh.h"
#include "render/material.h"
#include "render/draw_object.h"
#include "components/camera.h"

namespace engine {
namespace render {

class painter {
public:
	void init();
	void cleanup();
	bool draw(camera *cam, const std::vector<draw_object> &drawlist);
	vulkan::surface *surface = nullptr;
private:
	vulkan::forward *forward = nullptr;
};

}}

