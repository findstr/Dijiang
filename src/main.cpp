#include <iostream>
#include "render/render.h"
#include "resource/resource.h"
#include "utils/file.h"

int main() {
	int ret = engine::render::init("帝江", 800, 600);
	if (ret != 0)
		std::cerr << "err" << std::endl;
	engine::resource::init();
	while (engine::render::drawframe() == 0);
	engine::render::cleanup();
	std::cout << "over" << std::endl;
	return 0;
}
