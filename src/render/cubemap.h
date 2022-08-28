#pragma once
#include <memory>
#include <vector>
#include <array>
#include "gpu.h"

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
	cubemap(int width, int height,
		texture_format format = texture_format::RGBA32,
		bool linear = false, int mipmap = true):texture(CUBE) {
		this->width_ = width;
		this->height_ = height;
		this->format = format;
		this->linear = linear;
		this->miplevels =  mipmap ?mip_levels(width, height) :0;
		auto tex_size = (uint64_t)width_ * height_ * format.size();
		pixel.resize((int)face::FACE_COUNT * tex_size);
	}
	~cubemap() {
		if (handle_ != INVALID_HANDLE)
			gpu.unload_texture(handle_);
	}
	void apply() {
		handle_ = gpu.upload_texture(*this);
		if (readonly) {
			std::vector<uint8_t> clr;
			pixel.swap(clr);
		}
	}
public:
	void setpixel(enum face face, std::vector<uint8_t> &data) {
		auto tex_size = width_ * height_ * format.size();
		assert(data.size() == tex_size);
		memcpy(&pixel[(int)face * tex_size], data.data(), tex_size);
	}
};


}}

