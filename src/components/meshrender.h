#pragma once
#include <memory>
#include "render/material.h"
#include "component.h"

namespace engine {

class meshrender : public component {
private:
	std::shared_ptr<render::material> material;
public:
	meshrender(gameobject *go) :component(go) {}
	meshrender(gameobject *go, std::shared_ptr<render::material> &m);
	render::material *get_material();
	void set_material(std::shared_ptr<render::material> &m);
public:
	const std::string type() override { return "meshrender"; }
};

}


