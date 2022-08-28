#pragma once
#include <SDL2/SDL.h>
#include "conf.h"
#include "math/math.h"
#include "render/vulkan/vk_mesh.h"


#include "vk_ctx.h"
#include "render/texture.h"
#include "render/vulkan/vk_shader_variables.h"
#include "render/vulkan/vk_set_write.h"
#include "render/vulkan/vk_sampler_pool.h"

namespace engine {
namespace vulkan {

class vk_gpu {
public:
	static vk_gpu &inst() {
		static vk_gpu inst;
		return inst;
	}
public:
	mesh_handle_t upload_mesh(
		const std::vector<vector3f> &vertices,
		const std::vector<vector2f> &uv,
		const std::vector<vector3f> &colors,
		const std::vector<vector3f> &tangents,
		const std::vector<vector3f> &normals,
		const std::vector<bone_weight> &bone_weights,
		const std::vector<int> &triangles) 
	{
		return vk_mesh::inst().upload(vertices, 
			uv, colors, tangents, normals,
			bone_weights, triangles);
	}
	constexpr void unload_mesh(mesh_handle_t h) 
	{
		vk_mesh::inst().unload(h);
	}

	std::vector<vk_texture> texture_pool;
	std::vector<texture_handle_t> freeids;
	texture_handle_t upload_texture(render::texture &tex) 
	{
		texture_handle_t handle;
		if (freeids.size()) {
			handle = freeids.back();
			freeids.pop_back();
		} else {
			handle = texture_pool.size();
			texture_pool.emplace_back();
		}
		auto &native = texture_pool[handle];
		switch (tex.type()) {
		case render::texture::TEX2D: {
			auto usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			native.destroy();
			native.create(&tex, usage);
			vk_buffer staging(vk_buffer::STAGING, tex.pixel.size());
			staging.upload(tex.pixel.data(), tex.pixel.size());
			native.transition_layout(&tex, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
			native.fill(&tex, staging);
			native.gen_mipmap(&tex);
			break;}
		case render::texture::CUBE: {
			auto usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			native.destroy();
			native.create(&tex, usage, 6);
			vk_buffer staging(vk_buffer::STAGING, tex.pixel.size());
			staging.upload(tex.pixel.data(), tex.pixel.size());
			native.transition_layout(&tex, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 6);
			native.fill(&tex, staging, 6);
			native.gen_mipmap(&tex, 6);
			break;}
		default:
			assert(0);
			break;
		}

		VkDescriptorImageInfo descriptor_image_info;
		descriptor_image_info.sampler = vk_sampler_pool::inst().fetch(&tex);
		descriptor_image_info.imageView = native.view;
		descriptor_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;


		vk_set_write::inst().write(
			VK_CTX.engine_bindless_texture_set,
			ENGINE_BINDLESS_TEXTURE_BINDING,
			handle,
			descriptor_image_info);	
		return handle;
	}
	inline void unload_texture(texture_handle_t handle) 
	{
		texture_pool[handle].destroy();
		freeids.emplace_back(handle);
	}
};

}}

