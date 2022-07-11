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

namespace engine {
namespace render {
namespace ubo {

struct per_frame {
	glm::vec3 engine_light_ambient;
	glm::vec3 engine_light_position;
	glm::vec3 engine_light_direction;
	glm::vec3 engine_light_radiance;
};

struct per_camera{
	glm::mat4 view;
	glm::mat4 proj;
	glm::vec3 engine_camera_pos;
};

struct per_object {
	glm::mat4 model;
	std::array<glm::mat4, ENGINE_MAX_BONE_PER_SKELETON> skeleton_pose;
};

}}};

