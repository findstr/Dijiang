#include <optional>
#include "vk_ctx.h"
#include "vk_format.h"
#include "vk_render_texture.h"

namespace engine {
namespace vulkan {

void
vk_render_texture::create_colorbuffer()
{
	assert(format != texture_format::D32 && format != texture_format::D24S8 && format != texture_format::D32S8);
	for (int i = 0; i < conf::MAX_FRAMES_IN_FLIGHT; i++) {
		if (enable_msaa) {
			msaa_buffer[i].create(this, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1, VK_CTX.msaaSamples);
			msaa_buffer[i].sampler(this);
		}
		color_buffer[i].create(this, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		color_buffer[i].sampler(this);
	}
}

void
vk_render_texture::create_depthbuffer()
{
	if (depth_format == texture_format::INVALID)
		return ;
	render::texture tex(this->type_);
	tex = *this;
	assert(depth_format == texture_format::D32 || depth_format == texture_format::D24S8 || depth_format == texture_format::D32S8);
	tex.linear = true;
	tex.miplevels = 1;
	tex.anisolevels = 1;
	tex.format = depth_format;
	tex.wrap_mode_u = texture_wrap::CLAMP;
	tex.wrap_mode_v = texture_wrap::CLAMP;
	tex.wrap_mode_w = texture_wrap::CLAMP;
	tex.filter_mode = texture_filter::BILINEAR;
	if (enable_msaa) 
		depth_buffer.create(&tex, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 1, VK_CTX.msaaSamples);
	else
		depth_buffer.create(&tex, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
	depth_buffer.sampler(&tex);
}

void
vk_render_texture::apply()
{
	vk_format color_format(this->format, this->linear);
	int attachment_count = 0;
	VkSubpassDescription subpass = {};
	std::array<VkAttachmentDescription, 3> attachments;
	std::array<VkAttachmentReference, 3> attachment_refs;
	create_colorbuffer();

	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;

	if (enable_msaa) {
		auto &msaa_attachment = attachments[attachment_count];
		auto &msaa_ref = attachment_refs[attachment_count];

		msaa_ref.attachment = attachment_count;
		msaa_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		msaa_attachment = {};
		msaa_attachment.format = color_format;
		msaa_attachment.samples = VK_CTX.msaaSamples;
		msaa_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		msaa_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		msaa_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		msaa_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		msaa_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		msaa_attachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		
		subpass.pColorAttachments = &msaa_ref;
		++attachment_count;
	}

	auto &color_attachment = attachments[attachment_count];
	auto &color_ref = attachment_refs[attachment_count];

	color_ref.attachment = attachment_count;
	color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	color_attachment = {};
	color_attachment.format = color_format;
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	++attachment_count;

	if (enable_msaa) {
		subpass.pResolveAttachments = &color_ref;
	} else {
		subpass.pColorAttachments = &color_ref;
		subpass.pResolveAttachments = nullptr;
	}
	
	if (depth_format != texture_format::INVALID) {
		vk_format depth_format(this->depth_format, this->linear);
		create_depthbuffer();
		auto &depth_attachment = attachments[attachment_count];
		auto &depth_ref = attachment_refs[attachment_count];
		depth_attachment = {};
		depth_ref.attachment = attachment_count;
		depth_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depth_attachment.format = depth_format;
		depth_attachment.samples = enable_msaa ? VK_CTX.msaaSamples : VK_SAMPLE_COUNT_1_BIT;
		depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		subpass.pDepthStencilAttachment = &depth_ref;
		++attachment_count;
	} else {
		subpass.pDepthStencilAttachment = nullptr;
	}

	int depend_count = 1;
	std::array<VkSubpassDependency, 2> dependencies;

	if (enable_msaa) {
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		++depend_count;
	} else {
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;;
		dependencies[0].srcAccessMask = 0;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	}

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = attachment_count;
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = depend_count;
	renderPassInfo.pDependencies = dependencies.data();
	auto ret = vkCreateRenderPass(VK_CTX.device, &renderPassInfo, nullptr, &render_pass);
	assert(ret == VK_SUCCESS);

	for (int i = 0; i < conf::MAX_FRAMES_IN_FLIGHT; i++) {
		if (enable_msaa) {
			std::array<VkImageView, 3> attachments = {
				msaa_buffer[i].view,
				color_buffer[i].view,
				depth_buffer.view,
			};
			VkFramebufferCreateInfo framebuffInfo = {};
			framebuffInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebuffInfo.renderPass = render_pass;
			framebuffInfo.attachmentCount = attachments.size();
			framebuffInfo.pAttachments = attachments.data();
			framebuffInfo.width = width();
			framebuffInfo.height = height();
			framebuffInfo.layers = 1;
			auto ret = vkCreateFramebuffer(VK_CTX.device, &framebuffInfo, nullptr, &framebuffers[i]);
			assert(ret == VK_SUCCESS);	
		} else {
			std::array<VkImageView, 2> attachments = {
				color_buffer[i].view,
				depth_buffer.view,
			};
			VkFramebufferCreateInfo framebuffInfo = {};
			framebuffInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebuffInfo.renderPass = render_pass;
			framebuffInfo.attachmentCount = attachments.size();
			framebuffInfo.pAttachments = attachments.data();
			framebuffInfo.width = width();
			framebuffInfo.height = height();
			framebuffInfo.layers = 1;
			auto ret = vkCreateFramebuffer(VK_CTX.device, &framebuffInfo, nullptr, &framebuffers[i]);
			assert(ret == VK_SUCCESS);	
		}
	}
}

void
vk_render_texture::begin()
{
	auto &cmdbuf = vulkan::VK_CTX.cmdbuf;
	VkRenderPassBeginInfo renderPassInfo{};
	std::array<VkClearValue, 3> clearColor{};
	clearColor[0].color =  {{0.0f, 0.0f, 0.0f, 1.0f}} ;
	clearColor[1].color =  {{0.0f, 0.0f, 0.0f, 1.0f}} ;
	clearColor[1].depthStencil = { 1.0f, 0 };
	clearColor[2].depthStencil = { 1.0f, 0 };
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = render_pass;
	renderPassInfo.framebuffer = framebuffer();
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent.width = width();
	renderPassInfo.renderArea.extent.height = height();
	renderPassInfo.clearValueCount = enable_msaa ? static_cast<uint32_t>(clearColor.size()) : 2;
	renderPassInfo.pClearValues = clearColor.data();
	vkCmdBeginRenderPass(cmdbuf, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void
vk_render_texture::end()
{
	vkCmdEndRenderPass(VK_CTX.cmdbuf);
}

}}

