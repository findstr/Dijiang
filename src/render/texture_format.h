#pragma once
#include <assert.h>
namespace engine {

struct texture_format {
	enum value : int {
		RGBA64,
		RGBA32,
		RGB48,
		RGB24,
	};
	texture_format() = default;
	constexpr texture_format(value v) : value_(v) {}
	constexpr operator value() const { return value_; }
	explicit operator bool() = delete;
	constexpr std::string str() {
		switch (value_) {
		case RGBA64:
			return "RGBA64";
		case RGBA32:
			return "RGBA32";
                case RGB48:
			return "RGB48";
		case RGB24:
			return "RGB24";
		default:
			assert(!"unsupport format");
			return 0;
		}
	}
	constexpr size_t size() {
		switch (value_) {
		case RGBA64:
			return 8;
		case RGBA32:
			return 4;
                case RGB48:
			return 6;
		case RGB24:
			return 3;
		default:
			assert(!"unsupport format");
			return 0;
		}
	}
private:
	value value_;
};

}

