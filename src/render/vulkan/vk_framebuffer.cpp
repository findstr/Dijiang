#include "vk_framebuffer.h"

namespace engine {
namespace vulkan {

void
vk_framebuffer::init_semaphores()
{
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	for (size_t i = 0; i < sems.size(); i++) {
		vkCreateSemaphore(VK_CTX.device, &semaphoreInfo, nullptr, &sems[i].imgavailable);
		vkCreateSemaphore(VK_CTX.device, &semaphoreInfo, nullptr, &sems[i].renderfin);
		vkCreateFence(VK_CTX.device, &fenceInfo, nullptr, &sems[i].fence);
		assert(sems[i].imgavailable && sems[i].renderfin && sems[i].fence);
	}
}

void
vk_framebuffer::destroy_semaphores()
{
	for (size_t i = 0; i < sems.size(); i++) {
		auto &sem = sems[i];
		vkDestroySemaphore(VK_CTX.device, sem.imgavailable, nullptr);
		vkDestroySemaphore(VK_CTX.device, sem.renderfin, nullptr);
		vkDestroyFence(VK_CTX.device, sem.fence, nullptr);
		sem.imgavailable = VK_NULL_HANDLE;
		sem.renderfin = VK_NULL_HANDLE;
		sem.fence = VK_NULL_HANDLE;
	}
}

void
vk_framebuffer::init_depth_textures()
{
	auto &depth = depth_texture;
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = static_cast<uint32_t>(VK_CTX.swapchain.extent.width);
	imageInfo.extent.height = static_cast<uint32_t>(VK_CTX.swapchain.extent.height);
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = VK_CTX.depth_format;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.flags = 0;

	VmaAllocationCreateInfo vaci = {};
	vaci.usage = VMA_MEMORY_USAGE_AUTO;
	vaci.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
	vaci.priority = 1.0f;

	auto ret = vmaCreateImage(VK_CTX.allocator, &imageInfo, &vaci, &depth.image, &depth.allocation, nullptr);
	assert(ret == VK_SUCCESS);

	VkImageViewCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.image = depth.image;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = VK_CTX.depth_format;
	createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 1;
	ret = vkCreateImageView(VK_CTX.device, &createInfo, nullptr, &depth.view);
	assert(ret == VK_SUCCESS);
}

void
vk_framebuffer::destroy_depth_textures()
{
	vkDestroyImageView(VK_CTX.device, depth_texture.view, nullptr);
	vmaDestroyImage(VK_CTX.allocator, depth_texture.image, depth_texture.allocation);
}

void
vk_framebuffer::init_frame_buffers()
{
	frame_buffers.resize(VK_CTX.swapchain.imageviews.size());
	for (size_t i = 0; i < frame_buffers.size(); i++) {
		std::array<VkImageView, 2> attachments = {
			VK_CTX.swapchain.imageviews[i],
			depth_texture.view,
		};

		VkFramebufferCreateInfo framebuffInfo = {};
		framebuffInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffInfo.renderPass = VK_CTX.render_pass; //指定Framebuffer需要兼容的renderPass,并不定是这个Renderpass会使用这个Framebuffer
		framebuffInfo.attachmentCount = attachments.size();
		framebuffInfo.pAttachments = attachments.data();
		framebuffInfo.width = VK_CTX.swapchain.extent.width;
		framebuffInfo.height = VK_CTX.swapchain.extent.height;
		framebuffInfo.layers = 1;
		auto ret = vkCreateFramebuffer(VK_CTX.device, &framebuffInfo, nullptr, &frame_buffers[i]);
		assert(ret == VK_SUCCESS);
	}
}

void
vk_framebuffer::destroy_frame_buffers()
{
	for (size_t i = 0; i < frame_buffers.size(); i++) {
		vkDestroyFramebuffer(VK_CTX.device, frame_buffers[i], nullptr);
		frame_buffers[i] = VK_NULL_HANDLE;
	}
}
	
void 
vk_framebuffer::resize()
{
	destroy_frame_buffers();
	destroy_depth_textures();
	init_depth_textures();
	init_frame_buffers();
}


vk_framebuffer::vk_framebuffer()
{
	init_semaphores();
	init_depth_textures();
	init_frame_buffers();
}
	
vk_framebuffer::~vk_framebuffer()
{
	destroy_frame_buffers();
	destroy_depth_textures();
	destroy_semaphores();
}

vk_framebuffer::acquire_result
vk_framebuffer::acquire()
{
	vk_ctx_frame_begin();
	auto &sem = sems[VK_CTX.frame_index];
	vkWaitForFences(VK_CTX.device, 1, &sem.fence, VK_TRUE, UINT64_MAX);
	auto result = vkAcquireNextImageKHR(VK_CTX.device,
		VK_CTX.swapchain.handle,
		UINT64_MAX,
		sem.imgavailable,
		VK_NULL_HANDLE,
		&image_index);
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		return acquire_result::RECREATE_SWAPCHAIN;
	} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		return acquire_result::NOT_READY;
	} 
	return acquire_result::SUCCESS;
}

int
vk_framebuffer::submit(VkCommandBuffer cmdbuf)
{
	auto &sem = sems[VK_CTX.frame_index];
	VkSemaphore waitSemaphores[] = { sem.imgavailable };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdbuf;
	VkSemaphore signalSemaphores[] = { sem.renderfin };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;
	vkResetFences(VK_CTX.device, 1, &sem.fence);
	auto result = vkQueueSubmit(VK_CTX.graphicsqueue, 1, &submitInfo, sem.fence);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer");
	}
	VkSwapchainKHR swapChains[] = { VK_CTX.swapchain.handle };
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &image_index;
	presentInfo.pResults = nullptr;
	result = vkQueuePresentKHR(VK_CTX.presentqueue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		return -1;
	} else if (result != VK_SUCCESS) {
		return -1;
	}
	return 0;
}

VkFramebuffer
vk_framebuffer::current() const
{
	return frame_buffers[image_index];
}


}}

