#pragma once
#include "vk_object.h"
namespace engine {
namespace vulkan {

VkCommandBuffer cmdbuf_single_begin(const renderctx *ctx);
void cmdbuf_single_end(const renderctx *ctx, VkCommandBuffer commandBuffer);


}}

