#pragma once
#include <memory>
#include <vulkan/vulkan.h>
#include "vulkan/vk_object.h"
#include "vulkan/vk_surface.h"
#include "conf.h"

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
		VkDevice device = VK_NULL_HANDLE;
		VkQueue graphicsqueue = VK_NULL_HANDLE;
		VkQueue presentqueue = VK_NULL_HANDLE;
		VkDescriptorPool descriptorpool = VK_NULL_HANDLE;
		VmaAllocator allocator = VK_NULL_HANDLE;
		VkDescriptorSetLayout engine_desc_set_layout = VK_NULL_HANDLE;
		VkDescriptorSet engine_desc_set[conf::MAX_FRAMES_IN_FLIGHT];
		VkCommandPool commandpool;
		swapchainst swapchain;
		VkSurfaceKHR surface = VK_NULL_HANDLE;
		VkDebugReportCallbackEXT dbgcallback = VK_NULL_HANDLE;
		VkFormat depth_format;
		VkRenderPass shadowmap_pass;
		VkRenderPass render_pass;
		int graphicsfamily = -1;
		int frame_index = 0;
		VkCommandBuffer cmdbuf = VK_NULL_HANDLE;
		std::array<VkCommandBuffer, conf::MAX_FRAMES_IN_FLIGHT> cmdbufs;
	};
	extern const struct vk_ctx VK_CTX;
	int vk_ctx_init(const char *name, surface *s, int width, int height);
	void vk_ctx_recreate_swapchain(int width, int height);
	void vk_ctx_init_lighting();
	void vk_ctx_frame_begin();
	void vk_ctx_frame_end();
	void vk_ctx_cleanup();
}}


