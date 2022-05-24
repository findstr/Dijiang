#pragma once
#include "camera.h"
#include "component.h"
#include "render/vulkan/vk_surface.h"

namespace engine {

class camcontrol : public component {
public:
	camcontrol(gameobject *go) : component(go) {}
	const std::string type()  override { return "camcontrol"; }
	void tick(float delta) override {
		camera *cam = (camera *)get_sibling("camera");
		if (vulkan::surface_key_down(vulkan::key::W)) {
			transform->position += cam->forward() * 0.1f;
		} else if (vulkan::surface_key_down(vulkan::key::S)) {
			transform->position += cam->back() * 0.1f;
		} else if (vulkan::surface_key_down(vulkan::key::A)) {
			transform->position += cam->left() * 0.1f;
		} else if (vulkan::surface_key_down(vulkan::key::D)) {
			transform->position += cam->right() * 0.1f;
		}
	}
};

}

