#include <optional>
#include "vk_object.h"
#include "samplermgr.h"
#include "cmdbuf.h"
#include "vk_buffer.h"
#include "vk_texture2d.h"

namespace engine {
namespace vulkan {

void
vk_texture2d::apply()
{
	auto usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	native.destroy();
	native.create(this, usage);
	vk_buffer staging(vk_buffer::STAGING, pixel.size());
	staging.upload(pixel.data(), pixel.size());
	native.transition_layout(this, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	native.fill(this, staging);
	native.gen_mipmap(this);
}


}}

