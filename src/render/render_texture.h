#pragma once
#include <memory>
#include <array>
#include "texture_format.h"
#include "texture.h"

#if USE_VULKAN
#include "vk_render_texture.h"
namespace engine {
using render_texture = vulkan::vk_render_texture;
}
#else
#error("render_texture not implemented");
#endif


