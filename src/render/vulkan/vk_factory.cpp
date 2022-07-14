#include "vk_cubemap.h"
#include "vk_debugger.h"
#include "vk_mesh.h"
#include "vk_material.h"

namespace engine {
namespace render {

#define FACTOR(type, ...) \
type *\
type::create(__VA_ARGS__) {\
	return new vulkan::vk_##type(__VA_ARGS__); \
}

FACTOR(mesh);

material*
material::create(std::shared_ptr<class shader> &s, bool ztest, bool shadowcaster) 
{
	return new vulkan::vk_material(s, ztest, shadowcaster); 
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

}}

