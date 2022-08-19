#pragma once
#include <SDL2/SDL.h>
#include "conf.h"
#include "math/math.h"

namespace engine {
namespace vulkan {

class ENGINE_API vk_input {
public:
	static vk_input &inst();
public:
	enum class key_code : uint32_t {
		A,
		S,
		D,
		W,
		COUNT,
	};

	enum class mouse_button : uint32_t {
		NONE,
		LEFT,
		MIDDLE,
		RIGHT,
		COUNT,
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
		
	void process(SDL_Window *win, SDL_Event &e, float delta);
	void tick(float delta);
	key_action get_key(key_code key);
	int touch_count();
	void touch_get(int index, struct touch *t);

	float mouse_scroll_delta();
	bool mouse_get_button(mouse_button button);
	vector2f mouse_get_position();
private:
	key_code to_key_code(SDL_Keycode code);
	mouse_button to_mouse_button(int mouse_index);
	void update_mouse_position(SDL_Window *win);
private:
	bool mouse_press = false;
	touch touch_data; 
	float x_offset = 0.0f;
	float y_offset = 0.0f;
	vector2f mouse_position = vector2f(0, 0);
	enum key_action key_status[(int)key_code::COUNT] = {};
	bool mouse_button_status[(int)mouse_button::COUNT] = {};
};

}}

