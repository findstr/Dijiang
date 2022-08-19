#include <assert.h>
#include <SDL.h>
#include "math/math.h"
#include "vk_input.h"

namespace engine {
namespace vulkan {

vk_input &
vk_input::inst()
{
	static vk_input input;
	return input;
}
	
vk_input::key_code
vk_input::to_key_code(SDL_Keycode code)
{
	switch (code) {
	case SDLK_a:
		return key_code::A;
	case SDLK_s:
		return key_code::S;
	case SDLK_d:
		return key_code::D;
	case SDLK_w:
		return key_code::W;
	default:
		return key_code::A;
	}
}
	
vk_input::mouse_button 
vk_input::to_mouse_button(int mouse_index)
{
	switch (mouse_index) {
	case SDL_BUTTON_LEFT:
		return mouse_button::LEFT;
	case SDL_BUTTON_MIDDLE:
		return mouse_button::MIDDLE;
	case SDL_BUTTON_RIGHT:
		return mouse_button::RIGHT;
	default:
		return mouse_button::NONE;
	}
}	

void 
vk_input::update_mouse_position(SDL_Window *w)
{
	int x, y;
	int height, width;
	SDL_GetMouseState(&x, &y);
	SDL_GetWindowSize(w, &width, &height);
	y = height - y;
	mouse_position.x() = (float)x / width;
	mouse_position.y() = (float)y / height;
}

void
vk_input::process(SDL_Window *w, SDL_Event &e, float delta)
{
	switch (e.type) {
	case SDL_MOUSEMOTION:
		update_mouse_position(w);
		break;
	case SDL_MOUSEBUTTONDOWN:
		update_mouse_position(w);
		mouse_button_status[(int)to_mouse_button(e.button.button)] = true;
		if (mouse_press == false) {
			mouse_press = true;
			touch_data.phase = touch_phase::BEGAN;
			touch_data.position = mouse_position;
			touch_data.delta_time = delta;
			touch_data.delta_position = vector2f(0, 0);
		}
		break;
	case SDL_MOUSEBUTTONUP:
		update_mouse_position(w);
		mouse_button_status[(int)to_mouse_button(e.button.button)] = false;
		if (mouse_press) {
			mouse_press = false;
			touch_data.phase = touch_phase::ENDED;
		}
		break;
	case SDL_KEYDOWN:
		key_status[(int)to_key_code(e.key.keysym.sym)] = key_action::DOWN;
		break;
	case SDL_KEYUP:
		key_status[(int)to_key_code(e.key.keysym.sym)] = key_action::UP;
		break;
	};
}

void
vk_input::tick(float delta)
{
	if (mouse_press) {
		touch_data.phase = touch_phase::MOVED;
		touch_data.delta_position = mouse_position - touch_data.position;
		touch_data.position = mouse_position;
		touch_data.delta_time = delta;
	}
}

vk_input::key_action
vk_input::get_key(key_code key)
{
	return key_status[(int)key];
}

int 
vk_input::touch_count()
{
	return touch_data.phase == touch_phase::ENDED ? 0 : 1;
}

void
vk_input::touch_get(int index, struct touch *t) 
{
	assert(index == 0);
	*t = touch_data;
	t->finger_id = 0;
}

float
vk_input::mouse_scroll_delta()
{
	float y = y_offset;
	y_offset = 0.0f;
	return y;
}

bool 
vk_input::mouse_get_button(mouse_button button)
{
	return mouse_button_status[(int)button];
}

vector2f
vk_input::mouse_get_position()
{
	return mouse_position;
}


}}

