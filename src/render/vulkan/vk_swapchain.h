#pragma once
#include <array>
#include <memory>
#include "render_texture.h"
#include "conf.h"

namespace engine {
namespace vulkan {

class vk_swapchain {
public:
	enum class acquire_result {
		SUCCESS = 0,
		NOT_READY = 1,
		RECREATE_SWAPCHAIN = 2,
	};
	struct support_details {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};
public:
	void init(VkInstance instace, VkPhysicalDevice phydevice, VkDevice device, VkSurfaceKHR surface, int width, int height);
	void destroy();
	void resize(int wdith, int height);
	VkFramebuffer framebuffer() const;
	acquire_result acquire();
	int submit(VkCommandBuffer cmdbuf);
private:
	struct semaphore {
		VkSemaphore imgavailable = VK_NULL_HANDLE;
		VkSemaphore renderfin = VK_NULL_HANDLE;
		VkFence fence = VK_NULL_HANDLE;
	};
        std::array<semaphore, conf::MAX_FRAMES_IN_FLIGHT> sems;
	std::vector<VkFramebuffer> frame_buffers;
	uint32_t image_index = 0;
	bool rebuild = false;
	VkInstance instance = VK_NULL_HANDLE;
	VkPhysicalDevice phydevice = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkSwapchainKHR handle = VK_NULL_HANDLE;
	VkFormat imageformat;
	std::vector<VkImageView> imageviews;
public:
	VkExtent2D extent = {0, 0};
	VkRenderPass render_pass = VK_NULL_HANDLE;
	static support_details query_support(VkPhysicalDevice device, VkSurfaceKHR surface);
	static VkExtent2D choose_extent(const VkSurfaceCapabilitiesKHR &cap, int width, int height);
	static VkSurfaceFormatKHR choose_surface_format(const std::vector<VkSurfaceFormatKHR> &availableFormats);
	static 	VkPresentModeKHR choose_present_mode(const std::vector<VkPresentModeKHR> &list);
private:
	VkImageView new_view(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
private:
	void create_swapchain(int width, int height);
	void destroy_swapchain();
	void init_renderpass();
	void destroy_renderpass();
	void init_semaphores();
	void destroy_semaphores();
	void init_frame_buffers();
	void destroy_frame_buffers();
};

}}

