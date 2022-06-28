#pragma once
#include <memory>
#include <functional>
#include "render/texture.h"
#include "render/shader.h"
#include "render/material.h"
#include "render/mesh.h"
#include "render/cubemap.h"
#include "gameobject.h"
#include "animation/skeleton.h"
#include "animation/clip.h"

namespace engine {
namespace resource {

void init();
void cleanup();

std::shared_ptr<render::texture>	load_texture2d(const std::string &file);
std::shared_ptr<render::texture>	load_cubemap(
	const std::array<std::string, render::cubemap::FACE_COUNT> &path);
std::shared_ptr<render::shader>		load_shader(const std::string &file);
std::shared_ptr<render::material>	load_material(const std::string &file);
std::shared_ptr<render::mesh>		load_mesh(const std::string &file);
std::shared_ptr<animation::skeleton>	load_skeleton(const std::string &file);
std::shared_ptr<animation::clip>	load_animation_clip(const std::string &file);
void load_level(const std::string &file, std::function<void(gameobject *, int)> add_go);

}}

