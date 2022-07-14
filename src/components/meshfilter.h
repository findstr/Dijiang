#pragma once
#include <memory>
#include "render/mesh.h"
#include "component.h"

namespace engine {

class meshfilter : public component {
private:
	std::shared_ptr<render::mesh> mesh;
public:
	meshfilter(gameobject *go) : component(go) {}
	meshfilter(gameobject *go, std::shared_ptr<render::mesh> &m);
	render::mesh *get_mesh();
	void set_mesh(std::shared_ptr<render::mesh> &m);
	const std::string name() override { return "meshfilter"; }
};

}

