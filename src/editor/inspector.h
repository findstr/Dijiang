#pragma once
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include "engine.h"
#include "level.h"
#include "math/math.h"
#include "gameobject.h"
#include "scene_camera.h"

namespace engine {
namespace editor {

class inspector {
public:
	virtual void draw() = 0;
};

}}

