#pragma once
#include <memory>
#include <array>
#include "texture_format.h"
#include "texture.h"

namespace engine {

class render_texture : public render::texture {
public:
	render_texture() : render::texture(render::texture::TEX2D) {}
	virtual intptr_t handle() const = 0; 
	virtual intptr_t sampler() const = 0;
protected:
	texture_format depth_format;
public:
	static render_texture *create(int width, int height,
		texture_format color_format = texture_format::RGBA32,
		texture_format depth_format = texture_format::D32S8,
		bool linear = false, int miplevels = 1);
};

}

