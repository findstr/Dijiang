#pragma once
#include <vulkan/vulkan.h>
#include "texture_wrap.h"

namespace engine {
namespace vulkan {

struct vk_sampler_address_mode {
	constexpr vk_sampler_address_mode(VkSamplerAddressMode v) : value_(v) {}
	constexpr vk_sampler_address_mode(texture_wrap filter) {
		switch (filter) {
		case texture_wrap::CLAMP:
			value_ = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			break;
		case texture_wrap::REPEAT:
			value_ = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			break;
		case texture_wrap::MIRROR:
			value_ = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
			break;
		case texture_wrap::MIRROR_ONCE:
			value_ = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
			break;
		default:
			assert(!"unsupport wrap mode");
			break;
		}
	}
	constexpr operator VkSamplerAddressMode() const { return value_; }
	vk_sampler_address_mode() = delete;
	explicit operator bool() = delete;
private:
	VkSamplerAddressMode value_;
};

}}

