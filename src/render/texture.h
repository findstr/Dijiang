#pragma once
#include "render/texture_wrap.h"
#include "render/texture_format.h"
#include "render/texture_filter.h"
#include <memory>
#include <vector>

namespace engine {
namespace render {

class texture {
public:
	static texture *create(int width, int height,
		texture_format format = texture_format::RGBA32,
		bool linear = false, int miplevels = 0);
	virtual ~texture() {}
public:
	int width() const;
	int height() const;
	void setpixel(std::vector<uint8_t> &pixel);
public:
	virtual void apply() = 0;
public:
	int width_ = 0;
	int height_ = 0;
	bool linear = false;
	int miplevels = 0;
	int anisolevels = 0;
	texture_wrap wrap_mode_u = texture_wrap::REPEAT;
	texture_wrap wrap_mode_v = texture_wrap::REPEAT;
	texture_wrap wrap_mode_w = texture_wrap::CLAMP;
	texture_format format = texture_format::RGBA32;
	texture_filter filter = texture_filter::BILINEAR;
protected:
	std::vector<uint8_t> pixel;
};


}}

