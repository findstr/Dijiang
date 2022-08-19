#include "vk_cubemap.h"
#include "vk_debugger.h"
#include "vk_mesh.h"
#include "vk_texture2d.h"
#include "vk_render_texture.h"

namespace engine {
namespace render {

#define FACTOR(type, ...) \
type *\
type::create(__VA_ARGS__) {\
	return new vulkan::vk_##type(__VA_ARGS__); \
}

texture2d *
texture2d::create(int width, int height,
	texture_format format,
	bool linear, int miplevels)
{
	texture2d *tex = new vulkan::vk_texture2d();
	tex->width_ = width;
	tex->height_ = height;
	tex->format = format;
	tex->linear = linear;
	tex->miplevels = miplevels;
	return tex;
}

cubemap *
cubemap::create(int width, int height,
	texture_format format,
	bool linear, int miplevels)
{
	cubemap *tex = new vulkan::vk_cubemap();
	tex->width_ = width;
	tex->height_ = height;
	tex->linear = false;
	tex->miplevels = miplevels;
	return tex;
}
debugger &debugger::inst() {
	static debugger *_inst = new vulkan::vk_debugger();
	return *_inst;
}

}

render_texture *
render_texture::create(int width, int height,
	texture_format color_format,
	texture_format depth_format,
	bool linear, int miplevels)
{
	auto *tex = new vulkan::vk_render_texture();
	tex->width_ = width;
	tex->height_ = height;
	tex->format = color_format;
	tex->depth_format = depth_format;
	tex->linear = linear;
	tex->miplevels = miplevels;
	tex->apply();
	return tex;
}

}

