#pragma once
#include "conf.h"
#include "vk_ctx.h"
#include "vk_object.h"
#include "vk_texture.h"
#include "render/render_texture.h"

namespace engine {
namespace vulkan {

class vk_render_texture : public render_texture {
public:
	void apply();
	intptr_t handle() const override { return (intptr_t)(color_buffer[VK_CTX.frame_index].view); }
	intptr_t sampler() const override { return (intptr_t)color_sampler[VK_CTX.frame_index]; }
	VkImageView imageview(int frame_index) { return color_buffer[frame_index].view; }
	VkSampler sampler(int frame_index) { return color_sampler[frame_index]; }
	VkFramebuffer framebuffer() { return framebuffers[VK_CTX.frame_index]; }
public:
	void begin();
	void end();
private:
	void create_colorbuffer();
	void create_depthbuffer();
public:
	bool enable_msaa = false;
	VkRenderPass render_pass;
	vk_texture color_buffer[conf::MAX_FRAMES_IN_FLIGHT];
	VkSampler color_sampler[conf::MAX_FRAMES_IN_FLIGHT];
	vk_texture msaa_buffer[conf::MAX_FRAMES_IN_FLIGHT];
	VkSampler msaa_sampler[conf::MAX_FRAMES_IN_FLIGHT];
	vk_texture depth_buffer;
	VkSampler depth_sampler;
	VkFramebuffer framebuffers[conf::MAX_FRAMES_IN_FLIGHT];
};

}}

