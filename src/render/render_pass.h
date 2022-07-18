#pragma once
#include <memory>
#include <array>
#include "render/texture2d.h"
#include "render/cubemap.h"

namespace engine {

class i_render_pass {
public:
	virtual void begin(float viewport_width, float viewport_height) = 0;
	virtual void end() = 0;
};

class render_pass {
public:
	enum path : int {
		FORWARD = 0,
		SHADOW = 1,
		COUNT,
	};
public:
	void reg(path m, i_render_pass *pass) {  passes[m] = pass; }
	i_render_pass *get (path m) { return passes[m]; }
	i_render_pass *unreg(path m) { auto *p = passes[m]; passes[m] = nullptr; return p; }
	void begin(path m, float viewport_width, float viewport_height) { passes[m]->begin(viewport_width, viewport_height); }
	void end(path m) { passes[m]->end(); }
private:
	std::array<i_render_pass *, path::COUNT> passes;
public:
	static render_pass &inst() {
		static render_pass *inst = new render_pass();
		return *inst;
	}
};

#define RENDER_PASS render_pass::inst()

}

