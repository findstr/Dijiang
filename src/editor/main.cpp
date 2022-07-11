#include <chrono>
#include <iostream>
#include <imgui.h>
#include "engine.h"
#include "editor.h"
#include "system/render_system.h"

//TODO:
//	重构uniform, per_object池化，每个camera去渲染前，去池中获取，如果够就复用，不够就新开一个Descriptorset, 池在frame_end时释放空闲的descriptorset和vk_buffer
//	重构Input变成input_system
//	重构vk_ctx 使用单例

int main()
{
	engine::engine engine;
	engine::editor::editor editor;
	//system("echo %cd% && cd asset/shaders && shaderc.sh");
	RENDER_SYSTEM;
	engine.init();
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
	}
	editor.cleanup();
	engine.cleanup();
	std::cout << "over" << std::endl;
	return 0;
}
