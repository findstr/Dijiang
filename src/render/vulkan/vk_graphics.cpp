#include "level.h"
#include "render_pass.h"
#include "render/vulkan/vk_ctx.h"
#include "render/vulkan/vk_native.h"
#include "render/vulkan/vk_surface.h"
#include "render/vulkan/vk_shader.h"
#include "render/vulkan/vk_shader_variables.h"
#include "render/vulkan/vk_set_write.h"
#include "vk_graphics.h"
#include "vk_ctx.h"

namespace engine {
namespace vulkan {

void
vk_graphics::init()
{
	int width, height;
	vulkan::vk_surface::inst().init("帝江", 1024, 768);
	vulkan::vk_surface::inst().resolution(&width, &height);
	vulkan::vk_ctx_init("帝江", width, height);
	vulkan::vk_surface::inst().init_ui(&vulkan::VK_CTX.surface);

	auto *ptr = uniform_lights.get();

	uniform_lights.reset(new vulkan::vk_uniform<ubo::lights, vulkan::ENGINE_PER_FRAME_BINDING>());
	uniform_camera.reset(new vulkan::vk_uniform<ubo::camera, vulkan::ENGINE_PER_CAMERA_BINDING>());
	for (int i = 0; i < conf::MAX_FRAMES_IN_FLIGHT; i++) {
		indirect_cmd_count[i] = 0;
		indirect_buffer[i].create(vulkan::vk_buffer::type::INDIRECT, 1024*sizeof(VkDrawIndexedIndirectCommand));
	}
}

void
vk_graphics::cleanup()
{
	vulkan::vk_surface::inst().exit();
	vulkan::vk_ctx_cleanup();
}

void
vk_graphics::get_resolution(int *x, int *y)
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
update_uniformbuffer_per_object(ubo::camera *ubo_cam, ubo::per_object *ubo, const draw_object &draw) 
{
	ubo->material = draw.material_offset;
	ubo->model = to_mat4(draw.go->transform.local_to_world_matrix());
	ubo->model_view_proj = ubo_cam->view_proj * ubo->model;
	if (draw.skeleton_pose != nullptr) {
		/*
		int bone_count = std::min(draw.skeleton_pose->size(), ubo->skeleton_pose.size());
		for (int k = 0; k < bone_count; k++) {
			ubo->skeleton_pose[k] = to_mat4((*draw.skeleton_pose)[k].matrix);
		}
		*/
	}
}

	
#if 0
void
vk_graphics::shadowpass_begin()
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
vk_graphics::shadowpass_end()
{
	vulkan::vk_ctx_debug_label_end();
	vkCmdEndRenderPass(vulkan::VK_CTX.cmdbuf);
}
	
#endif

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
	first_draw.index_count = vk_mesh::inst().bind_info(
		first_draw.mesh->handle(),
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
			new_draw.index_count = vk_mesh::inst().bind_info(
				new_draw.mesh->handle(),
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
vk_graphics::bind_reset() 
{
	ubo_camera_dirty = false;
	ubo_lights_dirty = false;
	binding_vertex_buffer = VK_NULL_HANDLE;
	binding_index_buffer = VK_NULL_HANDLE;
}

void 
vk_graphics::bind_ubo() 
{
	if (ubo_camera_dirty) {
		ubo_camera_dirty = false;
		auto ubo_per_camera = uniform_camera->alloc();
		*ubo_per_camera = ubo_camera;
		ubo_offset[vulkan::ENGINE_PER_CAMERA_BINDING] = uniform_camera->offset();
		uniform_camera->unmap();
	}
	if (ubo_lights_dirty) {
		ubo_lights_dirty = false;
		auto *ubo_per_frame = uniform_lights->alloc();
		*ubo_per_frame = ubo_lights;
		ubo_offset[vulkan::ENGINE_PER_FRAME_BINDING] = uniform_lights->offset();
		uniform_lights->unmap();	
	}
	vkCmdBindDescriptorSets(
		vulkan::VK_CTX.cmdbuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
		vulkan::VK_CTX.engine_bindless_pipeline_layout,
		1,
		1, &vulkan::VK_CTX.engine_desc_set[vulkan::VK_CTX.frame_index],
		ubo_offset.size(), ubo_offset.data());
}

bool
vk_graphics::need_switch_mesh(VkBuffer vertex, VkBuffer index) 
{
	return (vertex != binding_vertex_buffer) ||
		(index != binding_index_buffer);
}



void
vk_graphics::bind_mesh(VkBuffer vertex, VkBuffer index) 
{
	if (vertex != binding_vertex_buffer) {
		binding_vertex_buffer = vertex;
		VkBuffer vertexBuffers[] = { vertex };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(vulkan::VK_CTX.cmdbuf, 0, 1, vertexBuffers, offsets);
	}
	if (index != binding_index_buffer) {
		binding_index_buffer = index;
		vkCmdBindIndexBuffer(vulkan::VK_CTX.cmdbuf, index, 0, VK_INDEX_TYPE_UINT32);
	}
}

void
vk_graphics::draw(std::vector<draw_object> &draw_list, rect &rt)
{
	VkPipeline last_binding_pipeline = VK_NULL_HANDLE;
	render::material *last_binding_mat = VK_NULL_HANDLE;
	VkBuffer last_vertex_buffer = VK_NULL_HANDLE;
	VkBuffer last_index_buffer = VK_NULL_HANDLE;

	auto vk_rt = (vk_render_texture *)render_target;
	VK_CTX.current_renderpass = vk_rt->render_pass;
	VK_CTX.current_framebuffer = vk_rt->framebuffer();
	VK_CTX.enable_msaa = vk_rt->enable_msaa;
	vk_rt->begin();

	VkViewport viewport;
	viewport.x = rt.x;
	viewport.y = rt.y;
	viewport.width = rt.width * vulkan::VK_CTX.swapchain.extent.width;
	viewport.height = rt.height * vulkan::VK_CTX.swapchain.extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(vulkan::VK_CTX.cmdbuf, 0, 1, &viewport);

	bind_ubo();

	std::vector<material_info> materials;
	auto draw_batches = compact_draws(draw_list, materials);
	uint8_t *material_buffer = (uint8_t *)vulkan::VK_CTX.engine_bindless_material[vulkan::VK_CTX.frame_index]->map();
	for (int i = 0; i < (int)materials.size(); i++) {
		auto &m = materials[i];
		m.material->copy_to(material_buffer + m.material_offset);
	}
	vulkan::VK_CTX.engine_bindless_material[vulkan::VK_CTX.frame_index]->unmap();
	

	auto *ubo = (ubo::per_object *)vulkan::VK_CTX.engine_bindless_object[vulkan::VK_CTX.frame_index]->map();
	for (int i = 0; i < draw_list.size(); i++) {
		auto &draw = draw_list[i];
		auto u = &ubo[ssbo_offset + i];
		update_uniformbuffer_per_object(&ubo_camera, u, draw);
	}
	vulkan::VK_CTX.engine_bindless_object[vulkan::VK_CTX.frame_index]->unmap();// ssbo_offset * sizeof(*ubo), draw_list.size() * sizeof(*ubo));
	ssbo_offset += draw_list.size();



	int indirect_count = 0;
	auto *indirect_cmd = (VkDrawIndexedIndirectCommand *)indirect_buffer[vulkan::VK_CTX.frame_index].map();
	int drawcall = 0;
	for (auto &draw:draw_batches) {
		auto mat = draw.material;
		auto *shader = (engine::vulkan::vk_shader *)mat->get_shader();
		auto pipeline =  shader->pipeline(vulkan::VK_CTX.current_renderpass, vulkan::VK_CTX.enable_msaa).handle;
		if (shader->zwrite == false)
			int a = 3;
		bool same_pipeline  = pipeline == last_binding_pipeline;
		if ((need_switch_mesh(draw.vertex_buffer, draw.index_buffer) || !same_pipeline) && indirect_count > 0) {
			++drawcall;
			vkCmdDrawIndexedIndirect(vulkan::VK_CTX.cmdbuf,
				indirect_buffer[vulkan::VK_CTX.frame_index].handle,
				indirect_offset, indirect_count, sizeof(*indirect_cmd));
			indirect_cmd = &indirect_cmd[indirect_count];
			indirect_offset += indirect_count * sizeof(*indirect_cmd);
			indirect_count = 0;
		}
		if (!same_pipeline) {
			last_binding_pipeline = pipeline;
			vkCmdBindPipeline(vulkan::VK_CTX.cmdbuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		}
		bind_mesh(draw.vertex_buffer, draw.index_buffer);
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

	vulkan::vk_ctx_renderpass_end();
}

void
vk_graphics::pre_tick(float delta)
{
	vulkan::vk_set_write::inst().flush();
	vk_mesh::inst().flush();
	vulkan::vk_ctx_frame_begin();
	vulkan::vk_surface::inst().pre_tick(delta);

	uniform_lights->frame_begin(1024);
	uniform_camera->frame_begin(1024);

	for (int i = 0; i < ubo_offset.size(); i++)
		ubo_offset[i] = 0;
	
	vkCmdBindDescriptorSets(
		vulkan::VK_CTX.cmdbuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
		vulkan::VK_CTX.engine_bindless_pipeline_layout,
		2,
		1, &vulkan::VK_CTX.engine_bindless_texture_set,
		0, nullptr);
	vkCmdBindDescriptorSets(
		vulkan::VK_CTX.cmdbuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
		vulkan::VK_CTX.engine_bindless_pipeline_layout,
		3,
		1, &vulkan::VK_CTX.engine_bindless_object_set[vulkan::VK_CTX.frame_index],
		0, nullptr);
	vkCmdBindDescriptorSets(
		vulkan::VK_CTX.cmdbuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
		vulkan::VK_CTX.engine_bindless_pipeline_layout,
		4,
		1, &vulkan::VK_CTX.engine_bindless_material_set[vulkan::VK_CTX.frame_index],
		0, nullptr);
	bind_reset();
}

void
vk_graphics::tick(float delta)
{
	ssbo_offset = 0;
	indirect_offset = 0;
	int width, height;
	auto result = vulkan::VK_CTX.swapchain.acquire();
	switch (result) {
	case vulkan::vk_swapchain::acquire_result::NOT_READY:
		vkEndCommandBuffer(vulkan::VK_CTX.cmdbuf);
		acquire_success = false;
		return ;
	case vulkan::vk_swapchain::acquire_result::RECREATE_SWAPCHAIN:
		vulkan::vk_surface::inst().resolution(&width, &height);
		vulkan::VK_CTX.swapchain.resize(width, height);
		vkEndCommandBuffer(vulkan::VK_CTX.cmdbuf);
		return ;
	case vulkan::vk_swapchain::acquire_result::SUCCESS:
		break;
	}
	vulkan::vk_surface::inst().tick(delta);
	if (vkEndCommandBuffer(vulkan::VK_CTX.cmdbuf) != VK_SUCCESS)
		return ;
	vulkan::VK_CTX.swapchain.submit(vulkan::VK_CTX.cmdbuf);
}

void
vk_graphics::post_tick(float delta)
{
	vulkan::vk_surface::inst().post_tick(delta);
	vulkan::vk_ctx_frame_end();
	uniform_lights->frame_end();
	uniform_camera->frame_end();
	return ;
}

void
vk_graphics::show_fps(int fps)
{
	char buf[256];
	snprintf(buf, sizeof(buf), "帝江(%d FPS)", fps);
	vulkan::vk_surface::inst().set_title(buf);
}

bool
vk_graphics::is_running()
{
	return vulkan::vk_surface::inst().is_running();
}

}}

