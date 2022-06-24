#pragma once
#include "render/debugger.h"
namespace engine {
namespace vulkan {

class vk_debugger : public render::debugger {
public:
	void init() override;
};
	
}}
