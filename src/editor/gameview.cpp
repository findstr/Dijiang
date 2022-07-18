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
	auto menu_bar_rect = ImGui::GetCurrentWindow()->MenuBarRect();
	std::array<int, 2> full_size = {1024, 768};
	RENDER_SYSTEM.get_resolution(&full_size[0], &full_size[1]);

	ImVec2 window_pos = ImGui::GetWindowPos();

	ImVec2 vMin = ImGui::GetWindowContentRegionMin();
	ImVec2 vMax = ImGui::GetWindowContentRegionMax();
	ImVec2 window_size(vMax.x - vMin.x, vMax.y - vMin.y);

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
	if (ImGui::IsWindowFocused()) 
		input::update(delta);
	vulkan::VK_FRAMEBUFFER.set_render_texture(render_texture.get());
	auto cameras = camera::all_cameras();
	for (auto cam:cameras) {
		camera tmp = *cam;
		float x_norm = window_size.x / full_size[0];
		float y_norm = window_size.y / full_size[1];
		tmp.viewport.x = window_pos.x / full_size[0] + x_norm * cam->viewport.x;
		tmp.viewport.y = window_pos.y / full_size[1] + y_norm * cam->viewport.y;
		tmp.viewport.width = x_norm * cam->viewport.width;
		tmp.viewport.height = y_norm * cam->viewport.height;
		tmp.render();
	}
	ImGui::Image(texture_id, ImVec2(render_texture->width(), render_texture->height()));
	vulkan::VK_FRAMEBUFFER.set_render_texture(nullptr);
	ImGui::End();
}


}}


