#pragma once
#include <string>
#include <vector>
#include <memory>
#include "texture.h"
#include "shader.h"
#include "render_pass.h"

namespace engine {
namespace render {

class material {
public:
	virtual ~material() {};
public:
	static material *create(render_pass::path pass, std::shared_ptr<shader> &s, bool ztest);
	const shader *get_shader() const { return shader.get(); }
	constexpr render_pass::path render_path() const { return render_path_; }
	virtual void set_shader(std::shared_ptr<shader> &s) = 0;
	virtual void set_texture(const std::string &name,
		std::shared_ptr<render::texture> &tex) = 0;
protected:
	std::shared_ptr<shader> shader;
	bool ztest;
	render_pass::path render_path_;
};

}}

