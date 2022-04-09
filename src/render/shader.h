#pragma once
#include <vector>
#include <string>
#include <memory>

namespace engine {
namespace render {

class shader {
public:
	enum stage {
		VERTEX,
		TESSELLATION_CONTROL,
		TESSELATION_EVALUTION,
		GEOMETRY,
		FRAGMENT,
		COMPUTE,
	};
	struct code {
		std::string name;
		enum stage stage;
		std::string spv;
	};
	static shader *create(const std::vector<code> &stags);
protected:
	shader() {};
};

}}

