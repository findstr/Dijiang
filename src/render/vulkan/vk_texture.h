#pragma once
#include <optional>
#include "vk_object.h"
#include "render/texture.h"
#include "vk_buffer.h"

namespace engine {
namespace vulkan {

struct vk_texture {
public:
	~vk_texture();
public:
	void destroy();
	void create(const render::texture *tex, VkImageUsageFlags usage, int layer_count = 1, VkSampleCountFlagBits samplecount = VK_SAMPLE_COUNT_1_BIT);
	void transition_layout(const render::texture *tex, VkImageLayout from, VkImageLayout to, int layer_count = 1);
	void fill(const render::texture *tex, vk_buffer &staging, int layer_count = 1);
	void gen_mipmap(const render::texture *tex, int layer_count = 1);
public:
	VkImage image = VK_NULL_HANDLE;
	VkImageView view = VK_NULL_HANDLE;
	VmaAllocation allocation = VK_NULL_HANDLE;
};

}}

