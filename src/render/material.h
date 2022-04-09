#pragma once
#include <string>
#include <vector>
#include <memory>
#include "texture.h"
#include "shader.h"
namespace engine {
namespace render {

class material {
public:
	virtual ~material() {};
public:
	static material *create(std::shared_ptr<shader> &s);
	const shader *get_shader() const { return shader.get(); }
	virtual void set_shader(std::shared_ptr<shader> &s) = 0;
	virtual void set_texture(const std::string &name,
		std::shared_ptr<render::texture> &tex) = 0;
protected:
	std::shared_ptr<shader> shader;
};

}}

