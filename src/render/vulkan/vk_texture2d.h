#pragma once
#include "texture2d.h"
#include "vk_texture.h"
#include "vk_object.h"
#include "vk_buffer.h"

namespace engine {
namespace vulkan {

class vk_texture2d : public render::texture2d {
public:
	vk_texture2d() : render::texture2d() {}
	~vk_texture2d() override {}
	void apply() override;
public:
	vk_texture native;
};

}}

