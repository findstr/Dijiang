#pragma once
#include "vk_texture.h"
#include "vk_object.h"
#include "vk_buffer.h"
#include "render/texture.h"
#include "render/texture_format.h"

namespace engine {
namespace vulkan {

class vk_depth_buffer : public render::texture {
public:
	vk_depth_buffer(int width, int height, texture_format format);
public:
	vk_texture native;
};

}}

