#pragma once
#include "gpu_resource.h"
#ifdef USE_VULKAN
#include "vulkan/vk_gpu.h"
namespace engine {
	using gpu_ = ::engine::vulkan::vk_gpu;
}

#else
#error("gpu interface not be implemented");
#endif


#define gpu gpu_::inst()
