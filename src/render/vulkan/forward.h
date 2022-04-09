#pragma once
#include "vk_object.h"
#include "vk_shader.h"
namespace engine {
namespace vulkan {

struct forward;

forward *forward_new(const renderctx *ctx);
void forward_del(const renderctx *ctx, forward *fw);
void forward_tick(const renderctx *ctx, forward *fw);


}}

