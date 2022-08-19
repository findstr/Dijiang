#pragma once
namespace engine {

struct texture_filter {
	enum value : int {	
		POINT = 0,
		BILINEAR = 1,
		TRILINEAR = 2,
		COUNT,
	};
	texture_filter() = default;
	constexpr texture_filter(value v) : value_(v) {}
	constexpr operator value() const { return value_; }
	explicit operator bool() = delete;
	constexpr std::string str() {
		switch (value_) {
		case POINT:
			return "point";
		case BILINEAR:
			return "bilinear";
		case TRILINEAR:
			return "trilinear";
		default:
			assert(!"unsupport filter");
			return 0;
		}
	}
private:
	value value_;
};


}

