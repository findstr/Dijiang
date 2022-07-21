#include "conf.h"
#include "ubo.h"
#include "shader.h"
#include "vk_buffer.h"
#include "vk_shader.h"
#include "vk_texture.h"
#include "vk_native.h"
#include "vk_material.h"

namespace engine {


namespace vulkan {

vk_material::vk_material(render_pass::path path, std::shared_ptr<render::shader> &s, bool ztest)
{
	this->render_path_ = path;
	this->ztest = ztest;
	set_shader(s);
}

void
vk_material::set_shader(std::shared_ptr<render::shader> &s)
{
	shader = s;
	class vk_shader *rs = static_cast<class vk_shader *>(s.get());
	auto desc_layout = rs->desc_set_layout();
	for (size_t i = 0; i < desc_set.size(); i++) {
		VkDescriptorSetAllocateInfo dsa{};
		dsa.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		dsa.descriptorPool = VK_CTX.descriptorpool;
		dsa.descriptorSetCount = 1;
		dsa.pSetLayouts = &desc_layout;
		auto result = vkAllocateDescriptorSets(VK_CTX.device,
			&dsa, &desc_set[i]);
		if (result != VK_SUCCESS)
			printf("====:%d\n", result);
		assert(result == VK_SUCCESS);
	}
}

vk_pipeline &
vk_material::pipeline(VkRenderPass pass, bool enablemsaa)
{
	for (auto &iter:pipelines) {
		if (iter.renderpass == pass)
			return *iter.pipeline;
	}
	auto pl = vk_pipeline::create(pass, (vk_shader *)shader.get(), ztest, enablemsaa);
	pipelines.emplace_back(pass, pl);
	return *pl;
}

void
vk_material::set_texture(const std::string &name,
	std::shared_ptr<render::texture> &tex)
{
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
	}
	vkUpdateDescriptorSets(VK_CTX.device,
		static_cast<uint32_t>(descriptorWrite.size()),
		descriptorWrite.data(), 0, nullptr);
}

}}


