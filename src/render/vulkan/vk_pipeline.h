#pragma once
#include <vulkan/vulkan.hpp>

namespace engine {
namespace vulkan {
struct vk_shader;
class vk_pipeline {
public:
	vk_pipeline(VkPipeline pl) :
		handle(pl) {}
	~vk_pipeline();
	static vk_pipeline *create(VkRenderPass pass, 
		vk_shader *shader, bool msaa = false);	
public:
	VkPipeline handle = VK_NULL_HANDLE;
};

}}

