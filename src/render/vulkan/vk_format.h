#pragma once
#include <vulkan/vulkan.h>
#include "texture_format.h"

namespace engine {
namespace vulkan {

struct vk_format {
	constexpr vk_format(VkFormat v) : value_(v) {}
	constexpr vk_format(texture_format fmt, bool linear = true) {
		switch (fmt) {
		case texture_format::RGBA32:
			value_ = VK_FORMAT_R8G8B8A8_SRGB;
			break;
		case texture_format::RGB24:
			value_ = VK_FORMAT_R8G8B8_SRGB;
			break;
		default:
			assert(!"unsupport format");
			break;
		}
	}
	constexpr size_t size() {
		switch (value_) {
		case VK_FORMAT_R8G8B8A8_SRGB:
			return 4;
		case VK_FORMAT_R8G8B8_SRGB:
			return 3;
		default:
			assert(!"unsupport format");
			return 0;
		}
	}
	constexpr operator VkFormat() const { return value_; }
	vk_format() = delete;
	explicit operator bool() = delete;
private:
	VkFormat value_;
};

}}

