#include <array>
#include "imgui.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_vulkan.h"
#include "components/camera.h"
#include "render/draw_object.h"
#include "system/render_system.h"
#include "input.h"
#include "level.h"
#include "gameview.h"

namespace engine {
namespace editor {

gameview::gameview() :
	title("Game")
{
	render_texture.reset(render_texture::create(
		1024, 768, texture_format::RGBA32, texture_format::D32S8, false, 1));
	texture_id = ImGui_ImplVulkan_AddTexture((VkSampler)render_texture->sampler(), 
		(VkImageView) render_texture->handle(), 
		VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL);
}

void
gameview::tick(engine *e, float delta)
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_MenuBar;
	if (!is_open)
		return;
	ImGui::Begin(title.c_str(), &is_open, window_flags);
	if (ImGui::IsWindowFocused()) 
		input::update(delta);
	auto cameras = camera::all_cameras();
	for (auto cam:cameras) {
		cam->render_target = render_texture.get();
		cam->render();
		cam->render_target = nullptr;
	}
	ImGui::Image(texture_id, ImGui::GetContentRegionAvail());
	ImGui::End();
}


}}


