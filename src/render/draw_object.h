#pragma once
#include "math/transform.h"
#include "render/mesh.h"
#include "render/material.h"

namespace engine {

struct draw_object {
	draw_object(const transform &trans,
		render::mesh *m,
		render::material *mat) :
		position(trans.position),
		rotation(trans.rotation),
		scale(trans.scale),
		mesh(m), material(mat) {}
	draw_object(const transform &trans,
		render::mesh *m,
		render::material *mat,
		const std::vector<matrix4f> &pose) :
		position(trans.position),
		rotation(trans.rotation),
		scale(trans.scale),
		mesh(m), material(mat),
		skeleton_pose(pose) {}
	vector3f position;
	quaternion rotation;
	vector3f scale;
	render::mesh *mesh;
	render::material *material;
	std::vector<matrix4f> skeleton_pose;
};

}

