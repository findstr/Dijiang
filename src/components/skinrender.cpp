#include "skinrender.h"

namespace engine {

skinrender::skinrender(
	gameobject *go,
	const std::shared_ptr<render::material> &mat,
	const std::shared_ptr<render::mesh> &m):
	component(go), material(mat), skinned_mesh(m)
{
	
}

render::material *
skinrender::get_material()
{
	return material.get();
}

render::mesh *
skinrender::get_mesh() 
{
	return skinned_mesh.get();
}

void
skinrender::set_material(std::shared_ptr<render::material> &mat)
{
	material = mat;
}

void 
skinrender::set_mesh(std::shared_ptr<render::mesh> &mesh) 
{
	skinned_mesh = mesh;
}



}

