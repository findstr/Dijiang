#pragma once
#include <optional>
#include <vulkan/vulkan.hpp>
#include "vk_mem_alloc.h"

namespace engine {
namespace vulkan {
	struct texture_setting {
		int mipmap_levels = 0;
		int aniso_level = 0;
		VkFormat format;
		VkImageTiling tiling;
		VkImageUsageFlags usage;
		VkImageAspectFlags aspectflags;
	};




}}

#include "vk_ctx.h"
