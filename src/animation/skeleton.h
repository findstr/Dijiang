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
	};
	struct pose {
		int id;
		vector3f position;
		quaternion rotation;
		vector3f scale;
	};
public:
	int parent(int bone) const;
	void add_bone(const bone &b);
	void apply_animation(transform &trans, std::vector<pose> &pose) const;
private:
	std::vector<bone> bones;
};
	
}}
