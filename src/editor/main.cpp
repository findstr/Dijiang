#include <chrono>
#include <iostream>
#include <imgui.h>
#include "engine.h"
#include "editor.h"
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

int main()
{
	engine::engine engine;
	engine::editor::editor editor;
	//system("echo %cd% && cd asset/shaders && shaderc.sh");
	RENDER_SYSTEM;
	engine.init();
	RENDER_SYSTEM.init_lighting();
	editor.init(&engine);
	std::chrono::time_point<std::chrono::high_resolution_clock> last_tick = std::chrono::high_resolution_clock::now();
	for (;;) {
		int ret;
		auto now = std::chrono::high_resolution_clock::now();
		float delta = std::chrono::duration<float, std::chrono::seconds::period>(
			now - last_tick).count();
		last_tick = now;
		engine.pre_tick(delta);
		editor.pre_tick(delta);
		ret = RENDER_SYSTEM.frame_begin(delta);
		if (ret < 0)
			break;
		if (ret > 0)
			continue;
		engine.tick(delta);
		editor.tick(delta);
		RENDER_SYSTEM.frame_end(delta);
		engine.post_tick(delta);
		editor.post_tick(delta);
		RENDER_SYSTEM.frame_submit();
	}
	editor.cleanup();
	engine.cleanup();
	std::cout << "over" << std::endl;
	return 0;
}
