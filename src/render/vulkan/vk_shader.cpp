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

#ifndef NDEBUG

static void
check_valid(const std::string &var_name, int binding) 
{
	#define ENTRY(name) {name##_BINDING, name##_NAME}
	static struct {
		int bnding;
		std::string name;
	} name_checks[] = {
		ENTRY(ENGINE_PER_FRAME),
		ENTRY(ENGINE_PER_CAMERA),
		ENTRY(ENGINE_PER_OBJECT),

		ENTRY(ENGINE_BRDF_TEX),
		ENTRY(ENGINE_SKYBOX_SPECULAR),
		ENTRY(ENGINE_SKYBOX_IRRADIANCE),
		ENTRY(ENGINE_SHADOWMAP),

		ENTRY(ENGINE_BRDF_TEX_SAMPLER),
		ENTRY(ENGINE_SKYBOX_SPECULAR_SAMPLER),
		ENTRY(ENGINE_SKYBOX_IRRADIANCE_SAMPLER),
		ENTRY(ENGINE_SHADOWMAP_SAMPLER),
	};
	#undef ENTRY
	for (int i = 0; i < sizeof(name_checks) / sizeof(name_checks[0]); i++) {
		if (name_checks[i].name == var_name) {
			assert(binding == name_checks[i].bnding);
			break;
		}
	}
}

#else

#define check_valid(a,b)	(void)a; (void)b;

#endif

void
vk_shader::analyze_uniform_buffers(
	spirv_cross::Compiler &compiler,
	spirv_cross::ShaderResources &res,
	VkShaderStageFlags stageflags)
{

	buffers.reserve(buffers.size() + res.uniform_buffers.size());
	for (auto &ub : res.uniform_buffers) {
		auto type = compiler.get_type(ub.type_id);
		auto &var_name = compiler.get_name(ub.id);
		auto ubsize = compiler.get_declared_struct_size(type);

		int set = compiler.get_decoration(ub.id, spv::DecorationDescriptorSet);
		int binding = compiler.get_decoration(ub.id, spv::DecorationBinding);
		if (set == ENGINE_DESC_SET) {
			check_valid(var_name, binding);
			continue;
		}
		auto &bi = buffers.emplace_back();
		bi.set = set;
		bi.binding = binding;
		bi.size = ubsize;
		bi.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		bi.name = var_name;
		var_stages[var_name] |= stageflags;
	}
}

void
vk_shader::analyze_storage_buffers(
	spirv_cross::Compiler &compiler,
	spirv_cross::ShaderResources &resources,
	VkShaderStageFlags stageflags)
{
	buffers.reserve(buffers.size() + resources.storage_buffers.size());
	for (auto &sb : resources.storage_buffers) {
		auto type = compiler.get_type(sb.type_id);
		auto &var_name = compiler.get_name(sb.id);
		auto &type_name = compiler.get_name(sb.base_type_id);

		int set = compiler.get_decoration(sb.id, spv::DecorationDescriptorSet);
		int binding = compiler.get_decoration(sb.id, spv::DecorationBinding);

		if (set == ENGINE_DESC_SET) {
			check_valid(var_name, binding);
			continue;
		}

		auto &bi = buffers.emplace_back();
		bi.set = set;
		bi.binding = binding;
		bi.size = 0;
		bi.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		bi.name = var_name;
		var_stages[var_name] |= stageflags;
	}
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

void
vk_shader::analyze_images(
	spirv_cross::Compiler &compiler,
	spirv_cross::ShaderResources &resources,
	VkShaderStageFlags stageflags)
{
	images.reserve(images.size() + resources.separate_images.size());
	for (auto &si : resources.separate_images) {
		auto type = compiler.get_type(si.type_id);
		auto &var_name = compiler.get_name(si.id);
		auto &type_name = compiler.get_name(si.base_type_id);

		int set = compiler.get_decoration(si.id, spv::DecorationDescriptorSet);
		int binding = compiler.get_decoration(si.id, spv::DecorationBinding);

		if (set == ENGINE_DESC_SET) {
			check_valid(var_name, binding);
			continue;
		}

		auto &img = images.emplace_back();
		img.set = set;
		img.binding = binding;
		img.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		img.name = var_name;
		var_stages[var_name] |= stageflags;
	}
}

void
vk_shader::analyze_samplers(
	spirv_cross::Compiler &compiler,
	spirv_cross::ShaderResources &resources,
	VkShaderStageFlags stageflags)
{
	samplers.reserve(samplers.size() + resources.separate_samplers.size());
	for (auto &ss : resources.separate_samplers) {
		auto type = compiler.get_type(ss.type_id);
		auto &var_name = compiler.get_name(ss.id);
		auto &type_name = compiler.get_name(ss.base_type_id);

		int set = compiler.get_decoration(ss.id, spv::DecorationDescriptorSet);
		int binding = compiler.get_decoration(ss.id, spv::DecorationBinding);

		if (set == ENGINE_DESC_SET) {
			check_valid(var_name, binding);
			continue;
		}

		auto &sam = samplers.emplace_back();
		sam.set = set;
		sam.binding = binding;
		sam.type = VK_DESCRIPTOR_TYPE_SAMPLER;
		sam.name = var_name;
		var_stages[var_name] |= stageflags;
	}
}

void
vk_shader::build_desc_set_layout()
{
	VkResult result;
	std::vector<VkDescriptorSetLayoutBinding> bindings;
	bindings.reserve(buffers.size() + images.size() + samplers.size());
	for (auto iter:buffers) {
		auto &dst = bindings.emplace_back();
		dst.binding = iter.binding;
		dst.descriptorType = iter.type;
		dst.descriptorCount = 1;
		dst.stageFlags = var_stages[iter.name];
		dst.pImmutableSamplers = nullptr;
		printf("shader buffer %s binding:%d\n", iter.name.c_str(), iter.binding);
	}
	for (auto iter:images) {
		auto &dst = bindings.emplace_back();
		dst.binding = iter.binding;
		dst.descriptorType = iter.type;
		dst.descriptorCount = 1;
		dst.stageFlags = var_stages[iter.name];
		dst.pImmutableSamplers = nullptr;
		printf("shader images %s binding:%d\n", iter.name.c_str(), iter.binding);
	}
	for (auto iter:samplers) {
		auto &dst = bindings.emplace_back();
		dst.binding = iter.binding;
		dst.descriptorType = iter.type;
		dst.descriptorCount = 1;
		dst.stageFlags = var_stages[iter.name];
		dst.pImmutableSamplers = nullptr;
		printf("shader sampler %s binding:%d\n", iter.name.c_str(), iter.binding);
	}
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	result = vkCreateDescriptorSetLayout(VK_CTX.device, &layoutInfo, nullptr, &desc_set_layout_);
	assert(result == VK_SUCCESS);
}

vk_shader::vk_shader(const std::vector<render::shader::code> &stages)
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
		analyze_uniform_buffers(compiler, res, stage);
		analyze_storage_buffers(compiler, res, stage);
		analyze_vertex_input(compiler, res, stage);
		analyze_images(compiler, res, stage);
		analyze_samplers(compiler, res, stage);
		createInfo.codeSize = code_size;
		createInfo.pCode = code;
		if (vkCreateShaderModule(VK_CTX.device, &createInfo,
			nullptr, &shader_module) != VK_SUCCESS) {
			assert(!"create shader fail");
		}
		modules.emplace_back(name, stage, shader_module);
	}
	build_desc_set_layout();
}

vk_shader::~vk_shader()
{
	auto device = VK_CTX.device;
	for (auto &si : modules)
		vkDestroyShaderModule(device, si.module, nullptr);
	if (desc_set_layout_ != VK_NULL_HANDLE)
		vkDestroyDescriptorSetLayout(device, desc_set_layout_, nullptr);
}

template<typename T>
const T *find(const std::string &name, const std::vector<T> &list)
{
	for (auto &x:list) {
		if (x.name == name)
			return &x;
	}
	return nullptr;
}

const vk_shader::buffer *
vk_shader::find_buffer(const std::string &name)
{
	return find<buffer>(name, buffers);
}

const vk_shader::image *
vk_shader::find_image(const std::string &name)
{
	return find<image>(name, images);
}

const vk_shader::sampler *
vk_shader::find_sampler(const std::string &name)
{
	return find<sampler>(name, samplers);
}

}

namespace render {

shader *
shader::create(const std::vector<shader::code> &stages)
{
	return new vulkan::vk_shader(stages);
}

}

}

