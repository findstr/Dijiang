#pragma once
#include <array>
#include "conf.h"
#include "vk_ctx.h"
#include "vk_texture.h"
#include "gpu_resource.h"
#include "render/texture_format.h"

namespace engine {
namespace vulkan {

class vk_render_texture : public render::texture {
public:

	vk_render_texture() : render::texture(render::texture::TEX2D) {}
	vk_render_texture(int width, int height,
		texture_format color_format = texture_format::RGBA32,
		texture_format depth_format = texture_format::D32S8,
		bool linear = false, bool mipmap = false) : 
		render::texture(render::texture::TEX2D) 
	{
		this->width_ = width;
		this->height_ = height;
		this->format = color_format;
		this->depth_format = depth_format;
		this->linear = linear;
		this->mipmap = mipmap;
	}
public:
	void apply();
	intptr_t handle(int i) const { return (intptr_t)(color_buffer[i].view); }
	intptr_t sampler(int i) const { return (intptr_t)color_sampler[i]; }
	VkImageView imageview(int frame_index) { return color_buffer[frame_index].view; }
	VkSampler sampler(int frame_index) { return color_sampler[frame_index]; }
	VkFramebuffer framebuffer() { return framebuffers[VK_CTX.frame_index]; }
public:
	void begin();
	void end();
private:
	void create_colorbuffer();
	void create_depthbuffer();
	texture_format depth_format;
public:
	bool mipmap = false;
	bool enable_msaa = false;
	VkRenderPass render_pass;
	vk_texture color_buffer[conf::MAX_FRAMES_IN_FLIGHT];
	VkSampler color_sampler[conf::MAX_FRAMES_IN_FLIGHT];
	vk_texture msaa_buffer[conf::MAX_FRAMES_IN_FLIGHT];
	VkSampler msaa_sampler[conf::MAX_FRAMES_IN_FLIGHT];
	vk_texture depth_buffer;
	VkSampler depth_sampler = VK_NULL_HANDLE;
	VkFramebuffer framebuffers[conf::MAX_FRAMES_IN_FLIGHT];
};

}}

