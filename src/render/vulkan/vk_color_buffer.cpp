#include <optional>
#include "vk_ctx.h"
#include "vk_color_buffer.h"

namespace engine {
namespace vulkan {

vk_color_buffer::vk_color_buffer(int width, int height, texture_format format) :
	render::texture(render::texture::TEX2D)
{
	assert(format != texture_format::D32 && format != texture_format::D24S8 && format == texture_format::D32S8);
	auto usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	native.create(this, usage);
}


}}

