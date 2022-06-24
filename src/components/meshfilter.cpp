#include "meshfilter.h"

namespace engine {


meshfilter::meshfilter(gameobject *go, std::shared_ptr<render::mesh> &m) :
	component(go)
{
	set_mesh(m);
}

render::mesh *
meshfilter::get_mesh()
{
	return mesh.get();
}

void
meshfilter::set_mesh(std::shared_ptr<render::mesh> &m)
{
	mesh = m;
}


}

