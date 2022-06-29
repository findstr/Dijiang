#include "framesync.h"

namespace engine {
namespace vulkan {

struct framesync {
	VkSemaphore imgavailable = VK_NULL_HANDLE;
	VkSemaphore renderfin = VK_NULL_HANDLE;
	VkFence fence = VK_NULL_HANDLE;
};

void
framesync_del(framesync *fs)
{
	vkDestroySemaphore(VK_CTX.logicdevice, fs->imgavailable, nullptr);
	vkDestroySemaphore(VK_CTX.logicdevice, fs->renderfin, nullptr);
	vkDestroyFence(VK_CTX.logicdevice, fs->fence, nullptr);
	delete fs;
}

framesync *
framesync_new()
{
	auto *fs = new framesync();
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	vkCreateSemaphore(VK_CTX.logicdevice, &semaphoreInfo, nullptr, &fs->imgavailable);
	vkCreateSemaphore(VK_CTX.logicdevice, &semaphoreInfo, nullptr, &fs->renderfin);
	vkCreateFence(VK_CTX.logicdevice, &fenceInfo, nullptr, &fs->fence);
	if (!(fs->renderfin && fs->imgavailable && fs->fence)) {
		framesync_del(fs);
		return nullptr;
	}
	return fs;

}


int
framesync_aquire(const framesync *fs, int *image_index)
{
	uint32_t index;
	vkWaitForFences(VK_CTX.logicdevice, 1, &fs->fence, VK_TRUE, UINT64_MAX);
	auto result = vkAcquireNextImageKHR(VK_CTX.logicdevice,
		VK_CTX.swapchain.handle,
		UINT64_MAX,
		fs->imgavailable,
		VK_NULL_HANDLE,
		&index);
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		return -1;
	} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		return -1;
	}
	*image_index = index;
	return 0;
}

int
framesync_submit(
	const framesync *fs,

	VkCommandBuffer cmdbuf,

	int image_index)
{
	uint32_t imageindex = image_index;
	VkSemaphore waitSemaphores[] = { fs->imgavailable };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdbuf;
	VkSemaphore signalSemaphores[] = { fs->renderfin };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;
	vkResetFences(VK_CTX.logicdevice, 1, &fs->fence);
	if (vkQueueSubmit(VK_CTX.graphicsqueue, 1, &submitInfo, fs->fence) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer");
	}
	VkSwapchainKHR swapChains[] = { VK_CTX.swapchain.handle };
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageindex;
	presentInfo.pResults = nullptr;
	auto result = vkQueuePresentKHR(VK_CTX.presentqueue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		return -1;
		/*TODO:
		framebufferResized = false;
		recreateSwapChain();
		*/
	} else if (result != VK_SUCCESS) {
		return -1;
	}
	return 0;
}


}}

