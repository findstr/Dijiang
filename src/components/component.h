#pragma once
#include <string>
#include <vector>
#include "gameobject.h"

namespace engine {

class component {
protected:
	bool has_tick_ = false;
	bool has_pre_tick_ = false;
	bool has_post_tick_ = false;
	gameobject *go = nullptr;
	component *get_sibling(const std::string &type) { return go->get_component(type); }
public:
	component(gameobject *parent): go(parent) { transform = &go->transform;}
	transform *transform = nullptr;
	bool has_tick() const { return has_tick_; }
	bool has_pre_tick() const { return has_pre_tick_; }
	bool has_post_tick() const { return has_post_tick_; }
	virtual ~component() { go = nullptr; }
public:
	virtual const std::string type() = 0;
	virtual void start() {};
	virtual void tick(float delta) {};
	virtual void pre_tick(float delta) {};
	virtual void post_tick(float delta) {};
	virtual void destroy() {};
};

};
