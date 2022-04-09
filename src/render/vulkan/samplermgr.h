#pragma once
#include <vulkan/vulkan.hpp>
#include "vk_object.h"
namespace engine {
namespace vulkan {

VkSampler samplermgr_newsampler(const renderctx *ctx, const texture_setting &args);
void samplermgr_delsampler(const renderctx *ctx, VkSampler sampler);

}}


