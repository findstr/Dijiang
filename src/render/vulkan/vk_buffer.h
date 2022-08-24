#pragma once
#include "conf.h"
#include "vk_mem_alloc.h"
#include "utils/delete_queue.h"
namespace engine {
namespace vulkan {

class vk_buffer {
public:
	enum type {
		NONE,
		UNIFORM,
		UNIFORM_DYNAMIC,
		STORAGE,
		VERTEX,
		INDEX,
		STAGING,
		INDIRECT,
	};
public:
	vk_buffer() {}
	vk_buffer(vk_buffer&& a) {
		clear();
		this->size = a.size;
		this->type = a.type;
		this->handle = a.handle;
		this->allocation = a.allocation;
		a.size = 0;
		a.type = type::NONE;
		a.handle = VK_NULL_HANDLE;
		a.allocation = VK_NULL_HANDLE;
	}
	vk_buffer(type t, size_t size);
	~vk_buffer();
	void *map();
	void unmap();
	void unmap(int offset, int size);
	void create(type t, size_t size);
	void destroy();
	void upload(const void *src, size_t size);
	void copy_from(vk_buffer *src);
	void copy_from(vk_buffer *src, size_t srcoffset, size_t dstoffset, size_t size);
public:
	vk_buffer &operator = (vk_buffer &&b);
public:
	size_t size = 0;
	type type = type::NONE;
	VkBuffer handle = VK_NULL_HANDLE;
	VmaAllocation allocation = VK_NULL_HANDLE;
private:
	friend delete_queue<vk_buffer>;
	void clear();
};

}}

