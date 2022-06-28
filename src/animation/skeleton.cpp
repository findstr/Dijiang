#include "resource/resource.h"
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
	bones.push_back(b);
}

void 
skeleton::apply_animation(float ratio, std::vector<skeleton::pose> &ani_pose) const 
{
	static auto anim = resource::load_animation_clip("asset/animation/robot/run.anim");
	ani_pose.resize(bones.size());
	float frame = (anim->frames.size() - 1) * ratio;
	int frame_low_index = (int)frame;
	int frame_high_index = (int)(frame + 0.9999999999999999999f);
	int lerp_ratio = frame - frame_low_index;
	auto &frame_low = anim->frames[frame_low_index];
	auto &frame_high = anim->frames[frame_high_index];
	for (size_t i = 0; i < bones.size(); i++) {
		auto &pose = ani_pose[i];
		auto &b = bones[i];
		pose.id = b.id;
		auto &pose_low = frame_low.bone_poses[b.id];
		auto &pose_high = frame_high.bone_poses[b.id];
		vector3f frame_position = vector3f::lerp(pose_low.position, pose_high.position, ratio);
		vector3f frame_scale = vector3f::lerp(pose_low.scale, pose_high.scale, ratio);
		quaternion frame_rotation = quaternion::slerp(pose_low.rotation, pose_high.rotation, ratio);
		if (b.parent >= 0) {
			auto &p = ani_pose[b.parent];
			pose.position = p.rotation * (p.scale * (frame_position + b.position)) + p.position;
			pose.rotation = p.rotation * b.rotation * frame_rotation;
			pose.scale = p.scale * frame_scale * b.scale;
			auto obj_matrix = matrix4f::trs(pose.position, pose.rotation, pose.scale);
			auto inverse_matrix = b.tpose_matrix;
			pose.matrix = obj_matrix * inverse_matrix; 
		} else {
			pose.position = frame_position + b.position;
			pose.rotation = b.rotation * frame_rotation;
			pose.scale = frame_scale;
			auto m1 = matrix4f::trs(pose.position, pose.rotation, pose.scale);
			auto m2 = b.tpose_matrix;
			pose.matrix = m1 * m2;
		}
	}
}	

	
}}
