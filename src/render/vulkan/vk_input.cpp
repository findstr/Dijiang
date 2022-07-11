#include <assert.h>
#include "math/math.h"
#include "vk_input.h"

namespace engine {
namespace input {

static GLFWwindow *window = nullptr;
static bool mouse_press = false;
static touch touch_data; 
float x_offset = 0.0f;
float y_offset = 0.0f;
vector2f mouse_position = vector2f(0, 0);

static void 
glfw_scroll_func(GLFWwindow* window, double xoffset, double yoffset)
{
	x_offset = xoffset;
	y_offset = yoffset;
}

void
init(GLFWwindow *win)
{
	window = win;
	glfwSetScrollCallback(window, glfw_scroll_func);
}

void 
update(float delta)
{
	auto press = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	if (press == GLFW_PRESS && mouse_press == false) {
		mouse_press = true;
		touch_data.phase = touch_phase::BEGAN;
		double x, y;
		int height, width;
		glfwGetCursorPos(window, &x, &y);
		glfwGetWindowSize(window, &height, &width);
		y = height - y;
		x /= width;
		y /= height;
		touch_data.position = vector2f(x, y);
	} else if (press == GLFW_RELEASE && mouse_press == true) {
		mouse_press = false;
		touch_data.phase = touch_phase::ENDED;
	} else if (press && mouse_press) {
		double x, y;
		int height, width;
		glfwGetCursorPos(window, &x, &y);
		glfwGetWindowSize(window, &height, &width);
		y = height - y;
		x /= width;
		y /= height;
		touch_data.phase = touch_phase::MOVED;
		vector2f pos =  vector2f(x, y);
		touch_data.delta_position = pos - touch_data.position;
		touch_data.delta_time = delta;
		touch_data.position = pos;
	}
}

key_action
get_key(key_code key)
{
	int state = GLFW_RELEASE;
	switch (key) {
	case key_code::A:
		state = glfwGetKey(window, GLFW_KEY_A);
		break;
	case key_code::S:
		state = glfwGetKey(window, GLFW_KEY_S);
		break;
	case key_code::D:
		state = glfwGetKey(window, GLFW_KEY_D);
		break;
	case key_code::W:
		state = glfwGetKey(window, GLFW_KEY_W);
		break;
	default:
		assert(!"unsupport key code");
	break;
	}
	switch (state) {
        case GLFW_PRESS:
        case GLFW_REPEAT:
		return key_action::DOWN;
        case GLFW_RELEASE:
		return key_action::UP;
        default:
		return key_action::NONE;
	}
}

int 
touch_count()
{
	return touch_data.phase == touch_phase::ENDED ? 0 : 1;
}

void
touch_get(int index, struct touch *t) 
{
	assert(index == 0);
	*t = touch_data;
	t->finger_id = 0;
}

float
mouse_scroll_delta()
{
	float y = y_offset;
	y_offset = 0.0f;
	return y;
}

bool 
mouse_get_button(int button)
{
	switch (button) {
	case 0:
		return glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
	case 1:
		return glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;
	case 2:
		return glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
	default:
		return false;
	}
}

vector2f
mouse_get_position()
{
	double x, y;
	int height, width;
	glfwGetCursorPos(window, &x, &y);
	glfwGetWindowSize(window, &height, &width);
	y = height - y;
	x /= width;
	y /= height;
	return vector2f(x, y);
}


}}

