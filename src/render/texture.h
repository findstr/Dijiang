#pragma once
#include <memory>
#include <vector>
#include <cmath>
#include "render/texture_wrap.h"
#include "render/texture_format.h"
#include "render/texture_filter.h"

namespace engine {
namespace render {

class texture {
public:
	enum type {
		UNKONW,
		TEX2D,
		TEX3D,
		CUBE,
		TEX2D_ARRAY,
		CUBE_ARRAY,
	};
	texture(enum type t) : type_(t) {}
public:
	bool linear = false;
	int miplevels = 0;
	int anisolevels = 0;
	texture_format format = texture_format::RGBA32;
	texture_wrap wrap_mode_u = texture_wrap::REPEAT;
	texture_wrap wrap_mode_v = texture_wrap::REPEAT;
	texture_wrap wrap_mode_w = texture_wrap::CLAMP;
	texture_filter filter_mode = texture_filter::BILINEAR;
	inline int width() const { return width_; }
	inline int height() const { return height_; }
	inline enum type type() const { return type_; }
public:
	static inline int mip_levels(int width, int height) {
		int max = std::max(width, height);
		return (int)(std::floor(std::log2(max)) + 1);
	}
protected:
	int width_;
	int height_;
	enum type type_;
};


}}

