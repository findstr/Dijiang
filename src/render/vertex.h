#pragma once
namespace engine {
namespace render {

struct vertex_type {
	enum type {
		NONE,
		POSITION,
		TEXCOORD,
		COLOR,
		COUNT,
	};
	constexpr vertex_type(type t) : value_(t) {}
	constexpr size_t offset() const {
		switch (value_) {
		case POSITION:
			return 0;
		case TEXCOORD:
			return 3;
		case COLOR:
			return (3+2);
		default:
			assert(!"unsupport format");
			return 0;
		}
	}
	static inline constexpr size_t size() {
		return (3+2+3);
	}
	constexpr operator type() const { return value_; }
	explicit operator bool() = delete;
private:
	type value_;
};

}}

