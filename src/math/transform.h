#pragma once
#include "math/math.h"
namespace engine {

struct transform {
	vector3f position;
	quaternion rotation;
	vector3f scale;
	vector3f local_position;
	quaternion local_rotation;
	vector3f local_scale;
};

}
