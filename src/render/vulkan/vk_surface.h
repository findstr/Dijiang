#pragma once
#include <vector>
#include <vulkan/vulkan.h>

namespace engine {
namespace vulkan {

struct surface;

surface *surface_new(const char *name, int width, int height);
std::vector<const char *> surface_required_extensions(surface *s);
void surface_size(surface *s, int *width, int *height);
int surface_bind(surface *s, VkInstance instance, VkSurfaceKHR *surface);
int surface_initui(surface *s, const VkSurfaceKHR *surface);
int surface_pre_tick(struct surface *s);
int surface_tick(struct surface *s);
int surface_post_tick(struct surface *s);
void surface_del(surface *s);

}}

