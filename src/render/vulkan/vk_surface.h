#pragma once
#include <vector>
#include <vulkan/vulkan.h>

namespace engine {
namespace vulkan {

struct surface;

surface *surface_new(const char *name, int width, int height);
std::vector<const char *> surface_required_extensions(surface *s);
int surface_bind(surface *s, VkInstance instance, VkSurfaceKHR *surface);
int surface_poll(surface *s);
void surface_del(surface *s);

}}

