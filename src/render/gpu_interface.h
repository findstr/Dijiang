#pragma once
#ifdef USE_VULKAN
#include "vulkan/vk_gpu_mesh.h"

using ::engine::vulkan::gpu_mesh;
#else
#error("gpu interface not be implemented");
#endif
