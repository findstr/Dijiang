#pragma once
#include <string>
#include <memory>
#include "engine.h"
#include "math/math.h"
#include "gameobject.h"
#include "scene_camera.h"

namespace engine {
namespace editor {

class sceneview {
public:
	sceneview();
	void pre_tick(engine *e, float delta);
	void tick(engine *e, float delta);
public:
	const std::string title;
	bool is_open = true;
private:
	vector2f mouse_position;
	ImTextureID texture_id = nullptr;
	std::unique_ptr<render_texture> render_texture;
	std::unique_ptr<gameobject> cam_go;
	std::unique_ptr<scene_camera> camera;
};

}}

