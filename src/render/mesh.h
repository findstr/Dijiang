#pragma once
#include <vector>
#include "math/math.h"
#include "render/bone_weight.h"
#include "gpu.h"

namespace engine {
namespace render {

class mesh {
public:
	void apply () {
		handle_ = gpu.upload_mesh(
			vertices,
			uv,
			colors,
			tangents,
			normals,
			bone_weights,
			triangles);
		if (readonly) {
			std::vector<int> clr;
			std::vector<vector3f> clrv3;
			std::vector<vector2f> clrv2;
			std::vector<bone_weight> clrbw;
			vertices.swap(clrv3);
			uv.swap(clrv2);
			colors.swap(clrv3);
			tangents.swap(clrv3);
			normals.swap(clrv3);
			bone_weights.swap(clrbw);
			triangles.swap(clr);
		}
	}
	~mesh() {
		if (handle_ != INVALID_HANDLE)
			gpu.unload_mesh(handle_);
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
	constexpr mesh_handle_t handle() { return handle_; }
private:
	mesh_handle_t handle_ = INVALID_HANDLE;
};

}}

