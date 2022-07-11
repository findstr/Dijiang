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
		DYNAMIC,
		VERTEX,
		INDEX,
		STAGING,
	};
public:
	vk_buffer() {}
	vk_buffer(type t, size_t size);
	~vk_buffer();
	void *map();
	void unmap();
	void unmap(int offset, int size);
	void create(type t, size_t size);
	void destroy();
	void upload(const void *src, size_t size);
	void copy_from(vk_buffer *src);
	void copy_from(VkCommandBuffer cmdbuf, vk_buffer *src);
public:
	vk_buffer &operator = (vk_buffer &&b);
public:
	size_t size = 0;
	type type = type::NONE;
	VkBuffer handle = VK_NULL_HANDLE;
	VmaAllocation allocation = VK_NULL_HANDLE;
};

}}

