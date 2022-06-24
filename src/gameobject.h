#pragma once
#include <memory>
#include <string>
#include <vector>
#include "math/transform.h"

namespace engine {

class component;

class gameobject {
public:
	typedef unsigned int id_t;
protected:
	id_t	id_;
	std::string name;
	gameobject *parent;
	std::vector<component *> components;
public:
	gameobject(int id, const std::string &name);
	~gameobject();
	transform transform;
	void set_parent(gameobject *go, bool keep_world = false);
	constexpr gameobject *get_parent() const { return parent;}
	constexpr id_t id() const { return id_; }
public:
	void add_component(component *);
	component *get_component(const std::string &type);
	void remove_component(const std::string &type);
	void start();
	void tick(float delta);
	void pre_tick(float delta);
	void post_tick(float delta);
};

}

