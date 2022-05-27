#pragma once
#include <vector>
#include <array>
#include <string>
#include <unordered_map>
#include <vulkan/vulkan.h>
#include "conf.h"
#include "render/shader.h"
#include "render/vertex.h"
#include "spirv_cross.hpp"

namespace engine {
namespace vulkan {

class vk_shader : public render::shader {
public:
	struct shader_inst {
		shader_inst(const std::string &n,
			VkShaderStageFlagBits s,
			VkShaderModule m) :
			name(n), stage(s), module(m) {}
		std::string name;
		VkShaderStageFlagBits stage;
		VkShaderModule module = VK_NULL_HANDLE;
	};
	struct buffer {
		int set = 0;
		int binding = 0;
		size_t size = 0;
		VkDescriptorType type;
		std::string name;
	};
	struct image {
		int set = 0;
		int binding = 0;
		VkDescriptorType type;
		std::string name;
	};
	struct sampler {
		int set = 0;
		int binding = 0;
		VkDescriptorType type;
		std::string name;
	};
	struct vertex_attr {
		render::vertex_type type = render::vertex_type::NONE;
		int location = -1;
	};
	struct vertex {
		std::vector<vertex_attr> attrs;
	};
private:
	std::vector<buffer> buffers;
	std::vector<image> images;
	std::vector<sampler> samplers;
	std::unordered_map<std::string, VkShaderStageFlags> var_stages;
	VkDescriptorSetLayout desc_set_layout_ = VK_NULL_HANDLE;
public:
	vertex vertex_input;
	std::vector<shader_inst> modules;
	VkDescriptorSetLayout desc_set_layout() const {
		return desc_set_layout_;
	}
	const buffer * find_buffer(const std::string &name);
	const image *find_image(const std::string &name);
	const sampler *find_sampler(const std::string &name);
public:
	vk_shader(const std::vector<render::shader::code> &stages);
	~vk_shader();
private:
	void analyze_uniform_buffers(
		spirv_cross::Compiler &compiler,
		spirv_cross::ShaderResources &resources,
		VkShaderStageFlags stageflags);
	void analyze_storage_buffers(
		spirv_cross::Compiler &compiler,
		spirv_cross::ShaderResources &resources,
		VkShaderStageFlags stageflags);
	void analyze_vertex_input(
		spirv_cross::Compiler &compiler,
		spirv_cross::ShaderResources &resources,
		VkShaderStageFlags stageflags);
	void analyze_images(
		spirv_cross::Compiler &compiler,
		spirv_cross::ShaderResources &resources,
		VkShaderStageFlags stageflags);
	void analyze_samplers(
		spirv_cross::Compiler &compiler,
		spirv_cross::ShaderResources &resources,
		VkShaderStageFlags stageflags);
	void build_desc_set_layout();
};

}}

