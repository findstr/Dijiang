#pragma once
#include <vulkan/vulkan.hpp>
#include "vk_object.h"
namespace engine {
namespace vulkan {

VkSampler samplermgr_newsampler(const texture_setting &args);
void samplermgr_delsampler(VkSampler sampler);

}}


