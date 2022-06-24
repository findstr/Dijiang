#pragma once
#include <string>
#include <memory>
#include <vector>
#include "math/math.h"
#include "math/transform.h"
#include "component.h"
#include "skeleton.h"

namespace engine {
namespace animation {
	
class animator {
public:
	std::shared_ptr<skeleton> skeleton;
};
	
}}
