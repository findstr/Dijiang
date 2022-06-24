#include "skeleton.h"

namespace engine {
namespace animation {

int 
skeleton::parent(int bone) const
{
	if (bone >= bones.size())
		return -1;
	return bones[bone].parent;
}

void
skeleton::add_bone(const bone &b) 
{
	bones.emplace_back(b);
}

void 
skeleton::apply_animation(transform &trans, std::vector<skeleton::pose> &ani_pose) const 
{
	quaternion rot;
	rot.from_axis_angle(vector3f::up(), -90);
	ani_pose.resize(bones.size());
	for (size_t i = 0; i < bones.size(); i++) {
		auto &pose = ani_pose[i];
		auto &b = bones[i];
		pose.id = b.id;
		if (b.parent >= 0) {
			auto &p = ani_pose[b.parent];
			pose.position = p.rotation * b.position + p.position;
			pose.rotation = p.rotation * b.rotation;
			pose.scale = b.scale * p.scale;
		} else {
			pose.position = trans.rotation * b.position + trans.position;
			pose.rotation = trans.rotation * b.rotation;
			pose.scale = b.scale * trans.scale;
		}
	}
}	

	
}}
