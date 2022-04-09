#pragma once
#include "math/math.h"
namespace engine {
namespace render {

class mesh {
public:
	static mesh *create();
	void set_dirty() { dirty = true; }
public:
	bool readonly = true;
	std::vector<vector3f> vertices;
	std::vector<vector2f> uv;
	std::vector<vector3f> colors;
	std::vector<int> triangles;
protected:
	bool dirty = false;
};

}}

