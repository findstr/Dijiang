#pragma once
#include "vk_object.h"
#include <optional>
#include "render/texture.h"
#include "vk_buffer.h"

namespace engine {
namespace vulkan {

void texture_del(textureEx &tex);
void texture_del(const renderctx *ctx, textureEx &tex);

std::optional<textureEx> texture_new(
	uint32_t width, uint32_t height,
	const texture_setting &setting);


std::optional<textureEx> texture_new(
	const renderctx *ctx,
	uint32_t width, uint32_t height,
	const texture_setting &setting);

std::optional<textureEx> texture_new_depth(
	const renderctx *ctx,
	uint32_t width, uint32_t height,
	const texture_setting &setting);


VkImageView texture_new_view(
		const renderctx *ctx,
		VkImage image,
		VkFormat format,
		VkImageAspectFlags aspectFlags,
		uint32_t mipLevels);

void texture_transition_layout(
		const renderctx *ctx,
		textureEx &tex, VkFormat format, int miplevels,
		VkImageLayout from, VkImageLayout to);


void texture_gen_mipmap(const renderctx *ctx,
		textureEx &tex, VkFormat imageFormat,
		int32_t width, int32_t height, int32_t mipLevels);

void texture_fill(const renderctx *ctx,
		textureEx &tex, const vk_buffer &buf,
		int32_t width, int32_t height);

void texture_upload(textureEx &tex, int width, int height, void *pixel, size_t sz, int miplevels);

class texture : public render::texture {
public:
	~texture() override {}
	void apply() override;
public:
	std::optional<textureEx> handle;
};

}}

