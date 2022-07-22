#pragma once
#include <vector>
#include "math/math.h"
#include "math/rect.h"
#include "render/color.h"
#include "draw_object.h"
#include "gameobject.h"
#include "component.h"
#include "glm/glm.hpp"

namespace engine {

class light: public component {
public:
	enum class type {
		DIRECTIONAL,
		POINT,
	};
public:
	light(gameobject *go);
	~light();
public:
	const std::string name() override { return "light"; }
	vector3f direction() const { return transform->rotation * vector3f::forward(); }
	void get_shadow_matrix(camera *cam, vector3f &center, glm::mat4 &view, glm::mat4 &proj);
public:
	void reg();
	void unreg();
public:
	color color;
	enum type type = type::DIRECTIONAL;
	float intensity = 1.0f;
protected:
	bool regged = false;
/////////static
public:
	static const std::vector<light *> &all_lights();
private:
	static std::vector<light *> light_list;
};



}


