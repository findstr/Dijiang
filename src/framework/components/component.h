#pragma once
#include <string>
#include <vector>
#include "framework/gameobject.h"

namespace engine {

class component {
protected:
	bool has_tick_ = false;
	gameobject *go = nullptr;
	component *get_sibling(const std::string &type) { return go->get_component(type); }
public:
	component(gameobject *parent): go(parent) { transform = &go->transform;}
	transform *transform = nullptr;
	bool has_tick() const { return has_tick_; }
	virtual ~component() { go = nullptr; }
public:
	virtual const std::string type() = 0;
	virtual void start() {};
	virtual void tick(float delta) {};
	virtual void destroy() {};
};

};
