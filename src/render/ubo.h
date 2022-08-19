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
	glm::vec4 engine_light_ambient;
	glm::vec4 engine_light_position;
	glm::vec4 engine_light_direction;
	glm::vec4 engine_light_radiance;
	glm::mat4 engine_light_matrix_view[ENGINE_MAX_DIRECTIONAL_LIGHT];
	glm::mat4 engine_light_matrix_project[ENGINE_MAX_DIRECTIONAL_LIGHT];
};

struct per_camera {
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 view_proj;
	glm::vec4 engine_camera_pos;
};

struct per_object {
	int material = 0;
	int dummy[3];
	glm::mat4 model;
	glm::mat4 model_view_proj;
	std::array<glm::mat4, ENGINE_MAX_BONE_PER_SKELETON> skeleton_pose;
};

}}};

