#pragma once

namespace engine {

struct rect {
	rect(float xx, float yy, float w, float h) : 
		x(xx), y(yy), width(w), height(h) {}
	float x;
	float y;
	float width;
	float height;
};

}

