#include <optional>
#include "vk_object.h"
#include "samplermgr.h"
#include "cmdbuf.h"
#include "vk_buffer.h"
#include "vk_texture2d.h"
#include "vk_sampler_pool.h"
#include "vk_shader_variables.h"
#include "vk_set_write.h"

namespace engine {
namespace vulkan {

static int id = 0;

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

        // Handle deferred writes to bindless textures.
        // TODO: use dynamic array instead.

        handle = id++;

        VkDescriptorImageInfo descriptor_image_info;
        descriptor_image_info.sampler = vk_sampler_pool::inst().fetch(this);
        descriptor_image_info.imageView = native.view;
        descriptor_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;


        vk_set_write::inst().write(
                VK_CTX.engine_bindless_texture_set,
                ENGINE_BINDLESS_TEXTURE_BINDING,
                handle,
                descriptor_image_info);
}


}}

