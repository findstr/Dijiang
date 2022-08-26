#include <chrono>
#include <vector>
#include <iostream>
#include <direct.h> // _getcwd
#include <imgui.h>
#include "SDL_main.h"
#include "engine.h"
#include "editor.h"
#include "input.h"
#include "graphics.h"
#include "luavm.h"
#include "resource/resource.h"
#include "lighting_asset.h"
#include "render/input.h"
#include "render/debugger.h"

//TODO:
//	�ع�uniform, per_object�ػ���ÿ��cameraȥ��Ⱦǰ��ȥ���л�ȡ��������͸��ã��������¿�һ��Descriptorset, ����frame_endʱ�ͷſ��е�descriptorset��vk_buffer
//	�ع�vk_ctx ʹ�õ���
//	ȥ��engine��
//	ʹRenderSystem���ھۣ���vulkan������ɾ�
//	����CSM��PCF
//	�ع���Դ������Ϊeditor��Դ���������ʱ��Դ����
//	�ع�AnimationSystem, InputSystem, ParticleSystem
//	component.name, shader.variable_name�����ַ����� str2int
//	�����

extern SDLMAIN_DECLSPEC 
int main(int argc, char *argv[])
{
	int fps = 0;
	int frame = 0;
	float frame_acc = 0.0f;
	char buf[256];
	engine::engine engine;
	engine::editor::editor editor;
	char cwd[256];
	if (_getcwd(cwd, sizeof(cwd)) != NULL) 
		printf("Current working dir: %s\n", cwd);
	printf("vector:%l\n", sizeof(std::vector<size_t>));
	//system("echo %cd% && cd asset/shaders && shaderc.sh");
	graphics.init();
	engine::render::lighting_asset::inst().init();
	engine::luavm::init();
	engine::resource::init();
	engine::render::debugger::inst().init();
	engine::level::load("asset/terrain.level");
	editor.init(&engine);
	std::chrono::time_point<std::chrono::high_resolution_clock> last_tick = std::chrono::high_resolution_clock::now();
	while (graphics.is_running()) {
		int ret;
		auto now = std::chrono::high_resolution_clock::now();
		float delta = std::chrono::duration<float, std::chrono::seconds::period>(
			now - last_tick).count();
		last_tick = now;

		engine.pre_tick(delta);
		editor.pre_tick(delta);
		graphics.pre_tick(delta);
		engine::input::inst().tick(delta);
		engine.tick(delta);
		editor.tick(delta);
		graphics.tick(delta);
		engine.post_tick(delta);
		editor.post_tick(delta);
		graphics.post_tick(delta);
		++frame;
		frame_acc += delta;	
		if (frame_acc >= 1.0f) {
			graphics.show_fps((int)(frame / frame_acc));
			frame = 0;
			frame_acc = 0.0f;
		}
	}
	//TODO: rhi::clieanup
	editor.cleanup();
	engine.cleanup();
	std::cout << "over" << std::endl;
	return 0;
}
