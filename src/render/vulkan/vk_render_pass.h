#pragma once
#include <memory>
#include <array>
#include "vk_ctx.h"
#include "vk_framebuffer.h"
#include "render/render_pass.h"

namespace engine {
namespace vulkan {

class vk_render_pass : public i_render_pass {
public:
	~vk_render_pass() {
		vkDestroyRenderPass(VK_CTX.device, renderpass, nullptr);
	}
	VkRenderPass handle() { return renderpass; }
	void begin(float width, float height) override 
	{
		auto &cmdbuf = VK_CTX.cmdbuf;
		VkFramebuffer framebuffer = vulkan::VK_FRAMEBUFFER.current();
		VkRenderPassBeginInfo renderPassInfo{};
		std::array<VkClearValue, 2> clearColor{};
		clearColor[0].color =  {{0.0f, 0.0f, 0.0f, 1.0f}} ;
		clearColor[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderpass;
		renderPassInfo.framebuffer = framebuffer;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent.width = width * vulkan::VK_CTX.swapchain.extent.width;
		renderPassInfo.renderArea.extent.height = height * vulkan::VK_CTX.swapchain.extent.height;
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearColor.size());
		renderPassInfo.pClearValues = clearColor.data();
		vkCmdBeginRenderPass(cmdbuf, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	}
	void end() override 
	{
		auto &cmdbuf = vulkan::VK_CTX.cmdbuf;
		vkCmdEndRenderPass(vulkan::VK_CTX.cmdbuf);
	}
protected:
	VkRenderPass renderpass = VK_NULL_HANDLE;
};


}}

