#pragma once
#include <memory>
#include <array>
#include <vector>
#include "conf.h"
#include "render/mesh.h"
#include "vk_buffer.h"

namespace engine {
namespace vulkan {

class gpu_mesh {
public:
	typedef int32_t mesh_handle_t;
	static constexpr mesh_handle_t invalid = -1;
	static gpu_mesh &instance() {
		static gpu_mesh inst;
		return inst;
	}
public:
	constexpr void upload(const render::mesh *m) {
		size_t n = m->vertices.size();
		if (n > max_loading_vertex_count)
			max_loading_vertex_count = n;
		loadings.emplace_back(m);
	}
	constexpr void unload(mesh_handle_t h) {
		unloadings.emplace_back(h);
	}
	void flush();
	int bind_info(mesh_handle_t h, VkBuffer *vertex, int *vertex_offset, VkBuffer *index, int *index_offset);
	void post_tick();
private:
	struct mem_chunk {
		mem_chunk() {}
		mem_chunk(mem_chunk &&mc) : block(mc.block) {
			buffer = std::move(mc.buffer);
		}
		VmaVirtualBlock block = VK_NULL_HANDLE;
		vulkan::vk_buffer buffer;
		void init(enum vulkan::vk_buffer::type type, uint32_t size);
		int alloc(uint32_t size, VmaVirtualAllocation *alloc);
		void free(VmaVirtualAllocation alloc);
	};
	struct mesh_desc {
		int vertex_chunk;
		int index_chunk;
		int vertex_offset;
		int index_offset;
		int index_count;
		VmaVirtualAllocation vertex_alloc;
		VmaVirtualAllocation index_alloc;
	};
	size_t max_loading_vertex_count = 0;
	std::vector<mem_chunk> vertex_chunks;
	std::vector<mem_chunk> index_chunks;
	std::vector<const render::mesh *> loadings;
	std::vector<mesh_handle_t> unloadings;
	std::vector<mesh_handle_t> freed_ids;
	std::vector<mesh_desc> mesh_descs;
private:
	constexpr mem_chunk &vertex_buffer(mesh_desc &md) { return vertex_chunks[md.vertex_chunk]; }
	constexpr mem_chunk &index_buffer(mesh_desc &md) { return index_chunks[md.index_chunk]; }
	void flush_unload();
	void flush_upload();
	int alloc_buffer(std::vector<mem_chunk> &chunks, size_t alignment, enum vulkan::vk_buffer::type type,
		size_t size, int *offset, VmaVirtualAllocation *alloc);
	int alloc_vertex(size_t size, int *offset, VmaVirtualAllocation *alloc);
	int alloc_index(size_t size, int *offset, VmaVirtualAllocation *alloc);
};

}}

