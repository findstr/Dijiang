#include "meshrender.h"

namespace engine {

meshrender::meshrender(gameobject *go, std::shared_ptr<render::material> &m) :
	 component(go)
{
	set_material(m);
}

render::material *
meshrender::get_material()
{
	return material.get();
}

void
meshrender::set_material(std::shared_ptr<render::material> &m)
{
	material = m;
}


}

