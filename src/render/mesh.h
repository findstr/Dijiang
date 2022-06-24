#pragma once
#include <vector>
#include "math/math.h"

namespace engine {
namespace render {

class mesh {
public:
	struct bone_weight {
		int index[4];
		float weight[4];
	};
public:
	static mesh *create();
	void set_dirty() { dirty = true; }
public:
	bool readonly = true;
	std::vector<vector3f> vertices;
	std::vector<vector2f> uv;
	std::vector<vector3f> colors;
	std::vector<vector3f> tangents;
	std::vector<vector3f> normals;
	std::vector<bone_weight> bone_weights;
	std::vector<int> triangles;
protected:
	bool dirty = false;
};

}}

