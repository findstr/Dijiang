#include "imgui.h"
#include "imgui_internal.h"
#include "math/math.h"
#include "system/render_system.h"
#include "engine.h"
#include "inspector.h"
#include "editor.h"

namespace engine {
namespace editor {

void
editor::init(::engine::engine *e)
{
	this->engine = e;
}

void
editor::cleanup()
{

}

void
editor::pre_tick(float delta)
{
	sceneview.pre_tick(engine, delta);
}

void
editor::draw_menu()
{
	ImGuiDockNodeFlags dock_flags   = ImGuiDockNodeFlags_DockSpace;
	ImGuiWindowFlags   window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar |
					ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
					ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground |
					ImGuiConfigFlags_NoMouseCursorChange | ImGuiWindowFlags_NoBringToFrontOnFocus;

	const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(main_viewport->WorkPos, ImGuiCond_Always);
	std::array<int, 2> window_size = {1024, 768};
	RENDER_SYSTEM.get_resolution(&window_size[0], &window_size[1]);
	ImGui::SetNextWindowSize(ImVec2((float)window_size[0], (float)window_size[1]), ImGuiCond_Always);

	ImGui::SetNextWindowViewport(main_viewport->ID);
	
	ImGui::Begin("Editor menu", &m_editor_menu_window_open, window_flags);
	ImGuiID main_docking_id = ImGui::GetID("Main Docking");
	if (ImGui::DockBuilderGetNode(main_docking_id) == nullptr) {
	    ImGui::DockBuilderRemoveNode(main_docking_id);

	    ImGui::DockBuilderAddNode(main_docking_id, dock_flags);
	    ImGui::DockBuilderSetNodePos(main_docking_id,
					 ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y + 18.0f));
	    ImGui::DockBuilderSetNodeSize(main_docking_id,
					  ImVec2((float)window_size[0], (float)window_size[1] - 18.0f));

	    ImGuiID center = main_docking_id;
	    ImGuiID left;
	    ImGuiID right = ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, 0.25f, nullptr, &left);

	    ImGuiID left_other;
	    ImGuiID left_file_content = ImGui::DockBuilderSplitNode(left, ImGuiDir_Down, 0.30f, nullptr, &left_other);

	    ImGuiID left_game_engine;
	    ImGuiID left_asset = ImGui::DockBuilderSplitNode(left_other, ImGuiDir_Left, 0.30f, nullptr, &left_game_engine);

	    ImGui::DockBuilderDockWindow(hierarchy.title.c_str(), left_asset);
	    ImGui::DockBuilderDockWindow("Inspector", right);
	    ImGui::DockBuilderDockWindow("File Content", left_file_content);
	    ImGui::DockBuilderDockWindow(gameview.title.c_str(), left_game_engine);
	    ImGui::DockBuilderDockWindow(sceneview.title.c_str(), left_game_engine);

	    ImGui::DockBuilderFinish(main_docking_id);
	}

	ImGui::DockSpace(main_docking_id);
	if (ImGui::BeginMenuBar())
	{
	    if (ImGui::BeginMenu("Menu"))
	    {
		if (ImGui::MenuItem("Reload Current Level"))
		{
		}
		if (ImGui::MenuItem("Save Current Level"))
		{
		}
		if (ImGui::MenuItem("Exit"))
		{
		    exit(0);
		}
		ImGui::EndMenu();
	    }
	    if (ImGui::BeginMenu("Window"))
	    {
		ImGui::MenuItem("World Objects", nullptr, &m_asset_window_open);
		ImGui::MenuItem(gameview.title.c_str(), nullptr, &gameview.is_open);
		ImGui::MenuItem(sceneview.title.c_str(), nullptr, &sceneview.is_open);
		ImGui::MenuItem("File Content", nullptr, &m_file_content_window_open);
		ImGui::MenuItem("Detail", nullptr, &m_detail_window_open);
		ImGui::EndMenu();
	    }
	    ImGui::EndMenuBar();
	}

	ImGui::End();
}

void
editor::draw_inspector(::engine::editor::inspector *insp)
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar;
	if (!is_inspector_open)
		return;
	ImGui::Begin("Inspector", &is_inspector_open, window_flags);
	if (insp != nullptr) 
		insp->draw();
	ImGui::End();
}


void
editor::tick(float delta)
{
	draw_menu();
	auto insp = hierarchy.tick(engine, delta);
	if (insp != nullptr)
		inspector = insp;
	gameview.tick(engine, delta);
	sceneview.tick(engine, delta);
	draw_inspector(inspector);
}

void
editor::post_tick(float delta)
{

}

}}


