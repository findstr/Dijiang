#pragma once
#include <optional>
#include <vulkan/vulkan.hpp>
#include "vk_mem_alloc.h"

namespace engine {
namespace vulkan {
	enum texture_wrap : int {
		TEX_WRAP_REPEAT = 0,
		TEX_WRAP_CLAMP = 1,
	};

	enum texture_filter : int {
		TEX_FILTER_POINT = 0,
		TEX_FILTER_BILINEAR = 1,
		TEX_FILTER_TRILINEAR = 2,
	};

	struct texture_setting {
		texture_wrap wrap_mode = TEX_WRAP_CLAMP;
		texture_filter filter_mode = TEX_FILTER_POINT;
		int mipmap_levels = 0;
		int aniso_level = 0;
		VkFormat format;
		VkImageTiling tiling;
		VkImageUsageFlags usage;
		VkImageAspectFlags aspectflags;
	};

	struct textureEx {
		VkImage image = VK_NULL_HANDLE;
		VkImageView view = VK_NULL_HANDLE;
		VkSampler sampler = VK_NULL_HANDLE;
		VmaAllocation allocation = VK_NULL_HANDLE;
	};

	struct swapchainst {
		VkSwapchainKHR handle = VK_NULL_HANDLE;
		VkFormat imageformat;
		VkExtent2D extent = {0, 0};
		std::vector<VkImageView> imageviews;
	};

	struct renderctx {
		VkInstance instance = VK_NULL_HANDLE;
		VkPhysicalDevice phydevice = VK_NULL_HANDLE;
		VkDevice logicdevice = VK_NULL_HANDLE;
		VkQueue graphicsqueue = VK_NULL_HANDLE;
		VkQueue presentqueue = VK_NULL_HANDLE;
		VkDescriptorPool descriptorpool = VK_NULL_HANDLE;
		VmaAllocator allocator = VK_NULL_HANDLE;
		VkCommandPool commandpool;
		swapchainst swapchain;
	};

	#define vk_ctx	renderctx_get()

}}

#include "renderctx.h"
