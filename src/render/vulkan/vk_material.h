#pragma once
#include <memory>
#include "render/material.h"
#include "vk_buffer.h"
#include "vk_pipeline.h"

namespace engine {
namespace vulkan {

class vk_material : public render::material {
public:
	vk_material(render_pass::path path, std::shared_ptr<render::shader> &s, bool ztest);
	~vk_material() override {}
	void set_shader(std::shared_ptr<render::shader> &s) override;
	void set_texture(const std::string &name, std::shared_ptr<render::texture> &tex) override;
	vk_pipeline &pipeline(VkRenderPass render_pass);
public:
	VkRenderPass render_pass = VK_NULL_HANDLE;
	std::unique_ptr<vk_pipeline> pipeline_;
	std::array<VkDescriptorSet, conf::MAX_FRAMES_IN_FLIGHT> desc_set;
	std::unordered_map<std::string, std::shared_ptr<render::texture>> tex_args;
};


}}

