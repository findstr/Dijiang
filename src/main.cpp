#include <iostream>
#include "render/painter.h"
#include "resource/resource.h"
#include "utils/file.h"
#include "engine.h"
#include "math/math.h"

int main()
{
	engine::init();
	engine::run();
	engine::cleanup();
	/*
	int ret = engine::render::init("帝江", 800, 600);
	if (ret != 0)
		std::cerr << "err" << std::endl;
	engine::resource::init();
	engine::resource::load_level("asset/main.level");
	while (engine::render::drawframe() == 0);
	engine::render::cleanup();
	*/
	std::cout << "over" << std::endl;
	return 0;
}
