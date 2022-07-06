#include "system/render_system.h"
#ifdef USE_VULKAN
#include "render/vulkan/vk_ctx.h"
#include "render/vulkan/vk_surface.h"
#include "render/vulkan/vk_framebuffer.h"
#include "render/vulkan/vk_mesh.h"
#include "render/vulkan/vk_material.h"

#endif

namespace engine {


#ifdef USE_VULKAN

render_system::render_system()
{
	surface = vulkan::surface_new("µÛ½­", 1024, 768);
	vulkan::vk_ctx_init("µÛ½­", surface, 1024, 768);
	surface_initui(surface, &vulkan::VK_CTX.surface);
	uniform_per_frame.reset(new vulkan::vk_uniform<render::ubo::per_frame, vulkan::ENGINE_PER_FRAME_BINDING>());
	uniform_per_draw.reset(new vulkan::vk_uniform<render::ubo::per_draw, vulkan::ENGINE_PER_DRAW_BINDING>());

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
update_uniformbuffer_per_draw(render::ubo::per_draw *ubo, camera *cam, const draw_object &draw) {
	vector3f axis;
	auto &pos = draw.position;
	auto &scale = draw.scale;
	auto angle = draw.rotation.to_axis_angle(&axis);

	auto eye = cam->transform->position;
	auto eye_dir = eye + cam->forward() * 5.0f;
	auto up = cam->up();
	ubo->engine_camera_pos = glm::vec3(
		cam->transform->position.x(),
		cam->transform->position.y(),
		cam->transform->position.z());
	auto model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(pos.x(), pos.y(), pos.z()));
	model = glm::rotate(model, glm::radians(angle), glm::vec3(axis.x(), axis.y(), axis.z()));
	ubo->model = to_mat4(matrix4f::trs(draw.position, draw.rotation, draw.scale));
	glm::mat x = glm::scale(model, glm::vec3(scale.x(), scale.y(), scale.z()));
	ubo->view = glm::lookAt(
			glm::vec3(eye.x(), eye.y(), eye.z()),
			glm::vec3(eye_dir.x(), eye_dir.y(), eye_dir.z()),
			glm::vec3(up.x(), up.y(), up.z()));
	ubo->proj = glm::perspective(glm::radians(cam->fov), cam->aspect,
		cam->clip_near_plane, cam->clip_far_plane);
	ubo->proj[1][1] *= -1;
	int bone_count = std::min(draw.skeleton_pose.size(), ubo->skeleton_pose.size());
	for (int k = 0; k < bone_count; k++) {
		ubo->skeleton_pose[k] = to_mat4(draw.skeleton_pose[k]);
	}
}


bool
render_system::frame_begin(float delta)
{
	int ok = vulkan::surface_framebegin(surface);
	if (ok == -1)
		return false;
	vulkan::vk_ctx_frame_begin();
	auto result = vulkan::VK_FRAMEBUFFER.acquire();
	switch (result) {
	case vulkan::vk_framebuffer::acquire_result::NOT_READY:
		acquire_success = false;
		return true;
	case vulkan::vk_framebuffer::acquire_result::RECREATE_SWAPCHAIN:
		return true;
	case vulkan::vk_framebuffer::acquire_result::SUCCESS:
		break;
	}

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
		return true;

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = vulkan::VK_CTX.render_pass;
	renderPassInfo.framebuffer = vulkan::VK_FRAMEBUFFER.current();
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = vulkan::VK_CTX.swapchain.extent;
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearColor.size());
	renderPassInfo.pClearValues = clearColor.data();
	vkCmdBeginRenderPass(cmdbuf, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	uniform_per_draw->frame_begin(vulkan::VK_CTX.frame_index, 1024);
	uniform_per_frame->frame_begin(vulkan::VK_CTX.frame_index, 8);
	std::tie(per_frame_buffer, per_frame_offset) = uniform_per_frame->per_begin();
	update_uniformbuffer_per_frame(per_frame_buffer);
	return true;
}

void
render_system::draw(camera *cam, draw_object &draw)
{
	vulkan::vk_mesh *mesh = (vulkan::vk_mesh *)draw.mesh;
	vulkan::vk_material *mat = (vulkan::vk_material *)draw.material;
	mesh->flush();
	auto descset = mat->desc_set[vulkan::VK_CTX.frame_index];
	render::ubo::per_draw *ubo;
	uint32_t ubo_offset[2];
	std::tie(ubo, ubo_offset[0]) = uniform_per_draw->per_begin();
	ubo_offset[1] = per_frame_offset;
	update_uniformbuffer_per_draw(ubo, cam, draw);
	uniform_per_draw->per_end();


	vkCmdBindPipeline(vulkan::VK_CTX.cmdbuf, VK_PIPELINE_BIND_POINT_GRAPHICS, mat->pipeline->handle);

	VkViewport viewport;
	viewport.x = cam->viewport.x * vulkan::VK_CTX.swapchain.extent.width;
	viewport.y = cam->viewport.y * vulkan::VK_CTX.swapchain.extent.height;
	viewport.width = cam->viewport.width * vulkan::VK_CTX.swapchain.extent.width;
	viewport.height = cam->viewport.height * vulkan::VK_CTX.swapchain.extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
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
		1, &descset,
		0, nullptr);
	vkCmdBindDescriptorSets(
		vulkan::VK_CTX.cmdbuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
		mat->pipeline->layout,
		1,
		1, &vulkan::VK_CTX.engine_desc_set,
		2, ubo_offset);
	vkCmdDrawIndexed(vulkan::VK_CTX.cmdbuf, mesh->index_count, 1, 0, 0, 0);
}

void
render_system::frame_end(float delta)
{
	vulkan::surface_frameend(surface);
	vkCmdEndRenderPass(vulkan::VK_CTX.cmdbuf);
	if (vkEndCommandBuffer(vulkan::VK_CTX.cmdbuf) != VK_SUCCESS)
		return ;
	vulkan::VK_FRAMEBUFFER.submit(vulkan::VK_CTX.cmdbuf);
	uniform_per_frame->per_end();
	uniform_per_frame->frame_end();
	uniform_per_draw->frame_end();
	vulkan::vk_ctx_frame_end();
}


#else

#error("render_system not be implemented");

#endif

}
