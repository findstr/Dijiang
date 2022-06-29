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
	constexpr size_t offset() const {
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
	static inline constexpr size_t size() {
		return (3+2+3+3+3+4+4);
	}
	constexpr operator type() const { return value_; }
	explicit operator bool() = delete;
private:
	type value_;
};

}}

