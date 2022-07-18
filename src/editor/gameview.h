#pragma once
#include <string>
#include <vector>
#include "engine.h"
#include "render/draw_object.h"
#include "render/render_texture.h"

namespace engine {
namespace editor {

class gameview {
public:
	gameview();
	void tick(engine *e, float delta);
public:
	const std::string title;
	bool is_open = true;
private:
	ImTextureID texture_id = nullptr;
	std::unique_ptr<render_texture> render_texture;
	std::vector<draw_object> drawlist;
};

}}

