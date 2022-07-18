#include <chrono>
#include <iostream>
#include <imgui.h>
#include "engine.h"
#include "editor.h"
#include "system/render_system.h"

//TODO:
//	重构uniform, per_object池化，每个camera去渲染前，去池中获取，如果够就复用，不够就新开一个Descriptorset, 池在frame_end时释放空闲的descriptorset和vk_buffer
//	重构vk_ctx 使用单例
//	去掉engine类
//	使RenderSystem更内聚，和vulkan剥离更干净
//	增加CSM，PCF
//	重构资源管理，分为editor资源管理和运行时资源管理
//	重构AnimationSystem, InputSystem, ParticleSystem
//	component.name, shader.variable_name改用字符串池 str2int
//	对象池

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
