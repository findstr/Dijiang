#pragma once
#include <optional>
#include "render/texture.h"
#include "vk_buffer.h"

namespace engine {
namespace vulkan {

struct vk_texture {
	friend std::vector<vk_texture>;
	friend delete_queue<vk_texture>;
public:
	vk_texture() {
	}
	vk_texture(vk_texture&& a) {
		clear();
		this->image = a.image;
		this->view = a.view;
		this->allocation = a.allocation;
		a.image = VK_NULL_HANDLE;
		a.view = VK_NULL_HANDLE;
		a.allocation = VK_NULL_HANDLE;
	}
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
private:
	void clear();
private:
	static std::vector<vk_texture> texture_pool;
	static std::vector<texture_handle_t> freeids;	
public:
	static texture_handle_t upload_texture(render::texture &tex);
	static void unload_texture(texture_handle_t handle);
	static const vk_texture *get(texture_handle_t handle) {
		return &texture_pool[handle];
	}
};

}}

