#include <array>
#include "imgui.h"
#include "level.h"
#include "imgui_internal.h"
#include "sceneview.h"

namespace engine {
namespace editor {

sceneview::sceneview() :
	title("Scene")
{

}

void
sceneview::tick(engine *e, float delta)
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_MenuBar;
	if (!is_open || !ImGui::IsWindowFocused())
		return;
	ImGui::Begin(title.c_str(), &is_open, window_flags);
	auto menu_bar_rect = ImGui::GetCurrentWindow()->MenuBarRect();
	
	ImVec2 window_pos = ImGui::GetWindowPos();
	ImVec2 window_size = ImGui::GetWindowSize();
	window_pos.y += menu_bar_rect.Min.y;
	window_size.y -= menu_bar_rect.Min.y;
#if defined(__MACH__)
	// The dpi_scale is not reactive to DPI changes or monitor switching, it might be a bug from ImGui.
	// Return value from ImGui::GetMainViewport()->DpiScal is always the same as first frame.
	// glfwGetMonitorContentScale and glfwSetWindowContentScaleCallback are more adaptive.
	window_pos.x *= main_viewport->DpiScale;
	window_pos.y *= main_viewport->DpiScale;
	window_size.x *= main_viewport->DpiScale;
	window_size.y *= main_viewport->DpiScale;

#endif
	e->set_viewport(window_pos.x, window_pos.y, window_size.x, window_size.y);
	ImGui::End();
	e->render.set_viewport(window_pos.x, window_pos.y, window_size.x, window_size.y);
	auto cameras = camera::all_cameras();
	for (auto cam:cameras) {
		drawlist.clear();
		level::cull(cam, drawlist);
		e->render.draw(cam, drawlist);
	}
}


}}

