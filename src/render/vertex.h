#pragma once
#include <assert.h>
namespace engine {
namespace render {

struct vertex_type {
	enum type {
		NONE,
		POSITION,
		TEXCOORD,
		NORMAL,
		TANGENT,
		COLOR,
		BLENDINDICES,
		BLENDWEIGHT,
		COUNT,
	};
	constexpr vertex_type(type t) : value_(t) {}
	constexpr size_t offset_in_float() const {
		switch (value_) {
		case POSITION:
			return 0;
		case TEXCOORD:
			return 3;
                case NORMAL:
			return (3+2);
                case TANGENT:
			return (3+2+3);
		case COLOR:
			return (3+2+3+3);
                case BLENDINDICES:
			return (3+2+3+3+3);
                case BLENDWEIGHT:
			return (3+2+3+3+3+4);
		default:
			assert(!"unsupport format");
			return 0;
		}
	}
	constexpr size_t offset_in_byte() const {
		return offset_in_float() * sizeof(float);
	}
	static inline constexpr size_t size_in_float() {
		return (3+2+3+3+3+4+4);
	}
	static inline constexpr size_t size_in_byte() {
		return size_in_float() * sizeof(float);
	}
	constexpr operator type() const { return value_; }
	explicit operator bool() = delete;
private:
	type value_;
};

}}

