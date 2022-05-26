#pragma once
#include "render/texture_wrap.h"
#include "render/texture_format.h"
#include "render/texture_filter.h"
#include "render/texture.h"
#include <memory>
#include <vector>

namespace engine {
namespace render {

class texture2d : public texture {
public:
	static texture2d *create(int width, int height,
		texture_format format = texture_format::RGBA32,
		bool linear = false, int miplevels = 0);
	texture2d() : texture(texture::TEX2D) {}
	virtual ~texture2d() {}
public:
	void setpixel(std::vector<uint8_t> &pixel) {
		this->pixel = std::move(pixel);
	}
public:
	virtual void apply() = 0;
protected:
	std::vector<uint8_t> pixel;
};


}}

