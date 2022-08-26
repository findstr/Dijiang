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
//	重构uniform, per_object池化，每个camera去渲染前，去池中获取，如果够就复用，不够就新开一个Descriptorset, 池在frame_end时释放空闲的descriptorset和vk_buffer
//	重构vk_ctx 使用单例
//	去掉engine类
//	使RenderSystem更内聚，和vulkan剥离更干净
//	增加CSM，PCF
//	重构资源管理，分为editor资源管理和运行时资源管理
//	重构AnimationSystem, InputSystem, ParticleSystem
//	component.name, shader.variable_name改用字符串池 str2int
//	对象池

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
