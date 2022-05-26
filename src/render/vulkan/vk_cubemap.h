#pragma once
#include "vk_object.h"
#include "render/cubemap.h"
#include "vk_buffer.h"

namespace engine {
namespace vulkan {

class vk_cubemap : public render::cubemap {
public:
	vk_cubemap() : render::cubemap() {}
	~vk_cubemap() override {};
	void apply() override;
public:
	vk_texture native;
};

}}

