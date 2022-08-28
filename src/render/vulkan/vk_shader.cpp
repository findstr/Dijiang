#include <vulkan/vulkan.hpp>
#include <string>
#include "vk_object.h"
#include "vk_shader.h"
#include "vk_shader_variables.h"

namespace engine {

namespace vulkan {

static VkShaderStageFlagBits
to_platform(render::shader::stage stage)
{
	switch(stage) {
	case render::shader::stage::VERTEX:
		return VK_SHADER_STAGE_VERTEX_BIT;
	case render::shader::stage::TESSELLATION_CONTROL:
		return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
	case render::shader::stage::TESSELATION_EVALUTION:
		return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
	case render::shader::stage::GEOMETRY:
		return VK_SHADER_STAGE_GEOMETRY_BIT;
	case render::shader::stage::FRAGMENT:
		return VK_SHADER_STAGE_FRAGMENT_BIT;
	case render::shader::stage::COMPUTE:
		return VK_SHADER_STAGE_COMPUTE_BIT;
	default:
		assert(!"unsupport shader stage");
		break;
	}
	return VK_SHADER_STAGE_VERTEX_BIT;
}

vk_pipeline &
vk_shader::pipeline(VkRenderPass pass, bool enablemsaa)
{
	for (auto &iter:pipelines) {
		if (iter.renderpass == pass)
			return *iter.pipeline;
	}
	auto pl = vk_pipeline::create(pass, this, enablemsaa);
	pipelines.emplace_back(pass, pl);
	return *pl;
}

static render::vertex_type
name_to_vertex_type(const std::string &name)
{
	static std::tuple<std::string, render::vertex_type> name_to_type[] = {
		{"in.var.POSITION0", render::vertex_type::POSITION},
		{"in.var.TEXCOORD0", render::vertex_type::TEXCOORD},
		{"in.var.COLOR0", render::vertex_type::COLOR}, 
		{"in.var.NORMAL0", render::vertex_type::NORMAL},
		{"in.var.TANGENT0", render::vertex_type::TANGENT},
		{"in.var.BLENDINDICES0", render::vertex_type::BLENDINDICES}, 
		{"in.var.BLENDWEIGHT0", render::vertex_type::BLENDWEIGHT},
	};
	for (auto &tp:name_to_type) {
		if (std::get<0>(tp) == name)
			return std::get<1>(tp);
	}
	assert(!"unsupport vertex_type format");
	return render::vertex_type::NONE;
}

void
vk_shader::analyze_vertex_input(
	spirv_cross::Compiler &compiler,
	spirv_cross::ShaderResources &resources,
	VkShaderStageFlags stageflags)
{
	if (stageflags != VK_SHADER_STAGE_VERTEX_BIT)
		return ;
	auto &attrs = vertex_input.attrs;
	for (auto &si : resources.stage_inputs) {
		render::vertex_type format = render::vertex_type::NONE;
		auto type = compiler.get_type(si.type_id);
		auto &type_name = compiler.get_name(si.id);
		auto &vt = attrs.emplace_back();
		vt.type = name_to_vertex_type(type_name);
		vt.location = compiler.get_decoration(si.id, spv::DecorationLocation);
	}
}

vk_shader::vk_shader(const std::vector<render::shader::stage_code> &stages)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	modules.reserve(stages.size());
	for (auto &s : stages) {
		VkShaderModule shader_module;
		auto &name = s.name;
		auto stage = to_platform(s.stage);
		const uint32_t *code = (const uint32_t *)(s.spv.data());
		auto code_size = s.spv.size();
		spirv_cross::Compiler compiler(code,
			code_size / sizeof(uint32_t));
		auto res = compiler.get_shader_resources();
		analyze_vertex_input(compiler, res, stage);
		createInfo.codeSize = code_size;
		createInfo.pCode = code;
		if (vkCreateShaderModule(VK_CTX.device, &createInfo,
			nullptr, &shader_module) != VK_SUCCESS) {
			assert(!"create shader fail");
		}
		modules.emplace_back(name, stage, shader_module);
	}
}

vk_shader::~vk_shader()
{
	auto device = VK_CTX.device;
	for (auto &si : modules)
		vkDestroyShaderModule(device, si.module, nullptr);
}


}

namespace render {

shader *
shader::create(const std::vector<shader::stage_code> &stages)
{
	return new vulkan::vk_shader(stages);
}

}

}

