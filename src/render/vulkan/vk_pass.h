#pragma once
#include <vulkan/vulkan.hpp>
namespace engine {
namespace vulkan {

class vk_pass {
public:
	vk_pass();
	~vk_pass();
	VkFormat get_depth_format();
	VkRenderPass get_renderpass();
private:

	VkFormat depth_format;
	VkRenderPass renderpass;
	VkDevice device = VK_NULL_HANDLE;
};

}}

