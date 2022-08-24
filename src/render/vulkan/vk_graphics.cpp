#include "system/render_system.h"
#include "level.h"
#include "render_pass.h"
#include "render/vulkan/vk_ctx.h"
#include "render/vulkan/vk_native.h"
#include "render/vulkan/vk_surface.h"
#include "render/vulkan/vk_shader.h"
#include "render/vulkan/vk_shader_variables.h"
#include "render/gpu_interface.h"

namespace engine {
namespace vulkan {
#if 0


render_system::render_system()
{
	int width, height;
	vulkan::vk_surface::inst().init("帝江", 1024, 768);
	vulkan::vk_surface::inst().resolution(&width, &height);
	vulkan::vk_ctx_init("帝江", width, height);
	vulkan::vk_surface::inst().init_ui(&vulkan::VK_CTX.surface);
	uniform_per_frame.reset(new vulkan::vk_uniform<render::ubo::per_frame, vulkan::ENGINE_PER_FRAME_BINDING>());
	uniform_per_camera.reset(new vulkan::vk_uniform<render::ubo::per_camera, vulkan::ENGINE_PER_CAMERA_BINDING>());
	uniform_per_object.reset(new vulkan::vk_uniform<render::ubo::per_object, vulkan::ENGINE_PER_OBJECT_BINDING>());
	for (int i = 0; i < conf::MAX_FRAMES_IN_FLIGHT; i++) {
		indirect_cmd_count[i] = 0;
		indirect_buffer[i].create(vulkan::vk_buffer::type::INDIRECT, 1024*sizeof(VkDrawIndexedIndirectCommand));
	}
}
	
render_system::~render_system()
{
	vulkan::vk_surface::inst().exit();
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
update_uniformbuffer_per_object(render::ubo::per_camera *ubo_cam, render::ubo::per_object *ubo, const draw_object &draw) 
{
	ubo->material = draw.material_offset;
	ubo->model = to_mat4(draw.go->transform.local_to_world_matrix());
	ubo->model_view_proj = ubo_cam->view_proj * ubo->model;
	if (draw.skeleton_pose != nullptr) {
		int bone_count = std::min(draw.skeleton_pose->size(), ubo->skeleton_pose.size());
		for (int k = 0; k < bone_count; k++) {
			ubo->skeleton_pose[k] = to_mat4((*draw.skeleton_pose)[k].matrix);
		}
	}
}
	
void
render_system::init_lighting()
{
	vulkan::vk_ctx_init_lighting();
}

int
render_system::frame_begin(float delta)
{
	int width, height;
	vulkan::vk_surface::inst().pre_tick(delta);
	vulkan::vk_ctx_frame_begin();
	auto result = vulkan::VK_CTX.swapchain.acquire();
	switch (result) {
	case vulkan::vk_swapchain::acquire_result::NOT_READY:
		acquire_success = false;
		return 0;
	case vulkan::vk_swapchain::acquire_result::RECREATE_SWAPCHAIN:
		vulkan::vk_surface::inst().resolution(&width, &height);
		vulkan::VK_CTX.swapchain.resize(width, height);
		return 1;
	case vulkan::vk_swapchain::acquire_result::SUCCESS:
		break;
	}
	vulkan::vk_surface::inst().tick(delta);
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

	
	uniform_per_frame->frame_begin(1024);
	uniform_per_camera->frame_begin(1024);
	uniform_per_object->frame_begin(1024);

	return 0;
}

void 
render_system::renderpass_begin(render_texture *rt)
{
	vulkan::vk_ctx_renderpass_begin(rt);
	gpu_mesh::instance().flush();
}

void
render_system::renderpass_end()
{
	vulkan::vk_ctx_renderpass_end();
}
	
void
render_system::shadowpass_begin()
{
	if (shadow_texture == nullptr) {
		shadow_texture.reset(render_texture::create(
			2048,
			2048,
			texture_format::R32, texture_format::D32, true));

		auto *st = (vulkan::vk_render_texture *)(shadow_texture.get());

		std::array<VkDescriptorImageInfo, conf::MAX_FRAMES_IN_FLIGHT> imageInfo;
		std::array<VkWriteDescriptorSet, conf::MAX_FRAMES_IN_FLIGHT * 2> desc_writes;
		for (int i = 0; i < conf::MAX_FRAMES_IN_FLIGHT; i++) {
			imageInfo[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo[i].imageView = st->imageview(i);
			imageInfo[i].sampler = st->sampler(i);
			
			desc_writes[i*2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			desc_writes[i*2].dstSet = vulkan::VK_CTX.engine_desc_set[i];
			desc_writes[i*2].dstBinding = vulkan::ENGINE_SHADOWMAP_BINDING;
			desc_writes[i*2].dstArrayElement = 0;
			desc_writes[i*2].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			desc_writes[i*2].descriptorCount = 1;
			desc_writes[i*2].pImageInfo = &imageInfo[i];
			desc_writes[i*2].pNext = VK_NULL_HANDLE;

			desc_writes[i*2+1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			desc_writes[i*2+1].dstSet = vulkan::VK_CTX.engine_desc_set[i];
			desc_writes[i*2+1].dstBinding = vulkan::ENGINE_SHADOWMAP_SAMPLER_BINDING;
			desc_writes[i*2+1].dstArrayElement = 0;
			desc_writes[i*2+1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
			desc_writes[i*2+1].descriptorCount = 1;
			desc_writes[i*2+1].pImageInfo = &imageInfo[i];
			desc_writes[i*2+1].pNext = VK_NULL_HANDLE;
		}

		vkUpdateDescriptorSets(vulkan::VK_CTX.device,
			static_cast<uint32_t>(desc_writes.size()),
			desc_writes.data(), 0, nullptr);
	}
	auto *st = (vulkan::vk_render_texture *)(shadow_texture.get());
	/*
	vulkan::VK_FRAMEBUFFER.switch_shadow_target();
	RENDER_PASS.begin(render_pass::SHADOW, 1, 1);
	render_pass = vulkan::native_of(RENDER_PASS.get(render_pass::SHADOW)).handle();
	*/
	auto &cmdbuf = vulkan::VK_CTX.cmdbuf;
	VkRenderPassBeginInfo renderPassInfo{};
	std::array<VkClearValue, 3> clearColor{};
	clearColor[0].color =  {{0.0f, 0.0f, 0.0f, 1.0f}} ;
	clearColor[1].color =  {{0.0f, 0.0f, 0.0f, 1.0f}} ;
	clearColor[1].depthStencil = { 1.0f, 0 };
	clearColor[2].depthStencil = { 1.0f, 0 };
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = st->render_pass;
	renderPassInfo.framebuffer = st->framebuffer();
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent.width = st->width();
	renderPassInfo.renderArea.extent.height = st->height();
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearColor.size());
	renderPassInfo.pClearValues = clearColor.data();
	vkCmdBeginRenderPass(cmdbuf, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vulkan::VK_CTX.current_renderpass = st->render_pass;
	vulkan::VK_CTX.enable_msaa = st->enable_msaa;

	viewport.x = 0;
	viewport.y = 0;
	viewport.width = st->width();
	viewport.height = st->height();
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	vulkan::vk_ctx_debug_label_begin("RenderShadowmap");

}

void
render_system::shadowpass_end()
{
	vulkan::vk_ctx_debug_label_end();
	vkCmdEndRenderPass(vulkan::VK_CTX.cmdbuf);
}
	
void 
render_system::set_light(light *li, camera *cam)
{
	vector3f center;
	ubo_per_frame = uniform_per_frame->alloc();
	vector3f direction = li->direction().normalized();
	auto c = li->color;
	c.r *= li->intensity;
	c.g *= li->intensity;
	c.b *= li->intensity;
	ubo_per_frame->engine_light_ambient = glm::vec4(1.0f);
	ubo_per_frame->engine_light_direction.x = direction.x();
	ubo_per_frame->engine_light_direction.y = direction.y();
	ubo_per_frame->engine_light_direction.z = direction.z();
	ubo_per_frame->engine_light_radiance = glm::vec4(c.r, c.g, c.b, 1.0f);

	li->get_shadow_matrix(cam,
		center,
		ubo_per_frame->engine_light_matrix_view[0],
		ubo_per_frame->engine_light_matrix_project[0]);
	ubo_offset[vulkan::ENGINE_PER_FRAME_BINDING] = uniform_per_frame->offset();
	uniform_per_frame->unmap();
}


void
render_system::set_light_camera(light *li, camera *cam)
{
	vector3f center;
	ubo_per_camera = uniform_per_camera->alloc();
	li->get_shadow_matrix(cam, center, 
		ubo_per_camera->view, 
		ubo_per_camera->proj);
	ubo_per_camera->engine_camera_pos = glm::vec4(center.x(), center.y(), center.z(), 1.0f);
	ubo_offset[vulkan::ENGINE_PER_CAMERA_BINDING] = uniform_per_camera->offset();
	uniform_per_camera->unmap();
}

void 
render_system::set_camera(camera *cam)
{
	ubo_per_camera = uniform_per_camera->alloc();
	auto eye = cam->transform->position();
	auto eye_dir = eye + cam->forward() * 5.0f;
	auto up = cam->up();
	ubo_per_camera->engine_camera_pos = glm::vec4(
		cam->transform->position().x(),
		cam->transform->position().y(),
		cam->transform->position().z(), 1.0);
	ubo_per_camera->view = glm::lookAt(
			glm::vec3(eye.x(), eye.y(), eye.z()),
			glm::vec3(eye_dir.x(), eye_dir.y(), eye_dir.z()),
			glm::vec3(up.x(), up.y(), up.z()));
	if (cam->perspective) {
		ubo_per_camera->proj = glm::perspectiveRH_ZO(
			glm::radians(cam->fov), cam->aspect,
			cam->clip_near_plane, cam->clip_far_plane);
	} else {
		ubo_per_camera->proj = glm::orthoRH_ZO(
			-cam->orthographic_size, cam->orthographic_size,
			-cam->orthographic_size, cam->orthographic_size, 
			cam->clip_near_plane, cam->clip_far_plane);
	}
	ubo_per_camera->proj[1][1] *= -1;
	ubo_per_camera->view_proj = ubo_per_camera->proj * ubo_per_camera->view;
	ubo_offset[vulkan::ENGINE_PER_CAMERA_BINDING] = uniform_per_camera->offset();
	viewport.x = cam->viewport.x * vulkan::VK_CTX.swapchain.extent.width;
	viewport.y = cam->viewport.y * vulkan::VK_CTX.swapchain.extent.height;
	viewport.width = cam->viewport.width * vulkan::VK_CTX.swapchain.extent.width;
	viewport.height = cam->viewport.height * vulkan::VK_CTX.swapchain.extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	uniform_per_camera->unmap();
}

int ssbo_offset = 0;
int indirect_offset = 0;

	
void
render_system::init_for_object(std::vector<draw_object> &draw_list)
{
}

struct indirect_batch {
	int material_offset = 0;
	render::mesh *mesh = nullptr;
	render::material *material = nullptr;
	VkBuffer vertex_buffer = VK_NULL_HANDLE;
	VkBuffer index_buffer = VK_NULL_HANDLE;
	int vertex_offset = 0;
	int index_offset = 0;
	int index_count = 0;
	uint32_t first;
	uint32_t count;
};

struct material_info {
	render::material *material;
	int material_offset;
};

static std::vector<indirect_batch> 
compact_draws(std::vector<draw_object> &draw_list, std::vector<material_info> &materials)
{
	std::vector<indirect_batch> draws;
	if (draw_list.size() == 0)
		return draws;
	int material_offset = 0;
	indirect_batch first_draw;
	first_draw.mesh = draw_list[0].mesh;
	first_draw.material = draw_list[0].material;
	first_draw.first = 0;
	first_draw.count = 1;
	first_draw.material_offset = material_offset;
	draw_list[0].material_offset = material_offset;
	first_draw.index_count = gpu_mesh::instance().bind_info(
		first_draw.mesh->handle,
		&first_draw.vertex_buffer,
		&first_draw.vertex_offset,
		&first_draw.index_buffer,
		&first_draw.index_offset);
	draws.push_back(first_draw);
	materials.emplace_back(first_draw.material, material_offset);
	material_offset += first_draw.material->argument_size();
	for (int i = 1; i < (int)draw_list.size(); i++) {
		bool same_mesh = draws.back().mesh == draw_list[i].mesh;
		bool same_material = draws.back().material == draw_list[i].material;
		if (same_mesh && same_material) {
			++draws.back().count;
		} else {
			indirect_batch new_draw;
			new_draw.mesh = draw_list[i].mesh;
			new_draw.material = draw_list[i].material;
			new_draw.first = i;
			new_draw.count = 1;
			if (!same_material) {
				new_draw.material_offset = material_offset;
				materials.emplace_back(new_draw.material, material_offset);
				material_offset += new_draw.material->argument_size();
			} else {
				new_draw.material_offset = draws.back().material_offset;
			}
			new_draw.index_count = gpu_mesh::instance().bind_info(
				new_draw.mesh->handle,
				&new_draw.vertex_buffer,
				&new_draw.vertex_offset,
				&new_draw.index_buffer,
				&new_draw.index_offset);
			draws.emplace_back(new_draw);
		}
		draw_list[i].material_offset = draws.back().material_offset;
	}
	return draws;
}

void
render_system::draw(std::vector<draw_object> &draw_list)
{
	VkPipelineLayout last_binding_layout = VK_NULL_HANDLE;
	VkPipeline last_binding_pipeline = VK_NULL_HANDLE;
	render::material *last_binding_mat = VK_NULL_HANDLE;
	VkBuffer last_vertex_buffer = VK_NULL_HANDLE;
	VkBuffer last_index_buffer = VK_NULL_HANDLE;
	ubo_offset[vulkan::ENGINE_PER_OBJECT_BINDING] = 0;
#if IS_EDITOR
	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = {(uint32_t) viewport.width, (uint32_t)viewport.height };
	vkCmdSetScissor(vulkan::VK_CTX.cmdbuf, 0, 1, &scissor);
#endif
	vkCmdSetViewport(vulkan::VK_CTX.cmdbuf, 0, 1, &viewport);
	std::vector<material_info> materials;
	auto draw_batches = compact_draws(draw_list, materials);
	uint8_t *material_buffer = (uint8_t *)vulkan::VK_CTX.engine_bindless_material[vulkan::VK_CTX.frame_index]->map();
	for (int i = 0; i < (int)materials.size(); i++) {
		auto &m = materials[i];
		m.material->copy_to(material_buffer + m.material_offset);
	}
	vulkan::VK_CTX.engine_bindless_material[vulkan::VK_CTX.frame_index]->unmap();
	

	auto *ubo = (render::ubo::per_object *)vulkan::VK_CTX.engine_bindless_object[vulkan::VK_CTX.frame_index]->map();
	for (int i = 0; i < draw_list.size(); i++) {
		auto &draw = draw_list[i];
		update_uniformbuffer_per_object(ubo_per_camera, &ubo[i], draw);
	}
	vulkan::VK_CTX.engine_bindless_object[vulkan::VK_CTX.frame_index]->unmap(ssbo_offset * sizeof(*ubo), draw_list.size() * sizeof(*ubo));
	ssbo_offset += draw_list.size();
	int indirect_count = 0;
	auto *indirect_cmd = (VkDrawIndexedIndirectCommand *)indirect_buffer[vulkan::VK_CTX.frame_index].map();
	int drawcall = 0;
	for (auto &draw:draw_batches) {
		auto mat = draw.material;
		auto *shader = (engine::vulkan::vk_shader *)mat->get_shader();
		if (mat != last_binding_mat) {
			last_binding_mat = mat;
			vkCmdBindDescriptorSets(
				vulkan::VK_CTX.cmdbuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
				vulkan::VK_CTX.engine_bindless_pipeline_layout,
				1,
				1, &vulkan::VK_CTX.engine_desc_set[vulkan::VK_CTX.frame_index],
				ubo_offset.size(), ubo_offset.data());
		}
		if (last_binding_layout != vulkan::VK_CTX.engine_bindless_pipeline_layout) {
			last_binding_layout = vulkan::VK_CTX.engine_bindless_pipeline_layout;
			vkCmdBindDescriptorSets(
				vulkan::VK_CTX.cmdbuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
				last_binding_layout,
				2,
				1, &vulkan::VK_CTX.engine_bindless_texture_set,
				0, nullptr);
			vkCmdBindDescriptorSets(
				vulkan::VK_CTX.cmdbuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
				last_binding_layout,
				3,
				1, &vulkan::VK_CTX.engine_bindless_object_set[vulkan::VK_CTX.frame_index],
				0, nullptr);
			vkCmdBindDescriptorSets(
				vulkan::VK_CTX.cmdbuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
				last_binding_layout,
				4,
				1, &vulkan::VK_CTX.engine_bindless_material_set[vulkan::VK_CTX.frame_index],
				0, nullptr);
		}
		auto pipeline =  shader->pipeline(vulkan::VK_CTX.current_renderpass, vulkan::VK_CTX.enable_msaa).handle;
		if (pipeline != last_binding_pipeline) {
			last_binding_pipeline = pipeline;
			vkCmdBindPipeline(vulkan::VK_CTX.cmdbuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		}
		if ((draw.vertex_buffer != last_vertex_buffer || draw.index_buffer != last_index_buffer) && indirect_count > 0) {
			++drawcall;
			vkCmdDrawIndexedIndirect(vulkan::VK_CTX.cmdbuf,
				indirect_buffer[vulkan::VK_CTX.frame_index].handle,
				indirect_offset, indirect_count, sizeof(*indirect_cmd));
			indirect_cmd = &indirect_cmd[indirect_count];
			indirect_offset += indirect_count * sizeof(*indirect_cmd);
			indirect_count = 0;
		}
		if (draw.vertex_buffer != last_vertex_buffer) {
			last_vertex_buffer = draw.vertex_buffer;
			VkBuffer vertexBuffers[] = { draw.vertex_buffer };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(vulkan::VK_CTX.cmdbuf, 0, 1, vertexBuffers, offsets);
		}
		if (draw.index_buffer != last_index_buffer) {
			last_index_buffer = draw.index_buffer;
			vkCmdBindIndexBuffer(vulkan::VK_CTX.cmdbuf, draw.index_buffer, 0, VK_INDEX_TYPE_UINT32);
		}
		auto &cmd = indirect_cmd[indirect_count++];
		cmd.firstIndex = draw.index_offset;
		cmd.firstInstance = draw.first;
		cmd.indexCount = draw.index_count;
		cmd.instanceCount = draw.count;
		cmd.vertexOffset = draw.vertex_offset;
	}
	if (indirect_count > 0) {
		++drawcall;
		vkCmdDrawIndexedIndirect(vulkan::VK_CTX.cmdbuf,
			indirect_buffer[vulkan::VK_CTX.frame_index].handle,
			indirect_offset, indirect_count, sizeof(*indirect_cmd));
	}
	indirect_buffer[vulkan::VK_CTX.frame_index].unmap();
}


void
render_system::frame_end(float delta)
{
	ssbo_offset = 0;
	indirect_offset = 0;
	vulkan::vk_surface::inst().post_tick(delta);
	gpu_mesh::instance().post_tick();
	uniform_per_frame->frame_end();
	uniform_per_camera->frame_end();
	uniform_per_object->frame_end();
}

void
render_system::frame_submit()
{
	if (vkEndCommandBuffer(vulkan::VK_CTX.cmdbuf) != VK_SUCCESS)
		return ;
	vulkan::VK_CTX.swapchain.submit(vulkan::VK_CTX.cmdbuf);
	vulkan::vk_ctx_frame_end();
}

void
render_system::show_fps(int fps)
{
	char buf[256];
	snprintf(buf, sizeof(buf), "帝江(%d FPS)", fps);
	vulkan::vk_surface::inst().set_title(buf);
}

bool
render_system::is_running()
{
	return vulkan::vk_surface::inst().is_running();
}


#else


#endif


}}
