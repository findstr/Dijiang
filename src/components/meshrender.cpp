#include "meshrender.h"

namespace engine {

meshrender::meshrender(gameobject *go, std::shared_ptr<render::material> &m) :
	 component(go)
{
	add_material(m);
}

render::material *
meshrender::get_material(render_pass::path path)
{
	if (path == render_pass::SHADOW)
		return shadow.get();
	else
		return material.get();
}

void
meshrender::add_material(std::shared_ptr<render::material> &m)
{
	if (m->render_path() == render_pass::SHADOW) {
		shadow = m;
	} else {
		assert(material.get() == nullptr); //TODO: support multi material
		material = m;
	}
}

}

