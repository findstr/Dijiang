#include <array>
#include "imgui.h"
#include "imgui_internal.h"
#include "components/camera.h"
#include "render/draw_object.h"
#include "system/render_system.h"
#include "level.h"
#include "gameview.h"

namespace engine {
namespace editor {

gameview::gameview() :
	title("Game")
{}

void
gameview::tick(engine *e, float delta)
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_MenuBar;
	if (!is_open)
		return;
	ImGui::Begin(title.c_str(), &is_open, window_flags);
	auto menu_bar_rect = ImGui::GetCurrentWindow()->MenuBarRect();
	
	ImVec2 window_pos = ImGui::GetWindowPos();

	ImVec2 vMin = ImGui::GetWindowContentRegionMin();
	ImVec2 vMax = ImGui::GetWindowContentRegionMax();
	ImVec2 window_size(vMax.x - vMin.x, vMax.y - vMin.y);

	//window_pos.y += menu_bar_rect.Min.y;
	//window_size.y -= menu_bar_rect.Min.y;

#if defined(__MACH__)
	// The dpi_scale is not reactive to DPI changes or monitor switching, it might be a bug from ImGui.
	// Return value from ImGui::GetMainViewport()->DpiScal is always the same as first frame.
	// glfwGetMonitorContentScale and glfwSetWindowContentScaleCallback are more adaptive.
	window_pos.x *= main_viewport->DpiScale;
	window_pos.y *= main_viewport->DpiScale;
	window_size.x *= main_viewport->DpiScale;
	window_size.y *= main_viewport->DpiScale;

#endif
	ImGui::End();
	auto cameras = camera::all_cameras();
	for (auto cam:cameras) {
		drawlist.clear();
		level::cull(cam, drawlist);
		for (auto &d:drawlist) 
		RENDER_SYSTEM.draw(cam, d);
	}
}


}}


