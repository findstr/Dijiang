#pragma once
#include "math/transform.h"
#include "render/mesh.h"
#include "render/material.h"

namespace engine {

struct draw_object {
	draw_object(const transform &trans, render::mesh *m, render::material *mat) :
		transform(trans), mesh(m), material(mat) {}
	transform transform;
	render::mesh *mesh;
	render::material *material;
};

}

