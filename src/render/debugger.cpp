#include <vector>
#include "conf.h"
#include "color.h"
#include "mesh.h"
#include "math/math.h"
#include "resource/resource.h"
#include "debugger.h"

namespace engine {
namespace render {

void
debugger::begin() 
{
	vertices.clear();
	colors.clear();
	triangles.clear();
}


void
debugger::cull(camera *cam, std::vector<draw_object> &list) 
{
	int idx = frameidx;
	frameidx = (idx + 1) % conf::MAX_FRAMES_IN_FLIGHT;
	if (vertices.size() == 0)
		return ;
	auto *m = meshes[idx].get();
	m->vertices = vertices;
	m->colors = colors;
	m->triangles = triangles;
	m->set_dirty();
	list.emplace_back(trans, m, mat.get());
	return ;
}

void
debugger::draw_line(const vector3f &v1, const color &c1, const vector3f &v2, const color &c2)
{
	int id = vertices.size();
	vertices.emplace_back(v1);
	vertices.emplace_back(v2);
	colors.emplace_back(vector3f(c1.r, c1.g, c1.b));
	colors.emplace_back(vector3f(c2.r, c2.g, c2.b));
	triangles.emplace_back(id);
	triangles.emplace_back(id+1);
}

}}

