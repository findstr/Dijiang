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

forward *forward_new();
void forward_del(forward *fw);
void forward_begin(forward *fw, int obj_count);
void forward_end(forward *fw);
void forward_tick(camera *cam, forward *fw, const draw_object &draw);


}}

