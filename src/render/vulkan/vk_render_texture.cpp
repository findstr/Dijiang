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
	depth_buffer.create(&tex, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 1);
	depth_buffer.sampler(&tex);
}

void
vk_render_texture::apply()
{
	vk_format color_format(this->format, this->linear);
	int attachment_count = 1;
	VkSubpassDescription subpass = {};
	std::array<VkAttachmentDescription, 2> attachments;
	std::array<VkAttachmentReference, 2> attachment_refs;
	create_colorbuffer();

	auto &color_attachment = attachments[0];
	auto &color_ref = attachment_refs[0];

	color_ref.attachment = 0;
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
	
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_ref;

	if (depth_format != texture_format::INVALID) {
		vk_format depth_format(this->depth_format, this->linear);
		++attachment_count;
		create_depthbuffer();
		auto &depth_attachment = attachments[1];
		auto &depth_ref = attachment_refs[1];
		depth_attachment = {};
		depth_ref.attachment = 1;
		depth_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depth_attachment.format = depth_format;
		depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		subpass.pDepthStencilAttachment = &depth_ref;
	} else {
		subpass.pDepthStencilAttachment = nullptr;
	}

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = attachment_count;
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;
	auto ret = vkCreateRenderPass(VK_CTX.device, &renderPassInfo, nullptr, &render_pass);
	assert(ret == VK_SUCCESS);

	for (int i = 0; i < conf::MAX_FRAMES_IN_FLIGHT; i++) {
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

}}

