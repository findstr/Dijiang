#pragma once
#include "vk_object.h"

namespace engine {
namespace vulkan {

struct framesync;
framesync *framesync_new(const renderctx *ctx);
void framesync_del(const renderctx *ctx, framesync *fs);
int framesync_aquire(const renderctx *ctx, const framesync *fs, int *image_index);
int framesync_submit(const renderctx *ctx, const framesync *fs,
	VkCommandBuffer cmdbuf, int image_index);

}}

