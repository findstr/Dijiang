#pragma once
#include "conf.h"
#include "math/math.h"

namespace engine {
namespace input {

enum class key_code : int {
	A,
	S,
	D,
	W,
};

enum class key_action {
	NONE,
	DOWN,
	UP,
};

enum class touch_phase {
	BEGAN,
	MOVED,
	STATIONARY,
	ENDED,
	CANCELED,
};


struct touch {
	int finger_id;
	touch_phase phase;
	vector2f position;
	float delta_time;
	vector2f delta_position;
};
	
ENGINE_API void update(float delta);
ENGINE_API key_action get_key(key_code key);
ENGINE_API int touch_count();
ENGINE_API void touch_get(int index, struct touch *t);

ENGINE_API float mouse_scroll_delta();
ENGINE_API bool mouse_get_button(int button);
ENGINE_API vector2f mouse_get_position();

}}




