#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


namespace engine {
namespace render {
namespace ubo {

struct per_draw{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

}}};

