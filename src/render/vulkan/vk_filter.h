#pragma once
#include <vulkan/vulkan.h>
#include "texture_filter.h"

namespace engine {
namespace vulkan {

struct vk_filter {
	constexpr vk_filter(VkFilter v) : value_(v) {}
	constexpr vk_filter(texture_filter filter) {
		switch (filter) {
		default:
		case texture_filter::POINT:
			value_ = VK_FILTER_NEAREST;
			break;
		case texture_filter::BILINEAR:
			value_ = VK_FILTER_LINEAR;
			break;
		case texture_filter::TRILINEAR:
			value_ = VK_FILTER_CUBIC_IMG;
			break;
		}
	}
	constexpr operator VkFilter() const { return value_; }
	vk_filter() = delete;
	explicit operator bool() = delete;
private:
	VkFilter value_;
};

}}

