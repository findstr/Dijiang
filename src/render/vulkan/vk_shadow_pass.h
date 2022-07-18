#pragma once
#include <vulkan/vulkan.hpp>
#include "vk_render_pass.h"
namespace engine {
namespace vulkan {

class vk_shadow_pass : public vk_render_pass {
public:
	vk_shadow_pass();
};

}}

