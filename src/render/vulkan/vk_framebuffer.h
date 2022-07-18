#pragma once
#include <array>
#include <memory>
#include "vk_ctx.h"
#include "render_texture.h"
#include "vk_depth_buffer.h"
#include "vk_render_texture.h"
#include "conf.h"

namespace engine {
namespace vulkan {

class vk_framebuffer {
public:
	enum class acquire_result {
		SUCCESS = 0,
		NOT_READY = 1,
		RECREATE_SWAPCHAIN = 2,
	};
public:
	vk_framebuffer();
	~vk_framebuffer();
public:
	void resize();
	acquire_result acquire();
	int submit(VkCommandBuffer cmdbuf);
	void set_render_texture(::engine::render_texture *rt);
	VkFramebuffer current() const;
public:
	static vk_framebuffer &inst() {
		static vk_framebuffer *fs = new vk_framebuffer();
		return *fs;
	}
	uint32_t image_index = 0;
private:
	struct semaphore {
		VkSemaphore imgavailable = VK_NULL_HANDLE;
		VkSemaphore renderfin = VK_NULL_HANDLE;
		VkFence fence = VK_NULL_HANDLE;
	};
	struct depth_texture {
		VkImage image = VK_NULL_HANDLE;
		VkImageView view = VK_NULL_HANDLE;
		VmaAllocation allocation = VK_NULL_HANDLE;
	};
	render_texture *rendertexture = nullptr;
        std::array<semaphore, conf::MAX_FRAMES_IN_FLIGHT> sems;
	std::unique_ptr<vk_depth_buffer> depth_texture;
	std::vector<VkFramebuffer> frame_buffers;
private:
	void init_semaphores();
	void destroy_semaphores();
	void init_depth_textures();
	void destroy_depth_textures();
	void init_frame_buffers();
	void destroy_frame_buffers();
};

#define VK_FRAMEBUFFER vk_framebuffer::inst()

}}

