#pragma once
#include "menu.h"
#include "engine.h"
#include "gameview.h"
#include "sceneview.h"

namespace engine {
namespace editor {

class editor {
public:
	void init(engine *e);
	void cleanup();
	void pre_tick(float delta);
	void tick(float delta);
	void post_tick(float delta);
private:
	void draw_menu();
	void draw_game();
	void draw_scene();
private:
	engine *engine;
	gameview gameview;
	sceneview sceneview;
	bool m_editor_menu_window_open       = true;
	bool m_asset_window_open             = true;
	bool m_file_content_window_open      = true;
	bool m_detail_window_open            = true;
	bool m_scene_lights_window_open      = true;
	bool m_scene_lights_data_window_open = true;
};

}}

