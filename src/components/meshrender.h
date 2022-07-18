#pragma once
#include <memory>
#include "render/material.h"
#include "render/render_pass.h"
#include "component.h"

namespace engine {

class meshrender : public component {
private:
	std::shared_ptr<render::material> material;
	std::shared_ptr<render::material> shadow;
public:
	meshrender(gameobject *go) :component(go) {}
	meshrender(gameobject *go, std::shared_ptr<render::material> &m);
	render::material *get_material(render_pass::path path);
	void add_material(std::shared_ptr<render::material> &m);
public:
	const std::string name() override { return "meshrender"; }
};

}


