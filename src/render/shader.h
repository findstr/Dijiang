#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

namespace engine {
namespace render {

class shader {
public:
	enum class stage {
		VERTEX,
		TESSELLATION_CONTROL,
		TESSELATION_EVALUTION,
		GEOMETRY,
		FRAGMENT,
		COMPUTE,
	};

	enum class type {
		OPAQUE,
		TRANSPARENT,
	};
	enum class queue {
		GEOMETRY,
	};
	enum class light_mode {
		FORWARD,
		SHADOWCASTER,
	};
	enum class prop_type {
		NONE,
		INT,
		FLOAT,
		TEXTURE2D,
	};
	struct prop_desc {
		int index;
		prop_type type;
	};

	struct stage_code {
		std::string name;
		enum stage stage;
		std::string spv;
	};

	static shader *create(const std::vector<stage_code> &stags);
	
	bool ztest = true;
	bool zwrite = true;
	bool topology_triangle = true;
	enum type render_type = type::OPAQUE;
	enum queue render_queue = queue::GEOMETRY;
	enum light_mode light_mode = light_mode::FORWARD;
	void add_prop(const std::string &name, prop_type type) {
		int id = props.size();
		auto &v = props[prop_id(name)];
		v.index = id;
		v.type = type;
	}
	prop_type get_prop_type(const std::string &name) const {
		return get_prop_type(prop_id(name));
	}
	int get_prop_index(const std::string &name) const {
		return get_prop_index(prop_id(name));
	}
	prop_type get_prop_type(int propid) const {
		auto iter = props.find(propid);
		if (iter == props.end())
			return prop_type::NONE;
		return iter->second.type;

	}
	int get_prop_index(int propid) const {
		auto iter = props.find(propid);
		if (iter == props.end())
			return -1;
		return iter->second.index;
	}
	inline int prop_count() const {
		return (int)props.size();
	}
protected:
	shader() {};
	std::unordered_map<int, prop_desc> props;
public:
	static std::unordered_map<std::string, int> prop_ids;
	static int prop_id(const std::string &name) {
		auto &n = prop_ids[name];
		if (n == 0)
			n = prop_ids.size();
		return n;
	}
};

}}

