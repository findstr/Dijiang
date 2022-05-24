#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "conf.h"
#include "vk_surface.h"

namespace engine {
namespace vulkan {

struct surface {
	GLFWwindow *window = nullptr;
};

struct surface *SF = nullptr;

std::vector<const char *>
surface_required_extensions(surface *s)
{
	(void)s;
	std::vector<const char *> extensions;
	unsigned int glfwExtensionCount = 0;
	const char **glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	for (unsigned int i = 0; i < glfwExtensionCount; i++) {
		extensions.push_back(glfwExtensions[i]);
	}
	if (conf::enable_validate) {
		extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}
	return extensions;
}


static void
window_resized(GLFWwindow *win, int width, int height) {
	auto s = reinterpret_cast<surface *>(glfwGetWindowUserPointer(win));
	//TODO:
}

surface *
surface_new(const char *name, int width, int height)
{
	struct surface *s = new surface();
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	s->window = glfwCreateWindow(width, height, name, nullptr, nullptr);
	glfwSetWindowUserPointer(s->window, s);
	glfwSetFramebufferSizeCallback(s->window, window_resized);
	printf("surface_new:%p\n", s->window);
	SF = s;
	return s;
}

void
surface_del(surface *s)
{
	auto window = s->window;
	if (window != nullptr) {
		glfwDestroyWindow(window);
		s->window = nullptr;
	}
	glfwTerminate();
	delete s;
}

int
surface_bind(surface *s, VkInstance instance, VkSurfaceKHR *surface)
{
	auto ret = glfwCreateWindowSurface(instance, s->window, nullptr, surface);
	return (ret == VK_SUCCESS) ? 0 : -1;
}

int
surface_poll(struct surface *s)
{
	if (glfwWindowShouldClose(s->window)) {
		return -1;
	}
	glfwPollEvents();
	return 0;
}

bool
surface_key_down(key k)
{
	auto *s = SF;
	int state;
	switch (k) {
	case key::A:
		state = glfwGetKey(s->window, GLFW_KEY_A);
		break;
	case key::S:
		state = glfwGetKey(s->window, GLFW_KEY_S);
		break;
	case key::D:
		state = glfwGetKey(s->window, GLFW_KEY_D);
		break;
	case key::W:
		state = glfwGetKey(s->window, GLFW_KEY_W);
		break;
	default:
		assert(!"unsupport key code");
		break;
	}
	return state == GLFW_PRESS;
}

}}

