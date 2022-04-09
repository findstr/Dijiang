#pragma once
#include "vk_mem_alloc.h"
#include "vk_object.h"
namespace engine {
namespace vulkan {

class vk_buffer {
public:
	enum type {
		NONE,
		UNIFORM,
		VERTEX,
		INDEX,
		STAGING,
	};
public:
	vk_buffer(type t, size_t size);
	~vk_buffer();
	void *map();
	void unmap();
	void upload(const void *src, size_t size);
	void copy_from(vk_buffer *src);
	void copy_from(VkCommandBuffer cmdbuf, vk_buffer *src);
public:
	size_t size = 0;
	type type = type::NONE;
	VkBuffer handle = VK_NULL_HANDLE;
	VmaAllocation allocation = VK_NULL_HANDLE;
};

/*

void buffer_del(const renderctx *ctx, buffer &b);
std::optional<buffer> buffer_new_uniform(const renderctx *ctx, size_t size);
std::optional<buffer> buffer_new_vertex(const renderctx *ctx, size_t size);
std::optional<buffer> buffer_new_index(const renderctx *ctx, size_t size);
std::optional<buffer> buffer_new_staging(const renderctx *ctx, size_t size, void **ptr);
void buffer_copy(const renderctx *ctx, buffer &dst, buffer &src, size_t size);
void buffer_upload(const renderctx *ctx, buffer &b, const void *src, size_t sz);

*/

}}

