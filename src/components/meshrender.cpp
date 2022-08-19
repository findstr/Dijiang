#include "meshrender.h"

namespace engine {

meshrender::meshrender(gameobject *go, std::shared_ptr<render::material> &m) :
	 component(go)
{
	add_material(m);
}

render::material *
meshrender::get_material(enum render::shader::light_mode mode)
{
	if (mode == render::shader::light_mode::SHADOWCASTER)
		return shadow.get();
	else
		return material.get();
}

void
meshrender::add_material(std::shared_ptr<render::material> &m)
{
	if (m->get_shader()->light_mode == render::shader::light_mode::SHADOWCASTER) {
		shadow = m;
	} else {
		assert(material.get() == nullptr); //TODO: support multi material
		material = m;
	}
}

}

