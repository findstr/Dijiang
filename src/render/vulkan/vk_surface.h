#pragma once
#include <vector>
#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

namespace engine {
namespace vulkan {

class vk_surface {
public:
	static vk_surface &inst() {
		static vk_surface s;
		return s;
	};
public:
	void init(const char *title, int width, int height);
	void bind(VkInstance instance, VkSurfaceKHR *surface);
	std::vector<const char *> required_extensions();
	void resolution(int *width, int *height);
	void init_ui(VkSurfaceKHR *surface);
	void pre_tick(float delta);
	void tick(float delta);
	void post_tick(float delta);
	void set_title(const char *name);
	void exit();
	constexpr bool is_running() { return running; };
private:
	bool running = true;
};

}}

