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
	bool frame_begin(float delta);
	void draw(camera *cam, draw_object &draw);
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
	std::unique_ptr<vulkan::vk_uniform<render::ubo::per_frame, vulkan::ENGINE_PER_FRAME_BINDING>> uniform_per_frame;
	std::unique_ptr<vulkan::vk_uniform<render::ubo::per_draw, vulkan::ENGINE_PER_DRAW_BINDING>> uniform_per_draw;
	render::ubo::per_frame *per_frame_buffer = nullptr;
	int per_frame_offset = 0;
#endif
};


#define RENDER_SYSTEM	::engine::render_system::inst()


}

