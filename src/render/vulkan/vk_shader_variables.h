#pragma once
#include <string>
namespace engine {
namespace vulkan {

constexpr const char *ENGINE_PER_FRAME_NAME = "engine_per_frame";
constexpr const char *ENGINE_PER_CAMERA_NAME = "engine_per_camera";
constexpr const char *ENGINE_PER_OBJECT_NAME = "engine_per_object";
constexpr const int ENGINE_DESC_SET	= 1;
constexpr const int ENGINE_PER_FRAME_BINDING	= 0;
constexpr const int ENGINE_PER_CAMERA_BINDING	= 1;
constexpr const int ENGINE_PER_OBJECT_BINDING	= 2;

}}

