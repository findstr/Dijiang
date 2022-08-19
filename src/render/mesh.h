#pragma once
#include <vector>
#include "math/math.h"
#include "render/gpu_resource.h"

namespace engine {
namespace render {

class mesh {
public:
	struct bone_weight {
		int index[4];
		float weight[4];
	};
public:
	void set_dirty() {
		dirty = true; 
	}
public:
	bool readonly = true;
	std::string name;
	std::vector<vector3f> vertices;
	std::vector<vector2f> uv;
	std::vector<vector3f> colors;
	std::vector<vector3f> tangents;
	std::vector<vector3f> normals;
	std::vector<bone_weight> bone_weights;
	std::vector<int> triangles;
	mutable bool dirty = false;
	mutable mesh_handle_t handle = invalid_handle;
};

}}

