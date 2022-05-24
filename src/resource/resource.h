#pragma once
#include <memory>
#include <functional>
#include "render/texture.h"
#include "render/shader.h"
#include "render/material.h"
#include "render/mesh.h"
#include "framework/gameobject.h"

namespace engine {
namespace resource {

void init();
void cleanup();

std::shared_ptr<render::texture> load_texture(const std::string &file);
std::shared_ptr<render::shader>  load_shader(const std::string &file);
std::shared_ptr<render::material>load_material(const std::string &file);
std::shared_ptr<render::mesh>    load_mesh(const std::string &file);
void load_level(const std::string &file, std::function<void(gameobject *)> add_go);

}}

