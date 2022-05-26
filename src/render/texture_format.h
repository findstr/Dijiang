#pragma once
namespace engine {

struct texture_format {
	enum value : int {
		RGBA32,
		RGB24,
	};
	texture_format() = default;
	constexpr texture_format(value v) : value_(v) {}
	constexpr operator value() const { return value_; }
	explicit operator bool() = delete;
	constexpr size_t size() {
		switch (value_) {
		case RGBA32:
			return 4;
		case RGB24:
			return 3;
		default:
			return 0;
		}
	}
private:
	value value_;
};

}

