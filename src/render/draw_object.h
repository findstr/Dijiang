#pragma once
#include "gameobject.h"
#include "math/transform.h"
#include "render/mesh.h"
#include "render/material.h"
#include "animation/skeleton.h"

namespace engine {

struct draw_object {
	draw_object(const transform &trans,
		render::mesh *m,
		render::material *mat) :
		position(trans.position()),
		rotation(trans.rotation()),
		scale(trans.scale()),
		mesh(m), material(mat) {}
	draw_object(const transform &trans,
		render::mesh *m,
		render::material *mat,
		const std::vector<animation::skeleton::pose> &pose) :
		position(trans.position()),
		rotation(trans.rotation()),
		scale(trans.scale()),
		mesh(m), material(mat),
		skeleton_pose(&pose) {}
	vector3f position;
	quaternion rotation;
	vector3f scale;
	render::mesh *mesh;
	render::material *material;
	int material_offset = 0;
	const std::vector<animation::skeleton::pose> *skeleton_pose = nullptr;
	gameobject *go = nullptr;
};

}

