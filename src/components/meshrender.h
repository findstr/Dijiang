#pragma once
#include <memory>
#include "render/material.h"
#include "component.h"

namespace engine {

class meshrender : public component {
private:
	std::shared_ptr<render::material> material;
	std::shared_ptr<render::material> shadowcaster;
public:
	meshrender(gameobject *go) :component(go) {}
	meshrender(gameobject *go, std::shared_ptr<render::material> &m);
	render::material *get_material();
	render::material *get_shadowcaster();
	void set_material(std::shared_ptr<render::material> &m);
	void set_shadowcaster(std::shared_ptr<render::material> &m);
public:
	const std::string name() override { return "meshrender"; }
};

}


