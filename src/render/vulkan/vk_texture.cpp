#include <optional>
#include "vk_object.h"
#include "samplermgr.h"
#include "cmdbuf.h"
#include "vk_buffer.h"
#include "vk_texture.h"
#include "renderctx.h"

namespace engine {
namespace vulkan {

void
texture_del(textureEx &tex)
{
	return texture_del(renderctx_get(), tex);
}
void
texture_del(const renderctx *ctx, textureEx &tex)
{
	if (tex.sampler != VK_NULL_HANDLE)
		samplermgr_delsampler(ctx, tex.sampler);
	vkDestroyImageView(ctx->logicdevice, tex.view, nullptr);
	vmaDestroyImage(ctx->allocator, tex.image, tex.allocation);
}

VkImageView
texture_new_view(const renderctx *ctx,
	VkImage image,
	VkFormat format,
	VkImageAspectFlags aspectFlags,
	uint32_t mipLevels)
{
	VkImageView imageView;
	VkImageViewCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.image = image;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = format;
	createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.subresourceRange.aspectMask = aspectFlags;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = mipLevels;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 1;
	if (vkCreateImageView(ctx->logicdevice, &createInfo, nullptr, &imageView) != VK_SUCCESS)
		return VK_NULL_HANDLE;
	return imageView;
}


std::optional<textureEx>
texture_new(const renderctx *ctx,
	uint32_t width, uint32_t height,
	const texture_setting &setting)
{
	textureEx tex;
	VkFormat format = setting.format;
	uint32_t miplevels = setting.mipmap_levels;
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = static_cast<uint32_t>(width);
	imageInfo.extent.height = static_cast<uint32_t>(height);
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = miplevels;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format,
	imageInfo.tiling = setting.tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = setting.usage,
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.flags = 0;

	VmaAllocationCreateInfo vaci = {};
	vaci.usage = VMA_MEMORY_USAGE_AUTO;
	vaci.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
	vaci.priority = 1.0f;
	auto ret = vmaCreateImage(ctx->allocator, &imageInfo, &vaci, &tex.image, &tex.allocation, nullptr);
	printf("texture_new:%d\n", ret);
	if (ret != VK_SUCCESS)
		return std::nullopt;
	tex.view = texture_new_view(ctx, tex.image, format, setting.aspectflags, miplevels);
	tex.sampler = samplermgr_newsampler(ctx, setting);
	return tex;
}

std::optional<textureEx> texture_new(
	uint32_t width, uint32_t height,
	const texture_setting &setting)
{
	return texture_new(renderctx_get(), width, height, setting);
}


std::optional<textureEx>
texture_new_depth(
	const renderctx *ctx,
	uint32_t width, uint32_t height,
	const texture_setting &setting)
{
	textureEx tex;
	VkImageCreateInfo imageInfo{};
	VkFormat format = setting.format;
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = static_cast<uint32_t>(width);
	imageInfo.extent.height = static_cast<uint32_t>(height);
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = setting.mipmap_levels;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = setting.tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = setting.usage,
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.flags = 0;

	VmaAllocationCreateInfo vaci = {};
	vaci.usage = VMA_MEMORY_USAGE_AUTO;
	vaci.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
	vaci.priority = 1.0f;

	auto ret = vmaCreateImage(ctx->allocator, &imageInfo, &vaci, &tex.image, &tex.allocation, nullptr);
	if (ret != VK_SUCCESS)
		return std::nullopt;
	tex.view = texture_new_view(ctx, tex.image, format, setting.aspectflags, setting.mipmap_levels);
	tex.sampler = VK_NULL_HANDLE;
	return tex;
}

bool hasStencilComponent(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void
texture_transition_layout(
	const renderctx *ctx,
	textureEx &tex, VkFormat format, int miplevels,
	VkImageLayout from, VkImageLayout to)
{
	VkCommandBuffer commandBuffer = cmdbuf_single_begin(ctx);

	VkPipelineStageFlags srcStage;
	VkPipelineStageFlags dstStage;

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = from;
	barrier.newLayout = to;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = tex.image;
	if (to == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		if (hasStencilComponent(format))
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	} else {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = miplevels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = 0;
	if (from == VK_IMAGE_LAYOUT_UNDEFINED && to == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (from == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && to == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} else if (from == VK_IMAGE_LAYOUT_UNDEFINED && to == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	} else {
		throw std::invalid_argument("unsupport layout layout transition");
	}
	vkCmdPipelineBarrier(
		commandBuffer,
		srcStage, dstStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);
	cmdbuf_single_end(ctx, commandBuffer);
}

void
texture_gen_mipmap(const renderctx *ctx,
	textureEx &tex, VkFormat imageFormat,
	int32_t width, int32_t height, int32_t mipLevels)
{
	if (mipLevels == 0)
		return ;
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(ctx->phydevice, imageFormat, &formatProperties);
	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
		throw std::runtime_error("texture image format does not support linear blitting!");
	}
	VkCommandBuffer commandBuffer = cmdbuf_single_begin(ctx);
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = tex.image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;
	int32_t mipWidth = width;
	int32_t mipHeight = height;
	for (uint32_t i = 1; i < mipLevels; i++) {
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		VkImageBlit blit{};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;

		mipWidth = mipWidth > 1 ? mipWidth / 2 : 1;
		mipHeight = mipHeight > 1 ? mipHeight / 2 : 1;

		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { mipWidth, mipHeight, 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		barrier.subresourceRange.baseMipLevel = i;
		barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		vkCmdBlitImage(commandBuffer,
			tex.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			tex.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit,
			VK_FILTER_LINEAR);

		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);
	}
	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);
	cmdbuf_single_end(ctx, commandBuffer);
}

void
texture_fill(const renderctx *ctx,
	textureEx &tex, const vk_buffer &buf,
	int32_t width, int32_t height)
{
	VkCommandBuffer commandBuffer = cmdbuf_single_begin(ctx);

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { (uint32_t)width, (uint32_t)height, 1 };

	vkCmdCopyBufferToImage(
		commandBuffer,
		buf.handle,
		tex.image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);

	cmdbuf_single_end(ctx, commandBuffer);
}

void
texture_upload(textureEx &tex, int width, int height, void *pixel, size_t sz, int miplevels)
{
	void *data;
	auto *ctx = renderctx_get();
	vk_buffer staging(vk_buffer::STAGING, sz);
	staging.upload(pixel, static_cast<uint32_t>(sz));
	texture_transition_layout(ctx, tex,
		VK_FORMAT_R8G8B8A8_SRGB, 1,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	texture_fill(ctx, tex, staging, width, height);
	texture_gen_mipmap(ctx, tex, VK_FORMAT_R8G8B8A8_SRGB, width, height, miplevels);
}

void
texture::apply()
{
	auto *ctx = renderctx_get();
	if (handle)
		texture_del(*handle);
	texture_setting setting = {
		.wrap_mode = TEX_WRAP_REPEAT,
		.filter_mode = TEX_FILTER_POINT,
		.mipmap_levels = (int)miplevels,
		.aniso_level = 0,
		.format = VK_FORMAT_R8G8B8A8_SRGB,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		.aspectflags = VK_IMAGE_ASPECT_COLOR_BIT,
	};
	handle = texture_new(ctx, width_, height_, setting);
	texture_upload(*handle, width_, height_, pixel.data(), pixel.size(), miplevels);
}



}

namespace render {

texture *
texture::create(int width, int height,
	texture_format format,
	bool linear, int miplevels)
{
	texture *tex = new vulkan::texture();
	tex->width_ = width;
	tex->height_ = height;
	tex->linear = false;
	tex->miplevels = miplevels;
	return tex;
}

}

}

