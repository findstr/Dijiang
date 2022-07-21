#include "render_pass.h"
#include "vk_ctx.h"
#include "vk_render_texture.h"
#include "vk_native.h"
#include "vk_swapchain.h"

namespace engine {
namespace vulkan {

VkImageView
vk_swapchain::new_view(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
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
	if (vkCreateImageView(device, &createInfo, nullptr, &imageView) != VK_SUCCESS)
		return VK_NULL_HANDLE;
	return imageView;
}

vk_swapchain::support_details
vk_swapchain::query_support(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	uint32_t formatCount;
	uint32_t presentModeCount;
	support_details details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}
	return details;
}

VkExtent2D 
vk_swapchain::choose_extent(const VkSurfaceCapabilitiesKHR &cap, int width, int height)
{
	if (cap.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return cap.currentExtent;
	} else {
		VkExtent2D ext = {(uint32_t)width, (uint32_t)height};
		ext.width = std::max(cap.minImageExtent.width, std::min(cap.maxImageExtent.width, ext.width));
		ext.height = std::max(cap.minImageExtent.height, std::min(cap.minImageExtent.height, ext.height));
		return ext;
	}
}
	
VkSurfaceFormatKHR
vk_swapchain::choose_surface_format(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
	for (const auto &fmt : availableFormats) {
		if (fmt.format == VK_FORMAT_B8G8R8A8_SRGB && fmt.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return fmt;
	}
	return availableFormats[0];
}

VkPresentModeKHR
vk_swapchain::choose_present_mode(const std::vector<VkPresentModeKHR> &list)
{
	VkPresentModeKHR besetMode = VK_PRESENT_MODE_FIFO_KHR;
	for (auto const p : list) {
		if (p == VK_PRESENT_MODE_MAILBOX_KHR)
			return p;
		if (p == VK_PRESENT_MODE_IMMEDIATE_KHR)
			besetMode = p;
	}
	return besetMode;
}


void
vk_swapchain::destroy_swapchain()
{
	vkDestroySwapchainKHR(VK_CTX.device, handle, nullptr);
}

void
vk_swapchain::create_swapchain(int width, int height)
{
	auto chain = query_support(phydevice, surface);
	auto surfaceFormat = choose_surface_format(chain.formats);
	auto presentMode = choose_present_mode(chain.presentModes);
	extent = choose_extent(chain.capabilities, width, height);
	uint32_t imageCount = chain.capabilities.minImageCount + 1;
	if (chain.capabilities.maxImageCount > -1 && imageCount > chain.capabilities.maxImageCount)
		imageCount = chain.capabilities.maxImageCount;
	printf("createSwapChain:%d - %d\n", chain.capabilities.minImageCount, chain.capabilities.maxImageCount);
	imageformat = surfaceFormat.format;
	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	uint32_t queueIndices[] = {(uint32_t)VK_CTX.graphicsfamily, (uint32_t)VK_CTX.presentfamily};
	if (VK_CTX.graphicsfamily != VK_CTX.presentfamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueIndices;
	} else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}
	createInfo.preTransform = chain.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;
	auto result = vkCreateSwapchainKHR(device, &createInfo, nullptr, &handle);
	if (result != VK_SUCCESS)
		throw std::runtime_error("failed to create swap chain");
	std::vector<VkImage> images;
	vkGetSwapchainImagesKHR(device, handle, &imageCount, nullptr);
	images.resize(imageCount);
	vkGetSwapchainImagesKHR(device, handle, &imageCount, images.data());
	imageviews.resize(images.size());
	for (size_t i = 0; i < images.size(); i++) {
		imageviews[i] = new_view(images[i], imageformat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}
}
	
void
vk_swapchain::destroy_renderpass()
{
	vkDestroyRenderPass(VK_CTX.device, render_pass, nullptr);
}

void
vk_swapchain::init_renderpass()
{
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = imageformat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = nullptr;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;;

	std::array<VkAttachmentDescription, 1> attachments = { colorAttachment }; //Should keep order of renderframe

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	auto result = vkCreateRenderPass(VK_CTX.device, &renderPassInfo, nullptr, &render_pass);
	assert(result == VK_SUCCESS);
}


void
vk_swapchain::init_semaphores()
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
vk_swapchain::destroy_semaphores()
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
vk_swapchain::init_frame_buffers()
{
	frame_buffers.resize(imageviews.size());
	for (size_t i = 0; i < frame_buffers.size(); i++) {
		std::array<VkImageView, 1> attachments = {
			imageviews[i],
		};
		VkFramebufferCreateInfo framebuffInfo = {};
		framebuffInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffInfo.renderPass = render_pass; //指定Framebuffer需要兼容的renderPass,并不定是这个Renderpass会使用这个Framebuffer
		framebuffInfo.attachmentCount = attachments.size();
		framebuffInfo.pAttachments = attachments.data();
		framebuffInfo.width = extent.width;
		framebuffInfo.height = extent.height;
		framebuffInfo.layers = 1;
		auto ret = vkCreateFramebuffer(VK_CTX.device, &framebuffInfo, nullptr, &frame_buffers[i]);
		assert(ret == VK_SUCCESS);
	}
}

void
vk_swapchain::destroy_frame_buffers()
{
	for (size_t i = 0; i < frame_buffers.size(); i++) {
		vkDestroyFramebuffer(VK_CTX.device, frame_buffers[i], nullptr);
		frame_buffers[i] = VK_NULL_HANDLE;
	}
}
	
void 
vk_swapchain::resize(int width, int height)
{
	destroy_swapchain();
	destroy_frame_buffers();
	create_swapchain(width, height);
	init_frame_buffers();
}

void
vk_swapchain::init(VkInstance instace, VkPhysicalDevice phydevice, VkDevice device, VkSurfaceKHR surface, int width, int height)
{
	this->instance = instace;
	this->phydevice = phydevice;
	this->device = device;
	this->surface = surface;
	create_swapchain(width, height);
	init_renderpass();
	init_semaphores();
	init_frame_buffers();
}

VkFramebuffer 
vk_swapchain::framebuffer() const
{
	assert(image_index >= 0);
	return frame_buffers[image_index];
}

vk_swapchain::acquire_result
vk_swapchain::acquire()
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
vk_swapchain::submit(VkCommandBuffer cmdbuf)
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
	image_index = -1;
	return 0;
}

void
vk_swapchain::begin()
{
	auto &cmdbuf = vulkan::VK_CTX.cmdbuf;
	VkRenderPassBeginInfo renderPassInfo{};
	std::array<VkClearValue, 2> clearColor{};
	clearColor[0].color =  {{0.0f, 0.0f, 0.0f, 1.0f}} ;
	clearColor[1].depthStencil = { 1.0f, 0 };
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = render_pass;
	renderPassInfo.framebuffer = framebuffer();
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = extent;
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearColor.size());
	renderPassInfo.pClearValues = clearColor.data();
	vkCmdBeginRenderPass(cmdbuf, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void
vk_swapchain::end()
{
	vkCmdEndRenderPass(VK_CTX.cmdbuf);
}

void
vk_swapchain::destroy()
{
	destroy_swapchain();
	destroy_renderpass();
	destroy_semaphores();
	destroy_frame_buffers();
}


}}

