#pragma once
#include <assert.h>
#include "vk_texture.h"
#include "vk_texture2d.h"
#include "vk_cubemap.h"
#include "texture.h"

namespace engine {
namespace vulkan {

static inline vk_texture &
native_of(render::texture *tex)
{
	switch (tex->type()) {
	case render::texture::TEX2D:
		return ((vk_texture2d *)tex)->native;
	case render::texture::CUBE:
		return ((vk_cubemap *)tex)->native;
	default:
		assert(!"unsupport");
		vk_texture *null = nullptr;
		return *null;
	}
}


}}

