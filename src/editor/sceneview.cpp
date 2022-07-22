#include <array>
#include "imgui.h"
#include "imgui_internal.h"
#include "level.h"
#include "sceneview.h"
#include "input.h"
#include "backends/imgui_impl_vulkan.h"
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
	camera->clip_far_plane = 5000.0f;
	camera->clip_near_plane = 0.1f;

	render_texture.reset(render_texture::create(
		1024, 768, texture_format::RGBA32, texture_format::D32S8, false, 1));
	texture_id = ImGui_ImplVulkan_AddTexture((VkSampler)render_texture->sampler(), 
		(VkImageView) render_texture->handle(), 
		VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL);
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
	ImGui::BeginMenuBar();
	if (ImGui::BeginMenu("Gizoms")) {
		ImGui::MenuItem("Skeleton", "", &camera->show_skeleton);
		ImGui::MenuItem("Camera", "", &camera->show_camera);
                ImGui::EndMenu();
	}
	ImGui::EndMenuBar();
	vector2f mouse_new_pos = input::mouse_get_position();
	if (ImGui::IsWindowFocused()) {
		float mouse_delta = input::mouse_scroll_delta();
		if (std::abs(mouse_delta) > 0.00001f) {
			camera->transform->position += camera->forward() * mouse_delta * delta * 50.0f;
		} else if (input::mouse_get_button(1)) {
			vector2f mouse_delta = mouse_new_pos - mouse_position;
			camera->transform->position += (camera->right() * mouse_delta.x() + 
				camera->up() * mouse_delta.y() * -1.f)* delta * 500.0f;
		} else if (input::mouse_get_button(2)) {
			vector2f mouse_delta = mouse_new_pos - mouse_position;
			if (std::abs(mouse_delta.x()) + std::abs(mouse_delta.y()) > 0.0001f) {
				if (std::abs(mouse_delta.x()) > std::abs(mouse_delta.y()) && std::abs(mouse_delta.x()) > 0.0001f) {
					quaternion rot;
					rot.from_axis_angle(vector3f::up(), mouse_delta.x() * 50.f);
					camera->transform->rotation = rot * camera->transform->rotation;
				} else if (std::abs(mouse_delta.x()) < std::abs(mouse_delta.y()) && std::abs(mouse_delta.y()) > 0.0001f) {
					quaternion rot;
					rot.from_axis_angle(camera->right(), mouse_delta.y() * 50.f);
					camera->transform->rotation = rot * camera->transform->rotation;
				}
			}
		}
	}
	mouse_position = mouse_new_pos;
	camera->render_target = render_texture.get();
	camera->render();
	camera->render_target = nullptr;
	ImGui::Image(texture_id, ImGui::GetContentRegionAvail());
	ImGui::End();
}


}}

