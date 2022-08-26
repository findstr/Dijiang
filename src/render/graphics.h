#pragma once
#include <array>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../../asset/shaders/include/engine_constant.inc.hlsl"

#ifdef USE_VULKAN

#include "vulkan/vk_graphics.h"

namespace engine {
	using graphics_ = vulkan::vk_graphics;
}
	
#define graphics ::engine::graphics_::inst()
#else
#error("graphics not be implemented");
#endif

