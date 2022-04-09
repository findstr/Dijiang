#pragma once
#include <memory>
#include <vulkan/vulkan.h>
#include "vulkan/vk_object.h"
#include "vulkan/vk_surface.h"
namespace engine {
namespace vulkan {
	int renderctx_init(renderctx *ctx, const char *name, surface *s, int width, int height);
	const renderctx *renderctx_get();
	void renderctx_cleanup(renderctx *ctx);
}}

