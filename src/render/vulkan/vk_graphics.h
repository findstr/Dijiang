#pragma once
#include <memory>
#include "render/draw_object.h"
#include "components/camera.h"
#include "components/light.h"
#include "render/ubo.h"
#include "render/vulkan/vk_render_texture.h"
#include "render/vulkan/vk_surface.h"
#include "render/vulkan/vk_uniform.h"
#include "render/vulkan/vk_shader_variables.h"
#include "render/vulkan/vk_buffer.h"

namespace engine {
namespace vulkan {

class vk_graphics {
private:
	~vk_graphics() {};
private:
	bool ubo_camera_dirty = true;
	bool ubo_lights_dirty = true;
	VkBuffer binding_vertex_buffer = VK_NULL_HANDLE;
	VkBuffer binding_index_buffer = VK_NULL_HANDLE;

	void bind_reset();
	void bind_ubo();
	void bind_mesh(VkBuffer vertex, VkBuffer index);
public:
	void init();
	void cleanup();

	void pre_tick(float delta);
	void tick(float delta);
	void post_tick(float delta);
	/*
	void shadowpass_begin();
	void shadowpass_end();
	*/
	void draw(std::vector<draw_object> &draw, rect &viewport);
	void set_camera(ubo::camera &cam) {
		ubo_camera_dirty = true;
		ubo_camera = cam;
	}
	void set_lights(ubo::lights &li) {
		ubo_lights_dirty = true;
		ubo_lights = li;
	}
	void show_fps(int fps);
	void get_resolution(int *x, int *y);
	bool is_running();
public:
	render_texture *render_target = nullptr;
public:
	static vk_graphics &inst() {
		static vk_graphics *rs = nullptr;
		if (rs == nullptr)
			rs = new vk_graphics();
		return *rs;
	}
private:
	int ssbo_offset = 0;
	int indirect_offset = 0;
	bool acquire_success = true;
	ubo::lights ubo_lights;
	ubo::camera ubo_camera;
	std::array<uint32_t, 3> ubo_offset;
	std::unique_ptr<render_texture> shadow_texture;
	vulkan::vk_buffer indirect_buffer[conf::MAX_FRAMES_IN_FLIGHT];
	int indirect_cmd_count[conf::MAX_FRAMES_IN_FLIGHT];
	std::unique_ptr<vulkan::vk_uniform<ubo::lights, vulkan::ENGINE_PER_FRAME_BINDING>> uniform_lights;
	std::unique_ptr<vulkan::vk_uniform<ubo::camera, vulkan::ENGINE_PER_CAMERA_BINDING>> uniform_camera;
};

}
}

