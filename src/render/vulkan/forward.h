#pragma once
#include "vk_object.h"
#include "vk_shader.h"
#include "mesh.h"
#include "material.h"
#include "render/draw_object.h"
#include "components/camera.h"
namespace engine {
namespace vulkan {

struct forward;

forward *forward_new(const renderctx *ctx);
void forward_del(const renderctx *ctx, forward *fw);
void forward_begin(forward *fw, int obj_count);
void forward_end(forward *fw);
void forward_tick(camera *cam, const renderctx *ctx, forward *fw, const draw_object &draw);


}}

