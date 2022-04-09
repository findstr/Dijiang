#include <vector>
#include <memory>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "conf.h"
#include "vk_object.h"
#include "vk_buffer.h"
#include "vk_shader.h"
#include "vk_texture.h"
#include "framesync.h"
#include "forward.h"
#include "vk_material.h"
#include "vk_texture.h"
#include "vk_mesh.h"
#include "utils/file.h"
#include "ubo.h"
#include "vk_pass.h"
#include "vk_pipeline.h"
#include "resource/resource.h"

namespace engine {
namespace vulkan {

#define CBV_BIND_ID	(0*128)
#define UAV_BIND_ID	(1*128)
#define SRV_BIND_ID	(2*128)
#define SAM_BIND_ID	(3*128)

struct framebuffer {
	VkFramebuffer handle = VK_NULL_HANDLE;
};

struct renderframe {
	framesync *sync = nullptr;
	VkCommandBuffer cmdbuf = VK_NULL_HANDLE;
};


struct forward {
	int frameidx = 0;
	VkRenderPass renderpass;
	std::vector<framebuffer> framebuffers;
	std::vector<renderframe> renderframes;
	/*
	VkPipelineLayout pipelinelayout;
	VkPipeline pipeline;
	*/
	textureEx depthbuffer;
	std::shared_ptr<render::material> matx;
	std::shared_ptr<render::mesh> meshx;
	vk_material *mat;
	vk_mesh *mesh;
};

static VkFormat
find_supported_format(
		const renderctx *ctx,
		const std::vector<VkFormat> &candidates,
		VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (VkFormat fmt : candidates) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(ctx->phydevice, fmt, &props);
		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
			return fmt;
		} else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return fmt;
		}
	}
	throw std::runtime_error("failed to find supported format");
}

static VkFormat
find_depth_format(const renderctx *ctx)
{
	return find_supported_format(ctx,
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

static int
new_renderpass(const renderctx *ctx, forward *fw)
{
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = ctx->swapchain.imageformat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = find_depth_format(ctx);
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;;

	std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment }; //Should keep order of renderframe

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(ctx->logicdevice, &renderPassInfo, nullptr, &fw->renderpass) != VK_SUCCESS)
		return -1;
	return 0;
}

static void
del_framebuffers(const renderctx *ctx, forward *fw)
{
	for (auto &fb:fw->framebuffers)
		vkDestroyFramebuffer(ctx->logicdevice, fb.handle, nullptr);
	fw->framebuffers.clear();
}

static int
new_framebuffers(const renderctx *ctx, forward *fw)
{
	fw->framebuffers.reserve(ctx->swapchain.imageviews.size());
	for (auto view:ctx->swapchain.imageviews) {
		auto &fb = fw->framebuffers.emplace_back();
		std::array<VkImageView, 2> attachments = {
			view,
			fw->depthbuffer.view,
		};

		VkFramebufferCreateInfo framebuffInfo = {};
		framebuffInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffInfo.renderPass = fw->renderpass; //指定Framebuffer需要兼容的renderPass,并不定是这个Renderpass会使用这个Framebuffer
		framebuffInfo.attachmentCount = attachments.size();
		framebuffInfo.pAttachments = attachments.data();
		framebuffInfo.width = ctx->swapchain.extent.width;
		framebuffInfo.height = ctx->swapchain.extent.height;
		framebuffInfo.layers = 1;
		if (vkCreateFramebuffer(ctx->logicdevice, &framebuffInfo, nullptr, &fb.handle) != VK_SUCCESS) {
			del_framebuffers(ctx, fw);
			return -1;
		}
	}
	return 0;
}


static void
del_renderframe(const renderctx *ctx, renderframe &fb)
{
	framesync_del(ctx, fb.sync);
	/*
	buffer_del(ctx, fb.uniformbuffer);
	VkDescriptorSet descset = VK_NULL_HANDLE;
	VkCommandBuffer cmdbuf = VK_NULL_HANDLE;
	*/
}

static int
new_renderframe(const renderctx *ctx, forward *fw, renderframe *fb)
{
	VkResult result;
	VkCommandBufferAllocateInfo cba = {};
	cba.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cba.commandPool = ctx->commandpool;
	cba.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cba.commandBufferCount = 1;
	result = vkAllocateCommandBuffers(ctx->logicdevice, &cba, &fb->cmdbuf);
	if (result != VK_SUCCESS) {
		fprintf(stderr, "[render] new_renderframe result:%d\n", result);
		return -1;
	}

	fb->sync = framesync_new(ctx);
	return 0;

}
/*
static void
new_pipeline(const renderctx *ctx, forward *fw)
{
	const class shader *rs = static_cast<const class shader *>(fw->mat->get_shader());
	std::vector<VkPipelineShaderStageCreateInfo> shaderstages;
	shaderstages.reserve(5);
	for (auto &iter:rs->modules) {
		auto &ss = shaderstages.emplace_back();
		ss.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		ss.stage = iter.stage;
		ss.module = iter.module;
		ss.pName = iter.name.c_str();
		ss.pSpecializationInfo = nullptr;
	}
	auto desc_set_layout = ((const vulkan::shader *)fw->mat->get_shader())->desc_set_layout();
	printf("desc_set_layout:%p\n", &desc_set_layout);
	std::tie(fw->pipeline, fw->pipelinelayout) = pipelinex_create(ctx, fw->renderpass, desc_set_layout, shaderstages);
}*/

void
forward_del(const renderctx *ctx, forward *fw)
{
	vkDeviceWaitIdle(ctx->logicdevice);
	/*
	vkDestroyPipeline(ctx->logicdevice, fw->pipeline, nullptr);
	vkDestroyPipelineLayout(ctx->logicdevice, fw->pipelinelayout, nullptr);
	*/
	del_framebuffers(ctx, fw);
	for (auto &fb:fw->renderframes)
		del_renderframe(ctx, fb);
	texture_del(ctx, fw->depthbuffer);
	vkDestroyRenderPass(ctx->logicdevice, fw->renderpass, nullptr);
	delete fw;
}

forward *
forward_new(const renderctx *ctx)
{
	forward *fw = new forward();
	new_renderpass(ctx, fw);
	fw->renderframes.reserve(ctx->swapchain.imageviews.size());
	texture_setting setting = {
		.mipmap_levels = 1,
		.format = find_depth_format(ctx),
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		.aspectflags = VK_IMAGE_ASPECT_DEPTH_BIT,
	};
	fw->depthbuffer = *texture_new_depth(ctx, ctx->swapchain.extent.width, ctx->swapchain.extent.height, setting);
	new_framebuffers(ctx, fw);
	fw->matx = engine::resource::load_material("asset/material/test.mat");
	fw->meshx = engine::resource::load_mesh("asset/models/viking_room.obj");
	fw->mat = (vk_material *)fw->matx.get();
	fw->mesh = (vk_mesh*)fw->meshx.get();
	for (size_t i = 0; i < conf::MAX_FRAMES_IN_FLIGHT; i++) {
		auto &fb = fw->renderframes.emplace_back();
		new_renderframe(ctx, fw, &fb);
	}
	return fw;
}

static void
update_uniformbuffer(const renderctx *ctx, vk_buffer *ub) {
	static auto startTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	render::UniformBufferObject ubo{};
	ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(10.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.view = glm::lookAt(glm::vec3(2.0, 2.0, 2.0), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.proj = glm::perspective(glm::radians(45.0f), ctx->swapchain.extent.width / (float)ctx->swapchain.extent.height, 0.1f, 10.0f);
	ubo.proj[1][1] *= -1;
	ub->upload(&ubo, sizeof(ubo));
}

void
forward_tick(const renderctx *ctx, forward *fw)
{
	int image_index;
	auto &rf = fw->renderframes[fw->frameidx];
	if (framesync_aquire(ctx, rf.sync, &image_index)) {
		//TODO:recreate swapchain
		return ;
	}
	auto &fb = fw->framebuffers[image_index];
	fw->mesh->flush();
	auto descset = fw->mat->desc_set[fw->frameidx];

	update_uniformbuffer(ctx, fw->mat->uniformbuffer.get());

	std::array<VkClearValue, 2> clearColor{};
	clearColor[0].color =  {{0.0f, 0.0f, 0.0f, 1.0f}} ;
	clearColor[1].depthStencil = { 1.0f, 0 };

	vkResetCommandBuffer(rf.cmdbuf, 0);

	VkCommandBufferBeginInfo begin{};
	begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin.flags = 0;
	begin.pInheritanceInfo = nullptr;
	if (vkBeginCommandBuffer(rf.cmdbuf, &begin) != VK_SUCCESS)
		return ;

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = fw->renderpass;
	renderPassInfo.framebuffer = fb.handle;
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = ctx->swapchain.extent;
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearColor.size());
	renderPassInfo.pClearValues = clearColor.data();
	vkCmdBeginRenderPass(rf.cmdbuf, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(rf.cmdbuf, VK_PIPELINE_BIND_POINT_GRAPHICS, fw->mat->pipeline->handle);

	auto vertexBuffer = fw->mesh->vertex->handle;
	auto indexBuffer = fw->mesh->index->handle;

	VkBuffer vertexBuffers[] = { vertexBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(rf.cmdbuf, 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(rf.cmdbuf, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdBindDescriptorSets(rf.cmdbuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
		fw->mat->pipeline->layout, 0, 1, &descset, 0, nullptr);
	vkCmdDrawIndexed(rf.cmdbuf, fw->mesh->index_count, 1, 0, 0, 0);
	vkCmdEndRenderPass(rf.cmdbuf);

	if (vkEndCommandBuffer(rf.cmdbuf) != VK_SUCCESS)
		return ;

	framesync_submit(ctx, rf.sync, rf.cmdbuf, image_index);
	fw->frameidx = (fw->frameidx + 1) % fw->renderframes.size();
}

}}


