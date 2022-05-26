#pragma once
#include <memory>
#include <vector>
#include <array>

#include "render/texture_wrap.h"
#include "render/texture_format.h"
#include "render/texture_filter.h"
#include "render/texture.h"

namespace engine {
namespace render {

class cubemap : public texture {
public:
	enum face : int  {
		POSITIVE_X,
		NEGATIVE_X,
		POSITIVE_Y,
		NEGATIVE_Y,
		POSITIVE_Z,
		NEGATIVE_Z,
		FACE_COUNT,
	};
	static cubemap *create(int width, int height,
		texture_format format = texture_format::RGBA32,
		bool linear = false, int miplevels = 0);
	cubemap():texture(CUBE) {}
	virtual ~cubemap() {}
public:
	void setpixel(enum face face, std::vector<uint8_t> &pixel) {
		this->pixel[face] = std::move(pixel);
	}
public:
	virtual void apply() = 0;
protected:
	std::array<std::vector<uint8_t>, FACE_COUNT> pixel;
};


}}

