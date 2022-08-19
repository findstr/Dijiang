#pragma once
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "texture.h"
#include "texture2d.h"
#include "shader.h"
#include "render_pass.h"
#include "gpu_resource.h"

namespace engine {
namespace render {

class material {
public:
	struct prop_value {
		shader::prop_type type;
		std::shared_ptr<texture> texture;
		union {
			float fvalue;
			int ivalue;
		};
		prop_value() noexcept {
			this->type = shader::prop_type::NONE;
			this->ivalue = 0;
		}
		prop_value(prop_value &&other) noexcept {
			type = other.type;
			switch (type) {
			case shader::prop_type::TEXTURE2D:
				this->texture = std::move(other.texture);
				break;
			case shader::prop_type::INT:
				this->ivalue = other.ivalue;
				break;
			case shader::prop_type::FLOAT:
				this->fvalue = other.fvalue;
				break;
			}
		}
		~prop_value() {
			if (type == shader::prop_type::TEXTURE2D)
				texture.reset();
		}
	};
	union gpu_value {
		texture_handle_t texture;
		float fvalue;
		int ivalue;
	};
public:
	inline const shader *get_shader() const { return shader.get(); }
	void set_shader(std::shared_ptr<shader> &s) {
		shader = s;
		if (s != nullptr) {
			int n = s->prop_count();
			properties.resize(n);
			gpu_block.resize(n);
		}
	}
	void set_texture(const std::string &name, std::shared_ptr<render::texture> &tex) {
		set_texture(shader::prop_id(name), tex);
	}
	void set_float(const std::string &name, float v) {
		set_float(shader::prop_id(name), v);
	}
	void set_int(const std::string &name, int v) {
		set_int(shader::prop_id(name), v);
	}
	void set_texture(int id, std::shared_ptr<render::texture> &tex) {
		int index = shader->get_prop_index(id);
		assert(index >= 0);
		auto &p = properties[index];
		p.type = shader::prop_type::TEXTURE2D;
		p.texture = tex;
		assert(shader->get_prop_type(id) == p.type);
		gpu_block[index].texture = tex->handle;
	}
	void set_float(int id, float v) {
		int index = shader->get_prop_index(id);
		assert(index >= 0);
		auto &p = properties[index];
		p.type = shader::prop_type::FLOAT;
		p.fvalue = v;
		assert(shader->get_prop_type(id) == p.type);
		gpu_block[index].fvalue = v;
	}
	void set_int(int id, int v) {
		int index = shader->get_prop_index(id);
		assert(index >= 0);
		auto &p = properties[index];
		p.type = shader::prop_type::INT;
		p.ivalue = v;
		assert(shader->get_prop_type(id) == p.type);
		gpu_block[index].ivalue = v;
	}
	inline size_t argument_size() const { 
		return shader->prop_count() * sizeof(gpu_value); 
	}
	inline void copy_to(void *dst) {
		memcpy(dst, gpu_block.data(), gpu_block.size() * sizeof(gpu_value));
	}
protected:
	bool ztest;
	int id = 0;
	std::shared_ptr<shader> shader;
	std::vector<prop_value> properties;
	std::vector<gpu_value> gpu_block;
};

}}

