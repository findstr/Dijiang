#pragma once
#include <vulkan/vulkan.hpp>
#include "vk_shader.h"

namespace engine {
namespace vulkan {

class vk_pipeline {
public:
	vk_pipeline(VkPipeline pl, VkPipelineLayout lo) :
		handle(pl), layout(lo) {}
	~vk_pipeline();
	static vk_pipeline *create(VkRenderPass pass, vk_shader *shader, bool ztest = true, bool msaa = false);
public:
	VkPipeline handle = VK_NULL_HANDLE;
	VkPipelineLayout layout = VK_NULL_HANDLE;
public:
	static VkPrimitiveTopology primitive_topolgy;
};

}}

