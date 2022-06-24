#pragma once
#include "math/transform.h"
#include "render/mesh.h"
#include "render/material.h"

namespace engine {

struct draw_object {
	draw_object(const transform &trans, render::mesh *m, render::material *mat) :
		position(trans.position),
		rotation(trans.rotation),
		scale(trans.scale),
		mesh(m), material(mat) {}
	vector3f position;
	quaternion rotation;
	vector3f scale;
	render::mesh *mesh;
	render::material *material;
};

}

