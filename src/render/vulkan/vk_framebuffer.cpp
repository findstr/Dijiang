#include "render_pass.h"
#include "render_texture.h"
#include "vk_native.h"
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
	depth_texture.reset(new vk_depth_buffer(VK_CTX.swapchain.extent.width, VK_CTX.swapchain.extent.height, texture_format::D32));
}

void
vk_framebuffer::destroy_depth_textures()
{
	depth_texture.reset(nullptr);
}

void
vk_framebuffer::init_frame_buffers()
{
	frame_buffers.resize(VK_CTX.swapchain.imageviews.size());
	for (size_t i = 0; i < frame_buffers.size(); i++) {
		std::array<VkImageView, 2> attachments = {
			VK_CTX.swapchain.imageviews[i],
			depth_texture->native.view,
		};

		VkFramebufferCreateInfo framebuffInfo = {};
		framebuffInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffInfo.renderPass = native_of(RENDER_PASS.get(render_pass::FORWARD)).handle(); //指定Framebuffer需要兼容的renderPass,并不定是这个Renderpass会使用这个Framebuffer
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
	
void
vk_framebuffer::set_render_texture(render_texture *rt)
{
	this->rendertexture = rt;
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
	if (rendertexture != nullptr)
		return ((vk_render_texture *)rendertexture)->framebuffer();
	else
		return frame_buffers[image_index];
}

}}

