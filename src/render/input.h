#pragma once

namespace engine {
namespace input {

enum key {
	A,
	S,
	D,
	W,
};

void init();
int get_key(key keycode);


}}



