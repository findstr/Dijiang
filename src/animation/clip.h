#pragma once
#include <string>
#include <memory>
#include <vector>
#include "math/math.h"

namespace engine {
namespace animation {
	
class clip {
public:
	struct transform {

	};
	struct bone_pose {
		vector3f position;
		vector3f scale;
		quaternion rotation;
	};
	struct frame {
		std::vector<bone_pose> bone_poses;
	};
public:
	int bone_count;
	std::vector<int> bone_id;
	std::vector<frame> frames;
};
	
}}
