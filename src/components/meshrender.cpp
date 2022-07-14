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

render::material *
meshrender::get_shadowcaster()
{
	return shadowcaster.get();
}

void
meshrender::set_material(std::shared_ptr<render::material> &m)
{
	material = m;
}

void
meshrender::set_shadowcaster(std::shared_ptr<render::material> &m)
{
	shadowcaster = m;
}


}

