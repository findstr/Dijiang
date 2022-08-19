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
#include "vk_pipeline.h"

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
public:
	vertex vertex_input;
	std::vector<shader_inst> modules;
	vk_pipeline &pipeline(VkRenderPass render_pass, bool enable_msaa);
public:
	vk_shader(const std::vector<render::shader::stage_code> &stages);
	~vk_shader();
private:
	void analyze_vertex_input(
		spirv_cross::Compiler &compiler,
		spirv_cross::ShaderResources &resources,
		VkShaderStageFlags stageflags);
	struct pipeline_cache {
		VkRenderPass renderpass = VK_NULL_HANDLE;
		vk_pipeline *pipeline = nullptr;
		pipeline_cache(pipeline_cache &&other) {
			renderpass = other.renderpass;
			pipeline = other.pipeline;
			other.renderpass = VK_NULL_HANDLE;
			other.pipeline = nullptr;
		}
		pipeline_cache(VkRenderPass rp, vk_pipeline *pl) :
			renderpass(rp), pipeline(pl) {}
		~pipeline_cache() {
			if (pipeline != nullptr)
				delete pipeline;
		}
	};
	std::vector<pipeline_cache> pipelines;
};

}}

