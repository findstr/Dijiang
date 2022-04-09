#include <stdio.h>
#include "renderctx.h"
#include "cmdbuf.h"
#include "vk_buffer.h"
namespace engine {
namespace vulkan {
/*
void
buffer_del(const renderctx *ctx, buffer &b)
{
	vmaDestroyBuffer(ctx->allocator, b.handle, b.allocation);
}


std::optional<buffer>
buffer_new_uniform(const renderctx *ctx, size_t size)
{
	buffer b;
	VkResult ret;
	VkBufferCreateInfo bi = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
	bi.size = size;
	bi.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	VmaAllocationCreateInfo ci = {};
	ci.usage = VMA_MEMORY_USAGE_AUTO;
	ci.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
	if ((ret = vmaCreateBuffer(ctx->allocator, &bi, &ci,
		&b.handle, &b.allocation, nullptr)) != VK_SUCCESS) {
		fprintf(stderr, "[render] buffer_uniform_new size:%ld fail:%d\n",
			size, ret);
		return std::nullopt;
	} else {
		return b;
	}
}

std::optional<buffer>
buffer_new_vertex(const renderctx *ctx, size_t size)
{
	buffer b;
	VkResult ret;
	VkBufferCreateInfo bi{};
	bi.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bi.size = size;
	bi.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo ci = {};
	ci.usage = VMA_MEMORY_USAGE_AUTO;
	ci.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

	if ((ret = vmaCreateBuffer(ctx->allocator, &bi, &ci,
		&b.handle, &b.allocation, nullptr)) != VK_SUCCESS) {
		fprintf(stderr, "[render] buffer_new_vertex size:%ld fail:%d\n",
			size, ret);
		return std::nullopt;
	} else {
		return b;
	}
}

std::optional<buffer>
buffer_new_index(const renderctx *ctx, size_t size)
{
	buffer b;
	VkResult ret;
	VkBufferCreateInfo bi{};
	bi.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bi.size = size;
	bi.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	bi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo ci = {};
	ci.usage = VMA_MEMORY_USAGE_AUTO;
	ci.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

	if ((ret = vmaCreateBuffer(ctx->allocator, &bi, &ci,
		&b.handle, &b.allocation, nullptr)) != VK_SUCCESS) {
		fprintf(stderr, "[render] buffer_new_vertex size:%ld fail:%d\n",
			size, ret);
		return std::nullopt;
	} else {
		return b;
	}
}

std::optional<buffer>
buffer_new_staging(const renderctx *ctx, size_t size, void **ptr)
{
	buffer b;
	VkResult ret;
	VkBufferCreateInfo bci = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bci.size = size;
	bci.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	VmaAllocationCreateInfo vaci = {};
	vaci.usage = VMA_MEMORY_USAGE_AUTO;
	vaci.flags =
		VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
		VMA_ALLOCATION_CREATE_MAPPED_BIT;
	VmaAllocationInfo allocInfo;
	ret = vmaCreateBuffer(ctx->allocator, &bci, &vaci,
		&b.handle, &b.allocation, &allocInfo);
	if (ret != VK_SUCCESS) {
		*ptr = nullptr;
		return std::nullopt;
	}
	*ptr = allocInfo.pMappedData;
	return b;
}

void
buffer_copy(const renderctx *ctx, buffer &dst, buffer &src, size_t size)
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = ctx->commandpool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer = cmdbuf_single_begin(ctx);
	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, src.handle, dst.handle, 1, &copyRegion);
	cmdbuf_single_end(ctx, commandBuffer);
}

void
buffer_upload(const renderctx *ctx, buffer &b, const void *src, size_t sz)
{
	void* mappedData;
	vmaMapMemory(ctx->allocator, b.allocation, &mappedData);
	memcpy(mappedData, src, sz);
	vmaUnmapMemory(ctx->allocator, b.allocation);
}*/

vk_buffer::~vk_buffer()
{
	vmaDestroyBuffer(vk_ctx->allocator, handle, allocation);
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
vk_buffer::upload(const void *src, size_t size)
{
	void* mappedData = map();
	memcpy(mappedData, src, size);
	unmap();
}

vk_buffer::vk_buffer(enum vk_buffer::type t, size_t sz)
{
	VkResult ret;
	VkBufferCreateInfo bi = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
	VmaAllocationCreateInfo ci = {};
	bi.size = sz;
	ci.usage = VMA_MEMORY_USAGE_AUTO;
	switch(t) {
	case vk_buffer::type::UNIFORM:
		bi.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		ci.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
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

}}

