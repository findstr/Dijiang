#include <optional>
#include "vk_ctx.h"
#include "vk_depth_buffer.h"

namespace engine {
namespace vulkan {

vk_depth_buffer::vk_depth_buffer(int width, int height, texture_format format) :
	render::texture(render::texture::TEX2D)
{
	assert(format == texture_format::D32 || format == texture_format::D24S8 || format == texture_format::D32S8);
	auto usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	this->linear = true;
	this->miplevels = 1;
	this->anisolevels = 1;
	this->format = format;
	this->width_ = width;
	this->height_ = height;
	this->wrap_mode_u = texture_wrap::CLAMP;
	this->wrap_mode_v = texture_wrap::CLAMP;
	this->wrap_mode_w = texture_wrap::CLAMP;
	this->filter_mode = texture_filter::BILINEAR;
	native.destroy();
	native.create(this, usage);
}


}}

