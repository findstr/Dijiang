#pragma once
#include <memory>
#include "render/draw_object.h"
#include "components/camera.h"
#include "render/ubo.h"
#ifdef USE_VULKAN
#include "render/vulkan/vk_surface.h"
#include "render/vulkan/vk_uniform.h"
#include "render/vulkan/vk_shader_variables.h"
#endif

namespace engine {


class render_system {
public:
	render_system();
	~render_system();
	int frame_begin(float delta);
	void set_camera(camera *cam);
	void draw(draw_object &draw);
	void frame_end(float delta);
	void get_resolution(int *x, int *y);
public:
	static render_system &inst() {
		static render_system *rs = nullptr;
		if (rs == nullptr)
			rs = new render_system();
		return *rs;
	}
private:
#if USE_VULKAN
	bool acquire_success = true;
	vulkan::surface *surface = nullptr;
	std::array<uint32_t, 3> ubo_offset;
	VkViewport viewport;
	std::unique_ptr<vulkan::vk_uniform<render::ubo::per_frame, vulkan::ENGINE_PER_FRAME_BINDING>> uniform_per_frame;
	std::unique_ptr<vulkan::vk_uniform<render::ubo::per_camera, vulkan::ENGINE_PER_CAMERA_BINDING>> uniform_per_camera;
	std::unique_ptr<vulkan::vk_uniform<render::ubo::per_object, vulkan::ENGINE_PER_OBJECT_BINDING>> uniform_per_object;
#endif
};


#define RENDER_SYSTEM	::engine::render_system::inst()


}

