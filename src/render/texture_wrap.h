#pragma once
#include <assert.h>
#include <string>
namespace engine {

struct texture_wrap {
	enum value : int {		
		REPEAT = 0,
		CLAMP = 1,
		MIRROR = 2,
		MIRROR_ONCE = 3,
	};
	texture_wrap() = default;
	constexpr texture_wrap(value v) : value_(v) {}
	constexpr operator value() const { return value_; }
	explicit operator bool() = delete;
	constexpr std::string str() {
		switch (value_) {
		case REPEAT:
			return "repeat";
		case CLAMP:
			return "clamp";
		case MIRROR:
			return "mirror";
		case MIRROR_ONCE:
			return "mirror_once";
		default:
			assert(!"unsupport filter");
			return 0;
		}
	}
private:
	value value_;
};

}

