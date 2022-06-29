#pragma once
#include "vk_object.h"
namespace engine {
namespace vulkan {

VkCommandBuffer cmdbuf_single_begin();
void cmdbuf_single_end(VkCommandBuffer commandBuffer);


}}

