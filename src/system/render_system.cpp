#include "system/render_system.h"
#ifdef USE_VULKAN
#include "render/vulkan/vk_ctx.h"
#include "render/vulkan/vk_surface.h"
#include "render/vulkan/vk_framebuffer.h"
#include "render/vulkan/vk_mesh.h"
#include "render/vulkan/vk_material.h"
#include "render/vulkan/vk_framebuffer.h"
#include "render/vulkan/vk_shader_variables.h"
#endif

namespace engine {


#ifdef USE_VULKAN

render_system::render_system()
{
	int width, height;
	surface = vulkan::surface_new("帝江", 1024, 768);
	vulkan::surface_size(surface, &width, &height);
	vulkan::vk_ctx_init("帝江", surface, width, height);
	surface_initui(surface, &vulkan::VK_CTX.surface);
	uniform_per_frame.reset(new vulkan::vk_uniform<render::ubo::per_frame, vulkan::ENGINE_PER_FRAME_BINDING>());
	uniform_per_camera.reset(new vulkan::vk_uniform<render::ubo::per_camera, vulkan::ENGINE_PER_CAMERA_BINDING>());
	uniform_per_object.reset(new vulkan::vk_uniform<render::ubo::per_object, vulkan::ENGINE_PER_OBJECT_BINDING>());
}
	
render_system::~render_system()
{
	vulkan::surface_del(surface);
	vulkan::vk_ctx_cleanup();
}

void
render_system::get_resolution(int *x, int *y)
{
	*x = vulkan::VK_CTX.swapchain.extent.width;
	*y = vulkan::VK_CTX.swapchain.extent.height;
}

static glm::mat4
to_mat4(const matrix4f &m) 
{
	glm::mat4 result;
	auto &source = m;
	for (size_t i = 0; i < 4; ++i) {
		for (size_t j = 0; j < 4; ++j) {
			result[i][j] = source(j, i);
		}
	}
	return result;
}

static void
update_uniformbuffer_per_frame(render::ubo::per_frame *ubo)
{
	float p = 15.0f;

	ubo->engine_light_ambient = glm::vec3(1.0f);
	ubo->engine_light_direction.x = p;
	ubo->engine_light_direction.y = p*0.5f;
	ubo->engine_light_direction.z = p;

	ubo->engine_light_radiance = glm::vec3(1.0f);
}

static void
update_uniformbuffer_per_object(render::ubo::per_object *ubo, const draw_object &draw) {
	vector3f axis;
	auto &pos = draw.position;
	auto &scale = draw.scale;
	auto angle = draw.rotation.to_axis_angle(&axis);
	auto model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(pos.x(), pos.y(), pos.z()));
	model = glm::rotate(model, glm::radians(angle), glm::vec3(axis.x(), axis.y(), axis.z()));
	ubo->model = to_mat4(matrix4f::trs(draw.position, draw.rotation, draw.scale));
	if (draw.skeleton_pose != nullptr) {
		int bone_count = std::min(draw.skeleton_pose->size(), ubo->skeleton_pose.size());
		for (int k = 0; k < bone_count; k++) {
			ubo->skeleton_pose[k] = to_mat4((*draw.skeleton_pose)[k].matrix);
		}
	}
}

int
render_system::frame_begin(float delta)
{
	int width, height;
	int ok = vulkan::surface_pre_tick(surface);
	if (ok < 0)
		return -1;
	vulkan::vk_ctx_frame_begin();
	auto result = vulkan::VK_FRAMEBUFFER.acquire();
	switch (result) {
	case vulkan::vk_framebuffer::acquire_result::NOT_READY:
		acquire_success = false;
		return 0;
	case vulkan::vk_framebuffer::acquire_result::RECREATE_SWAPCHAIN:
		vulkan::surface_size(surface, &width, &height);
		vulkan::vk_ctx_recreate_swapchain(width, height);
		vulkan::VK_FRAMEBUFFER.resize();
		return 1;
	case vulkan::vk_framebuffer::acquire_result::SUCCESS:
		break;
	}
	vulkan::surface_tick(surface);
	auto &cmdbuf = vulkan::VK_CTX.cmdbuf;
	std::array<VkClearValue, 2> clearColor{};
	clearColor[0].color =  {{0.0f, 0.0f, 0.0f, 1.0f}} ;
	clearColor[1].depthStencil = { 1.0f, 0 };
	vkResetCommandBuffer(cmdbuf, 0);

	VkCommandBufferBeginInfo begin{};
	begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin.flags = 0;
	begin.pInheritanceInfo = nullptr;
	if (vkBeginCommandBuffer(cmdbuf, &begin) != VK_SUCCESS)
		return 1;

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = vulkan::VK_CTX.render_pass;
	renderPassInfo.framebuffer = vulkan::VK_FRAMEBUFFER.current();
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = vulkan::VK_CTX.swapchain.extent;
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearColor.size());
	renderPassInfo.pClearValues = clearColor.data();
	vkCmdBeginRenderPass(cmdbuf, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	uniform_per_frame->frame_begin(1024);
	uniform_per_camera->frame_begin(1024);
	uniform_per_object->frame_begin(1024);
	auto *per_frame_buffer = uniform_per_frame->alloc();
	update_uniformbuffer_per_frame(per_frame_buffer);
	ubo_offset[vulkan::ENGINE_PER_FRAME_BINDING] = uniform_per_frame->offset();
	uniform_per_frame->unmap();
	return 0;
}
	
void 
render_system::set_camera(camera *cam)
{
	auto *ubo = uniform_per_camera->alloc();
	auto eye = cam->transform->position;
	auto eye_dir = eye + cam->forward() * 5.0f;
	auto up = cam->up();
	ubo->engine_camera_pos = glm::vec3(
		cam->transform->position.x(),
		cam->transform->position.y(),
		cam->transform->position.z());
	ubo->view = glm::lookAt(
			glm::vec3(eye.x(), eye.y(), eye.z()),
			glm::vec3(eye_dir.x(), eye_dir.y(), eye_dir.z()),
			glm::vec3(up.x(), up.y(), up.z()));
	ubo->proj = glm::perspective(glm::radians(cam->fov), cam->aspect,
		cam->clip_near_plane, cam->clip_far_plane);
	ubo->proj[1][1] *= -1;
	ubo_offset[vulkan::ENGINE_PER_CAMERA_BINDING] = uniform_per_camera->offset();
	viewport.x = cam->viewport.x * vulkan::VK_CTX.swapchain.extent.width;
	viewport.y = cam->viewport.y * vulkan::VK_CTX.swapchain.extent.height;
	viewport.width = cam->viewport.width * vulkan::VK_CTX.swapchain.extent.width;
	viewport.height = cam->viewport.height * vulkan::VK_CTX.swapchain.extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	uniform_per_camera->unmap();

	VkClearAttachment clear;
	VkClearRect clear_rect;
	clear.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;// | VK_IMAGE_ASPECT_STENCIL_BIT;
	clear.clearValue.color = { {0.0f, 0.0f, 0.0f, 1.0f} };
	clear.clearValue.depthStencil = { 1.0f, 0 };
	clear_rect.baseArrayLayer = 0;
	clear_rect.layerCount = 1;
	clear_rect.rect.extent = vulkan::VK_CTX.swapchain.extent;
	clear_rect.rect.offset = {0, 0};
	vkCmdClearAttachments(vulkan::VK_CTX.cmdbuf, 1, &clear, 1, &clear_rect);
}

void
render_system::draw(draw_object &draw)
{
	render::ubo::per_object *ubo = uniform_per_object->alloc();
	vulkan::vk_mesh *mesh = (vulkan::vk_mesh *)draw.mesh;
	vulkan::vk_material *mat = (vulkan::vk_material *)draw.material;
	ubo_offset[vulkan::ENGINE_PER_OBJECT_BINDING] = uniform_per_object->offset();
	mesh->flush();
	update_uniformbuffer_per_object(ubo, draw);
	uniform_per_object->unmap();
	vkCmdBindPipeline(vulkan::VK_CTX.cmdbuf, VK_PIPELINE_BIND_POINT_GRAPHICS, mat->pipeline->handle);
#if IS_EDITOR
	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = vulkan::VK_CTX.swapchain.extent;
	vkCmdSetScissor(vulkan::VK_CTX.cmdbuf, 0, 1, &scissor);
#endif
	vkCmdSetViewport(vulkan::VK_CTX.cmdbuf, 0, 1, &viewport);
	auto vertexBuffer = mesh->vertex->handle;
	auto indexBuffer = mesh->index->handle;

	VkBuffer vertexBuffers[] = { vertexBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(vulkan::VK_CTX.cmdbuf, 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(vulkan::VK_CTX.cmdbuf, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdBindDescriptorSets(
		vulkan::VK_CTX.cmdbuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
		mat->pipeline->layout,
		0,
		1, &mat->desc_set[vulkan::VK_CTX.frame_index],
		0, nullptr);
	vkCmdBindDescriptorSets(
		vulkan::VK_CTX.cmdbuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
		mat->pipeline->layout,
		1,
		1, &vulkan::VK_CTX.engine_desc_set[vulkan::VK_CTX.frame_index],
		ubo_offset.size(), ubo_offset.data());
	vkCmdDrawIndexed(vulkan::VK_CTX.cmdbuf, mesh->index_count, 1, 0, 0, 0);
}

void
render_system::frame_end(float delta)
{
	vulkan::surface_post_tick(surface);
	vkCmdEndRenderPass(vulkan::VK_CTX.cmdbuf);
	if (vkEndCommandBuffer(vulkan::VK_CTX.cmdbuf) != VK_SUCCESS)
		return ;
	uniform_per_frame->frame_end();
	uniform_per_camera->frame_end();
	uniform_per_object->frame_end();
	vulkan::VK_FRAMEBUFFER.submit(vulkan::VK_CTX.cmdbuf);
	vulkan::vk_ctx_frame_end();
}


#else

#error("render_system not be implemented");

#endif

}
