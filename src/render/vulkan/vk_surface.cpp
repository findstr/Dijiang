#include <assert.h>
#include <memory>
#include <iostream>
#include <vulkan/vulkan.hpp>
#include <SDL.h>
#include <SDL_Vulkan.h>
#include <SDL_render.h>
#include "conf.h"
#include "imgui.h"
#include "vk_ctx.h"
#include "vk_native.h"
#include "render_pass.h"
#include "backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_sdl.h"
#include "vk_input.h"
#include "vk_surface.h"

namespace engine {
namespace vulkan {

struct {
	SDL_Window *window;
	SDL_Renderer *render;
} S;
	

static void
imgui_check_vk_result(VkResult err)
{
    if (err == 0)
        return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0)
        abort();
}

static float 
get_content_scale() 
{
	float x_scale, y_scale;
	SDL_RenderGetScale(S.render, &x_scale, &y_scale);
	return fmaxf(1.0f, fmaxf(x_scale, y_scale));
}

static void
window_content_scale_update(float sacle) 
{
#if defined(__MACH__)
    float font_scale               = fmaxf(1.0f, scale);
    ImGui::GetIO().FontGlobalScale = 1.0f / font_scale;
#endif
}


static void
imgui_set_default_style()
{
    ImGuiStyle* style  = &ImGui::GetStyle();
    ImVec4*     colors = style->Colors;

    colors[ImGuiCol_Text]                  = ImVec4(0.4745f, 0.4745f, 0.4745f, 1.00f);
    colors[ImGuiCol_TextDisabled]          = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg]              = ImVec4(0.0078f, 0.0078f, 0.0078f, 1.00f);
    colors[ImGuiCol_ChildBg]               = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg]               = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImGuiCol_Border]                = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
    colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]               = ImVec4(0.047f, 0.047f, 0.047f, 0.5411f);
    colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.196f, 0.196f, 0.196f, 0.40f);
    colors[ImGuiCol_FrameBgActive]         = ImVec4(0.294f, 0.294f, 0.294f, 0.67f);
    colors[ImGuiCol_TitleBg]               = ImVec4(0.0039f, 0.0039f, 0.0039f, 1.00f);
    colors[ImGuiCol_TitleBgActive]         = ImVec4(0.0039f, 0.0039f, 0.0039f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
    colors[ImGuiCol_MenuBarBg]             = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark]             = ImVec4(93.0f / 255.0f, 10.0f / 255.0f, 66.0f / 255.0f, 1.00f);
    colors[ImGuiCol_SliderGrab]            = colors[ImGuiCol_CheckMark];
    colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.3647f, 0.0392f, 0.2588f, 0.50f);
    colors[ImGuiCol_Button]                = ImVec4(0.0117f, 0.0117f, 0.0117f, 1.00f);
    colors[ImGuiCol_ButtonHovered]         = ImVec4(0.0235f, 0.0235f, 0.0235f, 1.00f);
    colors[ImGuiCol_ButtonActive]          = ImVec4(0.0353f, 0.0196f, 0.0235f, 1.00f);
    colors[ImGuiCol_Header]                = ImVec4(0.1137f, 0.0235f, 0.0745f, 0.588f);
    colors[ImGuiCol_HeaderHovered]         = ImVec4(5.0f / 255.0f, 5.0f / 255.0f, 5.0f / 255.0f, 1.00f);
    colors[ImGuiCol_HeaderActive]          = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
    colors[ImGuiCol_Separator]             = ImVec4(0.0f, 0.0f, 0.0f, 0.50f);
    colors[ImGuiCol_SeparatorHovered]      = ImVec4(45.0f / 255.0f, 7.0f / 255.0f, 26.0f / 255.0f, 1.00f);
    colors[ImGuiCol_SeparatorActive]       = ImVec4(45.0f / 255.0f, 7.0f / 255.0f, 26.0f / 255.0f, 1.00f);
    colors[ImGuiCol_ResizeGrip]            = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
    colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_Tab]                   = ImVec4(6.0f / 255.0f, 6.0f / 255.0f, 8.0f / 255.0f, 1.00f);
    colors[ImGuiCol_TabHovered]            = ImVec4(45.0f / 255.0f, 7.0f / 255.0f, 26.0f / 255.0f, 150.0f / 255.0f);
    colors[ImGuiCol_TabActive]             = ImVec4(47.0f / 255.0f, 6.0f / 255.0f, 29.0f / 255.0f, 1.0f);
    colors[ImGuiCol_TabUnfocused]          = ImVec4(45.0f / 255.0f, 7.0f / 255.0f, 26.0f / 255.0f, 25.0f / 255.0f);
    colors[ImGuiCol_TabUnfocusedActive]    = ImVec4(6.0f / 255.0f, 6.0f / 255.0f, 8.0f / 255.0f, 200.0f / 255.0f);
    colors[ImGuiCol_DockingPreview]        = ImVec4(47.0f / 255.0f, 6.0f / 255.0f, 29.0f / 255.0f, 0.7f);
    colors[ImGuiCol_DockingEmptyBg]        = ImVec4(0.20f, 0.20f, 0.20f, 0.00f);
    colors[ImGuiCol_PlotLines]             = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]      = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram]         = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg]         = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
    colors[ImGuiCol_TableBorderStrong]     = ImVec4(2.0f / 255.0f, 2.0f / 255.0f, 2.0f / 255.0f, 1.0f);
    colors[ImGuiCol_TableBorderLight]      = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
    colors[ImGuiCol_TableRowBg]            = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt]         = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget]        = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}

static void
fonts_upload()
{
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool                 = VK_CTX.commandpool;
	allocInfo.commandBufferCount          = 1;

	VkCommandBuffer commandBuffer = {};
	if (vkAllocateCommandBuffers(VK_CTX.device, &allocInfo, &commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags                    = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("Could not create one-time command buffer!");
	}

	ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}

	VkSubmitInfo submitInfo {};
	submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers    = &commandBuffer;

	vkQueueSubmit(VK_CTX.graphicsqueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(VK_CTX.graphicsqueue);

	vkFreeCommandBuffers(VK_CTX.device, VK_CTX.commandpool, 1, &commandBuffer);
	ImGui_ImplVulkan_DestroyFontUploadObjects();
}


// All the ImGui_ImplVulkanH_XXX structures/functions are optional helpers used by the demo.
// Your real engine/app may not use them.
static void 
imgui_init(VkSurfaceKHR surface, int width, int height)
{

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO&    io    = ImGui::GetIO();
	ImGuiStyle& style = ImGui::GetStyle();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	io.ConfigDockingAlwaysTabBar         = true;
	io.ConfigWindowsMoveFromTitleBarOnly = true;

	float content_scale = get_content_scale();
	window_content_scale_update(content_scale);

	io.Fonts->AddFontFromFileTTF("asset/fonts/Font.ttf", content_scale * 16, nullptr, nullptr);
	io.Fonts->Build();

	style.WindowPadding   = ImVec2(1.0, 0);
	style.FramePadding    = ImVec2(14.0, 2.0f);
	style.ChildBorderSize = 0.0f;
	style.FrameRounding   = 5.0f;
	style.FrameBorderSize = 1.5f;

	imgui_set_default_style();

	// implement init
	ImGui_ImplSDL2_InitForVulkan(S.window);
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance                  = VK_CTX.instance;
	init_info.PhysicalDevice            = VK_CTX.phydevice;
	init_info.Device                    = VK_CTX.device;
	init_info.QueueFamily               = VK_CTX.graphicsfamily;
	init_info.Queue                     = VK_CTX.graphicsqueue;
	init_info.DescriptorPool            = VK_CTX.descriptorpool;
	init_info.Subpass                   = 0;
	// may be diffirent from the real swapchain image count // see: ImGui_ImplVulkanH_GetMinImageCountFromPresentMode
	init_info.MinImageCount = conf::MAX_FRAMES_IN_FLIGHT;
	init_info.ImageCount    = conf::MAX_FRAMES_IN_FLIGHT;
	
	ImGui_ImplVulkan_Init(&init_info, VK_CTX.swapchain.render_pass);

	// fonts upload
	fonts_upload();
}


void
vk_surface::init(const char *title, int width, int height)
{
	SDL_Init(SDL_INIT_EVERYTHING);
	S.window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, 
		SDL_WINDOWPOS_CENTERED, width, height,
		SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
	    /* Create a Render */
	S.render = SDL_CreateRenderer(S.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (S.render == nullptr)
		std::cout << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
}

void
vk_surface::resolution(int *width, int *height)
{
	SDL_GetWindowSize(S.window, width, height);
}
	
void
vk_surface::bind(VkInstance instance, VkSurfaceKHR *surface)
{
	SDL_Vulkan_CreateSurface(S.window, instance, surface);
}

std::vector<const char *> 
vk_surface::required_extensions() 
{
	unsigned int n = 0;
	std::vector<const char *> extensions;
	SDL_Vulkan_GetInstanceExtensions(S.window, &n, nullptr);
	extensions.resize(n);
	SDL_Vulkan_GetInstanceExtensions(S.window, &n, extensions.data());
	return extensions;
}

void
vk_surface::init_ui(VkSurfaceKHR *surface)
{
	int width, height;
	SDL_Vulkan_GetDrawableSize(S.window, &width, &height);
	imgui_init(*surface, width, height);
}

void
vk_surface::pre_tick(float delta)
{

}

void
vk_surface::tick(float delta)
{
	SDL_Event e;
	while (SDL_PollEvent(&e) != 0) {
		ImGui_ImplSDL2_ProcessEvent(&e);
		switch (e.type) {
		case SDL_QUIT:
			running = false;
			break;
		default:
			vk_input::inst().process(S.window, e, delta);
			break;
		};
	}
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();
}

void 
vk_surface::post_tick(float delta)
{
	vk_ctx_renderpass_begin(nullptr);

	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), VK_CTX.cmdbuf);
	
	vk_ctx_renderpass_end();
	
	ImGui::UpdatePlatformWindows();
	ImGui::RenderPlatformWindowsDefault();
}


void
vk_surface::set_title(const char *title)
{
	SDL_SetWindowTitle(S.window, title);
}

void
vk_surface::exit()
{
	if (S.window != nullptr) {
		SDL_DestroyRenderer(S.render);
		SDL_DestroyWindow(S.window);
		S.window = nullptr;
		S.render = nullptr;
	}
}

}}

