#pragma once
#include <vector>
#include "conf.h"
#include "color.h"
#include "math/math.h"
#include "components/camera.h"
#include "render/draw_object.h"

namespace engine {
namespace render {

class debugger {
public:
	static debugger &inst();
public:
	virtual void init() = 0;
	void begin();
	void cull(camera *cam, std::vector<draw_object> &list);
	void draw_line(const vector3f &v1, const color &c1, const vector3f &v2, const color &c2);
protected:
	int frameidx = 0;
	transform trans;
	std::shared_ptr<material> mat;
	std::vector<vector3f> vertices;
	std::vector<vector3f> colors;
	std::vector<int> triangles;
	std::array<std::unique_ptr<mesh>, conf::MAX_FRAMES_IN_FLIGHT> meshes;
};

}}



