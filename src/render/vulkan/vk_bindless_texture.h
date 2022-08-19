#pragma once
#include "conf.h"
#include <vulkan/vulkan.hpp>
#include "vk_bindless_set.h"
#include "render/texture.h"

namespace engine {
namespace vulkan {

class vk_bindless_texture {
public:
	static vk_bindless_texture &inst() {
		static vk_bindless_texture inst;
		return inst;
	}
public:
	void init();
	void cleanup();
private:
	struct image_desc {
		
	};
	vk_bindless_set<conf::MAX_BINDLESS_RESOURCE> desc_set;
};

}}
