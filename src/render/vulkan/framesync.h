#pragma once
#include "vk_object.h"

namespace engine {
namespace vulkan {

struct framesync;
framesync *framesync_new();
void framesync_del(framesync *fs);
int framesync_aquire(const framesync *fs, int *image_index);
int framesync_submit(const framesync *fs,
 VkCommandBuffer cmdbuf,
	int image_index);

}}

