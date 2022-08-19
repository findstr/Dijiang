#include <chrono>
#include <vector>
#include <iostream>
#include <direct.h> // _getcwd
#include <imgui.h>
#include "SDL_main.h"
#include "engine.h"
#include "editor.h"
#include "input.h"
#include "system/render_system.h"

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
	RENDER_SYSTEM;
	engine.init();
	RENDER_SYSTEM.init_lighting();
	editor.init(&engine);
	std::chrono::time_point<std::chrono::high_resolution_clock> last_tick = std::chrono::high_resolution_clock::now();
	while (RENDER_SYSTEM.is_running()) {
		int ret;
		auto now = std::chrono::high_resolution_clock::now();
		float delta = std::chrono::duration<float, std::chrono::seconds::period>(
			now - last_tick).count();
		last_tick = now;
		engine.pre_tick(delta);
		editor.pre_tick(delta);
		ret = RENDER_SYSTEM.frame_begin(delta);
		if (ret > 0)
			continue;
		engine::input::inst().tick(delta);
		engine.tick(delta);
		editor.tick(delta);
		RENDER_SYSTEM.frame_end(delta);
		engine.post_tick(delta);
		editor.post_tick(delta);
		RENDER_SYSTEM.frame_submit();
		++frame;
		frame_acc += delta;	
		if (frame_acc >= 1.0f) {
			RENDER_SYSTEM.show_fps((int)(frame / frame_acc));
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
