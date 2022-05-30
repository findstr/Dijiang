#include "conf.h"
#include "ubo.h"
#include "shader.h"
#include "renderctx.h"
#include "vk_buffer.h"
#include "vk_shader.h"
#include "vk_texture.h"
#include "vk_pass.h"
#include "vk_native.h"
#include "vk_material.h"

namespace engine {


namespace vulkan {

vk_material::vk_material(std::shared_ptr<render::shader> &s)
{
	auto *ctx = renderctx_get();
	uniformsize = sizeof(render::UniformBufferObject);
	uniformbuffer.reset(new vk_buffer(vk_buffer::UNIFORM, uniformsize));
	set_shader(s);
}

void
vk_material::set_shader(std::shared_ptr<render::shader> &s)
{
	shader = s;
	class vk_shader *rs = static_cast<class vk_shader *>(s.get());
	auto *ctx = renderctx_get();
	auto desc_layout = rs->desc_set_layout();
	for (size_t i = 0; i < desc_set.size(); i++) {
		VkDescriptorSetAllocateInfo dsa{};
		dsa.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		dsa.descriptorPool = ctx->descriptorpool;
		dsa.descriptorSetCount = 1;
		dsa.pSetLayouts = &desc_layout;
		auto result = vkAllocateDescriptorSets(ctx->logicdevice,
			&dsa, &desc_set[i]);
		if (result != VK_SUCCESS)
			printf("====:%d\n", result);
		assert(result == VK_SUCCESS);
	}

	auto *uniform = rs->find_buffer("ubo");
	if (uniform) {
		assert(uniform);
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = uniformbuffer->handle;
		bufferInfo.offset = 0;
		bufferInfo.range = uniformsize;

		for (size_t i = 0; i < desc_set.size(); i++) {
			std::array<VkWriteDescriptorSet, 1> descriptorWrite{};
			descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite[0].dstSet = desc_set[i];
			descriptorWrite[0].dstBinding = uniform->binding;
			descriptorWrite[0].dstArrayElement = 0;
			descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite[0].descriptorCount = 1;
			descriptorWrite[0].pBufferInfo = &bufferInfo;
			descriptorWrite[0].pImageInfo = nullptr;
			descriptorWrite[0].pTexelBufferView = nullptr;

			vkUpdateDescriptorSets(ctx->logicdevice,
				static_cast<uint32_t>(descriptorWrite.size()),
				descriptorWrite.data(), 0, nullptr);
		}
	}
	auto pass = std::shared_ptr<vk_pass>(new vk_pass());
	set_renderpass(pass);
}

void
vk_material::set_texture(const std::string &name,
	std::shared_ptr<render::texture> &tex)
{
	auto *ctx = renderctx_get();
	class vk_shader *rs = static_cast<class vk_shader *>(shader.get());
	tex_args[name] = tex;
	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = native_of(tex.get()).view;
	imageInfo.sampler = native_of(tex.get()).sampler(tex.get());
	std::vector<VkWriteDescriptorSet> descriptorWrite;
	descriptorWrite.reserve(4);
	for (size_t i = 0; i < desc_set.size(); i++) {
		auto *image = rs->find_image(name);
		if (image) {
			auto &dw = descriptorWrite.emplace_back();
			dw.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			dw.dstSet = desc_set[i];
			dw.dstBinding = image->binding;
			dw.dstArrayElement = 0;
			dw.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			dw.descriptorCount = 1;
			dw.pImageInfo = &imageInfo;
		}
		auto *sampler = rs->find_sampler(name + "_sampler");
		if (sampler) {
			auto &dw = descriptorWrite.emplace_back();
			dw.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			dw.dstSet = desc_set[i];
			dw.dstBinding = sampler->binding;
			dw.dstArrayElement = 0;
			dw.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
			dw.descriptorCount = 1;
			dw.pImageInfo = &imageInfo;
		}
		vkUpdateDescriptorSets(ctx->logicdevice,
			static_cast<uint32_t>(descriptorWrite.size()),
			descriptorWrite.data(), 0, nullptr);
	}
}

void
vk_material::set_renderpass(std::shared_ptr<vk_pass> &rp)
{
	renderpass = rp;
	pipeline = std::unique_ptr<vk_pipeline>(vk_pipeline::create(rp.get(), (vk_shader *)shader.get()));
}


}

namespace render {

material *
material::create(std::shared_ptr<class shader> &s)
{
	return new vulkan::vk_material(s);
}

}

}


