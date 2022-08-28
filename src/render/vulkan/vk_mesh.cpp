#include <iostream>
#include "vertex.h"
#include "vk_ctx.h"
#include "render/vertex.h"
#include "vk_mem_alloc.h"
#include "vk_mesh.h"

namespace engine {
namespace vulkan {

const int CHUNK_SIZE = 64*1024*1024;

void
vk_mesh::mem_chunk::init(enum vulkan::vk_buffer::type type, uint32_t size)
{
	//round to next highest power of 2
	size--;
	size |= size >> 1;
	size |= size >> 2;
	size |= size >> 4;
	size |= size >> 8;
	size |= size >> 16;
	size++;
	if (size < CHUNK_SIZE)
		size = CHUNK_SIZE;
	VmaVirtualBlockCreateInfo blockCreateInfo = {};
	blockCreateInfo.size = size; // 1 MB
	VkResult res = vmaCreateVirtualBlock(&blockCreateInfo, &block);
	assert(res == VK_SUCCESS);
	buffer.create(type, size);
}

int
vk_mesh::mem_chunk::alloc(uint32_t size, VmaVirtualAllocation *alloc)
{
	VkDeviceSize offset;
	VmaVirtualAllocationCreateInfo allocCreateInfo = {};
	allocCreateInfo.size = size;
	allocCreateInfo.alignment = sizeof(uint32_t);
	auto res = vmaVirtualAllocate(block, &allocCreateInfo, alloc, &offset);
	return (res == VK_SUCCESS) ? offset : -1;
}

void
vk_mesh::mem_chunk::free(VmaVirtualAllocation alloc)
{
	vmaVirtualFree(block, alloc);
}
	
int
vk_mesh::alloc_buffer(
	std::vector<mem_chunk> &chunks, 
	size_t alignment, 
	enum vulkan::vk_buffer::type type,
	size_t size, int *off, VmaVirtualAllocation *alloc)
{
	int i;
	for (i = 0; i < (int)chunks.size(); i++) {
		auto &chunk = chunks[i];
		int offset = chunk.alloc(size,  alloc);
		if (offset >= 0) {
			*off = offset;
			return i;
		}
	}
	chunks.emplace_back();
	auto &chunk = chunks.back();
	chunk.init(type, size);
	*off = chunk.alloc(size, alloc);
	return i;
}

int
vk_mesh::alloc_vertex(size_t count, int *offset, VmaVirtualAllocation *alloc)
{
	size_t sz = render::vertex_type::size_in_byte();
 	return alloc_buffer(vertex_chunks, sz, 
		engine::vulkan::vk_buffer::type::VERTEX, count * sz, offset, alloc);
}

	
int
vk_mesh::alloc_index(size_t count, int *offset, VmaVirtualAllocation *alloc)
{
	return alloc_buffer(index_chunks, sizeof(uint32_t), vk_buffer::type::INDEX, 
		count * sizeof(uint32_t), offset, alloc);
}

	
void
vk_mesh::flush()
{
	flush_unload();
}

void 
vk_mesh::flush_unload()
{
	for (auto handle : unloadings) {
		auto &md = mesh_descs[handle];
		if (md.index_chunk == -1)
			continue;
		vertex_chunks[md.vertex_chunk].free(md.vertex_alloc);
		index_chunks[md.index_chunk].free(md.index_alloc);
		md.vertex_chunk = -1;
		md.index_chunk = -1;
	}
	freed_ids.insert(freed_ids.end(), unloadings.begin(), unloadings.end());
	unloadings.clear();
}

mesh_handle_t 
vk_mesh::upload(
	const std::vector<vector3f> &vertices,
	const std::vector<vector2f> &uv,
	const std::vector<vector3f> &colors,
	const std::vector<vector3f> &tangents,
	const std::vector<vector3f> &normals,
	const std::vector<bone_weight> &bone_weights,
	const std::vector<int> &triangles) 
{
	mesh_handle_t handle;
	if (freed_ids.size() > 0) {
		handle = freed_ids.back();
		freed_ids.pop_back();
	} else {
		handle = mesh_descs.size();
		mesh_descs.emplace_back();
	}
	auto &md = mesh_descs[handle];
	render::vertex_type t_pos = render::vertex_type::POSITION;
	render::vertex_type t_uv = render::vertex_type::TEXCOORD;
	render::vertex_type t_color = render::vertex_type::COLOR;
	render::vertex_type t_normal = render::vertex_type::NORMAL;
	render::vertex_type t_tangent = render::vertex_type::TANGENT;
	render::vertex_type t_indices = render::vertex_type::BLENDINDICES;
	render::vertex_type t_weights = render::vertex_type::BLENDWEIGHT;
	size_t vertex_size = render::vertex_type::size_in_float();
	md.index_count = triangles.size();

	int vertice_count = vertices.size(); 
	int vertex_need_size = vertice_count * vertex_size * sizeof(float);
	int index_need_size =  md.index_count * sizeof(int);

	vk_buffer staging(vk_buffer::STAGING, vertex_need_size + index_need_size);
	void *stage_buf =  staging.map();
	float *data = (float*)stage_buf;

	bool hasuv = uv.size() >= vertices.size();
	bool hascolor = colors.size() >= vertices.size();
	bool isskin = bone_weights.size() > 0;
	bool hasnormal = normals.size() > 0;
	bool hastangent = tangents.size() > 0;
	for (int i = 0; i < vertice_count; i++) {
		auto *p = &data[t_pos.offset_in_float()];
		p[0] = vertices[i].x();
		p[1] = vertices[i].y();
		p[2] = vertices[i].z();
		if (hasuv) {
			auto *p = &data[t_uv.offset_in_float()];
			p[0] = uv[i].x();
			p[1] = uv[i].y();
		}
		if (hascolor) {
			auto *p = &data[t_color.offset_in_float()];
			p[0] = colors[i].x();
			p[1] = colors[i].y();
			p[2] = colors[i].z();
		}
		if (hasnormal) {
			auto *p = &data[t_normal.offset_in_float()];
			p[0] = normals[i].x();
			p[1] = normals[i].y();
			p[2] = normals[i].z();
		}
		if (hastangent) {
			auto *p = &data[t_tangent.offset_in_float()];
			p[0] = tangents[i].x();
			p[1] = tangents[i].y();
			p[2] = tangents[i].z();
		}
		if (isskin) {
			int j;
			uint32_t *indices = (uint32_t *)&data[t_indices.offset_in_float()];
			auto *weights = &data[t_weights.offset_in_float()];
			for (j = 0; j < 4; j++) {
				indices[j] =  bone_weights[i].index[j];
				weights[j] = bone_weights[i].weight[j];
			}
		}
		data += vertex_size;
	}

	memcpy((char *)stage_buf + vertex_need_size, triangles.data(), index_need_size);
	staging.unmap();

	md.vertex_chunk = alloc_vertex(vertice_count, &md.vertex_offset, &md.vertex_alloc);
	vertex_buffer(md).buffer.copy_from(&staging, 0, md.vertex_offset, vertex_need_size);

	md.index_chunk = alloc_index(triangles.size(), &md.index_offset, &md.index_alloc);
	index_buffer(md).buffer.copy_from(&staging, vertex_need_size, md.index_offset, index_need_size);

	return handle;
}

int
vk_mesh::bind_info(mesh_handle_t h, VkBuffer *vertex, int *vertex_offset, VkBuffer *index, int *index_offset)
{
	auto &md = mesh_descs[h];
	*vertex = vertex_buffer(md).buffer.handle;
	*vertex_offset = md.vertex_offset / render::vertex_type::size_in_byte();
 	*index = index_buffer(md).buffer.handle;
	*index_offset = md.index_offset / sizeof(uint32_t);
	return md.index_count;
}



}}


