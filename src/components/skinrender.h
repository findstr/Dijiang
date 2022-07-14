#pragma once
#include <memory>
#include "render/material.h"
#include "render/mesh.h"
#include "component.h"

namespace engine {

class skinrender : public component {
private:
	std::shared_ptr<render::material> material;
	std::shared_ptr<render::material> shadowcaster;
	std::shared_ptr<render::mesh> skinned_mesh;	
public:
	skinrender(gameobject *go) :component(go) {}
	skinrender(gameobject *go, 
		const std::shared_ptr<render::material> &mat,
		const std::shared_ptr<render::mesh> &mesh
	);
	render::material *get_material();
	render::material *get_shadowcaster();
	render::mesh *get_mesh();
	void set_material(std::shared_ptr<render::material> &m);
	void set_shadowcaster(std::shared_ptr<render::material> &m);
	void set_mesh(std::shared_ptr<render::mesh> &mesh);
public:
	const std::string name() override { return "skillrender"; }
};

}


