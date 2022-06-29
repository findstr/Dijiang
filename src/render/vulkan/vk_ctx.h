#pragma once
#include <memory>
#include <vulkan/vulkan.h>
#include "vulkan/vk_object.h"
#include "vulkan/vk_surface.h"
namespace engine {
namespace vulkan {
	struct vk_ctx {
		VkInstance instance = VK_NULL_HANDLE;
		VkPhysicalDevice phydevice = VK_NULL_HANDLE;
		VkPhysicalDeviceProperties properties;
		VkDevice logicdevice = VK_NULL_HANDLE;
		VkQueue graphicsqueue = VK_NULL_HANDLE;
		VkQueue presentqueue = VK_NULL_HANDLE;
		VkDescriptorPool descriptorpool = VK_NULL_HANDLE;
		VmaAllocator allocator = VK_NULL_HANDLE;
		VkDescriptorSetLayout engine_desc_set_layout = VK_NULL_HANDLE;
		VkDescriptorSet engine_desc_set = VK_NULL_HANDLE;
		VkCommandPool commandpool;
		swapchainst swapchain;
	};
	extern vk_ctx VK_CTX;
	int vk_ctx_init(const char *name, surface *s, int width, int height);
	void vk_ctx_cleanup();
}}


