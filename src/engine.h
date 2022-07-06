#pragma once

namespace engine {

class engine {
public:
	void init();
	bool pre_tick(float delta);
	void tick(float delta);
	void post_tick(float delta);
	void cleanup();
};

}

