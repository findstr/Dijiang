#pragma once
#include <string>
#include <vector>
#include "math/math.h"
#include "math/transform.h"

namespace engine {
namespace animation {
	
class skeleton {
public:
	struct bone {
		int id;
		int parent;
		std::string name;
		vector3f position;
		quaternion rotation;
		vector3f scale;
		matrix4f tpose_matrix;
		bone() {}
		bone(const bone &b) {
			this->id = b.id;
			this->parent = b.parent;
			this->name = b.name;
			this->position = b.position;
			this->rotation = b.rotation;
			this->scale = b.scale;
			this->tpose_matrix = b.tpose_matrix;
		}
	};
	struct pose {
		int id;
		vector3f position;
		quaternion rotation;
		vector3f scale;
		matrix4f matrix;
		pose() {}
		pose(const pose &p) {
			this->id = p.id;
			this->position = p.position;
			this->rotation = p.rotation;
			this->scale = p.scale;
			this->matrix = p.matrix;
		}
	};
public:
	int parent(int bone) const;
	void add_bone(const bone &b);
	void apply_animation(float progress, std::vector<pose> &pose) const;
private:
	std::vector<bone> bones;
};
	
}}
