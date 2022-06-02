#include <stdio.h>
#include "renderctx.h"
#include "cmdbuf.h"
#include "vk_buffer.h"
namespace engine {
namespace vulkan {

void
vk_buffer::destroy()
{
	if (handle != VK_NULL_HANDLE) {
		vmaDestroyBuffer(vk_ctx->allocator, handle, allocation);
		handle = VK_NULL_HANDLE;
	}
}

vk_buffer::~vk_buffer()
{
	destroy();
}

void
vk_buffer::copy_from(VkCommandBuffer cmdbuf, vk_buffer *src)
{
	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = src->size;
	vkCmdCopyBuffer(cmdbuf, src->handle, handle, 1, &copyRegion);
}

void
vk_buffer::copy_from(vk_buffer *src)
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = vk_ctx->commandpool;
	allocInfo.commandBufferCount = 1;
	assert(size == src->size);
	VkCommandBuffer commandBuffer = cmdbuf_single_begin(vk_ctx);
	copy_from(commandBuffer, src);
	cmdbuf_single_end(vk_ctx, commandBuffer);
}

void *
vk_buffer::map()
{
	void* mappedData;
	vmaMapMemory(vk_ctx->allocator, allocation, &mappedData);
	return mappedData;
}

void
vk_buffer::unmap()
{
	vmaUnmapMemory(vk_ctx->allocator, allocation);
}

void
vk_buffer::unmap(int offset, int size)
{
	vmaFlushAllocation(vk_ctx->allocator, allocation, offset, size);
	vmaUnmapMemory(vk_ctx->allocator, allocation);
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
	case vk_buffer::type::DYNAMIC:
		bi.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
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
	default:
		assert(!"unsupport vk_buffer type");
		break;
	}
	ret = vmaCreateBuffer(vk_ctx->allocator, &bi, &ci, &handle, &allocation, nullptr);
	assert(ret == VK_SUCCESS);
	this->size = sz;

}

vk_buffer::vk_buffer(enum vk_buffer::type t, size_t sz)
{
	create(t, sz);
}

}}

