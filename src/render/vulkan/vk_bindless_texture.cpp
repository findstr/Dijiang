#include "vk_ctx.h"
#include "vk_shader_variables.h"
#include "vk_bindless_texture.h"

namespace engine {
namespace vulkan {

void
vk_bindless_texture::init()
{
	VkDescriptorSetLayoutBinding vk_binding;
	vk_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	vk_binding.descriptorCount = conf::MAX_BINDLESS_RESOURCE;
	vk_binding.binding = ENGINE_BINDLESS_TEXTURE_BINDING;
	vk_binding.stageFlags = VK_SHADER_STAGE_ALL;
	vk_binding.pImmutableSamplers = nullptr;
	desc_set.bind(vk_binding);
	desc_set.create();
}

void
vk_bindless_texture::cleanup()
{
	desc_set.destroy();
}


}}
