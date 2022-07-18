#pragma once
#include <vulkan/vulkan.h>
#include "texture_format.h"

namespace engine {
namespace vulkan {

struct vk_format {
	constexpr vk_format(VkFormat v) : value_(v) {}
	constexpr vk_format(texture_format fmt, bool linear) {
		if (linear) {
			switch (fmt) {
			case texture_format::R32:
				value_ = VK_FORMAT_R32_SFLOAT;
				break;
			case texture_format::RGBA64:
				value_ = VK_FORMAT_R16G16B16A16_UNORM;
				break;
			case texture_format::RGBA32:
				value_ = VK_FORMAT_R8G8B8A8_UNORM;
				break;
			case texture_format::RGB24:
				value_ = VK_FORMAT_R8G8B8_UNORM;
				break;
			case texture_format::D32:
				value_ = VK_FORMAT_D32_SFLOAT;
				break;
			case texture_format::D24S8:
				value_ = VK_FORMAT_D24_UNORM_S8_UINT;
				break;
			case texture_format::D32S8:
				value_ = VK_FORMAT_D32_SFLOAT_S8_UINT;
				break;
			default:
				assert(!"unsupport format");
				break;
			}
		} else {
			switch (fmt) {
			case texture_format::R32:
				value_ = VK_FORMAT_R32_SFLOAT;
				break;
			case texture_format::RGBA64:
				value_ = VK_FORMAT_R16G16B16A16_SFLOAT;
				break;
			case texture_format::RGBA32:
				value_ = VK_FORMAT_R8G8B8A8_SRGB;
				break;
			case texture_format::RGB24:
				value_ = VK_FORMAT_R8G8B8_SRGB;
				break;
			case texture_format::D32:
				value_ = VK_FORMAT_D32_SFLOAT;
				break;
			case texture_format::D24S8:
				value_ = VK_FORMAT_D24_UNORM_S8_UINT;
				break;
			case texture_format::D32S8:
				value_ = VK_FORMAT_D32_SFLOAT_S8_UINT;
				break;
			default:
				assert(!"unsupport format");
				break;
			}
		}
	}
	constexpr size_t size() const {
		switch (value_) {
		case VK_FORMAT_R16G16B16A16_UNORM:
		case VK_FORMAT_R16G16B16A16_SFLOAT:
			return 8;
		case VK_FORMAT_R32_SFLOAT:
		case VK_FORMAT_R8G8B8A8_UNORM:
		case VK_FORMAT_R8G8B8A8_SRGB:
			return 4;
		case VK_FORMAT_R8G8B8_UNORM:
		case VK_FORMAT_R8G8B8_SRGB:
			return 3;
		default:
			assert(!"unsupport format");
			return 0;
		}
	}
	constexpr VkImageAspectFlags aspect() const {
		switch(value_) {
		case VK_FORMAT_R32_SFLOAT:
		case VK_FORMAT_R16G16B16A16_SFLOAT:
		case VK_FORMAT_R8G8B8A8_SRGB:
		case VK_FORMAT_R8G8B8_SRGB:
		case VK_FORMAT_R16G16B16A16_UNORM:
		case VK_FORMAT_R8G8B8A8_UNORM:
		case VK_FORMAT_R8G8B8_UNORM:
			return VK_IMAGE_ASPECT_COLOR_BIT;
		case VK_FORMAT_D32_SFLOAT:
			return VK_IMAGE_ASPECT_DEPTH_BIT;
		case VK_FORMAT_D24_UNORM_S8_UINT:
		case VK_FORMAT_D32_SFLOAT_S8_UINT:
			return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		default:
			assert(!"unsupport format");
			return VK_IMAGE_ASPECT_COLOR_BIT;
		}
	}
	constexpr operator VkFormat() const { return value_; }
	vk_format() = delete;
	explicit operator bool() = delete;
private:
	VkFormat value_;
};

}}

