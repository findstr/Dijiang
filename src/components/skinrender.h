#pragma once
#include <memory>
#include "render/material.h"
#include "render/mesh.h"
#include "components/meshrender.h"
#include "component.h"

namespace engine {

class skinrender : public meshrender {
private:
	std::shared_ptr<render::mesh> skinned_mesh;	
public:
	skinrender(gameobject *go) : meshrender(go) {}
	skinrender(gameobject *go, 
		std::shared_ptr<render::material> &mat,
		std::shared_ptr<render::mesh> &mesh
	);
	render::mesh *get_mesh();
	void set_mesh(std::shared_ptr<render::mesh> &mesh);
public:
	const std::string name() override { return "skinrender"; }
};

}


