#pragma once
#include <memory>
#include "vulkan/renderctx.h"
#include "vulkan/forward.h"
#include "math/math.h"
#include "render/mesh.h"
#include "render/material.h"
#include "render/draw_object.h"
#include "render/vulkan/renderctx.h"
#include "framework/components/camera.h"

namespace engine {
namespace render {

class painter {
public:
	void init();
	void cleanup();
	bool draw(camera *cam, const std::vector<draw_object> &drawlist);
private:
	std::unique_ptr<vulkan::renderctx> ctx;
	vulkan::surface *surface = nullptr;
	vulkan::forward *forward = nullptr;
};

}}

