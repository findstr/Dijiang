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
	vk_material(std::shared_ptr<render::shader> &s, bool ztest, bool shadowcaster);
	~vk_material() override {}
	void set_shader(std::shared_ptr<render::shader> &s) override;
	void set_texture(const std::string &name, std::shared_ptr<render::texture> &tex) override;
public:
	std::array<VkDescriptorSet, conf::MAX_FRAMES_IN_FLIGHT> desc_set;
	std::unique_ptr<vk_pipeline> pipeline;
	std::unordered_map<std::string, std::shared_ptr<render::texture>> tex_args;
private:
	bool shadowcaster;
};


}}

