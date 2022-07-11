#pragma once
#include <array>
#include "vk_ctx.h"
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
	VkFramebuffer current() const;
public:
	static vk_framebuffer &inst() {
		static vk_framebuffer *fs = new vk_framebuffer();
		return *fs;
	}
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
	uint32_t image_index = 0;
        std::array<semaphore, conf::MAX_FRAMES_IN_FLIGHT> sems;
	depth_texture depth_texture;
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

