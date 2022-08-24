#include <assert.h>
#include "vk_object.h"
#include "vk_buffer.h"
#include "vk_texture.h"
#include "vk_filter.h"
#include "vk_cubemap.h"

namespace engine {
namespace vulkan {

void
vk_cubemap::apply()
{
	auto usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	native.destroy();
	native.create(this, usage, FACE_COUNT);
	auto tex_size = width_ * height_ * format.size();
	auto cube_size = FACE_COUNT * tex_size;
	vk_buffer staging(vk_buffer::STAGING, cube_size);
	uint8_t *data = (uint8_t *)staging.map();
	for (int i = 0; i < FACE_COUNT; i++)
		memcpy(&data[i * tex_size], pixel[i].data(), tex_size);
	staging.unmap();
	native.transition_layout(this, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, FACE_COUNT);
	native.fill(this, staging, FACE_COUNT);
	native.gen_mipmap(this, FACE_COUNT);
}

}}

