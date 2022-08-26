#pragma once
#include <memory>
#include <vulkan/vulkan.h>
#include "vulkan/vk_object.h"
#include "vulkan/vk_surface.h"
#include "vulkan/vk_swapchain.h"
#include "vulkan/vk_buffer.h"
#include "vulkan/vk_texture.h"
#include "utils/delete_queue.h"
#include "conf.h"

namespace engine {
namespace vulkan {

	struct vk_ctx {
		VkInstance instance = VK_NULL_HANDLE;
		VkPhysicalDevice phydevice = VK_NULL_HANDLE;
		VkPhysicalDeviceProperties properties;
		VkDevice device = VK_NULL_HANDLE;
		VkQueue graphicsqueue = VK_NULL_HANDLE;
		VkQueue presentqueue = VK_NULL_HANDLE;
		VkDescriptorPool descriptorpool = VK_NULL_HANDLE;
		VmaAllocator allocator = VK_NULL_HANDLE;
		VkPipelineCache pipeline_cache = VK_NULL_HANDLE;
		VkDescriptorSetLayout engine_desc_set_layout = VK_NULL_HANDLE;
		VkDescriptorSet engine_desc_set[conf::MAX_FRAMES_IN_FLIGHT];
		VkCommandPool commandpool;
		vk_swapchain swapchain;
		VkSurfaceKHR surface = VK_NULL_HANDLE;
		VkDebugReportCallbackEXT dbgcallback = VK_NULL_HANDLE;
		VkFormat depth_format;
		int graphicsfamily = -1;
		int presentfamily = -1;
		int frame_index = 0;
		uint32_t cmdbuf_index = 0;
		VkCommandBuffer cmdbuf = VK_NULL_HANDLE;
		delete_queue<vk_buffer> *buf_delq = nullptr;
		delete_queue<vk_texture>* tex_delq = nullptr;
		VkFramebuffer current_framebuffer = VK_NULL_HANDLE;
		VkRenderPass current_renderpass = VK_NULL_HANDLE;
		bool enable_msaa = false;	
		std::array<VkCommandBuffer, conf::MAX_FRAMES_IN_FLIGHT+1> cmdbufs;
		std::array<delete_queue<vk_buffer>, conf::MAX_FRAMES_IN_FLIGHT + 1> buf_delqs;
		std::array<delete_queue<vk_texture>, conf::MAX_FRAMES_IN_FLIGHT + 1> tex_delqs;
		PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXT = VK_NULL_HANDLE;
		PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXT = VK_NULL_HANDLE;
		VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
		
		VkPipelineLayout engine_bindless_pipeline_layout = VK_NULL_HANDLE;
		VkDescriptorSetLayout engine_bindless_texture_layout = VK_NULL_HANDLE;
		VkDescriptorSet engine_bindless_texture_set = VK_NULL_HANDLE;
		VkDescriptorSetLayout engine_bindless_object_layout = VK_NULL_HANDLE;
		VkDescriptorSetLayout engine_bindless_material_layout = VK_NULL_HANDLE;
		VkDescriptorSet engine_bindless_object_set[conf::MAX_FRAMES_IN_FLIGHT];
		VkDescriptorSet engine_bindless_material_set[conf::MAX_FRAMES_IN_FLIGHT];
		std::unique_ptr<vk_buffer> engine_bindless_object[conf::MAX_FRAMES_IN_FLIGHT];
		std::unique_ptr<vk_buffer> engine_bindless_material[conf::MAX_FRAMES_IN_FLIGHT];
	};
	int vk_ctx_init(const char *name, int width, int height);
	void vk_ctx_init_lighting();
	void vk_ctx_frame_begin();
	void vk_ctx_frame_end();
	void vk_ctx_renderpass_begin(render_texture *rt);
	void vk_ctx_renderpass_end();
	void vk_ctx_cleanup();
	void vk_ctx_debug_label_begin(const char *label);
	void vk_ctx_debug_label_end();
	extern struct vk_ctx VK_CTX;
}}


