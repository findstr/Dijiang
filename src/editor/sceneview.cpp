#include <array>
#include "imgui.h"
#include "imgui_internal.h"
#include "level.h"
#include "sceneview.h"
#include "input.h"
#include "system/render_system.h"
#include "render/debugger.h"

namespace engine {
namespace editor {

sceneview::sceneview() :
	title("Scene")
{
	cam_go.reset(new gameobject(0, "scene_camera_go"));
	camera.reset(new scene_camera(cam_go.get()));
	camera->transform->position = vector3f(0, 2, -1);
	camera->transform->scale = vector3f(1,1,1);
	camera->transform->rotation = quaternion::identity();
	camera->fov = 60.0f;
	camera->aspect = 1.0f;
	camera->clip_far_plane = 100.0f;
	camera->clip_near_plane = 0.1f;
}

void
sceneview::pre_tick(engine *e, float delta)
{
	render::debugger::inst().begin();
}

void
sceneview::tick(engine *e, float delta)
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_MenuBar;
	if (!is_open)
		return;
	ImGui::Begin(title.c_str(), &is_open, window_flags);
	auto menu_bar_rect = ImGui::GetCurrentWindow()->MenuBarRect();
	ImVec2 window_pos = ImGui::GetWindowPos();
	ImVec2 window_size = ImGui::GetWindowSize();
	ImVec2 content_region = ImGui::GetWindowContentRegionMax();

	ImGui::BeginMenuBar();

	if (ImGui::BeginMenu("Gizoms")) {
		ImGui::MenuItem("ShowSkeleton", "", &gizmos_show_skeleton);
                ImGui::EndMenu();
	}
	ImGui::EndMenuBar();
	std::array<int, 2> full_size = {1024, 768};
	RENDER_SYSTEM.get_resolution(&full_size[0], &full_size[1]);


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
	vector2f mouse_new_pos = input::mouse_get_position();
	if (ImGui::IsWindowFocused()) {
		float mouse_delta = input::mouse_scroll_delta();
		if (std::abs(mouse_delta) > 0.00001f) {
			camera->transform->position += camera->forward() * mouse_delta * delta * 10.0f;
		} else if (input::mouse_get_button(2)) {
			vector2f mouse_delta = mouse_new_pos - mouse_position;
			if (std::abs(mouse_delta.x()) + std::abs(mouse_delta.y()) > 0.0001f) {
				if (std::abs(mouse_delta.x()) > std::abs(mouse_delta.y()) && std::abs(mouse_delta.x()) > 0.0001f) {
					quaternion rot;
					rot.from_axis_angle(vector3f::up(), mouse_delta.x() * 50.f);
					vector3f forward = rot * camera->forward();
					vector3f up = rot * camera->up();
					camera->transform->rotation = quaternion::look_at(forward, up);
				} else if (std::abs(mouse_delta.x()) < std::abs(mouse_delta.y()) && std::abs(mouse_delta.y()) > 0.0001f) {
					quaternion rot;
					rot.from_axis_angle(camera->right(), mouse_delta.y() * 50.f);
					vector3f up = rot * camera->up();
					vector3f forward = rot * camera->forward();
					camera->transform->rotation = quaternion::look_at(forward, up);
				}
			}
		}
	}

	mouse_position = mouse_new_pos;
	float x_norm = window_size.x / full_size[0];
	float y_norm = window_size.y / full_size[1];
	camera->viewport.x = window_pos.x / full_size[0];
	camera->viewport.y = window_pos.y / full_size[1];
	camera->viewport.width = x_norm;
	camera->viewport.height = y_norm;
	camera->show_skeleton(gizmos_show_skeleton);
	camera->render();
	ImGui::End();
}


}}

