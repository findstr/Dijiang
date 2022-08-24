#include <stdio.h>
#include "vk_buffer.h"

#include "vk_ctx.h"
namespace engine {
namespace vulkan {


void
vk_buffer::destroy()
{
	if (this->handle != VK_NULL_HANDLE)
		VK_CTX.buf_delq->exec(*this);
}

void
vk_buffer::clear()
{
	if (handle != VK_NULL_HANDLE) {
		vmaDestroyBuffer(VK_CTX.allocator, handle, allocation);
		handle = VK_NULL_HANDLE;
	}
}

vk_buffer::~vk_buffer()
{
	destroy();
}

void
vk_buffer::copy_from(vk_buffer *src)
{
	assert(size == src->size);
	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = src->size;
	vkCmdCopyBuffer(VK_CTX.cmdbuf, src->handle, handle, 1, &copyRegion);
}

void
vk_buffer::copy_from(vk_buffer *src, size_t srcoffset, size_t dstoffset, size_t size)
{
	assert((srcoffset + size) <= src->size);
	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = srcoffset;
	copyRegion.dstOffset = dstoffset;
	copyRegion.size = size;
	vkCmdCopyBuffer(VK_CTX.cmdbuf, src->handle, handle, 1, &copyRegion);
}

void *
vk_buffer::map()
{
	void* mappedData;
	vmaMapMemory(VK_CTX.allocator, allocation, &mappedData);
	return mappedData;
}

void
vk_buffer::unmap()
{
	vmaUnmapMemory(VK_CTX.allocator, allocation);
}

void
vk_buffer::unmap(int offset, int size)
{
	vmaFlushAllocation(VK_CTX.allocator, allocation, offset, size);
	vmaUnmapMemory(VK_CTX.allocator, allocation);
}

void
vk_buffer::upload(const void *src, size_t size)
{
	void* mappedData = map();
	memcpy(mappedData, src, size);
	unmap();
}

void
vk_buffer::create(enum vk_buffer::type t, size_t sz)
{
	VkResult ret;
	VkBufferCreateInfo bi = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
	VmaAllocationCreateInfo ci = {};
	destroy();
	bi.size = sz;
	ci.usage = VMA_MEMORY_USAGE_AUTO;
	type = t;
	switch(t) {
	case vk_buffer::type::UNIFORM:
		bi.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		ci.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT;
		break;
	case vk_buffer::type::UNIFORM_DYNAMIC:
		bi.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		ci.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
		ci.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT;
		break;
	case vk_buffer::type::STORAGE:
		bi.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		ci.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
		ci.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT;
		break;
	case vk_buffer::type::VERTEX:
		bi.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		bi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		ci.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
		break;
	case vk_buffer::type::INDEX:
		bi.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		bi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		ci.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
		break;
	case vk_buffer::type::STAGING:
		bi.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		ci.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
		break;
	case vk_buffer::type::INDIRECT:
		bi.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		ci.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
		break;
	default:
		assert(!"unsupport vk_buffer type");
		break;
	}
	ret = vmaCreateBuffer(VK_CTX.allocator, &bi, &ci, &handle, &allocation, nullptr);
	assert(ret == VK_SUCCESS);
	this->size = sz;

}

vk_buffer::vk_buffer(enum vk_buffer::type t, size_t sz)
{
	create(t, sz);
}

vk_buffer &vk_buffer::operator = (vk_buffer &&src)
{
	size = src.size;
	type = src.type;
	handle = src.handle;
	allocation = src.allocation;

	src.size = 0;
	src.type = type::NONE;
	src.handle = VK_NULL_HANDLE;
	src.allocation = VK_NULL_HANDLE;
	return *this;
}

}}

