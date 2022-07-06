#pragma once
#include <memory>
#include <vulkan/vulkan.h>
#include "vulkan/vk_object.h"
#include "vulkan/vk_surface.h"
namespace engine {
namespace vulkan {
	struct swapchainst {
		bool rebuild = false;
		VkSwapchainKHR handle = VK_NULL_HANDLE;
		VkFormat imageformat;
		VkExtent2D extent = {0, 0};
		std::vector<VkImageView> imageviews;
	};
	struct vk_ctx {
		VkInstance instance = VK_NULL_HANDLE;
		VkPhysicalDevice phydevice = VK_NULL_HANDLE;
		VkPhysicalDeviceProperties properties;
		VkDevice logicdevice = VK_NULL_HANDLE;
		int graphicsfamily = -1;
		VkQueue graphicsqueue = VK_NULL_HANDLE;
		VkQueue presentqueue = VK_NULL_HANDLE;
		VkDescriptorPool descriptorpool = VK_NULL_HANDLE;
		VmaAllocator allocator = VK_NULL_HANDLE;
		VkDescriptorSetLayout engine_desc_set_layout = VK_NULL_HANDLE;
		VkDescriptorSet engine_desc_set = VK_NULL_HANDLE;
		VkCommandPool commandpool;
		swapchainst swapchain;
		VkSurfaceKHR surface = VK_NULL_HANDLE;
		VkDebugReportCallbackEXT dbgcallback = VK_NULL_HANDLE;
		VkCommandBuffer cmdbuf = VK_NULL_HANDLE;
		VkViewport viewport;
	};
	extern const struct vk_ctx VK_CTX;
	int vk_ctx_init(const char *name, surface *s, int width, int height);
	void vk_ctx_set_cmdbuf(VkCommandBuffer cmdbuf);
	void vk_ctx_set_viewport(float x, float y, float width, float height);
	void vk_ctx_cleanup();
}}


