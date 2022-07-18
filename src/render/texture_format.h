#pragma once
#include <assert.h>
namespace engine {

struct texture_format {
	enum value : int {
		INVALID,
		R32,
		RGBA64,
		RGBA32,
		RGB48,
		RGB24,
		D32,
		D24S8,
		D32S8,
	};
	texture_format() = default;
	constexpr texture_format(value v) : value_(v) {}
	constexpr operator value() const { return value_; }
	explicit operator bool() = delete;
	constexpr std::string str() {
		switch (value_) {
		case R32:
			return "R32";
		case RGBA64:
			return "RGBA64";
		case RGBA32:
			return "RGBA32";
                case RGB48:
			return "RGB48";
		case RGB24:
			return "RGB24";
		case D32:
			return "D32";
		case D24S8:
			return "D24S8";
		case D32S8:
			return "D32S8";
		default:
			assert(!"unsupport format");
			return 0;
		}
	}
	constexpr size_t size() {
		switch (value_) {
		case R32:
			return 4;
		case RGBA64:
			return 8;
		case RGBA32:
			return 4;
                case RGB48:
			return 6;
		case RGB24:
			return 3;
		case D32:
			return 4;
		case D24S8:
			return 4;
		case D32S8:
			return 5;
		default:
			assert(!"unsupport format");
			return 0;
		}
	}
private:
	value value_;
};

}

