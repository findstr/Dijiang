#pragma once
#include "render/texture_wrap.h"
#include "render/texture_format.h"
#include "render/texture_filter.h"
#include "render/texture.h"
#include "render/gpu.h"
#include <memory>
#include <vector>

namespace engine {
namespace render {

class texture2d : public texture {
public:
	texture2d(int width, int height, 
		texture_format format = texture_format::RGBA32,
		bool linear = false,
		bool mipmap = false) :
		texture(texture::TEX2D) {
			this->width_= width;
			this->height_ = height;
			this->format = format;
			this->linear = linear;
			miplevels = mipmap ?  mip_levels(width, height) : 0;
	}
	~texture2d() {
		if (handle_ != INVALID_HANDLE)
			gpu.unload_texture(handle_);
	}
public:
	constexpr texture_handle_t handle() {
		return handle_;
	};
	void setpixel(std::vector<uint8_t> &pixel) {
		this->pixel = std::move(pixel);
	}
public:
	void apply() {
		handle_ =  gpu.upload_texture(*this);
		if (readonly) {
			std::vector<uint8_t> clr;
			pixel.swap(clr);
		}
	}
};


}}

