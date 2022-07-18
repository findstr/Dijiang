#include "skinrender.h"

namespace engine {

skinrender::skinrender(
	gameobject *go,
	std::shared_ptr<render::material> &mat,
	std::shared_ptr<render::mesh> &m):
	meshrender(go, mat), skinned_mesh(m)
{
	
}

render::mesh *
skinrender::get_mesh() 
{
	return skinned_mesh.get();
}

void 
skinrender::set_mesh(std::shared_ptr<render::mesh> &mesh) 
{
	skinned_mesh = mesh;
}



}

