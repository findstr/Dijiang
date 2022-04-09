#pragma once
#include <memory>
#include "render/material.h"
#include "vk_buffer.h"
#include "vk_pass.h"
#include "vk_pipeline.h"

namespace engine {
namespace vulkan {

class vk_material : public render::material {
public:
	vk_material(std::shared_ptr<render::shader> &s);
	~vk_material() override {}
	void set_shader(std::shared_ptr<render::shader> &s) override;
	void set_texture(const std::string &name, std::shared_ptr<render::texture> &tex) override;
public:
	void set_renderpass(std::shared_ptr<vk_pass> &rp);
public:
	std::array<VkDescriptorSet, conf::MAX_FRAMES_IN_FLIGHT> desc_set;
	size_t uniformsize = 0;
	std::shared_ptr<vk_buffer> uniformbuffer;
	std::shared_ptr<vk_pass> renderpass;
	std::unique_ptr<vk_pipeline> pipeline;
};


}}

