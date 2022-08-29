#pragma once
#include <SDL2/SDL.h>
#include "conf.h"
#include "math/math.h"
#include "vk_mesh.h"
#include "vk_texture_mgr.h"



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

	texture_handle_t upload_texture(render::texture &tex) 
	{
		return vk_texture_mgr::inst().upload_texture(tex);
	}
	inline void unload_texture(texture_handle_t handle) 
	{
		return vk_texture_mgr::inst().unload_texture(handle);
	}
};

}}

